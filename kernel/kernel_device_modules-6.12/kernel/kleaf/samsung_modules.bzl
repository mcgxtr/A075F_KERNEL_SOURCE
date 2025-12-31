load("//kernel_device_modules-6.12:kernel/kleaf/samsung_product_modules.bzl",
     "product_device_modules_srcs",
     "product_device_modules_kconfig",
     "product_device_modules",
     "product_gki_modules",
)

load("@mgk_info//:kernel_version.bzl",
     "kernel_version",
)

_device_modules_srcs = [
    # keep sorted
    # "//kernel_device_modules-{}/drivers/samsung:ddk_srcs".format(kernel_version),
    # "//kernel_device_modules-{}/drivers/samsung/debug:ddk_srcs".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/ss_function:ddk_srcs".format(kernel_version),
]

samsung_device_modules_srcs = _device_modules_srcs + product_device_modules_srcs

# kconfigs in android/kernel/kernel_device_modules-6.x
_device_modules_kconfig = [
    "//kernel_device_modules-{}/drivers/block/zram:ddk_kconfigs".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung:ddk_kconfigs".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung/debug:ddk_kconfigs".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/ss_function:ddk_kconfigs".format(kernel_version),
    "//kernel_device_modules-{}/block:ddk_kconfigs".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/notify:ddk_kconfigs".format(kernel_version),
    "//kernel_device_modules-{}/mm/sec_mm:ddk_kconfigs".format(kernel_version),
	"//kernel_device_modules-{}/drivers/input/input_boost:ddk_kconfigs".format(kernel_version),
]

samsung_device_modules_kconfigs = _device_modules_kconfig + product_device_modules_kconfig

# modules in android/kernel/kernel_device_modules-6.x
_device_modules = [
    "//kernel_device_modules-{}/drivers/block/zram:zram".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung/debug:sec_debug".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung/debug:sec_extra_info".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung:sec_reboot".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung:sec_reset".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung:sec_ext".format(kernel_version),
    "//kernel_device_modules-{}/drivers/samsung:sec_bootstat".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/ss_function:usb_f_conn_gadget".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/ss_function:usb_f_dm".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/ss_function:usb_f_ss_mon_gadget".format(kernel_version),
    "//kernel_device_modules-{}/block:blk-sec-common".format(kernel_version),
    "//kernel_device_modules-{}/block:blk-sec-stats".format(kernel_version),
    "//kernel_device_modules-{}/block:ssg".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/notify:usb_notify_layer".format(kernel_version),
    "//kernel_device_modules-{}/drivers/usb/notify:usb_notifier".format(kernel_version),
    "//kernel_device_modules-{}/mm/sec_mm:sec_mm".format(kernel_version),
	"//kernel_device_modules-{}/drivers/input/input_boost:input_booster_lkm".format(kernel_version),
    "//kernel_device_modules-{}/drivers/ufs:ufs_sec".format(kernel_version),
]

samsung_device_modules = _device_modules + product_device_modules

# gki modules in android/kernel-6.x
_gki_modules = [
    "crypto/twofish_common.ko",
    "crypto/twofish_generic.ko",
    "drivers/watchdog/softdog.ko",
]

samsung_gki_modules = _gki_modules + product_gki_modules
