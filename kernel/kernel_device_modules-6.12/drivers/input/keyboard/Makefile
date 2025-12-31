# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the input core drivers.
#

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
subdir-ccflags-y += -Werror -I$(srctree)/drivers/misc/mediatek/include/mt-plat
ccflags-y += -I$(srctree)/drivers/misc/mediatek/eccci/inc/

obj-$(CONFIG_KEYBOARD_MTK_KPD)		+= mtk-kpd.o
obj-$(CONFIG_DEVICE_MODULES_KEYBOARD_MTK_PMIC)		+= mtk-pmic-keys.o
