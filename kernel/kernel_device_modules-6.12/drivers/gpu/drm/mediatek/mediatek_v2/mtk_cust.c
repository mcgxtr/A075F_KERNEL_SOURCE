// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc.
 */

#include "mtk_drm_drv.h"

int set_lcm(struct mtk_ddic_dsi_msg *cmd_msg)
{
	return set_lcm_wrapper(cmd_msg, 1);
}

int read_lcm(struct mtk_ddic_dsi_msg *cmd_msg)
{
	return read_lcm_wrapper(cmd_msg);
}
