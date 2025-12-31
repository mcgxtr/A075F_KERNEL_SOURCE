#ifndef __HUAQIN_HPERF_HANDLE_INTERNAL_H
#define __HUAQIN_HPERF_HANDLE_INTERNAL_H

#include <linux/sched.h>

#include "hperf_kernel_api.h"

#define PNAME_SIZE_MAX_KB 1024

// handle_kinfo start
struct task_struct* get_task_by_pid(pid_t pid);
int get_mm_cmdline(struct task_struct *task, char *buffer, int buflen);
int do_get_pids_by_pname(const char *pname, int *pids, int *pids_cnt);
int do_get_tids_by_pname(const char *pname,const char *tname,int *tids,int* tids_cnt);
// handle_kinfo end

// handle_event start
int send_signal_by_pid(int target_pid, int signal);
void hmem_exit(void);
// handle_event end

// handle_sched start
int set_max_freq_init(void);
// handle_sched end

// handle_mem start
// handle_mem end

// handle_other start
// handle_other end

#endif // __HUAQIN_HPERF_HANDLE_INTERNAL_H

