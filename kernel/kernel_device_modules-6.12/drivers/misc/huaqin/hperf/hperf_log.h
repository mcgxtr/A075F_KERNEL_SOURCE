#ifndef __HUAQIN_HPERF_LOG_H
#define __HUAQIN_HPERF_LOG_H

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/printk.h>  // 确保包含 printk 相关定义

#define LOG_TAG "hperf"

// 核心修复：为不同级别显式指定 KERN_<LEVEL> 前缀，并兼容 pr_debug
#define __LOG(level, kern_level, fmt, ...) \
    pr_##level(kern_level "[%s:%s:%d] " fmt, \
               LOG_TAG, __func__, __LINE__, ##__VA_ARGS__)

// 1. 调试日志（pr_debug）：必须显式指定 KERN_DEBUG 级别，并处理动态调试
// 注意：pr_debug 默认依赖 CONFIG_DYNAMIC_DEBUG，若未启用需定义 DEBUG 宏
#define Logd(fmt, ...) __LOG(debug, KERN_DEBUG, fmt, ##__VA_ARGS__)

// 2. 其他日志级别（info/warning/err）：使用对应 KERN_<LEVEL> 前缀
#define Logi(fmt, ...) __LOG(info, KERN_INFO, fmt, ##__VA_ARGS__)
#define Logw(fmt, ...) __LOG(warning, KERN_WARNING, fmt, ##__VA_ARGS__)
#define Loge(fmt, ...) __LOG(err, KERN_ERR, fmt, ##__VA_ARGS__)

#else
#ifndef LOG_TAG
#error "Please define LOG_TAG before include log.h"
#endif // LOG_TAG
#include <log/log.h>
#define Loge(format, ...) do{__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "[%s][%04d]" format, __func__, __LINE__, ##__VA_ARGS__);}while(0)
#define Logi(format, ...) do{__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[%s][%04d]" format, __func__, __LINE__, ##__VA_ARGS__);}while(0)
#define Logd(format, ...) do{__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "[%s][%04d]" format, __func__, __LINE__, ##__VA_ARGS__);}while(0)
#define Logv(format, ...) do{__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "[%s][%04d]" format, __func__, __LINE__, ##__VA_ARGS__);}while(0)
#endif // __KERNEL__

#endif // __HUAQIN_HPERF_LOG_H