subdir-ccflags-y += -Wall -Werror
subdir-ccflags-y += -I$(srctree)/drivers/power/supply/
subdir-ccflags-y += -I$(srctree)/drivers/misc/mediatek/rt-regmap/
subdir-ccflags-y += -I$(srctree)/drivers/mfd/

# Tab A9 code for SR-AX6739A-01-497 by wenyaqi at 20230506 start
obj-$(CONFIG_TCPC_AW35615)		+= tcpc_aw35615/
# Tab A9 code for SR-AX6739A-01-497 by wenyaqi at 20230506 end
#Tab A9 code for SR-AX6739A-01-498 by lina at 20230505 start
obj-$(CONFIG_TCPC_CPS8851)		+= tcpc_cps8851.o
#Tab A9 code for SR-AX6739A-01-498 by lina at 20230505 end
#HS07 code for HS07-10 by xiongxiaoliang at 20250221 start
obj-$(CONFIG_TCPC_UPM6922P)		+= tcpc_upm6922p.o
#HS07 code for HS07-10 by xiongxiaoliang at 20250221 end
# HS07 code for SR-AL7761A-01-164 by lina at 20250314 start
obj-$(CONFIG_TCPC_SCV89601P_PD)		+= tcpc_scv89601p_pd.o
# HS07 code for SR-AL7761A-01-164 by lina at 20250314 end
