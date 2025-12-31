load("@mgk_info//:kernel_version.bzl",
     "kernel_version",
)

product_device_modules_srcs = [
    # keep sorted
    # "//kernel_device_modules-{}/drivers/samsung:ddk_srcs".format(kernel_version),
    # "//kernel_device_modules-{}/drivers/samsung/debug:ddk_srcs".format(kernel_version),
]

product_device_modules_kconfig = [
    # "//kernel_device_modules-{}/drivers/samsung:ddk_kconfigs".format(kernel_version),
    # "//kernel_device_modules-{}/drivers/samsung/debug:ddk_kconfigs".format(kernel_version),
]

product_device_modules = [
    # "//kernel_device_modules-{}/drivers/samsung/debug:sec_debug".format(kernel_version),
    # "//kernel_device_modules-{}/drivers/samsung/sec_reboot".format(kernel_version),
]

product_gki_modules = [
    # "drivers/watchdog/softdog.ko",
]
