#ifndef __HUAQIN_HPERF_KERNEL_API_H
#define __HUAQIN_HPERF_KERNEL_API_H
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <sys/types.h>
#include<stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_CPUS 8
#define PID_SIZE_MAX 64
#define PNAME_SIZE_MAX 256
#define HPERF_INFO_NAME_MAX 32
#define TASK_COMM_LEN 16

typedef struct hperf_generic_params {
    int cmd_id;
    int args_length;
    void *args;
    int out_length;
    void *out;
} hperf_generic_params_t;

typedef struct hperf_info {
    int version;
    char name[HPERF_INFO_NAME_MAX];
} hperf_info_t;

typedef struct hperf_cpu_boost {
    pid_t pid;
    uint32_t small_core_min_freq;
    uint32_t  big_core_min_freq;
    uint32_t small_core_max_freq;
    uint32_t  big_core_max_freq;
} hperf_cpu_boost_t;

typedef struct hperf_process {
    int signal;
    int oom_score_adj;
    int priority;
    int pids[PID_SIZE_MAX];
} hperf_process_t;

typedef struct hperf_cpu_mask {
    pid_t pid;
    // 0000 0000 ,eg if CPU0 support,set 1000 0000
    uint32_t cpu_mask;
} hperf_cpu_mask_t;

typedef struct hperf_pinfo {
    char pname[PNAME_SIZE_MAX];
    int pids_cnt;
    int pids[PID_SIZE_MAX];
} hperf_pinfo_t;

typedef struct hperf_task {
    pid_t pid;
    char comm[PNAME_SIZE_MAX];
    long state;
    pid_t tid;
    int oom_score_adj;
    int priority;
    uint32_t policy;
    uint32_t cpu_mask;
} hperf_task_t;

typedef struct hperf_gpu_boost {
    pid_t pid;
    uint32_t gpu_freqlevel;
} hperf_gpu_boost_t;

typedef struct hperf_ddr_boost {
    pid_t pid;
    uint32_t dram_opp;
} hperf_ddr_boost_t;

#define APP_POWER_DATA_LEN  15
// #define APP_NAME_LEN        30
enum app_power_cmd {
    APP_POWER_CMD_START = 0,
    APP_POWER_CMD_END,
};

typedef struct power_info {
    int app_power_cmd; // start or end
    // char app_name[APP_NAME_LEN];
    int app_power_data[APP_POWER_DATA_LEN]; // power, time,battery level,battery voltage,USB,charging,battery temparature,AP,GPU,modem,pmic,connect
} power_info_t;

typedef struct hperf_custom_signal_info {
    int receive_pid;
    char receive_pname[PNAME_SIZE_MAX];
    int tmp;
} hperf_custom_signal_info_t;

typedef enum generic_command {
    CMD_NONE = 0,
    CMD_VERSION_INFO, // 1
    CMD_GET_GPU_INFO, // 2
    CMD_GET_DDR_INFO, // 3
    CMD_GET_PIDS_BY_PNAME, // 4
    CMD_PROCESS_CTRL, // 5
    CMD_PROCESS_CPU_BIND, // 6
    CMD_CPU_FREQ, // 7
    CMD_GPU_BOOST, // 8
    CMD_KEEP_ALIVE, // 9
    CMD_GET_TASK_INFO,
    CMD_SET_TASK_INFO,
    CMD_SET_ADJ,
    CMD_SET_PRIORITY,
    CMD_GET_APP_POWER,
    CMD_SET_CUSTOM_SIGNAL
} generic_command_t;

#define HPERF_IOC_MAGIC 'h'
// Geneic IOC
#define HPERF_IOC_GENERIC _IOW(HPERF_IOC_MAGIC, 1, hperf_generic_params_t)
// CPU BIND
#define HPERF_IOC_CPU_BIND _IOW(HPERF_IOC_MAGIC, 11, hperf_cpu_mask_t)

// handle_kinfo start
int get_pids_by_pname(hperf_pinfo_t *pinfo, hperf_pinfo_t *out);
int get_tids_by_pid(hperf_task_t *task,hperf_task_t *out);
int get_gpu_info(hperf_gpu_boost_t *gpu_boost);
int get_ddr_info(hperf_ddr_boost_t *ddr_boost);
// handle_kinfo end

// handle_event start
int process_signal_ctrl(hperf_process_t *hperf_sig_process);
 // handle_event end

// handle_sched start
int bind_process_core(hperf_cpu_mask_t *cpu_mask);
int update_cpu_freq(hperf_cpu_boost_t *cpu_boost);
int set_gpu_freq(hperf_gpu_boost_t *gpu_boost);
int set_task_info(hperf_task_t *hperf_task);
int set_adj(hperf_process_t *hperf_process);
int set_priority(hperf_task_t *hperf_task);
// handle_sched end

// handle_mem start
// handle_mem end

// handle_other start
int get_hperf_info(hperf_info_t *out_info);
int send_custom_signal(hperf_custom_signal_info_t *in_data, hperf_custom_signal_info_t *out_data);
// handle_other end

// handle power with something start
int power_get_apcpu_hw_temperature(void);
int power_get_ltepa_hw_temperature(void);
int power_get_charger_hw_temperature(void);
int power_get_md_hw_temperature(void);
int power_get_pmic_mt6366_hw_temperature(void);
int power_get_consys_hw_temperature(void);
int power_get_gpu1_hw_temperature(void);
int power_get_gpu2_hw_temperature(void);
int power_get_battery_temperature(void);
int power_get_battery_ocv(void);
int power_get_battery_vbat(void);
int power_get_battery_uisoc(void);
int power_get_battery_current(void);
int power_get_usb_online(void);
int power_get_coulomb(void);
struct timespec64 power_get_rtc_ktime_time(void);
struct rtc_time power_get_utc_time(struct timespec64 *timestamp);
int power_get_power_consumption_data(power_info_t *data, power_info_t *to_user);
void power_get_thread_hrtimer_init(int time_interval_seconds);
int power_get_power_consumption_by_delayed_work(power_info_t *data, power_info_t *to_user);
int power_get_power_consumption_by_kthread(power_info_t *data, power_info_t *to_user);
// handle power with something end

#ifdef __cplusplus
}
#endif
#endif  // __HUAQIN_HPERF_KERNEL_API_H