# Makefile for the Himax touchscreen drivers.

ifeq ($(CONFIG_DRM_MEDIATEK), y)
ccflags-$(CONFIG_DRM_MEDIATEK) += -I$(srctree)/drivers/gpu/drm/mediatek/
endif
ccflags-$(CONFIG_DRM_MEDIATEK_V2) += -I$(srctree)/drivers/gpu/drm/mediatek/mediatek_v2/

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat
ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/inc/

obj-m += hxchipset.o
hxchipset-y += himax_common.o
hxchipset-y += himax_debug.o
hxchipset-y += himax_firmware.o
hxchipset-y += himax_ic_HX83102.o
hxchipset-y += himax_ic_incell_core.o
hxchipset-y += himax_inspection.o
hxchipset-y += himax_platform.o
