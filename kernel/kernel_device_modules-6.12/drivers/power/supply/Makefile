# SPDX-License-Identifier: GPL-2.0
subdir-ccflags-$(CONFIG_POWER_SUPPLY_DEBUG) := -DDEBUG
subdir-ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/usb/usb20/
subdir-ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/typec/tcpc/inc
subdir-ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/power_throttling
subdir-ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/misc/mediatek/include
subdir-ccflags-y += -I$(DEVICE_MODULES_PATH)/drivers/power/supply/ufcs

ifeq ($(CONFIG_CHARGER_SGM41516D),m)
obj-$(CONFIG_CHARGER_SGM41516D) +=  mtk_chg_det.o
#else
#obj-$(CONFIG_MTK_CHARGER) += mtk_charger_framework.o mtk_pd_charging.o  mtk_pd_adapter.o mtk_ufcs_adapter.o
endif
#mtk_charger_framework-objs := mtk_charger.o mtk_charger_intf.o mtk_basic_charger.o mtk_pulse_charger.o mtk_adapter_switch.o

mtk_pd_charging-objs := mtk_pd.o mtk_pd_hal.o
#obj-$(CONFIG_CHARGER_RT9465)	+= rt9465.o
#obj-$(CONFIG_BATTERY_MT6359P) += mt6359p_battery.o
#obj-$(CONFIG_DEVICE_MODULES_CHARGER_MT6360) += mt6360_charger.o
#obj-$(CONFIG_DEVICE_MODULES_CHARGER_MT6370)	+= mt6370-charger.o
#obj-$(CONFIG_DEVICE_MODULES_CHARGER_MT6360)	+= mt6360_pmu_chg.o
#obj-$(CONFIG_CHARGER_MT6362)	+= mt6362-chg.o
#obj-$(CONFIG_CHARGER_MT6375)	+= mt6375-charger.o
#obj-$(CONFIG_CHARGER_MT6379)	+= mt6379-chg.o
#mt6379-chg-objs := mt6379-charger.o mt6379-chgdev.o

ifneq ($(wildcard $(src)/k6985v1_64_alpha),)
obj-$(CONFIG_GAUGE_BQ28Z610)      += k6985v1_64_alpha/
endif

obj-$(CONFIG_CHARGER_GAUGE_MAX17332) += max1720x_battery.o
# HS07 code for HS07-10 by xiongxiaoliang at 20250221 start
obj-$(CONFIG_CHARGER_UPM6922P) += upm6922p_charger.o
# HS07 code for HS07-10 by xiongxiaoliang at 20250221 end
# Tab A9 code for SR-AX6739A-01-515 by lina at 20230505 start
obj-$(CONFIG_BATTERY_ID_ADC) += battery_id_adc.o
# Tab A9 code for SR-AX6739A-01-515 by lina at 20230505 end
#obj-$(CONFIG_BATTERY_MM8013) += mm8013.o

obj-$(CONFIG_CHARGER_TYPE_MT6357) += mt6357-charger-type.o
#obj-$(CONFIG_MTK_CHARGER_TYPE_DETECT)	+= mtk_chg_type_det.o
#obj-$(CONFIG_MTK_HVBP_SUPPORT) += mtk_hvbpc.o
#mtk_hvbpc-objs := mtk_hvbp.o mtk_hvbp_hal.o
#obj-$(CONFIG_MTK_PUMP_EXPRESS_PLUS_20_SUPPORT) += mtk_pep20.o
#mtk_pep20-objs := mtk_pe2.o mtk_pe2_hal.o
#obj-$(CONFIG_MTK_PUMP_EXPRESS_PLUS_SUPPORT) += mtk_pep.o
#mtk_pep-objs := mtk_pe.o mtk_pe_hal.o
#obj-$(CONFIG_MTK_PUMP_EXPRESS_40_SUPPORT) += mtk_pep40.o
#mtk_pep40-objs := mtk_pe4.o mtk_pe4_hal.o
#obj-$(CONFIG_MTK_PUMP_EXPRESS_45_SUPPORT) += mtk_pep45.o
#mtk_pep45-objs := mtk_pe4_5.o mtk_pe4_5_hal.o
#obj-$(CONFIG_MTK_PUMP_EXPRESS_50_SUPPORT) += mtk_pep50.o
#mtk_pep50-objs := mtk_pe5.o mtk_pe5_hal.o
# Tab A9 code for SR-AX6739A-01-490 by qiaodan at 20230504 start
obj-$(CONFIG_GXY_PSY_SYSFS) += gxy_psy_sysfs.o
# Tab A9 code for SR-AX6739A-01-490 by qiaodan at 20230504 end
#obj-$(CONFIG_MTK_PUMP_EXPRESS_50_PLUS_SUPPORT) += mtk_pep50p.o
#mtk_pep50p-objs := mtk_pe5p.o mtk_pe5p_hal.o
obj-$(CONFIG_CURRENT_SELECTOR_MAX17332) += max1720x_battery.o
#obj-$(CONFIG_CURRENT_SELECTOR_MTK_2P_CHARGER) += mtk_2p_charger.o
#obj-$(CONFIG_CHARGER_RT9759)	+= rt9759.o
#obj-$(CONFIG_CHARGER_RT9490)	+= rt9490-charger.o
#obj-$(CONFIG_CHARGER_RT9758)	+= rt9758-charger.o
#obj-$(CONFIG_UFCS_CLASS)	+= ufcs/

#obj-$(CONFIG_CHARGER_BQ2589X)	+= bq2589x_charger.o
# Tab A9 code for SR-AX6739A-01-493 by wenyaqi at 20230502 start
obj-$(CONFIG_CHARGER_SC89601D)	+= sc89601d_charger.o
# Tab A9 code for SR-AX6739A-01-493 by wenyaqi at 20230502 end
# Tab A9 code for SR-AX6739A-01-494 by qiaodan at 20230502 start
obj-$(CONFIG_CHARGER_UPM6910X)	+= upm6910x_charger.o
# Tab A9 code for SR-AX6739A-01-494 by qiaodan at 20230502 end
# HS07 code for SR-AL7761A-01-164 by lina at 20250314 start
obj-$(CONFIG_SCV89601P_CHARGER) += scv89601p_charger.o
# HS07 code for SR-AL7761A-01-164 by lina at 20250314 end
# Tab A9 code for SR-AX6739A-01-490 by qiaodan at 20230504 start
obj-$(CONFIG_GXY_PSY_SYSFS) += gxy_psy_sysfs.o
# Tab A9 code for SR-AX6739A-01-490 by qiaodan at 20230504 end
# HS07 code for HS07-12 by yexuedong at 20250221 start
obj-$(CONFIG_CHARGER_CP_PPS) += charge_pump/
# HS07 code for HS07-12 by yexuedong at 20250221 end
# Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 start
obj-$(CONFIG_AFC_CHARGER) += gpio_afc.o
gpio_afc-objs := afc_charger.o afc_charger_hal.o
# Tab A9 code for SR-AX6739A-01-499 by wenyaqi at 20230515 end
# Tab A9 code for SR-AX6739A-01-461 by qiaodan at 20230607 start
obj-$(CONFIG_GXY_VBUS_CTRL) += gxy_vbus_ctrl.o
# Tab A9 code for SR-AX6739A-01-461 by qiaodan at 20230607 end
