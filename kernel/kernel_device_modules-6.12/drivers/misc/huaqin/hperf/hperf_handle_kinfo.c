#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/mm_types.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>

#include <mtk_gpu_utility.h>
#include <dvfsrc-exp.h>

#include "hperf_handle_internal.h"
#include "hperf_log.h"

/**
 * Get the pname from path object
 * eg: path is "/xxx/yyy/pname", it will get pname.
 */
static void get_pname_from_path(const char *path, char *pname, size_t pname_size) {
    const char *p, *start;
    size_t name_len;
    if (!path || !pname || pname_size == 0) {
        if (pname && pname_size > 0) pname[0] = '\0';
        return;
    }
    p = path + strlen(path);
    while (p > path && *(p - 1) == '/') { p--; }
    start = p;
    while (start > path && *(start - 1) != '/') { start--; }
    name_len = p - start;
    if (name_len == 0 && pname_size > 1) {
        strncpy(pname, "/", pname_size);
        pname[pname_size - 1] = '\0';
        return;
    }
    if (name_len >= pname_size) {
        name_len = pname_size - 1;
    }
    strncpy(pname, start, name_len);
    pname[name_len] = '\0';
    return;
}

/**
 *  get task_struct by pid
 *  note: after using this task_struct, must call "put_task_struct(task);" to release memory.
 * */
struct task_struct* get_task_by_pid(pid_t pid) {
    struct pid *pid_struct = NULL;
    struct task_struct *task = NULL;
    pid_struct = find_get_pid(pid);
    if (!pid_struct) {
        return NULL;
    }
    task =  pid_task(pid_struct, PIDTYPE_PID);

    if (task) {
        get_task_struct(task);
    }
    put_pid(pid_struct);
    return task;
}

int get_mm_cmdline(struct task_struct *task, char *buffer, int buflen) {
    int res = 0;
    unsigned int len;
    struct mm_struct *mm;
    unsigned long arg_start, arg_end, env_start, env_end;
    mm = get_task_mm(task);
    if (!mm) {
        if (task->__state == EXIT_ZOMBIE || task->__state == EXIT_DEAD) {
            Loge("pid(%d) is exiting", task->pid);
        } else {
            Loge("pid(%d) unknown reason for missing mm", task->pid);
        }
        goto out;
    }
    if (!mm->arg_end) {
        Loge("mm arg_end is null");
        goto out_mm;
    }

    spin_lock(&mm->arg_lock);
    arg_start = mm->arg_start;
    arg_end = mm->arg_end;
    env_start = mm->env_start;
    env_end = mm->env_end;
    spin_unlock(&mm->arg_lock);

    len = arg_end - arg_start;

    if (len > buflen) {
        len = buflen;
    }

    res = access_process_vm(task, arg_start, buffer, len, FOLL_FORCE);

    /*
    * If the nul at the end of args has been overwritten, then
    * assume application is using setproctitle(3).
    */
    if (res > 0 && buffer[res-1] != '\0' && len < buflen) {
        len = strnlen(buffer, res);
        if (len < res) {
            res = len;
        } else {
            len = env_end - env_start;
            if (len > buflen - res) {
                len = buflen - res;
            }
            res += access_process_vm(task, env_start, buffer+res, len, FOLL_FORCE);
            res = strnlen(buffer, res);
        }
    }
out_mm:
    mmput(mm);
out:
    return res;
}

int do_get_pids_by_pname(const char *pname, int *pids, int *pids_cnt) {
    struct task_struct *task = NULL;
    char cmdline[PNAME_SIZE_MAX_KB];
    char pname_from_cmd[PNAME_SIZE_MAX_KB];
    int len = 0;
    int cnt = 0;
    Logi("E, pname is %s", pname);
    rcu_read_lock();
    for_each_process(task) {
        if (task->flags & PF_KTHREAD) {
            if (strncmp(task->comm, pname, PNAME_SIZE_MAX_KB) == 0) {
                if (PID_SIZE_MAX == cnt) {
                    Loge("pids is full, cant add any more");
                    break;
                }
                pids[cnt] = task->pid;
                Logd("[kt]: pname=%s, pids[%d]=%d", task->comm, cnt, task->pid);
                cnt++;
            }
        } else {
            len = get_mm_cmdline(task, cmdline, sizeof(cmdline));
            if (len > 0) {
                get_pname_from_path(cmdline, pname_from_cmd, PNAME_SIZE_MAX_KB);
                if (strncmp(pname_from_cmd, pname, PNAME_SIZE_MAX_KB) == 0) {
                    if (PID_SIZE_MAX == cnt) {
                        Loge("pids is full, cant add any more");
                        break;
                    }
                    pids[cnt] = task->pid;
                    Logi("[ut]: pname=%s, pids[%d]=%d", pname_from_cmd, cnt, task->pid);
                    cnt++;
                }
            }
        }
    }
    *pids_cnt = cnt;
    rcu_read_unlock();
    Logi("X, pids_cnt is %d", *pids_cnt);
    return 0;
}

int do_get_tids_by_pname(const char *pname,const char *tname,int *tids,int *tids_cnt) {
    int ret = -1;
    return ret;
}

int get_pids_by_pname(hperf_pinfo_t *pinfo, hperf_pinfo_t *out) {
    if (pinfo == NULL || out == NULL) {
        return -EINVAL;
    }
    do_get_pids_by_pname(pinfo->pname, pinfo->pids, &(pinfo->pids_cnt));
    memcpy(out, pinfo, sizeof(hperf_pinfo_t));
    Logi("out pid cnt %d", pinfo->pids_cnt);
    return 0;
}

int get_tids_by_pid(hperf_task_t *task,hperf_task_t *out) {
    struct task_struct* t = NULL;
    struct task_struct* process = NULL;
    Logi("E pid:%d , tname:%s",task->pid,task->comm);
    if(task == NULL || out == NULL|| task->pid ==0) {
        return -EINVAL;
    }
    process =  get_task_by_pid(task->pid);
    if(process == NULL) {
        return -EINVAL;
    }
    rcu_read_lock();
    for_each_thread(process,t) {
        if(likely(t)) {
            if (strncmp(task->comm, t->comm, TASK_COMM_LEN) == 0) {
                Logi("pname:%s,tname:%s,pid:%d,tid:%d",process->comm,t->comm,process->pid,t->pid);
                out->pid = process->pid;
                memcpy(out->comm,  t->comm, TASK_COMM_LEN);
                out->state = t->__state;
                out->tid = t->pid;
                out->oom_score_adj = t->signal->oom_score_adj;
                out->policy = t->policy;
                out->priority = t->prio;
                break;
            }
        }

    }
    rcu_read_unlock();
    put_task_struct(process);
    Logi("X");
    return 0;
}


int get_gpu_info(hperf_gpu_boost_t *gpu_boost)  {
    int ret = 0;
    unsigned int loading;
    unsigned int block;
    unsigned int idle;
    unsigned int pMemUsage;
    unsigned int puiFreq;
    int piIndex;
    unsigned long bottompulFreq;
    if(!gpu_boost)
        return EINVAL;
    mtk_get_gpu_loading(&loading);
    mtk_get_gpu_block(&block);
    mtk_get_gpu_idle(&idle);
    mtk_get_gpu_memory_usage(&pMemUsage);
    mtk_dump_gpu_memory_usage();
    mtk_get_gpu_cur_freq(&puiFreq);
    mtk_get_gpu_cur_oppidx(&piIndex);
    mtk_get_gpu_bottom_freq(&bottompulFreq);

    Logi("loading %u block %u idle %u pMemUsage %u puiFreq %u piIndex %u bottompulFreq %lu\n",
    loading, block, idle,
    pMemUsage,puiFreq,piIndex,
    bottompulFreq
    );
    gpu_boost->gpu_freqlevel = piIndex;
    return ret;
}

int get_ddr_info(hperf_ddr_boost_t *ddr_boost) {
    int ret = 0;
    int curr_dvfs_opp;
    int curr_dram_khz;
    int curr_dvfs_level;
    int curr_dram_opp;
    unsigned int floor_limiter = 0;
    if(!ddr_boost)
        return EINVAL;
    mtk_get_gpu_floor_limiter(&floor_limiter);
    Logi("GPU floor limiter: %u", floor_limiter);
    Logi("E");
    curr_dvfs_opp = mtk_dvfsrc_query_opp_info(MTK_DVFSRC_CURR_DVFS_OPP);
    curr_dram_khz =  mtk_dvfsrc_query_opp_info(MTK_DVFSRC_CURR_DRAM_KHZ);
    curr_dvfs_level =  mtk_dvfsrc_query_opp_info(MTK_DVFSRC_CURR_DVFS_LEVEL);
    curr_dram_opp =  mtk_dvfsrc_query_opp_info(MTK_DVFSRC_NUM_DRAM_OPP);
    Logi("curr_dvfs_opp %d curr_dram_khz %d curr_dvfs_level %d curr_dram_opp %d",
        curr_dvfs_opp,curr_dram_khz,curr_dvfs_level,curr_dram_opp
    );
    ddr_boost->dram_opp = curr_dram_opp;
    Logi("X");
    return ret;
}