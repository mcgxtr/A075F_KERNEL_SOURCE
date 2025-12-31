#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/signal.h>
#include <linux/pid_namespace.h>
#include <linux/pid.h>
#include <linux/oom.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/kthread.h>

#include "hperf_handle_internal.h"
#include "hperf_log.h"

// #define SIG_PROC_STAT_CTRL 129
#define SIG_FREEZE_EXCEPT_WHITELIST 130
#define SIG_UNFREEZE_EXCEPT_WHITELIST 131

/* SIGSTOP AUTO RECOVERY TIME: 500ms */
#define FREEZE_TIME 500
/* SIGSTOP AUTO SENT INTERVAL: 50ms */
#define FREEZE_INTERVAL 50
/* Freeze Thread Timeout: 10s */
#define FREEZE_THREAD_MAX_RUNNING 10000

typedef struct {
    struct list_head list;
    struct task_struct *task;
} zygote_child_t;

static ktime_t freeze_start_time;

typedef enum {
    FREEZE_THREAD_STOPPED,
    FREEZE_THREAD_RUNNING
} freeze_thread_state_t;

static struct task_struct *freeze_kthread = NULL;
static freeze_thread_state_t freeze_state = FREEZE_THREAD_STOPPED;

typedef struct {
    const char *name;
    pid_t pid;
} target_info_t;

static target_info_t zygote_proc[] = {
    {"/system/bin/app_process64", -1},
    {"/system/bin/app_process32", -1},
    {NULL, -1}
};

static target_info_t app_white_list[] = {
    {"system_server", -1},
    {"webview_zygote", -1},
    {"com.android.systemui", -1},
    {"com.android.phone", -1},
    {"com.android.launcher3", -1},
    {"com.android.permissioncontroller", -1},
    {"com.sec.android.app.camerasaver", -1},
    {"com.sec.android.app.camera", -1},
    {"com.android.providers.media.module", -1},
    {"android.process.media", -1},
    {"com.android.se", -1},
    {"com.android.traceur", -1},
    {"com.debug.loggerui", -1},
    // sec apps
    {"com.salab.act_agent", -1}, /* sec test agent */
    {"com.sec.android.app.launcher", -1}, /* sec launcher */
    {"com.google.android.permissioncontroller", -1}, /* google permission controller */
    {"com.sec.android.gallery3d", -1}, /* sec gallery */
    {"com.sec.epdg", -1}, /* sec app */
    {"com.google.android.providers.media.module", -1}, /* google media provider */
    {"com.google.android.googlequicksearchbox:search", -1}, /* sec launcher related */
    {"com.sec.phone", -1}, /* sec phone */
    {"com.android.bluetooth", -1}, /* bluetooth */
    {"com.sec.android.diagmonagent", -1}, /* sec diagmonagent */
    {"com.samsung.cmh", -1}, /* sec cmh */
    {NULL, -1}
};

void my_sleep(long timeout_ms) {
    long timeout_jiffies = msecs_to_jiffies(timeout_ms);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(timeout_jiffies);
}

/**
*  find zygote pid by:
*  exe_file == system/bin/app_process[32|64] && ppid == 1
*/
static int find_zygote_pid(void) {
    target_info_t *t;
    int cnt = 0;
    struct task_struct *task = NULL;
    struct mm_struct *mm;
    char *exe_path;
    char buf[PATH_MAX];
    struct path *exe_path_ptr;

    // Zygote process doesn't change if no soft reboot, so use cache firstly.
    for(t = zygote_proc; t->name; t++) {
        if(t->pid > 0 && t->pid < PID_MAX_LIMIT) {
            Logi("zygote process found %d (Cached)", t->pid);
            cnt++;
        }
    }
    if (cnt == 2) {
        return cnt;
    } else {
        Logi("Zygote Cache incomplete,  count %d, check directly!", cnt);
        cnt = 0;
    }

    /* check by traversal */
    rcu_read_lock();
    for_each_process(task) {
        mm = get_task_mm(task);
        if(!mm) {
            continue;
        }
        if (!mm->exe_file) {
            mmput(mm);
            continue;
        }

        if (!task->real_parent || task->real_parent->pid != 1) {
            mmput(mm);
            continue;
        }

        exe_path_ptr = &mm->exe_file->f_path;
        exe_path = d_path(exe_path_ptr, buf, sizeof(buf));
        if (exe_path) {
            for(t = zygote_proc; t->name; t++) {
                if (strcmp(t->name, exe_path) == 0) {
                    t->pid = task->pid;
                    cnt++;
                    Logi("find zygote process %d", t->pid);
                    break;
                }
            }
        }

        mmput(mm);
    }
    rcu_read_unlock();
    return cnt;
}

/**
* freeze/unfreeze java app not in whitelist
*/
static int ctrl_zygote_fork_proc(int signal) {
    struct task_struct *child = NULL;
    target_info_t *t;
    struct list_head zygote_children;
    zygote_child_t *entry = NULL;
    zygote_child_t *temp_entry = NULL;
    int ret = 0;

    char *cmdline;

    Logi("ctrl_zygote_fork_proc xxx E");
    cmdline = kmalloc(PNAME_SIZE_MAX_KB, GFP_KERNEL);
    if (!cmdline) {
        Loge("Failed to allocate cmdline");
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&zygote_children);

    for_each_process(child) {
        for (t = zygote_proc; t->name; t++) {
            if (t->pid > 0 && child->real_parent && t->pid == child->real_parent->pid) {
                if (child->exit_state != 0 || (child->flags & PF_EXITING)) {
                    continue;
                }

                entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
                if (!entry) {
                    Loge("Failed to allocate zygote_child entry");
                    continue;
                }

                entry->task = get_task_struct(child);
                list_add_tail(&entry->list, &zygote_children);
                break;
            }
        }
    }

    list_for_each_entry(entry, &zygote_children, list) {
        target_info_t *t_white;
        struct mm_struct *mm;
        int len;
        bool white_list_detected = false;

        struct task_struct* pos = entry->task;

        if (pos->exit_state != 0 || (pos->flags & PF_EXITING)) {
            put_task_struct(pos);
            continue;
        }

        mm = pos->mm;
        if (!mm) {
            put_task_struct(pos);
            continue;
        }

        len = get_mm_cmdline(pos, cmdline, PNAME_SIZE_MAX_KB);

        for (t_white = app_white_list; t_white->name; t_white++) {
            const size_t white_len = strlen(t_white->name);
            if (len < white_len || white_len == 0) {
                continue;
            }
            if (strncmp(cmdline, t_white->name, white_len) == 0 && cmdline[white_len] == '\0') {
                //Logd("whitelist proc detect %s, skip", cmdline);
                white_list_detected = true;
                break;
            }
        }

        if (!white_list_detected) {
            send_sig(signal, pos, 0);
            Logi("Not whitelist %d (%s), send signal %d !", pos->pid, cmdline, signal);
        }

        put_task_struct(pos);
    }

    list_for_each_entry_safe(entry, temp_entry, &zygote_children, list) {
        list_del(&entry->list);
        kfree(entry);
    }
    kfree(cmdline);

    Logi("ctrl_zygote_fork_proc xxx X");
    return ret;
}

/**
* clean work when exit
*/
void hmem_exit(void) {
    if (freeze_kthread && freeze_state == FREEZE_THREAD_RUNNING) {
        kthread_stop(freeze_kthread);
        freeze_kthread = NULL;
        freeze_state = FREEZE_THREAD_STOPPED;
        Logi("hmem_exit - Freeze thread stopped");
    }
}

/**
* hperf_freeze thread
*/
static int freeze_thread_func(void *data) {
    ktime_t current_time;
    unsigned long elapsed_ms;

    freeze_start_time = ktime_get_boottime();

    while (!kthread_should_stop()) {
        current_time = ktime_get_boottime();
        elapsed_ms = ktime_to_ms(ktime_sub(current_time, freeze_start_time));
        if (elapsed_ms >= FREEZE_THREAD_MAX_RUNNING) {
            Logi("Freeze thread timed out (elapsed: %lu ms), exit!", elapsed_ms);
            freeze_state = FREEZE_THREAD_STOPPED;
            return 0;
        }

        ctrl_zygote_fork_proc(SIGSTOP);
        my_sleep(FREEZE_TIME);

        ctrl_zygote_fork_proc(SIGCONT);
        my_sleep(FREEZE_INTERVAL);
    }

    ctrl_zygote_fork_proc(SIGCONT);
    Logi("Freeze thread exited");
    return 0;
}

int send_signal_by_pid(int target_pid, int signal) {
    struct task_struct *task;
    Logi("E");

    if (target_pid <=0 || target_pid >= PID_MAX_LIMIT) {
        Loge("PID %d out of range (1-%lu)", target_pid, PID_MAX_LIMIT - 1);
        return -EINVAL;
    }

    task = get_task_by_pid(target_pid);

    if (!task) {
        Loge("Process %d not found", target_pid);
        return -ESRCH;
    }

    if (task->exit_state != 0 || task->flags & PF_EXITING) {
        Loge("Process %d already exited or existing", target_pid);
        put_task_struct(task);
        return -EALREADY;
    }

    if (task->flags & PF_KTHREAD) {
        Loge("Process %d is kernel thread, skip", target_pid);
        put_task_struct(task);
        return -EOPNOTSUPP;
    }

    if (!task->signal) {
        Loge("Process %d  task->signal not found", target_pid);
        put_task_struct(task);
        return -ESRCH;
    }

    send_sig(signal, task,0);
    Logi("Sent Signal %d to PID %d (%s) oom %d", signal, target_pid, task->comm, task->signal->oom_score_adj);
    put_task_struct(task);
    Logi("X");
    return 0;
}

int process_signal_ctrl(hperf_process_t *hperf_sig_process) {
    int hperf_signal, i;

    Logi("E");
    if (!hperf_sig_process) {
        Loge("Invalid hperf_process sent!");
        return -EINVAL;
    }

    hperf_signal = hperf_sig_process->signal;
    Logi("hperf_signal get %d!", hperf_signal);

    switch (hperf_signal) {
        case SIGKILL:
        case SIGSTOP:
        case SIGCONT:
            for (i = 0; i < PID_SIZE_MAX; i++) {
                if (hperf_sig_process->pids[i] > 0) {
                    send_signal_by_pid(hperf_sig_process->pids[i], hperf_signal);
                }
            }
            break;
        case SIG_FREEZE_EXCEPT_WHITELIST:
            if (find_zygote_pid() < 1) {
                Loge("zygote process not found!");
                break;
            }

            if (freeze_state == FREEZE_THREAD_STOPPED) {
                freeze_kthread = kthread_run(freeze_thread_func, NULL, "hperf_freeze");
                if (!IS_ERR(freeze_kthread)) {
                    freeze_state = FREEZE_THREAD_RUNNING;
                    Logi("Freeze thread started");
                } else {
                    Loge("Failed to start freeze thread: %ld", PTR_ERR(freeze_kthread));
                }
            } else {
                freeze_start_time = ktime_get_boottime();
                Logi("Freeze thread timeout reset");
            }
            break;
        case SIG_UNFREEZE_EXCEPT_WHITELIST:
            if (freeze_state == FREEZE_THREAD_RUNNING) {
                kthread_stop(freeze_kthread);
                freeze_kthread = NULL;
                freeze_state = FREEZE_THREAD_STOPPED;
                Logi("Freeze thread stopped");
            }
            break;
        default:
            Loge("Unsupported Signal %d!", hperf_signal);
            return -EINVAL;
    }
    Logi("X");
    return 0;
}
