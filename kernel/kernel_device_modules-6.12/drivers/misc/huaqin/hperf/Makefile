obj-$(CONFIG_HPERF_ENABLE) += hperf.o

hperf-objs := hperf_main.o \
	hperf_monitor.o \
	hperf_handle_event.o \
	hperf_handle_kinfo.o \
	hperf_handle_mem.o \
	hperf_handle_sched.o \
	hperf_handle_other.o \
	hperf_handle_power.o \

ccflags-y += \
	-I$(DEVICE_MODULES_PATH)/drivers/gpu/mediatek/mt-plat/ \
	-I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/include/mt-plat/
