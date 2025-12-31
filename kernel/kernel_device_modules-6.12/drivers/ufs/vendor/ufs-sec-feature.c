// SPDX-License-Identifier: GPL-2.0
/*
 * Samsung Specific feature
 *
 * Copyright (C) 2025 Samsung Electronics Co., Ltd.
 *
 * Authors:
 * Storage Driver <storage.sec@samsung.com>
 */

#include "ufs-sec-feature.h"
#include "ufs-sec-sysfs.h"

#include <trace/hooks/ufshcd.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_proto.h>

struct ufs_sec_feature_info ufs_sec_features;

#undef pr_fmt
#define pr_fmt(fmt) "ufs_sec: " fmt

#define ufs_sec_pr(level, fmt, ...) \
	pr_##level(fmt, ##__VA_ARGS__)

static void ufs_sec_set_unique_number(struct ufs_hba *hba, u8 *desc_buf)
{
	char *unique_number = get_vdi_member(unique_number);
	u8 manid = desc_buf[DEVICE_DESC_PARAM_MANF_ID + 1];
	u8 serial_num_index = desc_buf[DEVICE_DESC_PARAM_SN];
	u8 snum_buf[SERIAL_NUM_SIZE];
	u8 *str_desc_buf = NULL;
	int err;

	str_desc_buf = kzalloc(QUERY_DESC_MAX_SIZE, GFP_KERNEL);
	if (!str_desc_buf)
		return;

	err = ufshcd_read_desc_param(hba,
			QUERY_DESC_IDN_STRING, serial_num_index, 0,
			str_desc_buf, QUERY_DESC_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: Failed reading string descriptor. err %d",
				__func__, err);
		goto out;
	}

	memset(snum_buf, 0, sizeof(snum_buf));
	memcpy(snum_buf, str_desc_buf + QUERY_DESC_HDR_SIZE, SERIAL_NUM_SIZE);
	memset(unique_number, 0, UFS_UN_MAX_DIGITS);

	snprintf(unique_number, UFS_UN_MAX_DIGITS, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			manid,
			desc_buf[DEVICE_DESC_PARAM_MANF_DATE],
			desc_buf[DEVICE_DESC_PARAM_MANF_DATE + 1],
			snum_buf[0], snum_buf[1], snum_buf[2], snum_buf[3],
			snum_buf[4], snum_buf[5], snum_buf[6]);

	dev_dbg(hba->dev, "%s: ufs un: %s\n", __func__, unique_number);
out:
	kfree(str_desc_buf);
}

void ufs_sec_get_health_desc(struct ufs_hba *hba)
{
	u8 *desc_buf = NULL;
	int err;

	desc_buf = kzalloc(QUERY_DESC_MAX_SIZE, GFP_KERNEL);
	if (!desc_buf)
		return;

	err = ufshcd_read_desc_param(hba,
			QUERY_DESC_IDN_HEALTH, 0, 0,
			desc_buf, QUERY_DESC_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: Failed reading health descriptor. err %d",
				__func__, err);
		goto out;
	}

	set_vdi_member(lt, desc_buf[HEALTH_DESC_PARAM_LIFE_TIME_EST_A]);

	dev_info(hba->dev, "LT: 0x%02x\n",
			((desc_buf[HEALTH_DESC_PARAM_LIFE_TIME_EST_A] << 4) |
			 desc_buf[HEALTH_DESC_PARAM_LIFE_TIME_EST_B]));
out:
	kfree(desc_buf);
}

/* SEC error info : begin */
inline bool ufs_sec_is_err_cnt_allowed(void)
{
	return ufs_sec_features.ufs_err && ufs_sec_features.ufs_err_backup;
}

void ufs_sec_inc_hwrst_cnt(void)
{
	if (!ufs_sec_is_err_cnt_allowed())
		return;

	SEC_UFS_OP_ERR_CNT_INC(HW_RESET_cnt, UINT_MAX);
}
EXPORT_SYMBOL_GPL(ufs_sec_inc_hwrst_cnt);

static void ufs_sec_inc_link_startup_error_cnt(void)
{
	SEC_UFS_OP_ERR_CNT_INC(link_startup_cnt, UINT_MAX);
}

static void ufs_sec_inc_ah8_error_cnt(void)
{
	SEC_UFS_OP_ERR_CNT_INC(AH8_err_cnt, UINT_MAX);
}

static void ufs_sec_inc_uic_cmd_error(u32 cmd)
{
	struct SEC_UFS_UIC_cmd_cnt *uiccmd_cnt = NULL;

	if (!ufs_sec_is_err_cnt_allowed())
		return;

	uiccmd_cnt = &get_err_member(UIC_cmd_cnt);

	switch (cmd & COMMAND_OPCODE_MASK) {
	case UIC_CMD_DME_GET:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_GET_err, U8_MAX);
		break;
	case UIC_CMD_DME_SET:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_SET_err, U8_MAX);
		break;
	case UIC_CMD_DME_PEER_GET:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_PEER_GET_err, U8_MAX);
		break;
	case UIC_CMD_DME_PEER_SET:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_PEER_SET_err, U8_MAX);
		break;
	case UIC_CMD_DME_POWERON:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_POWERON_err, U8_MAX);
		break;
	case UIC_CMD_DME_POWEROFF:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_POWEROFF_err, U8_MAX);
		break;
	case UIC_CMD_DME_ENABLE:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_ENABLE_err, U8_MAX);
		break;
	case UIC_CMD_DME_RESET:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_RESET_err, U8_MAX);
		break;
	case UIC_CMD_DME_END_PT_RST:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_END_PT_RST_err, U8_MAX);
		break;
	case UIC_CMD_DME_LINK_STARTUP:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_LINK_STARTUP_err, U8_MAX);
		break;
	case UIC_CMD_DME_HIBER_ENTER:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_HIBER_ENTER_err, U8_MAX);
		SEC_UFS_OP_ERR_CNT_INC(Hibern8_enter_cnt, UINT_MAX);
		break;
	case UIC_CMD_DME_HIBER_EXIT:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_HIBER_EXIT_err, U8_MAX);
		SEC_UFS_OP_ERR_CNT_INC(Hibern8_exit_cnt, UINT_MAX);
		break;
	case UIC_CMD_DME_TEST_MODE:
		SEC_UFS_ERR_CNT_INC(uiccmd_cnt->DME_TEST_MODE_err, U8_MAX);
		break;
	default:
		break;
	}

	SEC_UFS_ERR_CNT_INC(uiccmd_cnt->UIC_cmd_err, UINT_MAX);
}

static void ufs_sec_inc_uic_fatal(u32 errors)
{
	struct SEC_UFS_Fatal_err_cnt *f_ec = &get_err_member(Fatal_err_cnt);

	if (errors & DEVICE_FATAL_ERROR) {
		SEC_UFS_ERR_CNT_INC(f_ec->DFE, U8_MAX);
		SEC_UFS_ERR_CNT_INC(f_ec->Fatal_err, UINT_MAX);
	}
	if (errors & CONTROLLER_FATAL_ERROR) {
		SEC_UFS_ERR_CNT_INC(f_ec->CFE, U8_MAX);
		SEC_UFS_ERR_CNT_INC(f_ec->Fatal_err, UINT_MAX);
	}
	if (errors & SYSTEM_BUS_FATAL_ERROR) {
		SEC_UFS_ERR_CNT_INC(f_ec->SBFE, U8_MAX);
		SEC_UFS_ERR_CNT_INC(f_ec->Fatal_err, UINT_MAX);
	}
	if (errors & CRYPTO_ENGINE_FATAL_ERROR) {
		SEC_UFS_ERR_CNT_INC(f_ec->CEFE, U8_MAX);
		SEC_UFS_ERR_CNT_INC(f_ec->Fatal_err, UINT_MAX);
	}
	if (errors & UIC_LINK_LOST) {
		SEC_UFS_ERR_CNT_INC(f_ec->LLE, U8_MAX);
		SEC_UFS_ERR_CNT_INC(f_ec->Fatal_err, UINT_MAX);
	}
}

static void ufs_sec_inc_uic_error(enum ufs_event_type evt, u32 reg)
{
	struct SEC_UFS_UIC_err_cnt *uicerr_cnt = &get_err_member(UIC_err_cnt);
	unsigned int bit_count = 0;
	int val = 0;

	switch (evt) {
	case UFS_EVT_PA_ERR:
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->PAERR_cnt, U8_MAX);
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->UIC_err, UINT_MAX);

		if (reg & UIC_PHY_ADAPTER_LAYER_GENERIC_ERROR)
			SEC_UFS_ERR_CNT_INC(uicerr_cnt->PAERR_linereset,
					UINT_MAX);

		val = reg & UIC_PHY_ADAPTER_LAYER_LANE_ERR_MASK;
		if (val)
			SEC_UFS_ERR_CNT_INC(uicerr_cnt->PAERR_lane[val - 1],
					UINT_MAX);
		break;
	case UFS_EVT_DL_ERR:
		if (reg & UIC_DATA_LINK_LAYER_ERROR_PA_INIT)
			SEC_UFS_ERR_CNT_INC(uicerr_cnt->DL_PA_INIT_ERR_cnt, U8_MAX);
		if (reg & UIC_DATA_LINK_LAYER_ERROR_NAC_RECEIVED)
			SEC_UFS_ERR_CNT_INC(uicerr_cnt->DL_NAC_RCVD_ERR_cnt, U8_MAX);
		if (reg & UIC_DATA_LINK_LAYER_ERROR_TCx_REPLAY_TIMEOUT)
			SEC_UFS_ERR_CNT_INC(uicerr_cnt->DL_TC_REPLAY_ERR_cnt, U8_MAX);
		if (reg & UIC_DATA_LINK_LAYER_ERROR_FCX_PRO_TIMER_EXP)
			SEC_UFS_ERR_CNT_INC(uicerr_cnt->DL_FC_PROTECT_ERR_cnt, U8_MAX);

		reg &= UIC_DATA_LINK_LAYER_ERROR_CODE_MASK;

		bit_count = __builtin_popcount(reg);

		SEC_UFS_ERR_CNT_ADD(uicerr_cnt->DLERR_cnt, bit_count, UINT_MAX);
		SEC_UFS_ERR_CNT_ADD(uicerr_cnt->UIC_err, bit_count, UINT_MAX);
		break;
	case UFS_EVT_NL_ERR:
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->NLERR_cnt, U8_MAX);
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->UIC_err, UINT_MAX);
		break;
	case UFS_EVT_TL_ERR:
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->TLERR_cnt, U8_MAX);
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->UIC_err, UINT_MAX);
		break;
	case UFS_EVT_DME_ERR:
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->DMEERR_cnt, U8_MAX);
		SEC_UFS_ERR_CNT_INC(uicerr_cnt->UIC_err, UINT_MAX);
		break;
	default:
		break;
	}
}

static void ufs_sec_inc_tm_error(u8 tm_cmd)
{
	struct SEC_UFS_UTP_cnt *utp_err = &get_err_member(UTP_cnt);

	if (!ufs_sec_is_err_cnt_allowed())
		return;

	switch (tm_cmd) {
	case UFS_QUERY_TASK:
		SEC_UFS_ERR_CNT_INC(utp_err->UTMR_query_task_cnt, U8_MAX);
		break;
	case UFS_ABORT_TASK:
		SEC_UFS_ERR_CNT_INC(utp_err->UTMR_abort_task_cnt, U8_MAX);
		break;
	case UFS_LOGICAL_RESET:
		SEC_UFS_ERR_CNT_INC(utp_err->UTMR_logical_reset_cnt, U8_MAX);
		break;
	default:
		break;
	}

	SEC_UFS_ERR_CNT_INC(utp_err->UTP_err, UINT_MAX);
}

static void ufs_sec_inc_utp_error(struct ufs_hba *hba, int tag)
{
	struct SEC_UFS_UTP_cnt *utp_err = &get_err_member(UTP_cnt);
	struct ufshcd_lrb *lrbp = NULL;
	int opcode = 0;

	if (!ufs_sec_is_err_cnt_allowed())
		return;

	if (tag >= hba->nutrs)
		return;

	lrbp = &hba->lrb[tag];
	if (!lrbp || !lrbp->cmd || (lrbp->task_tag != tag))
		return;

	opcode = lrbp->cmd->cmnd[0];

	switch (opcode) {
	case WRITE_10:
		SEC_UFS_ERR_CNT_INC(utp_err->UTR_write_err, U8_MAX);
		break;
	case READ_10:
	case READ_16:
		SEC_UFS_ERR_CNT_INC(utp_err->UTR_read_err, U8_MAX);
		break;
	case SYNCHRONIZE_CACHE:
		SEC_UFS_ERR_CNT_INC(utp_err->UTR_sync_cache_err, U8_MAX);
		break;
	case UNMAP:
		SEC_UFS_ERR_CNT_INC(utp_err->UTR_unmap_err, U8_MAX);
		break;
	default:
		SEC_UFS_ERR_CNT_INC(utp_err->UTR_etc_err, U8_MAX);
		break;
	}

	SEC_UFS_ERR_CNT_INC(utp_err->UTP_err, UINT_MAX);
}

static enum utp_ocs ufshcd_sec_get_tr_ocs(struct ufshcd_lrb *lrbp,
				      struct cq_entry *cqe)
{
	if (cqe)
		return le32_to_cpu(cqe->status) & MASK_OCS;

	return lrbp->utr_descriptor_ptr->header.ocs & MASK_OCS;
}

static void ufs_sec_inc_query_error(struct ufs_hba *hba,
		struct ufshcd_lrb *lrbp, bool timeout)
{
	struct SEC_UFS_QUERY_cnt *query_cnt = NULL;
	struct ufs_query_req *request = &hba->dev_cmd.query.request;
	struct ufs_hw_queue *hwq = NULL;
	struct cq_entry *cqe = NULL;
	enum query_opcode opcode = request->upiu_req.opcode;
	enum dev_cmd_type cmd_type = hba->dev_cmd.type;
	enum utp_ocs ocs;

	if (!ufs_sec_is_err_cnt_allowed())
		return;

	if (hba->mcq_enabled) {
		hwq = hba->dev_cmd_queue;
		cqe = ufshcd_mcq_cur_cqe(hwq);
	}

	ocs = ufshcd_sec_get_tr_ocs(lrbp, cqe);

	if (!timeout && (ocs == OCS_SUCCESS))
		return;

	/* get last query cmd information when timeout occurs */
	if (timeout) {
		opcode = ufs_sec_features.last_qcmd;
		cmd_type = ufs_sec_features.qcmd_type;
	}

	query_cnt = &get_err_member(Query_cnt);

	if (cmd_type == DEV_CMD_TYPE_NOP) {
		SEC_UFS_ERR_CNT_INC(query_cnt->NOP_err, U8_MAX);
	} else {
		switch (opcode) {
		case UPIU_QUERY_OPCODE_READ_DESC:
			SEC_UFS_ERR_CNT_INC(query_cnt->R_Desc_err, U8_MAX);
			break;
		case UPIU_QUERY_OPCODE_WRITE_DESC:
			SEC_UFS_ERR_CNT_INC(query_cnt->W_Desc_err, U8_MAX);
			break;
		case UPIU_QUERY_OPCODE_READ_ATTR:
			SEC_UFS_ERR_CNT_INC(query_cnt->R_Attr_err, U8_MAX);
			break;
		case UPIU_QUERY_OPCODE_WRITE_ATTR:
			SEC_UFS_ERR_CNT_INC(query_cnt->W_Attr_err, U8_MAX);
			break;
		case UPIU_QUERY_OPCODE_READ_FLAG:
			SEC_UFS_ERR_CNT_INC(query_cnt->R_Flag_err, U8_MAX);
			break;
		case UPIU_QUERY_OPCODE_SET_FLAG:
			SEC_UFS_ERR_CNT_INC(query_cnt->Set_Flag_err, U8_MAX);
			break;
		case UPIU_QUERY_OPCODE_CLEAR_FLAG:
			SEC_UFS_ERR_CNT_INC(query_cnt->Clear_Flag_err, U8_MAX);
			break;
		case UPIU_QUERY_OPCODE_TOGGLE_FLAG:
			SEC_UFS_ERR_CNT_INC(query_cnt->Toggle_Flag_err,
					U8_MAX);
			break;
		default:
			break;
		}
	}

	SEC_UFS_ERR_CNT_INC(query_cnt->Query_err, UINT_MAX);
}

void ufs_sec_inc_op_err(struct ufs_hba *hba, enum ufs_event_type evt,
		void *data)
{
	u32 error_val = *(u32 *)data;

	if (!ufs_sec_is_err_cnt_allowed())
		return;

	switch (evt) {
	case UFS_EVT_LINK_STARTUP_FAIL:
		ufs_sec_inc_link_startup_error_cnt();
		break;
	case UFS_EVT_DEV_RESET:
		break;
	case UFS_EVT_PA_ERR:
	case UFS_EVT_DL_ERR:
	case UFS_EVT_NL_ERR:
	case UFS_EVT_TL_ERR:
	case UFS_EVT_DME_ERR:
		if (error_val)
			ufs_sec_inc_uic_error(evt, error_val);
		break;
	case UFS_EVT_FATAL_ERR:
		if (error_val)
			ufs_sec_inc_uic_fatal(error_val);
		break;
	case UFS_EVT_ABORT:
		ufs_sec_inc_utp_error(hba, (int)error_val);
		break;
	case UFS_EVT_HOST_RESET:
		break;
	case UFS_EVT_SUSPEND_ERR:
	case UFS_EVT_RESUME_ERR:
		break;
	case UFS_EVT_AUTO_HIBERN8_ERR:
		ufs_sec_inc_ah8_error_cnt();
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL_GPL(ufs_sec_inc_op_err);

static bool ufs_sec_is_sense_logging_needed(struct ufshcd_lrb *lrbp,
		u8 sense_key)
{
	bool ret;
	struct utp_upiu_rsp *ucd_rsp_ptr;
	u32 result = 0;

	ucd_rsp_ptr = lrbp->ucd_rsp_ptr;
	result = be32_to_cpu(ucd_rsp_ptr->header.dword_1);
	if (!(result & SAM_STAT_CHECK_CONDITION))
		return false;

	switch (sense_key) {
	case NO_SENSE:
	case UNIT_ATTENTION:
	case COPY_ABORTED:
		ret = false;
		break;
	default:
		ret = true;
		break;
	}

	return ret;
}

static void ufs_sec_print_cmd_info(struct ufshcd_lrb *lrbp,
		struct ufs_sec_cmd_info *ufs_cmd, u8 sense_key)
{
	u8 asc = lrbp->ucd_rsp_ptr->sr.sense_data[12];
	u8 ascq = lrbp->ucd_rsp_ptr->sr.sense_data[13];

	ufs_sec_pr(err, "UFS: LU%u: sense key 0x%x(asc 0x%x, ascq 0x%x),"
			"opcode 0x%x, lba 0x%x, len 0x%x.\n",
			ufs_cmd->lun, sense_key, asc, ascq,
			ufs_cmd->opcode, ufs_cmd->lba, ufs_cmd->transfer_len);
}

static void ufs_sec_inc_sense_err(struct ufshcd_lrb *lrbp,
		struct ufs_sec_cmd_info *ufs_cmd)
{
	struct SEC_SCSI_SENSE_cnt *sense_err = NULL;
	u8 sense_key = 0;
	bool secdbgMode = false;

	sense_key = lrbp->ucd_rsp_ptr->sr.sense_data[2] & 0x0F;
	if (!ufs_sec_is_sense_logging_needed(lrbp, sense_key))
		return;

#if IS_ENABLED(CONFIG_SEC_DEBUG)
	secdbgMode = sec_debug_get_force_upload();
#endif

	ufs_sec_print_cmd_info(lrbp, ufs_cmd, sense_key);

	if (!ufs_sec_is_err_cnt_allowed())
		goto out;

	sense_err = &get_err_member(sense_cnt);

	switch (sense_key) {
	case MEDIUM_ERROR:
		sense_err->scsi_medium_err++;
		break;
	case HARDWARE_ERROR:
		sense_err->scsi_hw_err++;
		break;
	case ILLEGAL_REQUEST:
		sense_err->scsi_illegal_req++;
		break;
	case DATA_PROTECT:
		sense_err->scsi_data_prot++;
		break;
	default:
		sense_err->scsi_others++;
		break;
	}

out:
	if (secdbgMode) {
		if (sense_key == MEDIUM_ERROR)
			panic("ufs medium error\n");
		else if (sense_key == HARDWARE_ERROR)
			panic("ufs hardware error\n");
	}
}

void ufs_sec_print_err_info(struct ufs_hba *hba)
{
	dev_err(hba->dev, "Count: %u UIC: %u UTP: %u QUERY: %u\n",
		SEC_UFS_ERR_INFO_GET_VALUE(op_cnt, HW_RESET_cnt),
		SEC_UFS_ERR_INFO_GET_VALUE(UIC_err_cnt, UIC_err),
		SEC_UFS_ERR_INFO_GET_VALUE(UTP_cnt, UTP_err),
		SEC_UFS_ERR_INFO_GET_VALUE(Query_cnt, Query_err));

	dev_err(hba->dev, "Sense Key: medium: %u, hw: %u, illegal: %u, prot: %u, others: %u\n",
		SEC_UFS_ERR_INFO_GET_VALUE(sense_cnt, scsi_medium_err),
		SEC_UFS_ERR_INFO_GET_VALUE(sense_cnt, scsi_hw_err),
		SEC_UFS_ERR_INFO_GET_VALUE(sense_cnt, scsi_illegal_req),
		SEC_UFS_ERR_INFO_GET_VALUE(sense_cnt, scsi_data_prot),
		SEC_UFS_ERR_INFO_GET_VALUE(sense_cnt, scsi_others));
}
EXPORT_SYMBOL_GPL(ufs_sec_print_err_info);

static void ufs_sec_init_error_logging(struct device *dev)
{
	struct ufs_sec_err_info *ufs_err = NULL;
	struct ufs_sec_err_info *ufs_err_backup = NULL;

	ufs_err = devm_kzalloc(dev, sizeof(struct ufs_sec_err_info), GFP_KERNEL);
	ufs_err_backup = devm_kzalloc(dev, sizeof(struct ufs_sec_err_info), GFP_KERNEL);
	if (!ufs_err || !ufs_err_backup) {
		dev_err(dev, "%s: Failed allocating ufs_err(backup)(%lu)",
				__func__, sizeof(struct ufs_sec_err_info));
		devm_kfree(dev, ufs_err);
		devm_kfree(dev, ufs_err_backup);
		return;
	}

	ufs_sec_features.ufs_err = ufs_err;
	ufs_sec_features.ufs_err_backup = ufs_err_backup;

	ufs_sec_features.ucmd_complete = true;
	ufs_sec_features.qcmd_complete = true;
}
/* SEC error info : end */

void ufs_sec_set_features(struct ufs_hba *hba)
{
	struct ufs_vendor_dev_info *vdi = ufs_sec_features.vdi;
	u8 *desc_buf = NULL;
	int err;

	if (vdi && vdi->hba)
		return;

	vdi->hba = hba;

	desc_buf = kzalloc(QUERY_DESC_MAX_SIZE, GFP_KERNEL);
	if (!desc_buf)
		return;

	err = ufshcd_read_desc_param(hba,
			QUERY_DESC_IDN_DEVICE, 0, 0,
			desc_buf, QUERY_DESC_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: Failed reading device desc. err %d",
				__func__, err);
		goto out;
	}

	ufs_sec_set_unique_number(hba, desc_buf);
	ufs_sec_get_health_desc(hba);

	ufs_sec_add_sysfs_nodes(hba);

out:
	kfree(desc_buf);
}

EXPORT_SYMBOL_GPL(ufs_sec_set_features);

void ufs_sec_init_logging(struct device *dev)
{
	ufs_sec_init_error_logging(dev);
}
EXPORT_SYMBOL_GPL(ufs_sec_init_logging);

void ufs_sec_remove_features(struct ufs_hba *hba)
{
	ufs_sec_remove_sysfs_nodes(hba);
}
EXPORT_SYMBOL_GPL(ufs_sec_remove_features);

static bool ufs_sec_get_scsi_cmd_info(struct ufshcd_lrb *lrbp,
		struct ufs_sec_cmd_info *ufs_cmd)
{
	struct scsi_cmnd *cmd;

	if (!lrbp || !lrbp->cmd || !ufs_cmd)
		return false;

	cmd = lrbp->cmd;

	ufs_cmd->opcode = (u8)(*cmd->cmnd);
	ufs_cmd->lba = ((cmd->cmnd[2] << 24) | (cmd->cmnd[3] << 16) |
			(cmd->cmnd[4] << 8) | cmd->cmnd[5]);
	ufs_cmd->transfer_len = (cmd->cmnd[7] << 8) | cmd->cmnd[8];
	ufs_cmd->lun = ufshcd_scsi_to_upiu_lun(cmd->device->lun);

	return true;
}

/*
 * vendor hooks
 * check include/trace/hooks/ufshcd.h
 */
static void sec_android_vh_ufs_send_command(void *data,
		struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufs_sec_cmd_info ufs_cmd = { 0, };
	struct ufs_query_req *request = NULL;
	enum dev_cmd_type cmd_type;
	enum query_opcode opcode;
	bool is_scsi_cmd = false;

	is_scsi_cmd = ufs_sec_get_scsi_cmd_info(lrbp, &ufs_cmd);

	if (!is_scsi_cmd) {
		/* in timeout error case, last cmd is not completed */
		if (!ufs_sec_features.qcmd_complete)
			ufs_sec_inc_query_error(hba, lrbp, true);

		request = &hba->dev_cmd.query.request;
		opcode = request->upiu_req.opcode;
		cmd_type = hba->dev_cmd.type;

		ufs_sec_features.last_qcmd = opcode;
		ufs_sec_features.qcmd_type = cmd_type;
		ufs_sec_features.qcmd_complete = false;
	}
}

static void sec_android_vh_ufs_compl_command(void *data,
		struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufs_sec_cmd_info ufs_cmd = { 0, };
	bool is_scsi_cmd = false;

	is_scsi_cmd = ufs_sec_get_scsi_cmd_info(lrbp, &ufs_cmd);

	if (is_scsi_cmd) {
		ufs_sec_inc_sense_err(lrbp, &ufs_cmd);

		/*
		 * check hba->req_abort_count, if the cmd is aborting
		 * it's the one way to check aborting
		 * hba->req_abort_count is cleared in queuecommand and after
		 * error handling
		 */
		if (hba->req_abort_count > 0)
			ufs_sec_inc_utp_error(hba, lrbp->task_tag);
	} else {
		ufs_sec_features.qcmd_complete = true;

		/* check and count error, except timeout */
		ufs_sec_inc_query_error(hba, lrbp, false);
	}
}

static void sec_android_vh_ufs_send_uic_command(void *data,
		struct ufs_hba *hba, const struct uic_command *ucmd, int str_t)
{
	u32 cmd;

	if (str_t == UFS_CMD_SEND) {
		/* in timeout error case, last cmd is not completed */
		if (!ufs_sec_features.ucmd_complete)
			ufs_sec_inc_uic_cmd_error(ufs_sec_features.last_ucmd);

		cmd = ucmd->command;
		ufs_sec_features.last_ucmd = cmd;
		ufs_sec_features.ucmd_complete = false;
	} else {
		cmd = ufshcd_readl(hba, REG_UIC_COMMAND);

		ufs_sec_features.ucmd_complete = true;

		if (((hba->active_uic_cmd->argument2 & MASK_UIC_COMMAND_RESULT)
					!= UIC_CMD_RESULT_SUCCESS))
			ufs_sec_inc_uic_cmd_error(cmd);
	}
}

static void sec_android_vh_ufs_send_tm_command(void *data,
		struct ufs_hba *hba, int tag, int str_t)
{
	struct utp_task_req_desc treq = { { 0 }, };
	u8 tm_func = 0;

	memcpy(&treq, hba->utmrdl_base_addr + tag, sizeof(treq));

	tm_func = (be32_to_cpu(treq.upiu_req.req_header.dword_1) >> 16) & 0xFF;

	if (str_t == UFS_TM_ERR)
		ufs_sec_inc_tm_error(tm_func);
}

void ufs_sec_register_vendor_hooks(void)
{
	register_trace_android_vh_ufs_send_command(sec_android_vh_ufs_send_command, NULL);
	register_trace_android_vh_ufs_compl_command(sec_android_vh_ufs_compl_command, NULL);
	register_trace_android_vh_ufs_send_uic_command(sec_android_vh_ufs_send_uic_command, NULL);
	register_trace_android_vh_ufs_send_tm_command(sec_android_vh_ufs_send_tm_command, NULL);
}
EXPORT_SYMBOL_GPL(ufs_sec_register_vendor_hooks);

static const struct of_device_id ufs_sec_driver_match[] = {
	{ .compatible = "samsung,ufs-sec-driver" },
	{},
};
MODULE_DEVICE_TABLE(of, ufs_sec_driver_match);

static int ufs_sec_driver_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ufs_vendor_dev_info *vdi = NULL;

	ufs_sec_features.dev = dev;

	vdi = devm_kzalloc(dev, sizeof(struct ufs_vendor_dev_info),
			GFP_KERNEL);
	if (!vdi) {
		ufs_sec_pr(err, "%s: Failed allocating ufs_vdi(%lu)",
				__func__, sizeof(struct ufs_vendor_dev_info));
		return -ENOMEM;
	}
	ufs_sec_features.vdi = vdi;

	return 0;
}

static void ufs_sec_driver_remove(struct platform_device *pdev)
{
	return;
}

static struct platform_driver ufs_sec_platform_driver = {
	.driver = {
		.name = "ufs-sec-driver",
		.owner = THIS_MODULE,
		.of_match_table = ufs_sec_driver_match,
	},
	.probe = ufs_sec_driver_probe,
	.remove = ufs_sec_driver_remove,
};

module_platform_driver(ufs_sec_platform_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Storage Driver <storage.sec@samsung.com>");
MODULE_DESCRIPTION("Samsung specific UFS feature");
MODULE_VERSION("1.0");
