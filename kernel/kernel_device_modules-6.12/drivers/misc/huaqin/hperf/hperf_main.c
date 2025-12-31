#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "hperf_log.h"
#include "hperf_handle_internal.h"

static struct proc_dir_entry *hperf_proc_entry;
static hperf_info_t hperf_data = {
    .version = 1,
    .name = "hperf_module",
};

static int hperf_proc_show(struct seq_file *m, void *v) {
    /*
    static int power_app_power_test = 0;
    static power_info_t data = {0};
    static power_info_t to_user = {0};
    */

    seq_printf(m, "hperf Module:\n");
    seq_printf(m, "Version: %d\n", hperf_data.version);
    seq_printf(m, "Name: %s\n", hperf_data.name);

    /*
    if (power_app_power_test == 0) {
        data.app_power_cmd = APP_POWER_CMD_START;
        power_get_power_consumption_data(&data, &to_user);
        Logi("[power]power_app_power_test=%d, data->app_power_cmd=%d\n",
                power_app_power_test, data.app_power_cmd);
        power_app_power_test++;
    } else if (power_app_power_test == 1) {
        data.app_power_cmd = APP_POWER_CMD_END;
        power_get_power_consumption_data(&data, &to_user);
        Logi("[power]power_app_power_test=%d, data->app_power_cmd=%d, power_avg=%dmA\n",
                power_app_power_test, data.app_power_cmd, to_user.app_power_data[0]);
        seq_printf(m, "power_avg=%dmA\n", to_user.app_power_data[0]);
        power_app_power_test = 0;
        to_user.app_power_data[0] = 0;
    } else {
        Logi("[power]other test\n");
    }
    Logi("[power]power_app_power_test=%d\n", power_app_power_test);
    seq_printf(m, "power_app_power_test=%d\n", power_app_power_test);
    */

    return 0;
}

static int hperf_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, hperf_proc_show, NULL);
}

int handle_ioc_generic(unsigned long arg) {
    hperf_generic_params_t params = { 0, 0, NULL, 0, NULL };
    int ret = 0;
    int cmd_id = 0;
    int in_data_length = 0;
    int out_data_length = 0;
    char *in_data = NULL;
    char *out_data = NULL;

    if (copy_from_user(&params, (void __user *)arg, sizeof(hperf_generic_params_t))) {
        Loge("gen_params get error");
        ret = -EFAULT;
        goto EXIT;
    }
    cmd_id = params.cmd_id;
    in_data_length = params.args_length;
    out_data_length = params.out_length;

    if (in_data_length <= 0 && out_data_length <= 0) {
        ret = -EINVAL;
        goto EXIT;
    }

    if (in_data_length > 0) {
        in_data = kzalloc(in_data_length, GFP_KERNEL);
        if (!in_data) {
            Loge("args alloc error");
            ret = -ENOMEM;
            goto EXIT;
        }
        if (copy_from_user(in_data, params.args, in_data_length)) {
            Loge("gen_params args cp error");
            ret = -EFAULT;
            goto EXIT;
        }
    }
    if (out_data_length > 0) {
        out_data = kzalloc(out_data_length, GFP_KERNEL);
        if (!out_data) {
            Loge("out alloc error");
            ret = -ENOMEM;
            goto EXIT;
        }
    }

    Logi("E cmd_id %d",cmd_id);
    switch(cmd_id) {
        case CMD_VERSION_INFO:
            get_hperf_info((hperf_info_t*) out_data);
            break;
        case CMD_GET_GPU_INFO:
            get_gpu_info((hperf_gpu_boost_t*)out_data);
            break;
        case CMD_GET_DDR_INFO:
            get_ddr_info((hperf_ddr_boost_t*)out_data);
            break;
        case CMD_GET_PIDS_BY_PNAME: {
            get_pids_by_pname((hperf_pinfo_t*)in_data, (hperf_pinfo_t*)out_data);
            break;
        }
        case CMD_PROCESS_CTRL:
            process_signal_ctrl((hperf_process_t*)in_data);
            break;
        case CMD_PROCESS_CPU_BIND:
            bind_process_core((hperf_cpu_mask_t*)in_data);
            break;
        case CMD_CPU_FREQ:
            update_cpu_freq((hperf_cpu_boost_t*)in_data);
            break;
        case CMD_GPU_BOOST:
            set_gpu_freq((hperf_gpu_boost_t*)in_data);
            break;
        case CMD_KEEP_ALIVE:
            break;
        case CMD_GET_TASK_INFO: {
            get_tids_by_pid((hperf_task_t*)in_data,(hperf_task_t*)out_data);
            break;
        }
        case CMD_SET_TASK_INFO: {
            set_task_info((hperf_task_t*)in_data);
            break;
        }
        case CMD_SET_ADJ: {
            set_adj((hperf_process_t*)in_data);
            break;
        }
        case CMD_SET_PRIORITY: {
            set_priority((hperf_task_t*)in_data);
            break;
        }
        case CMD_GET_APP_POWER:
            power_get_power_consumption_data((power_info_t *)in_data, (power_info_t *)out_data);
            break;
        case CMD_SET_CUSTOM_SIGNAL: {
            send_custom_signal((hperf_custom_signal_info_t *)in_data, (hperf_custom_signal_info_t *)out_data);
            Logi("Letian: tmp %d", ((hperf_custom_signal_info_t *)out_data)->tmp);
            break;
        }
        default: {
            Loge("no such cmd");
            ret = -EINVAL;
            break;
        }
    }
    if (ret != 0) {
        goto EXIT;
    }

    if (out_data_length > 0 && out_data != NULL) {
        if (copy_to_user((void __user *)params.out, out_data, out_data_length)) {
            Loge("gen_params out cp error");
            ret = -EFAULT;
            goto EXIT;
        }
    }
EXIT:
    if (in_data != NULL) {
        kfree(in_data);
    }
    if (out_data != NULL) {
        kfree(out_data);
    }
    Logi("X, ret %d", ret);
    return ret;
}

static long hperf_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int ret = 0;
    hperf_cpu_mask_t cpu_mask;

    Logi("E cmd 0x%x", cmd);
    switch (cmd) {
         /* HS07 code for HS07-899 by hujingcheng at 20250621 start*/
        case HPERF_IOC_CPU_BIND:
             if (copy_from_user(&cpu_mask, (void __user *)arg, sizeof(hperf_cpu_mask_t))) {
                ret = -EFAULT;
                break;
            }
            Logi("HPERF_IOC_CPU_BIND");
            bind_process_core(&cpu_mask);
            break;
         /* HS07 code for HS07-899 by hujingcheng at 20250621 end*/
        /* HST11 code for AX7800A-1996 by wangletian at 20250630 end */
        case HPERF_IOC_GENERIC:
            ret = handle_ioc_generic(arg);
            break;
        /* HST11 code for AX7800A-1996 by wangletian at 20250630 end */
        default:
            ret = -ENOTTY;
            break;
    }
    Logi("X cmd 0x%x", cmd);
    return ret;
}

static const struct proc_ops hperf_proc_fops = {
    .proc_open = hperf_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
    .proc_ioctl = hperf_ioctl,
};

static int __init hperf_init(void) {
    // create /proc/hperf
    hperf_proc_entry = proc_create("hperf", 0666, NULL, &hperf_proc_fops);
    if (!hperf_proc_entry) {
        Loge("Failed to create /proc/hperf\n");
        return -ENOMEM;
    }
    Logi("hperf module loaded\n");
    return 0;
}

static void __exit hperf_exit(void) {
    /* HS07 code for HS07-899 by hujingcheng at 20250626 start*/
    hmem_exit();
    /* HS07 code for HS07-899 by hujingcheng at 20250626 end*/
    proc_remove(hperf_proc_entry);
    Logi("hperf module unloaded\n");
}

module_init(hperf_init);
module_exit(hperf_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hperf driver");
