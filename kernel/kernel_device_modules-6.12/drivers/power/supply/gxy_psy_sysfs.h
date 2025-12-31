/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __GXY_PSY_SYSFS_H__
#define __GXY_PSY_SYSFS_H__

#include <linux/power_supply.h>
#include <linux/sysfs.h>


/* log debug */
#define GXY_PSY_TAG    "[GXY_PSY]"

#define GXY_PSY_DEBUG 1
#ifdef GXY_PSY_DEBUG
    #define GXY_PSY_INFO(fmt, args...)    pr_info(GXY_PSY_TAG fmt, ##args)
#else
    #define GXY_PSY_INFO(fmt, args...)
#endif

#define GXY_PSY_ERR(fmt, args...)    pr_err(GXY_PSY_TAG fmt, ##args)

/* battery psy attrs */
ssize_t gxy_bat_show_attrs(struct device *dev,
                            struct device_attribute *attr, char *buf);

ssize_t gxy_bat_store_attrs(struct device *dev,
                struct device_attribute *attr,
                const char *buf, size_t count);

#define GXY_BATTERY_ATTR(_name)                              \
{                                                      \
    .attr = {.name = #_name, .mode = 0664},        \
    .show = gxy_bat_show_attrs,                      \
    .store = gxy_bat_store_attrs,                      \
}

/* battery psy property */
enum {
    CHG_INFO = 0,
    /*Tab A9 code for SR-AX6739A-01-486 by qiaodan at 20230512 start*/
    INPUT_SUSPEND,
    /*Tab A9 code for SR-AX6739A-01-486 by qiaodan at 20230512 end*/
	/*Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 start*/
    AFC_RESULT,
    HV_DISABLE,
    /*Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 end*/
};

/*Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 start*/
/* AFC detected */
enum {
    AFC_INIT,
    NOT_AFC,
    AFC_FAIL,
    AFC_DISABLE,
    NON_AFC_MAX,
    AFC_5V = NON_AFC_MAX,
    AFC_9V,
    AFC_12V,
};
/*Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 end*/

/* battey psy property - chg_info data */
enum gxy_bat_chg_info {
    GXY_BAT_CHG_INFO_UNKNOWN = 0,
    GXY_BAT_CHG_INFO_SC89601D,
    GXY_BAT_CHG_INFO_UPM6910DH,
    GXY_BAT_CHG_INFO_UPM6910DS,
};

/* data struct type */
struct gxy_bat_hwinfo {
    enum gxy_bat_chg_info cinfo;
	/*Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 start*/
    int afc_result;
    /*Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 end*/
};

struct gxy_battery {
    struct power_supply *psy;
};

#endif /* __GXY_PSY_SYSFS_H__ */
