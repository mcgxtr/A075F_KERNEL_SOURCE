// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/backlight.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <drm/drm_modes.h>
#include <linux/delay.h>
#include <drm/drm_connector.h>
#include <drm/drm_device.h>

#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_graph.h>
#include <linux/platform_device.h>

#define CONFIG_MTK_PANEL_EXT
#if defined(CONFIG_MTK_PANEL_EXT)
#include "../../mediatek/mediatek_v2/mtk_panel_ext.h"
#include "../../mediatek/mediatek_v2/mtk_drm_graphics_base.h"
#endif

/*Tab A11 code for AX7800W-11 by jiangyue at 20250707 start*/
#include <linux/pinctrl/consumer.h>
#include "../panel-bias-hst11.h"
/*Tab A11 code for AX7800W-11 by jiangyue at 20250707 end*/

/* enable this to check panel self -bist pattern */
/* #define PANEL_BIST_PATTERN */

#define NOR_MAX_BRIGHTNESS    189 // (18   / 25) * 255
#define HBM_MAX_BRIGHTNESS    224 // (21.5 / 25) * 255

#include <linux/pinctrl/pinctrl.h>
static struct pinctrl *pinctl_rst = NULL;
static struct pinctrl_state *pTprst_low = NULL;
static struct pinctrl_state *pTprst_high = NULL;
static int current_fps = 60;

#include <linux/string.h>

struct lcm {
    struct device *dev;
    struct drm_panel panel;
    struct backlight_device *backlight;
    struct gpio_desc *reset_gpio;
    struct gpio_desc *bias_pos;
    struct gpio_desc *bias_neg;
    bool prepared;
    bool enabled;

    unsigned int gate_ic;

    int error;
};

#define lcm_dcs_write_seq(ctx, seq...)                                         \
    ({                                                                     \
        const u8 d[] = { seq };                                        \
        BUILD_BUG_ON_MSG(ARRAY_SIZE(d) > 64,                           \
                 "DCS sequence too big for stack");            \
        lcm_dcs_write(ctx, d, ARRAY_SIZE(d));                          \
    })

#define lcm_dcs_write_seq_static(ctx, seq...)                                  \
    ({                                                                     \
        static const u8 d[] = { seq };                                 \
        lcm_dcs_write(ctx, d, ARRAY_SIZE(d));                          \
    })

static inline struct lcm *panel_to_lcm(struct drm_panel *panel)
{
    return container_of(panel, struct lcm, panel);
}

static void lcm_dcs_write(struct lcm *ctx, const void *data, size_t len)
{
    struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
    ssize_t ret = 0;
    char *addr = NULL;

    if (ctx->error < 0) {
        return;
    }

    addr = (char *)data;
    if ((int)*addr < 0xB0) {
        ret = mipi_dsi_dcs_write_buffer(dsi, data, len);
    } else {
        ret = mipi_dsi_generic_write(dsi, data, len);
    }
    if (ret < 0) {
        dev_info(ctx->dev, "error %zd writing seq: %ph\n", ret, data);
        ctx->error = ret;
    }
}

static void lcm_suspend_setting(struct lcm *ctx)
{
    lcm_dcs_write_seq_static(ctx, 0x28);
    mdelay(30);
    lcm_dcs_write_seq_static(ctx, 0x10);
    mdelay(120);
};

/* Tab A11_BOS code for AX7800W-11 by jiangyue at 20250910 start */
static void lcm_panel_init(struct lcm *ctx)
{
    lcm_dcs_write_seq_static(ctx, 0x30, 0x01);
    lcm_dcs_write_seq_static(ctx, 0x78, 0x49, 0x61, 0x02, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x02);
    lcm_dcs_write_seq_static(ctx, 0x31, 0x34);
    lcm_dcs_write_seq_static(ctx, 0x32, 0x0c);
    lcm_dcs_write_seq_static(ctx, 0x33, 0x32);
    lcm_dcs_write_seq_static(ctx, 0x3d, 0x42);
    lcm_dcs_write_seq_static(ctx, 0x3f, 0x61);
    lcm_dcs_write_seq_static(ctx, 0x3c, 0x06);
    lcm_dcs_write_seq_static(ctx, 0x42, 0xa2);
    lcm_dcs_write_seq_static(ctx, 0x40, 0x08);
    lcm_dcs_write_seq_static(ctx, 0x4b, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x43, 0x70);
    lcm_dcs_write_seq_static(ctx, 0x44, 0x21);
    lcm_dcs_write_seq_static(ctx, 0x46, 0x17);
    lcm_dcs_write_seq_static(ctx, 0x49, 0xe0);
    lcm_dcs_write_seq_static(ctx, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x5a, 0x23, 0x23, 0x23, 0x23, 0x24, 0x24, 0x33, 0x23, 0x23, 0x23, 0x33);
    lcm_dcs_write_seq_static(ctx, 0x5b, 0x09, 0x0b, 0x0d, 0x0f, 0x11, 0x13, 0x05, 0x03, 0x01, 0x23, 0x23);
    lcm_dcs_write_seq_static(ctx, 0x5c, 0x23, 0x23, 0x23, 0x23, 0x24, 0x24, 0x33, 0x23, 0x23, 0x23, 0x33);
    lcm_dcs_write_seq_static(ctx, 0x5d, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x04, 0x02, 0x00, 0x23, 0x23);
    lcm_dcs_write_seq_static(ctx, 0x5e, 0x23, 0x23, 0x24, 0x24, 0x23, 0x23, 0x33, 0x23, 0x23, 0x23, 0x33);
    lcm_dcs_write_seq_static(ctx, 0x5f, 0x0e, 0x0c, 0x0a, 0x08, 0x12, 0x10, 0x04, 0x00, 0x02, 0x23, 0x23);
    lcm_dcs_write_seq_static(ctx, 0x60, 0x23, 0x23, 0x24, 0x24, 0x23, 0x23, 0x33, 0x23, 0x23, 0x23, 0x33);
    lcm_dcs_write_seq_static(ctx, 0x61, 0x0f, 0x0d, 0x0b, 0x09, 0x13, 0x11, 0x05, 0x01, 0x03, 0x23, 0x23);
    lcm_dcs_write_seq_static(ctx, 0x6d, 0x2e);
    lcm_dcs_write_seq_static(ctx, 0x6e, 0x1f);
    lcm_dcs_write_seq_static(ctx, 0x4c, 0x02, 0x02);
    lcm_dcs_write_seq_static(ctx, 0x4e, 0xff, 0xff);
    lcm_dcs_write_seq_static(ctx, 0x50, 0x02, 0x02);
    lcm_dcs_write_seq_static(ctx, 0x66, 0xfd, 0xfd);
    lcm_dcs_write_seq_static(ctx, 0x67, 0xfd, 0xfd);
    lcm_dcs_write_seq_static(ctx, 0x6f, 0x40, 0x04, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x70, 0x40, 0x04, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x71, 0x00, 0x00, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x72, 0x00, 0x00, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x07);
    lcm_dcs_write_seq_static(ctx, 0x31, 0x40);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x08);
    lcm_dcs_write_seq_static(ctx, 0x33, 0x05);
    lcm_dcs_write_seq_static(ctx, 0x40, 0x40);
    lcm_dcs_write_seq_static(ctx, 0x41, 0x80);
    lcm_dcs_write_seq_static(ctx, 0x42, 0x1a);
    lcm_dcs_write_seq_static(ctx, 0x47, 0x0a);
    lcm_dcs_write_seq_static(ctx, 0x48, 0x0c);
    lcm_dcs_write_seq_static(ctx, 0x50, 0x1c);
    lcm_dcs_write_seq_static(ctx, 0x5a, 0x20);
    lcm_dcs_write_seq_static(ctx, 0x5b, 0x3c);
    lcm_dcs_write_seq_static(ctx, 0x5c, 0x53);
    lcm_dcs_write_seq_static(ctx, 0x62, 0x04);
    lcm_dcs_write_seq_static(ctx, 0x65, 0x5f);
    lcm_dcs_write_seq_static(ctx, 0x75, 0x01);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x0a);
    lcm_dcs_write_seq_static(ctx, 0x33, 0x30);
    lcm_dcs_write_seq_static(ctx, 0x40, 0x22);
    lcm_dcs_write_seq_static(ctx, 0x49, 0x03);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x0b);
    lcm_dcs_write_seq_static(ctx, 0x33, 0x00, 0x5e);
    lcm_dcs_write_seq_static(ctx, 0x3c, 0x00, 0x92);
    lcm_dcs_write_seq_static(ctx, 0x43, 0xb3);
    lcm_dcs_write_seq_static(ctx, 0x44, 0x33);
    lcm_dcs_write_seq_static(ctx, 0x3e, 0x00, 0x06, 0x0e, 0x13, 0x1b);
    lcm_dcs_write_seq_static(ctx, 0x3f, 0x2c, 0x45, 0x4c, 0x54, 0x52, 0x6e, 0x73, 0x7b, 0x8c, 0x8b, 0x94, 0x9f, 0xb0, 0xb1);
    lcm_dcs_write_seq_static(ctx, 0x40, 0x57, 0x5b, 0x64, 0x73, 0x00, 0x06, 0x0e, 0x13, 0x1b);
    lcm_dcs_write_seq_static(ctx, 0x41, 0x2c, 0x45, 0x4c, 0x54, 0x52, 0x6e, 0x73, 0x7b, 0x8c, 0x8b, 0x94, 0x9f, 0xb0, 0xb1);
    lcm_dcs_write_seq_static(ctx, 0x42, 0x57, 0x5b, 0x64, 0x73);
    lcm_dcs_write_seq_static(ctx, 0x45, 0x61);
    lcm_dcs_write_seq_static(ctx, 0x46, 0x2f);
    lcm_dcs_write_seq_static(ctx, 0x48, 0x3a);
    lcm_dcs_write_seq_static(ctx, 0x49, 0x15);
    lcm_dcs_write_seq_static(ctx, 0x4a, 0x2a);
    lcm_dcs_write_seq_static(ctx, 0x4e, 0x05);
    lcm_dcs_write_seq_static(ctx, 0x4f, 0xaa);
    lcm_dcs_write_seq_static(ctx, 0x50, 0x5a);
    lcm_dcs_write_seq_static(ctx, 0x5c, 0x2f);
    lcm_dcs_write_seq_static(ctx, 0x5d, 0x4d);
    lcm_dcs_write_seq_static(ctx, 0x5e, 0x15);
    lcm_dcs_write_seq_static(ctx, 0x5f, 0x22);
    lcm_dcs_write_seq_static(ctx, 0x68, 0x23);
    lcm_dcs_write_seq_static(ctx, 0x64, 0x07);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x0c);
    lcm_dcs_write_seq_static(ctx, 0x4a, 0xab);
    lcm_dcs_write_seq_static(ctx, 0x62, 0x41);
    lcm_dcs_write_seq_static(ctx, 0x6f, 0x26);
    lcm_dcs_write_seq_static(ctx, 0x71, 0x77);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x0d);
    lcm_dcs_write_seq_static(ctx, 0x4a, 0x80);
    lcm_dcs_write_seq_static(ctx, 0x4c, 0x74);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x05);
    lcm_dcs_write_seq_static(ctx, 0x31, 0x39);
    lcm_dcs_write_seq_static(ctx, 0x57, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x58, 0x2d);
    lcm_dcs_write_seq_static(ctx, 0x5a, 0xf0, 0xf0);
    lcm_dcs_write_seq_static(ctx, 0x63, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x51, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x53, 0x2c);
    lcm_dcs_write_seq_static(ctx, 0x11, 0x00);
    mdelay(90);
    lcm_dcs_write_seq_static(ctx, 0x29, 0x00);
    mdelay(30);
    lcm_dcs_write_seq_static(ctx, 0x35, 0x00);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x0c);
    lcm_dcs_write_seq_static(ctx, 0x4b, 0x93);
    lcm_dcs_write_seq_static(ctx, 0x30, 0x00);
    pr_info("%s-\n", __func__);
}
/* Tab A11_BOS code for AX7800W-11 by jiangyue at 20250910 end */

static int lcm_disable(struct drm_panel *panel)
{
    struct lcm *ctx = panel_to_lcm(panel);
    pr_info("%s\n", __func__);

    if (!ctx->enabled) {
        return 0;
    }

    if (ctx->backlight) {
        ctx->backlight->props.power = FB_BLANK_POWERDOWN;
        backlight_update_status(ctx->backlight);
    }

    ctx->enabled = false;

    return 0;
}
static int lcm_unprepare(struct drm_panel *panel)
{

    struct lcm *ctx = panel_to_lcm(panel);

    if (!ctx->prepared) {
        return 0;
    }

    pr_info("%s+\n", __func__);
    if (gs_smart_wakeup_flag_get()) {
        lcm_suspend_setting(ctx);
    } else {
        lcm_suspend_setting(ctx);

        if (pinctl_rst != NULL && pTprst_low != NULL) {
            pinctrl_select_state(pinctl_rst, pTprst_low);
        }

        ctx->bias_neg = devm_gpiod_get_index(ctx->dev, "bias", 1, GPIOD_OUT_HIGH);
        gpiod_set_value(ctx->bias_neg, 0);
        devm_gpiod_put(ctx->dev, ctx->bias_neg);

        mdelay(7);

        ctx->bias_pos = devm_gpiod_get_index(ctx->dev, "bias", 0, GPIOD_OUT_HIGH);
        gpiod_set_value(ctx->bias_pos, 0);
        devm_gpiod_put(ctx->dev, ctx->bias_pos);
    }

    ctx->error = 0;
    ctx->prepared = false;
    pr_info("%s-\n", __func__);
    return 0;
}

static int lcm_prepare(struct drm_panel *panel)
{
    struct lcm *ctx = panel_to_lcm(panel);
    int ret = 0;

    pr_info("%s+\n", __func__);
    if (ctx->prepared) {
        return 0;
    }

    mdelay(1);

    ctx->bias_pos = devm_gpiod_get_index(ctx->dev, "bias", 0, GPIOD_OUT_HIGH);
    gpiod_set_value(ctx->bias_pos, 1);
    panel_bias_i2c_write_bytes(0x0, 0x14);
    mdelay(5);
    devm_gpiod_put(ctx->dev, ctx->bias_pos);

    ctx->bias_neg = devm_gpiod_get_index(ctx->dev, "bias", 1, GPIOD_OUT_HIGH);
    gpiod_set_value(ctx->bias_neg, 1);
    panel_bias_i2c_write_bytes(0x1, 0x14);
    mdelay(5);
    devm_gpiod_put(ctx->dev, ctx->bias_neg);

    if (pinctl_rst != NULL && pTprst_low != NULL) {
        pinctrl_select_state(pinctl_rst, pTprst_low);
    }

    ctx->reset_gpio = devm_gpiod_get_index(ctx->dev, "reset", 0,GPIOD_OUT_HIGH);//lcd_rst
    gpiod_set_value(ctx->reset_gpio, 0);
    mdelay(5);
    gpiod_set_value(ctx->reset_gpio, 1);
    mdelay(5);
    devm_gpiod_put(ctx->dev, ctx->reset_gpio);

    lcm_panel_init(ctx);//init code

    if (pinctl_rst != NULL && pTprst_high != NULL) {
        pinctrl_select_state(pinctl_rst, pTprst_high);
    }

    ret = ctx->error;
    if (ret < 0) {
        lcm_unprepare(panel);
    }

    ctx->prepared = true;

    pr_info("%s-\n", __func__);
    return ret;
}

static int lcm_enable(struct drm_panel *panel)
{
    struct lcm *ctx = panel_to_lcm(panel);
    pr_info("%s+\n", __func__);

    if (ctx->enabled) {
        return 0;
    }

    if (ctx->backlight) {
        ctx->backlight->props.power = FB_BLANK_UNBLANK;
        backlight_update_status(ctx->backlight);
    }

    ctx->enabled = true;
    pr_info("%s-\n", __func__);
    return 0;
}

#define HAC (800)
#define HFP (184)
#define HSA (48)
#define HBP (184)
#define VAC (1340)
#define VFP (984)
#define VFP_90 (200)
#define VSA (8)
#define VBP (20)
#define PHYSICAL_WIDTH    (113040)
#define PHYSICAL_HEIGHT    (189342)
#define HTOTAL (HAC + HFP + HSA + HBP)
#define VTOTAL (VAC + VFP+  VSA + VBP)
#define CURRENT_FPS  60
#define HTOTAL_90                       (HAC + HFP + HSA + HBP)
#define VTOTAL_90                       (VAC + VFP_90+  VSA + VBP)
#define FPS_60  60
#define FPS_90  90

static const struct drm_display_mode default_mode = {
    .clock = HTOTAL * VTOTAL * FPS_60 / 1000,
    .hdisplay = HAC,
    .hsync_start = HAC + HFP,
    .hsync_end = HAC + HFP + HSA,
    .htotal = HAC + HFP + HSA + HBP,
    .vdisplay = VAC,
    .vsync_start = VAC + VFP,
    .vsync_end = VAC + VFP + VSA,
    .vtotal = VAC + VFP + VSA + VBP,
    .width_mm = PHYSICAL_WIDTH / 1000,
    .height_mm = PHYSICAL_HEIGHT / 1000,
};

static const struct drm_display_mode performance_mode_90hz = {
    .clock = HTOTAL_90 * VTOTAL_90 * FPS_90 / 1000,
    .hdisplay = HAC,
    .hsync_start = HAC + HFP,
    .hsync_end = HAC + HFP + HSA,
    .htotal = HAC + HFP + HSA + HBP,
    .vdisplay = VAC,
    .vsync_start = VAC + VFP_90,
    .vsync_end = VAC + VFP_90 + VSA,
    .vtotal = VAC + VFP_90 + VSA + VBP,
    .width_mm = PHYSICAL_WIDTH / 1000,
    .height_mm = PHYSICAL_HEIGHT / 1000,
};

#if defined(CONFIG_MTK_PANEL_EXT)
static struct mtk_panel_params ext_params = {
    .pll_clk = 551,
    //.vfp_low_power = VFP,
    .cust_esd_check = 1,
    .esd_check_enable = 1,
    .lcm_esd_check_table[0] = {
        .cmd = 0x0D,
        .count = 1,
        .para_list[0] = 0x00,
    },
    .lcm_esd_check_table[1] = {
        .cmd = 0x0A,
        .count = 1,
        .para_list[0] = 0x9C,
    },
    .lcm_esd_check_table[2] = {
        .cmd = 0x09,
        .count = 3,
        .para_list[0] = 0x80,
        .para_list[1] = 0x03,
        .para_list[2] = 0x06,
    },
    .data_rate = 1102,
    .physical_width_um = PHYSICAL_WIDTH,
    .physical_height_um = PHYSICAL_HEIGHT,
    .phy_timcon = {
        .clk_trail = 10,
    },
        .dyn_fps = {
        .switch_en = 1,
        .vact_timing_fps = 90,
    },
};

static struct mtk_panel_params ext_params_90hz = {
    .pll_clk = 551,
    //.vfp_low_power = VFP,
    .cust_esd_check = 1,
    .esd_check_enable = 1,
    .lcm_esd_check_table[0] = {
        .cmd = 0x0D,
        .count = 1,
        .para_list[0] = 0x00,
    },
    .lcm_esd_check_table[1] = {
        .cmd = 0x0A,
        .count = 1,
        .para_list[0] = 0x9C,
    },
    .lcm_esd_check_table[2] = {
        .cmd = 0x09,
        .count = 3,
        .para_list[0] = 0x80,
        .para_list[1] = 0x03,
        .para_list[2] = 0x06,
    },
    .data_rate = 1102,
    .physical_width_um = PHYSICAL_WIDTH,
    .physical_height_um = PHYSICAL_HEIGHT,
    .phy_timcon = {
        .clk_trail = 10,
    },
    .dyn_fps = {
        .switch_en = 1,
        .vact_timing_fps = 90,
    },
};

static int lcm_setbacklight_cmdq(void *dsi, dcs_write_gce cb, void *handle,
                 unsigned int level)
{

    char bl_tb0[] = {0x51, 0xff};
    char bl_tb1[] = {0x53, 0x24};

    if (level == 0) {
        cb(dsi, handle, bl_tb1, ARRAY_SIZE(bl_tb1));
        pr_err("close diming\n");
    }

    if (level == 1) {
        level = 2;
    }

    level = hbm_map_level(level, NOR_MAX_BRIGHTNESS, HBM_MAX_BRIGHTNESS);

    bl_tb0[1] = level & 0xFF;

    pr_info("%s: level = %d, bl_tb0[1] = 0x%x\n", __func__, level, bl_tb0[1]);

    if (!cb) {
        return -1;
        pr_err("cb error\n");
    }

    cb(dsi, handle, bl_tb0, ARRAY_SIZE(bl_tb0));
    return 0;
}

struct drm_display_mode *get_mode_by_id_vfp(struct drm_connector *connector,
    unsigned int mode)
{
    struct drm_display_mode *m = NULL;
    unsigned int i = 0;

    list_for_each_entry(m, &connector->modes, head) {
        if (i == mode) {
            return m;
        }
        i++;
    }
    return NULL;
}
static int mtk_panel_ext_param_set(struct drm_panel *panel,
            struct drm_connector *connector, unsigned int mode)
{
    struct mtk_panel_ext *ext = find_panel_ext(panel);
    int ret = 0;
    struct drm_display_mode *m = get_mode_by_id_vfp(connector, mode);

    if (m == NULL) {
        pr_err("%s: invalid display_mode\n", __func__);
        return -1;
    }

    if (drm_mode_vrefresh(m) == 60) {
        ext->params = &ext_params;
    } else if (drm_mode_vrefresh(m) == 90) {
        ext->params = &ext_params_90hz;
    } else {
        ret = 1;
    }

    if (!ret) {
        current_fps = drm_mode_vrefresh(m);
    }

    return ret;
}

static int panel_ext_reset(struct drm_panel *panel, int on)
{
    struct lcm *ctx = panel_to_lcm(panel);

    pr_info("%s+\n", __func__);

    ctx->reset_gpio = devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
    if (IS_ERR(ctx->reset_gpio)) {
        dev_info(ctx->dev, "cannot get lcd reset-gpios %ld\n", PTR_ERR(ctx->reset_gpio));
        return PTR_ERR(ctx->reset_gpio);
    }
    gpiod_set_value(ctx->reset_gpio, on);
    devm_gpiod_put(ctx->dev, ctx->reset_gpio);

    pr_info("%s-\n", __func__);

    return 0;
}

static struct mtk_panel_funcs ext_funcs = {
    .reset = panel_ext_reset,
    .set_backlight_cmdq = lcm_setbacklight_cmdq,
    .ext_param_set = mtk_panel_ext_param_set,
};
#endif

static int lcm_get_modes(struct drm_panel *panel,
                    struct drm_connector *connector)
{
    struct drm_display_mode *mode = NULL;
    struct drm_display_mode *mode2 = NULL;

    mode = drm_mode_duplicate(connector->dev, &default_mode);
    if (!mode) {
        dev_info(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
             default_mode.hdisplay, default_mode.vdisplay,
             drm_mode_vrefresh(&default_mode));
        return -ENOMEM;
    }

    drm_mode_set_name(mode);
    mode->type = DRM_MODE_TYPE_DRIVER;
    drm_mode_probed_add(connector, mode);

    mode2 = drm_mode_duplicate(connector->dev, &performance_mode_90hz);
    if (!mode2) {
        dev_info(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
             performance_mode_90hz.hdisplay, performance_mode_90hz.vdisplay,
             drm_mode_vrefresh(&performance_mode_90hz));
        return -ENOMEM;
    }
    drm_mode_set_name(mode2);
    mode2->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
    drm_mode_probed_add(connector, mode2);

    connector->display_info.width_mm = 10;
    connector->display_info.height_mm = 17;

    return 2;
}

static const struct drm_panel_funcs lcm_drm_funcs = {
    .disable = lcm_disable,
    .unprepare = lcm_unprepare,
    .prepare = lcm_prepare,
    .enable = lcm_enable,
    .get_modes = lcm_get_modes,
};

static int lcm_probe(struct mipi_dsi_device *dsi)
{
    struct device *dev = &dsi->dev;
    struct device_node *dsi_node, *remote_node = NULL, *endpoint = NULL;
    struct lcm *ctx = NULL;
    struct device_node *backlight = NULL;
    int ret = 0;

    pr_info("%s+ lcm,jd9366tc_ld_hkc\n", __func__);

    if (lcm_detect_panel("jd9366tc_ld_hkc")) {
        pr_info("%s- lcm,jd9366tc_ld_hkc\n", __func__);
        return -ENODEV;
    }

    dsi_node = of_get_parent(dev->of_node);
    if (dsi_node) {
        endpoint = of_graph_get_next_endpoint(dsi_node, NULL);
        if (endpoint) {
            remote_node = of_graph_get_remote_port_parent(endpoint);
            if (!remote_node) {
                pr_err("No panel connected,skip probe lcm\n");
                return -ENODEV;
            }
            pr_err("device node name:%s\n", remote_node->name);
        }
    }
    if (remote_node != dev->of_node) {
        pr_err("%s+ skip probe due to not current lcm\n", __func__);
        return -ENODEV;
    }

    ctx = devm_kzalloc(dev, sizeof(struct lcm), GFP_KERNEL);
    if (!ctx) {
        return -ENOMEM;
    }

    mipi_dsi_set_drvdata(dsi, ctx);

    ctx->dev = dev;
    dsi->lanes = 4;
    dsi->format = MIPI_DSI_FMT_RGB888;
    dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
            MIPI_DSI_MODE_LPM | MIPI_DSI_MODE_NO_EOT_PACKET |
            MIPI_DSI_CLOCK_NON_CONTINUOUS;

    backlight = of_parse_phandle(dev->of_node, "backlight", 0);
    if (backlight) {
        ctx->backlight = of_find_backlight_by_node(backlight);
        of_node_put(backlight);

        if (!ctx->backlight) {
            return -EPROBE_DEFER;
        }
    }

    pr_info("%s+ parse dtbo\n", __func__);

    ctx->reset_gpio = devm_gpiod_get_index(dev, "reset", 0,GPIOD_OUT_HIGH);
    if (IS_ERR(ctx->reset_gpio)) {
        dev_info(dev, "cannot get lcd reset-gpios %ld\n", PTR_ERR(ctx->reset_gpio));
        return PTR_ERR(ctx->reset_gpio);
    }
    devm_gpiod_put(dev, ctx->reset_gpio);

    pinctl_rst = devm_pinctrl_get(ctx->dev);
    if (IS_ERR(pinctl_rst)) {
        pr_info("pinctl_tprst =%ld\n",PTR_ERR(pinctl_rst));
        pinctl_rst = NULL;
    } else {
        pTprst_low = pinctrl_lookup_state(pinctl_rst, "ctp_rst_pin_low");
        if (IS_ERR(pTprst_low)) {
            pr_err("lookup state pTprst_low failed\n");
            pTprst_low = NULL;
        }
        pTprst_high = pinctrl_lookup_state(pinctl_rst, "ctp_rst_pin_high");
        if (IS_ERR(pTprst_high)) {
            pr_err("lookup state pTprst_high failed\n");
            pTprst_high = NULL;
        }
    }

    ctx->bias_pos = devm_gpiod_get_index(dev, "bias", 0, GPIOD_OUT_HIGH);
    if (IS_ERR(ctx->bias_pos)) {
        dev_info(dev, "cannot get bias-gpios 0 %ld\n",
                PTR_ERR(ctx->bias_pos));
        return PTR_ERR(ctx->bias_pos);
    }
    devm_gpiod_put(dev, ctx->bias_pos);

    ctx->bias_neg = devm_gpiod_get_index(dev, "bias", 1, GPIOD_OUT_HIGH);
    if (IS_ERR(ctx->bias_neg)) {
        dev_info(dev, "cannot get bias-gpios 1 %ld\n",
                PTR_ERR(ctx->bias_neg));
        return PTR_ERR(ctx->bias_neg);
    }
    devm_gpiod_put(dev, ctx->bias_neg);


    ctx->prepared = true;
    ctx->enabled = true;
    drm_panel_init(&ctx->panel, dev, &lcm_drm_funcs, DRM_MODE_CONNECTOR_DSI);

    drm_panel_add(&ctx->panel);

    ret = mipi_dsi_attach(dsi);
    if (ret < 0) {
        drm_panel_remove(&ctx->panel);
    }

#if defined(CONFIG_MTK_PANEL_EXT)
    mtk_panel_tch_handle_reg(&ctx->panel);
    ret = mtk_panel_ext_create(dev, &ext_params_90hz, &ext_funcs, &ctx->panel);
    if (ret < 0) {
        return ret;
    }

#endif

    pr_info("%s- lcm,jd9366tc_ld_hkc\n", __func__);

    return ret;
}

static void lcm_remove(struct mipi_dsi_device *dsi)
{
    struct lcm *ctx = mipi_dsi_get_drvdata(dsi);
#if defined(CONFIG_MTK_PANEL_EXT)
    struct mtk_panel_ctx *ext_ctx = find_panel_ctx(&ctx->panel);
#endif

    mipi_dsi_detach(dsi);
    drm_panel_remove(&ctx->panel);
#if defined(CONFIG_MTK_PANEL_EXT)
    mtk_panel_detach(ext_ctx);
    mtk_panel_remove(ext_ctx);
#endif

}

static const struct of_device_id lcm_of_match[] = {
    {
        .compatible = "panel,lcm,video",
    },
    {}
};

MODULE_DEVICE_TABLE(of, lcm_of_match);

static struct mipi_dsi_driver lcm_driver = {
    .probe = lcm_probe,
    .remove = lcm_remove,
    .driver = {
        .name = "lcd_jd9366tc_ld_hkc_video",
        .owner = THIS_MODULE,
        .of_match_table = lcm_of_match,
    },
};

module_mipi_dsi_driver(lcm_driver);

MODULE_AUTHOR("Elon Hsu <elon.hsu@mediatek.com>");
MODULE_DESCRIPTION("lcm jd9366tc ld hkc Panel Driver");
MODULE_LICENSE("GPL v2");
