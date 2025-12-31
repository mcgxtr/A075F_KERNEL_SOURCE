#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/thermal.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/slab.h>
#include <linux/sched/signal.h>

#include <mtk_gpu_utility.h>
#include <dvfsrc-exp.h>

#include "hperf_handle_internal.h"
#include "hperf_log.h"
#include "hperf_kernel_api.h"

void kernel_print_binary(uint32_t val) {
    char buf[33];
    int i;
    int bits = 32;

    for (i = 0; i < bits; i++) {
        buf[i] = (val >> (bits - 1 - i)) & 1 ? '1' : '0';
    }
    buf[bits] = '\0';

    Logi("Binary (%d bits): %s\n", bits, buf);
    return;
}

static int set_cpu_mask(uint32_t cpu_mask,struct task_struct *task) {
    int ret = -1;
    uint32_t cpu_id;
    int i = 0;
    struct cpumask mask;
    cpumask_clear(&mask);
    
    Logi("E pid %d",task->pid);
    kernel_print_binary(cpu_mask);
    for (i = num_possible_cpus(); i > 0; i--) {
        bool cpu_enable =  cpu_mask >> (i-1) & 0x01;
        cpu_id = num_possible_cpus()-i;
        if(cpu_enable) {
            cpumask_set_cpu(cpu_id, &mask);
            Logi("cpu_enable %d cpu_id %d",cpu_enable,cpu_id);
        }

    }

    ret = set_cpus_allowed_ptr(task, &mask);
    if (ret == 0) {
        Loge("Process %d  bound success\n", task->pid);
    } else {
        Loge("Process %d bound failed\n", task->pid);
    }
    return ret;
}
int bind_process_core(hperf_cpu_mask_t *cpu_mask) {
    int ret = -1;
    struct task_struct *task;

    if (!cpu_mask || !cpu_mask->cpu_mask) {
        Loge("cpu_mask is 0");
        return ret;
    }

    task = get_task_by_pid(cpu_mask->pid);
    if (!task) {
        Loge("PID %d Not Found!", cpu_mask->pid);
        return -ESRCH;
    }
    ret = set_cpu_mask(cpu_mask->cpu_mask,task);
    put_task_struct(task);
    return ret;
}

int set_max_freq_init(void) {
    int i;
    unsigned int max_freq;
    struct cpufreq_policy *policy;
    for (i = 0; i < NUM_CPUS; i++) {
        policy = cpufreq_cpu_get(i);
        if (!policy) {
            Loge("Failed to get CPU %d policy\n", i);
            continue;
        }
        policy->policy = CPUFREQ_POLICY_PERFORMANCE;
        max_freq = policy->cpuinfo.max_freq;
        if (cpufreq_driver_target(policy, max_freq, CPUFREQ_RELATION_H)) {
            Loge("Failed to set max frequency for CPU %d\n", i);
        } else {
            Logi("Set CPU %d to max frequency: %u KHz\n", i, max_freq);
        }
        cpufreq_cpu_put(policy);
    }
    return 0;
}

int update_cpu_freq(hperf_cpu_boost_t *cpu_boost) {
    struct cpufreq_policy *small_core_policy = NULL;
    struct cpufreq_policy *big_core_policy = NULL;
    int small_cpu = 0;
    int big_cpu = -1;
    int ret = -1;
    if(!cpu_boost)
        return EINVAL;

    small_core_policy = cpufreq_cpu_get(small_cpu);
    if (!small_core_policy) {
        Loge("Failed to get policy for small CPU %d", small_cpu);
        goto exit;
    }

    //get big core cpufreq_policy
    for (big_cpu = 4; big_cpu < num_possible_cpus(); big_cpu++) {
        struct cpufreq_policy *temp_policy = cpufreq_cpu_get(big_cpu);
        if (!temp_policy) {
            continue;
        }

        if (temp_policy->cpuinfo.max_freq > small_core_policy->cpuinfo.max_freq) {
            big_core_policy = temp_policy;
            break;
        }
        cpufreq_cpu_put(temp_policy);
    }

    // check value
    if (cpu_boost->small_core_min_freq < 0 ||
        cpu_boost->small_core_min_freq > small_core_policy->cpuinfo.max_freq) {
        Loge("Invalid small core min frequency: %d", cpu_boost->small_core_min_freq);
    } else {
        Logi("small_core_min_freq %d",cpu_boost->small_core_min_freq);
        small_core_policy->min = cpu_boost->small_core_min_freq;
    }

    if (cpu_boost->big_core_min_freq < 0 ||
        !big_core_policy ||
        cpu_boost->big_core_min_freq > big_core_policy->cpuinfo.max_freq) {
        Loge("Invalid big core min frequency: %d", cpu_boost->big_core_min_freq);
    } else {
        Logi("big_core_min_freq %d",cpu_boost->big_core_min_freq);
        big_core_policy->min= cpu_boost->big_core_min_freq;
    }

    if (cpu_boost->small_core_max_freq < 0 ||
cpu_boost->small_core_max_freq > small_core_policy->cpuinfo.max_freq) {
        Loge("Invalid small core max frequency: %d", cpu_boost->small_core_max_freq);
    } else {
        Logi("small_core_max_freq %d",cpu_boost->small_core_max_freq);
        small_core_policy->max = cpu_boost->small_core_max_freq;
    }

    if (cpu_boost->big_core_max_freq < 0 ||
        !big_core_policy ||
        cpu_boost->big_core_max_freq > big_core_policy->cpuinfo.max_freq) {
        Loge("Invalid big core max frequency: %d", cpu_boost->big_core_max_freq);
    } else {
        Logi("big_core_max_freq %d",cpu_boost->big_core_max_freq);
        big_core_policy->max = cpu_boost->big_core_max_freq;
    }

    // set small core min freq
    ret = cpufreq_driver_target(small_core_policy, cpu_boost->small_core_min_freq, CPUFREQ_RELATION_H);
    if (ret) {
        Loge("Failed to set small core freq to %d", cpu_boost->small_core_min_freq);
        goto exit;
    }

    // set big core min freq
    ret = cpufreq_driver_target(big_core_policy, cpu_boost->big_core_min_freq, CPUFREQ_RELATION_H);
    if (ret) {
        Loge("Failed to set big core freq to %d", cpu_boost->big_core_min_freq);
        goto exit;
    }

exit:
    if (small_core_policy)
        cpufreq_cpu_put(small_core_policy);
    if (big_core_policy)
        cpufreq_cpu_put(big_core_policy);
    Logi("X");
    return ret;
}

int set_gpu_freq(hperf_gpu_boost_t *gpu_boost) {
    int ret;
    unsigned int max_opp;
    if(!gpu_boost)
        return EINVAL;
    // get max opp
    if (!mtk_custom_get_gpu_freq_level_count(&max_opp)) {
        Loge("Failed to get max GPU freq level");
        return EINVAL;
    }

    // check value
    if (gpu_boost->gpu_freqlevel >= max_opp || gpu_boost->gpu_freqlevel< 0) {
        Loge("Invalid GPU freq level %u (max %u)", gpu_boost->gpu_freqlevel, max_opp);
        return EINVAL;
    }

    ret = mtk_custom_boost_gpu_freq(gpu_boost->gpu_freqlevel);
    Logi("set_gpu_freq %u ,pid %d , ret %d ",
        gpu_boost->gpu_freqlevel,
        gpu_boost->pid,
        ret
    );
    return 0;
}

int set_task_info(hperf_task_t *task_info) {
    int ret;
    struct task_struct *task = NULL;
    if(task_info == 0) {
        return -EINVAL;
    }
    if(task_info->pid > 0 && strlen(task_info->comm) >0 ) {
        task = get_task_by_pid(task_info->pid);
    } else if( task_info->tid > 0 && strlen(task_info->comm) >0 ) {
        hperf_task_t task_info_out;
        ret = get_tids_by_pid(task_info,&task_info_out);
        task = get_task_by_pid(task_info_out.tid);
    }
    if(!task) {
        return -EINVAL;
    }
    set_cpu_mask(task_info->cpu_mask,task);
    task->signal->oom_score_adj = task_info->oom_score_adj;
    task->prio = task_info->priority;
    put_task_struct(task);
    return ret;
}
int set_adj(hperf_process_t *hperf_process) {
    int ret = 0;
    struct task_struct *task = NULL;
    int i = 0;
    if(hperf_process == 0 || hperf_process->oom_score_adj<0) {
        return -EINVAL;
    }
    Logi("E");
    for (i = 0; i < PID_SIZE_MAX; i++) {
        pid_t pid = hperf_process->pids[i];
        if(pid<=0)
            continue;
        Logi("get_task_by_pid pid:%d",pid);
        task = get_task_by_pid(pid);
        task->signal->oom_score_adj = hperf_process->oom_score_adj;
        Logi("set adj pid:%d,adj:%d",pid,hperf_process->oom_score_adj);
        if(!task) {
            continue;
        }
    }
    put_task_struct(task);
    Logi("X");
    return ret;
}

int set_priority(hperf_task_t *hperf_task) {
    int ret = 0;
    struct task_struct *task = NULL;
    Logi("E");
    if(hperf_task == 0||hperf_task->tid<=0||hperf_task->priority<0) {
        return -EINVAL;
    }
    task = get_task_by_pid(hperf_task->tid);
    if(!task) {
        return -EINVAL;
    }

    task->prio = hperf_task->priority;
    task->normal_prio = hperf_task->priority;
    put_task_struct(task);
    Logi("X");
    return ret;
}
