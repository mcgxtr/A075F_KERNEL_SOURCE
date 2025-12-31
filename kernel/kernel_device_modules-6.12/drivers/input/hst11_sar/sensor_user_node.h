#ifndef _SENSOR_USER_NODE_H_
#define _SENSOR_USER_NODE_H_

/*Tab A11 code for AX7800A-3476 by zhawei at 20240707 start*/
#define ANFR_IMPORT_ALL
/*Tab A11 code for AX7800A-3476 by zhawei at 20240707 end*/

typedef struct sar_driver_func {
    char *sar_name_drv;
    ssize_t (*set_onoff)(const char *buf, size_t count);
    ssize_t (*get_onoff)(char *buf);
    ssize_t (*set_dumpinfo)(const char *buf, size_t count);
    ssize_t (*get_dumpinfo)(char *buf);
    ssize_t (*set_enable)(const char *buf, size_t count);
    ssize_t (*get_enable)(char *buf);
    ssize_t (*set_receiver_turnon)(const char *buf, size_t count);
    void (*get_cali)(void);
} sar_driver_func_t;

extern sar_driver_func_t g_usernode;

#endif
