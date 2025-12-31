#include "hperf_log.h"
#include "hperf_handle_internal.h"

#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

int get_hperf_info(hperf_info_t *out_info) {
    if (!out_info)
        return EINVAL;
    out_info->version = 1;
    strcpy(out_info->name, "hperf");
    return 0;
}

static void do_send_custom_signal(int pid) {
    Logi("E, pid is %d", pid);
    struct task_struct *task;
    task = pid_task(find_get_pid((pid_t)pid), PIDTYPE_PID);
    if (task) {
        int res = send_sig(SIGUSR2, task, 0);
        Logi("signal sent to pid(%d) res %d", pid, res);
    }
    Logi("X");
}

int send_custom_signal(hperf_custom_signal_info_t *data, hperf_custom_signal_info_t *out_data) {
    Logi("E");
    if (!data) return EINVAL;
    hperf_pinfo_t pinfo;
    strncpy(pinfo.pname, data->receive_pname, sizeof(data->receive_pname));
    do_get_pids_by_pname(pinfo.pname, pinfo.pids, &(pinfo.pids_cnt));
    if (pinfo.pids_cnt > 0) {
        Logi("running now");
        out_data->tmp = -1;
        do_send_custom_signal(pinfo.pids[0]);
    } else {
        out_data->tmp = 1;
        Logi("no running, skip");
    }
    Logi("X");
    return 0;
}