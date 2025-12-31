#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/cpumask.h>
#include <linux/thermal.h>
#include <linux/power_supply.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/sched/task.h>
#include <linux/slab.h>
#include <mtk_gpu_utility.h>
#include <dvfsrc-exp.h>
#include <linux/alarmtimer.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include "hperf_handle_internal.h"
#include "hperf_log.h"

#define APCPU_THERMAL_DEFAULT 25
int power_get_apcpu_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("ap_ntc");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for ap_ntc\n");
		goto APCPU_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto APCPU_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get apcpu temperature=%d", temperature);
		return temperature;
	}

APCPU_THERM_FAIL:
	return APCPU_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_apcpu_hw_temperature);

#define LTEPA_THERMAL_DEFAULT 25
int power_get_ltepa_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("ltepa_ntc");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for ltepa_ntc\n");
		goto LTEPA_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto LTEPA_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get ltepa temperature=%d", temperature);
		return temperature;
	}

LTEPA_THERM_FAIL:
	return LTEPA_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_ltepa_hw_temperature);

#define CHARGER_THERMAL_DEFAULT 25
int power_get_charger_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("charger_therm");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for charger_therm\n");
		goto CHARGER_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto CHARGER_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get charger temperature=%d", temperature);
		return temperature;
	}

CHARGER_THERM_FAIL:
	return CHARGER_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_charger_hw_temperature);

#define MD_THERMAL_DEFAULT 25
int power_get_md_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("md");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for md\n");
		goto MD_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto MD_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get modem temperature=%d", temperature);
		return temperature;
	}

MD_THERM_FAIL:
	return MD_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_md_hw_temperature);

#define MT6366_THERMAL_DEFAULT 25
int power_get_pmic_mt6366_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("pmic6366_pmu");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for pmic6366_pmu\n");
		goto MT6366_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto MT6366_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get pmic mt6366 temperature=%d", temperature);
		return temperature;
	}

MT6366_THERM_FAIL:
	return MT6366_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_pmic_mt6366_hw_temperature);

#define CONSYS_THERMAL_DEFAULT 25
int power_get_consys_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("consys");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for consys\n");
		goto CONSYS_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto CONSYS_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get consys(BT/WIFI) temperature=%d", temperature);
		return temperature;
	}

CONSYS_THERM_FAIL:
	return CONSYS_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_consys_hw_temperature);

#define GPU1_THERMAL_DEFAULT 25
int power_get_gpu1_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("gpu1");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for gpu1\n");
		goto GPU1_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto GPU1_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get gpu1 temperature=%d", temperature);
		return temperature;
	}

GPU1_THERM_FAIL:
	return GPU1_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_gpu1_hw_temperature);

#define GPU2_THERMAL_DEFAULT 25
int power_get_gpu2_hw_temperature(void)
{
	struct thermal_zone_device *tz = NULL;
	int temperature = 0;
	int ret = 0;

	tz = thermal_zone_get_zone_by_name("gpu2");
	if (IS_ERR(tz)) {
		Loge("Unable to get thermal zone for gpu2\n");
		goto GPU2_THERM_FAIL;
	}
	ret = thermal_zone_get_temp(tz, &temperature);
	if (ret) {
		Loge("Failed to get thermal zone temperature");
		goto GPU2_THERM_FAIL;
	} else {
		temperature /= 1000;
		Loge("get gpu2 temperature=%d", temperature);
		return temperature;
	}

GPU2_THERM_FAIL:
	return GPU2_THERMAL_DEFAULT;
}
EXPORT_SYMBOL(power_get_gpu2_hw_temperature);

int power_get_battery_temperature(void)
{
    static struct power_supply *s_batt_psy = NULL;
    int ret = 0;
    union power_supply_propval prop = {0};
    const int bat_temp = 250; // 25℃

    if (!s_batt_psy) {
        s_batt_psy = power_supply_get_by_name("battery");
        if (!s_batt_psy) {
            Loge("get battery psy failed\n");
            return bat_temp;
        }
    }
    ret = power_supply_get_property(s_batt_psy, POWER_SUPPLY_PROP_TEMP, &prop);
    if (ret < 0) {
        Loge("get bat_temp property fail\n");
        return bat_temp;
    }
    return prop.intval;
}
EXPORT_SYMBOL(power_get_battery_temperature);

/*
int power_get_battery_ocv(void)
{
    static struct power_supply *s_batt_psy = NULL;
    int ret = 0;
    union power_supply_propval prop = {0};
    const int bat_ocv = 4000000; // 4000mV

    if (!s_batt_psy) {
        s_batt_psy = power_supply_get_by_name("battery");
        if (!s_batt_psy) {
            Loge("get battery psy failed\n");
            return bat_ocv;
        }
    }
    ret = power_supply_get_property(s_batt_psy, POWER_SUPPLY_PROP_VOLTAGE_OCV, &prop);
    if (ret < 0) {
        Loge("get bat_ocv property fail\n");
        return bat_ocv;
    }
    return prop.intval;
}
EXPORT_SYMBOL(power_get_battery_ocv);
*/

int power_get_battery_vbat(void)
{
    static struct power_supply *s_batt_psy = NULL;
    int ret = 0;
    union power_supply_propval prop = {0};
    const int vbat = 4000000; // 4000mV

    if (!s_batt_psy) {
        s_batt_psy = power_supply_get_by_name("battery");
        if (!s_batt_psy) {
            Loge("get battery psy failed\n");
            return vbat;
        }
    }
    ret = power_supply_get_property(s_batt_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &prop);
    if (ret < 0) {
        Loge("get vbat property fail\n");
        return vbat;
    }
    return prop.intval;
}
EXPORT_SYMBOL(power_get_battery_vbat);

int power_get_battery_uisoc(void)
{
    static struct power_supply *s_batt_psy = NULL;
    int ret = 0;
    union power_supply_propval prop = {0};
    const int bat_uisoc = 50; // 50%

    if (!s_batt_psy) {
        s_batt_psy = power_supply_get_by_name("battery");
        if (!s_batt_psy) {
            Loge("get battery psy failed\n");
            return bat_uisoc;
        }
    }
    ret = power_supply_get_property(s_batt_psy, POWER_SUPPLY_PROP_CAPACITY, &prop);
    if (ret < 0) {
        Loge("get bat_uisoc property fail\n");
        return bat_uisoc;
    }
    return prop.intval;
}
EXPORT_SYMBOL(power_get_battery_uisoc);

int power_get_battery_current(void)
{
    static struct power_supply *s_batt_psy = NULL;
    int ret = 0;
    union power_supply_propval prop = {0};
    const int bat_current_now = 50000; // 500mA

    if (!s_batt_psy) {
        s_batt_psy = power_supply_get_by_name("battery");
        if (!s_batt_psy) {
            Loge("get battery psy failed\n");
            return bat_current_now;
        }
    }
    ret = power_supply_get_property(s_batt_psy, POWER_SUPPLY_PROP_CURRENT_NOW, &prop);
    if (ret < 0) {
        Loge("get bat_current_now property fail\n");
        return bat_current_now;
    }
    return prop.intval;
}
EXPORT_SYMBOL(power_get_battery_current);

int power_get_usb_online(void)
{
    static struct power_supply *s_usb_psy = NULL;
    int ret = 0;
    union power_supply_propval prop = {0};
    const int usb_online = 0; // offline

    if (!s_usb_psy) {
        s_usb_psy = power_supply_get_by_name("usb");
        if (!s_usb_psy) {
            Loge("get battery psy failed\n");
            return usb_online;
        }
    }
    ret = power_supply_get_property(s_usb_psy, POWER_SUPPLY_PROP_ONLINE, &prop);
    if (ret < 0) {
        Loge("get usb_online property fail\n");
        return usb_online;
    }
    return prop.intval;
}
EXPORT_SYMBOL(power_get_usb_online);

/*
int power_get_coulomb(void)
{
    int car = 0;
    static struct power_supply *psy = NULL;
    struct mtk_gauge *gauge = NULL;

    if (!psy) {
        psy = power_supply_get_by_name("mtk-gauge");
        if (!psy) {
            Loge("psy is NULL\n");
            return -1;
        }
    }
    gauge = (struct mtk_gauge *)power_supply_get_drvdata(psy);

    car = gauge_get_int_property(gauge->gm, GAUGE_PROP_COULOMB);

    return car;
}
EXPORT_SYMBOL(power_get_coulomb);
*/

/*
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
struct timespec64 {
    time64_t    tv_sec; // seconds
    long    tv_nsec; // nanoseconds
};
struct rtc_time {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};
void ktime_get_real_ts64(struct timespec64 *ts);
void rtc_time64_to_tm(time64_t time, struct rtc_time *tm);
*/
struct timespec64 power_get_rtc_ktime_time(void)
{
    struct timespec64 ts = {0};

    ktime_get_real_ts64(&ts);
    Logi("tv_sec=%lld, tv_nsec=%ld\n", ts.tv_sec, ts.tv_nsec);

    return ts;
}
EXPORT_SYMBOL(power_get_rtc_ktime_time);

struct rtc_time power_get_utc_time(struct timespec64 *timestamp)
{
    // struct timespec64 ts = {0};
    struct rtc_time tm = {0};
    struct rtc_time ret_tm = {0};

    if (!timestamp) {
        Loge("timestamp is NULL\n");
        return ret_tm;
    }

    // ts = *timestamp;
    // ktime_get_real_ts64(&ts);
    ktime_get_real_ts64(timestamp);
    rtc_time64_to_tm(timestamp->tv_sec, &tm);

    ret_tm.tm_year = tm.tm_year + 1900;
    ret_tm.tm_mon = tm.tm_mon + 1;
    ret_tm.tm_mday = tm.tm_mday;
    ret_tm.tm_hour = tm.tm_hour + 8;
    ret_tm.tm_min = tm.tm_min;
    ret_tm.tm_sec = tm.tm_sec;
    Logi("UTC time: %d-%d-%d %d:%d:%d \n",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour + 8, tm.tm_min, tm.tm_sec);

    return ret_tm;
}
EXPORT_SYMBOL(power_get_utc_time);

#define ABS_POWER(x)  \
        ((x < 0) ? -x : x)
#define CAR_TIME_HOUR_SECONDS   3600
#define CAR_TIME_MINUTE_SECONDS     60
#define POWER_AVERAGE_MA    10
int power_get_power_consumption_data(power_info_t *data, power_info_t *to_user)
{
    static int car_start = 0;
    static int car_end = 0;
    static struct timespec64 timestamp_start = {0};
    static struct timespec64 timestamp_end = {0};
    static struct rtc_time car_time_start = {0};
    static struct rtc_time car_time_end = {0};
    static int bat_uisoc_start = 0;
    static int bat_uisoc_end = 0;
    static int bat_vbat_start = 0;
    static int bat_vbat_end = 0;
    static int bat_ocv_start = 0;
    static int bat_ocv_end = 0;
    int delta_car = 0;
    int delta_time = 0;
    int power_avg = 0;
    int apcpu_temperature = 0;
    int gpu_temperature = 0;
    int usb_online = 0;
    power_info_t *p_in_data = NULL;

    if (!data || !to_user) {
        Loge("data or to_user is NULL\n");
        return -EINVAL;
    }

    p_in_data = data;
    if (p_in_data->app_power_cmd == APP_POWER_CMD_START) {
        timestamp_start = power_get_rtc_ktime_time();
        car_time_start = power_get_utc_time(&timestamp_start);
        car_start = power_get_coulomb();
        bat_uisoc_start = power_get_battery_uisoc();
        bat_vbat_start = power_get_battery_vbat();
        bat_ocv_start = power_get_battery_ocv();
        // Logi("[power]%s start\n", p_in_data->app_name);
    } else if (p_in_data->app_power_cmd == APP_POWER_CMD_END) {
        timestamp_end = power_get_rtc_ktime_time();
        car_time_end = power_get_utc_time(&timestamp_end);
        car_end = power_get_coulomb();
        bat_uisoc_end = power_get_battery_uisoc();
        bat_vbat_end = power_get_battery_vbat();
        bat_ocv_end = power_get_battery_ocv();
    } else {
        // to be continued
    }

    if (p_in_data->app_power_cmd == APP_POWER_CMD_END) {
        delta_car = car_end - car_start;
        delta_time = (((car_time_end.tm_hour - car_time_start.tm_hour) * CAR_TIME_HOUR_SECONDS) + ((car_time_end.tm_min - car_time_start.tm_min) * CAR_TIME_MINUTE_SECONDS) + (car_time_end.tm_sec - car_time_start.tm_sec));
        power_avg = (delta_car * CAR_TIME_HOUR_SECONDS) / delta_time;
        power_avg = ABS_POWER(power_avg);
        p_in_data->app_power_data[0] = power_avg / POWER_AVERAGE_MA;

        apcpu_temperature = power_get_apcpu_hw_temperature();
        gpu_temperature = power_get_gpu1_hw_temperature();
        usb_online = power_get_usb_online();
        p_in_data->app_power_data[1] = delta_time;
        p_in_data->app_power_data[2] = bat_uisoc_start;
        p_in_data->app_power_data[3] = bat_uisoc_end;
        p_in_data->app_power_data[4] = bat_vbat_start;
        p_in_data->app_power_data[5] = bat_vbat_end;
        p_in_data->app_power_data[6] = bat_ocv_start;
        p_in_data->app_power_data[7] = bat_ocv_end;
        p_in_data->app_power_data[8] = usb_online;
        p_in_data->app_power_data[9] = delta_car < 0 ? 0 : 1;
        p_in_data->app_power_data[10] = apcpu_temperature;
        p_in_data->app_power_data[11] = gpu_temperature;

        memcpy(to_user, p_in_data, sizeof(power_info_t));
        Logi("[power]power_avg=%dmA, delta_car=%d 0.1C: %d %d, delta_time=%ds: %d:%d:%d\n",
            p_in_data->app_power_data[0], delta_car, car_start, car_end, delta_time,
            car_time_end.tm_hour - car_time_start.tm_hour, car_time_end.tm_min - car_time_start.tm_min, car_time_end.tm_sec - car_time_start.tm_sec);
        Logi("[power]uisoc[%d %d], vbat[%d %d]uv, ocv[%d %d]uv, USB %s, Battery %s, apcpu=%dC, gpu=%dC\n",
            bat_uisoc_start, bat_uisoc_end, bat_vbat_start, bat_vbat_end, bat_ocv_start, bat_ocv_end,
            usb_online ? "online" : "offline", delta_car < 0 ? "discharged" : "charged", apcpu_temperature, gpu_temperature);
    }

    return 0;
}
EXPORT_SYMBOL(power_get_power_consumption_data);

static int power_time_interval = 0;
enum hrtimer_restart power_get_thread_hrtimer_function(struct hrtimer *timer)
{
    ktime_t ktime = 0;
    u64 overruns = 0;

    if (!timer) {
        Loge("[power]timer in NULL\n");
        return HRTIMER_NORESTART;
    }

    // power_get_power_consumption_data(power_info_t *data, power_info_t *to_user);

    ktime = ktime_set(power_time_interval, 0);
    overruns = hrtimer_forward_now(timer, ktime); // loop
    Logi("[power]overruns=%llu\n", overruns);

    // return HRTIMER_NORESTART;
    return HRTIMER_RESTART;
}

void power_get_thread_hrtimer_init(int time_interval_seconds)
{
    ktime_t ktime = 0;
    struct hrtimer power_hrtimer;

    if (time_interval_seconds < 0) {
        Loge("[power]time_interval_seconds=%d\n", time_interval_seconds);
        time_interval_seconds = 10;
    }

    power_time_interval = time_interval_seconds;
    power_hrtimer.function = power_get_thread_hrtimer_function;
    ktime = ktime_set(power_time_interval, 0);
    hrtimer_init(&power_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    hrtimer_start(&power_hrtimer, ktime, HRTIMER_MODE_REL);
}
EXPORT_SYMBOL(power_get_thread_hrtimer_init);

static struct delayed_work power_get_delayed_work;
static void power_work_handler(struct work_struct *work)
{
    Logi("[power]power_work_handler start\n");

    // power_get_power_consumption_data(power_info_t *data, power_info_t *to_user);

    schedule_delayed_work(&power_get_delayed_work, msecs_to_jiffies(10000)); // 10000ms
}

int power_get_power_consumption_by_delayed_work(power_info_t *data, power_info_t *to_user)
{
    power_info_t *p_in_data = NULL;

    if (!data || !to_user) {
        Loge("data or to_user is NULL\n");
        return -1;
    }

    p_in_data = data;
    if (p_in_data->app_power_cmd == APP_POWER_CMD_START) {
        INIT_DELAYED_WORK(&power_get_delayed_work, power_work_handler);
        schedule_delayed_work(&power_get_delayed_work, msecs_to_jiffies(1000)); // 1000ms
    } else if (p_in_data->app_power_cmd == APP_POWER_CMD_END) {
        cancel_delayed_work_sync(&power_get_delayed_work);
    } else {
        // to be continued
    }

    return 0;
}
EXPORT_SYMBOL(power_get_power_consumption_by_delayed_work);

static struct task_struct *power_get_task = NULL;
static int power_kthread_handler(void *data)
{
    while (!kthread_should_stop()) {
        // power_get_power_consumption_data(power_info_t *data, power_info_t *to_user);
    }

    return 0;
}

int power_get_power_consumption_by_kthread(power_info_t *data, power_info_t *to_user)
{
    power_info_t *p_in_data = NULL;

    if (!data || !to_user) {
        Loge("data or to_user is NULL\n");
        return -1;
    }

    p_in_data = data;
    if (p_in_data->app_power_cmd == APP_POWER_CMD_START) {
        power_get_task = kthread_run(power_kthread_handler, NULL, "power_kthread");
        if (IS_ERR(power_get_task)) {
            printk("Error creating thread\n");
            return PTR_ERR(power_get_task);
        }
    } else if (p_in_data->app_power_cmd == APP_POWER_CMD_END) {
        if (power_get_task) {
            kthread_stop(power_get_task);
        }
    } else {
        // to be continued
    }

    return 0;
}
EXPORT_SYMBOL(power_get_power_consumption_by_kthread);
