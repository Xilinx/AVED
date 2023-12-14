// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_pci_dbg.c - This file contains PCI debug/printing code.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "ami_pci_dbg.h"


/*
 * NOTE: Most of the functions in this file are simply debug/printing functions.
 * For the majority of them, they accept a `dev` argument which is a pointer to
 * a PCI device struct, and an additional argument(s) which is used to determine
 * the data to print out. This is usually a numeric value from which relevant
 * fields can be extracted.
 */

/**
 * pcie_vendor_id_to_str() - Convert a PCI vendor ID to a human readable string.
 * @id: Vendor ID
 * 
 * Return: The vendor name or "UNKOWN".
 */
char *pcie_vendor_id_to_str(uint16_t id)
{
	char *ret = "UNKNOWN";
	if (id == PCIE_VENDOR_ID_XILINX)
		ret = "XILINX";
	return ret;
}

void print_pcie_vendor_id(struct pci_dev *dev, uint16_t id)
{
	if (dev)
		DEV_VDBG(dev, "PCI_VENDOR_ID : %s", pcie_vendor_id_to_str(id));
}

/**
 * pcie_device_id_to_str() - Convert a PCI device ID to a human readable string.
 * @id: device ID
 * 
 * This function should only be used as part of the driver initialization
 * procedure - to get a human readable name for display to the user,
 * the board info data should be used instead. This function may also be
 * used as a fallback when no board info data is found for a device.
 *
 * Return: The device name or "Unknown".
 */
char *pcie_device_id_to_str(uint16_t id)
{
	switch (id) {
	case AMI_PCIE_DEVICE_ID_V70_PF0:
	case AMI_PCIE_DEVICE_ID_V70_PF1:
		return "ALVEO V70";
	
	case AMI_PCIE_DEVICE_ID_VCK5000_PF0:
	case AMI_PCIE_DEVICE_ID_VCK5000_PF1:
		return "VCK5000";
	
	case AMI_PCIE_DEVICE_ID_V80:
		return "ALVEO V80";
	
	case AMI_PCIE_DEVICE_ID_V80P:
		return "ALVEO V80P";
	
	default:
		break;
	}

	return "Unknown";
}

void print_pcie_device_id(struct pci_dev *dev, uint16_t id)
{
	if (dev)
		DEV_VDBG(dev, "PCI_DEVICE_ID : %s", pcie_device_id_to_str(id));
}

void print_pcie_command(struct pci_dev *dev, uint16_t command)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- PCI_COMMAND_IO             : 0x%X",
		get_pcie_command_io(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_MEMORY         : 0x%X",
		get_pcie_command_memory(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_MASTER         : 0x%X",
		get_pcie_command_master(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_SPECIAL        : 0x%X",
		get_pcie_command_special(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_INVALIDATE     : 0x%X",
		get_pcie_command_invalidate(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_VGA_PALETTE    : 0x%X",
		get_pcie_command_palette(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_PARITY         : 0x%X",
		get_pcie_command_parity(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_WAIT           : 0x%X",
		get_pcie_command_wait(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_SERR           : 0x%X",
		get_pcie_command_serr(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_FAST_BACK      : 0x%X",
		get_pcie_command_fast_back(command));

	DEV_VDBG(dev, "\t- PCI_COMMAND_INTX_DISABLE   : 0x%X",
		get_pcie_command_intx_disable(command));
}

void print_pcie_status(struct pci_dev *dev, uint16_t status)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- PCI_STATUS_IMM_READY        : 0x%X",
		get_pcie_status_imm_ready(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_INTERRUPT        : 0x%X",
		get_pcie_status_interrupt(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_CAP_LIST         : 0x%X",
		get_pcie_status_cap_list(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_66MHZ            : 0x%X",
		get_pcie_status_66mhz(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_FAST_BACK        : 0x%X",
		get_pcie_status_fast_back(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_PARITY           : 0x%X",
		get_pcie_status_parity(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_DEVSEL_MASK      : 0x%X",
		get_pcie_status_devsel_mask(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_SIG_TARGET_ABORT : 0x%X",
		get_pcie_status_sig_target_abort(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_REC_TARGET_ABORT : 0x%X",
		get_pcie_status_rec_target_abort(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_REC_MASTER_ABORT : 0x%X",
		get_pcie_status_rec_master_abort(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_SIG_SYSTEM_ERROR : 0x%X",
		get_pcie_status_sig_system_error(status));

	DEV_VDBG(dev, "\t- PCI_STATUS_DETECTED_PARITY  : 0x%X",
		get_pcie_status_detected_parity(status));

}

void print_pcie_class_revision(struct pci_dev *dev, uint32_t class_revision)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- PCI_CLASS_REVISION_ID          : 0x%X",
		get_pcie_class_revision_id(class_revision));

	DEV_VDBG(dev, "\t- PCI_CLASS_REVISION_PROG_IF     : 0x%X",
		get_pcie_class_revision_prg_int(class_revision));

	DEV_VDBG(dev, "\t- PCI_CLASS_REVISION_SUBCLASS    : 0x%X",
		get_pcie_class_revision_subclass(class_revision));

	DEV_VDBG(dev, "\t- PCI_CLASS_REVISION_CLASSCODE   : 0x%X",
		get_pcie_class_revision_classcode(class_revision));

}

void print_pcie_bar_info( struct pci_dev *dev, int bar_idx, struct bar_header_struct bar)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "BAR : %d", bar_idx);
	DEV_VDBG(dev, "\t - Start address : 0x%llX", bar.start_addr);
	DEV_VDBG(dev, "\t - End address   : 0x%llX", bar.end_addr);
	DEV_VDBG(dev, "\t - Flags         : 0x%llX", bar.flags);
	DEV_VDBG(dev, "\t - Length        : 0x%llX", bar.len);
}

#ifdef VERBOSE_DEBUG
void find_pci_capability(struct pci_dev *dev, char *cap_name, int cap)
{
	int ret = 0;

	if (!dev || !cap_name)
		return;

	ret = pci_find_capability(dev, cap);
	if (ret == CAP_NOT_FOUND)
		DEV_VDBG(dev, "PCIe capability not found: %s", cap_name);
	else
		DEV_VDBG(dev, "Found PCIe capability: %s", cap_name);
}

void pcie_find_supported_cap(struct pci_dev *dev)
{
	if (!dev)
		return;

	find_pci_capability(dev, "PCI_CAP_ID_PM",       PCI_CAP_ID_PM);
	find_pci_capability(dev, "PCI_CAP_ID_AGP",      PCI_CAP_ID_AGP);
	find_pci_capability(dev, "PCI_CAP_ID_VPD",      PCI_CAP_ID_VPD);
	find_pci_capability(dev, "PCI_CAP_ID_SLOTID",   PCI_CAP_ID_SLOTID);
	find_pci_capability(dev, "PCI_CAP_ID_MSI",      PCI_CAP_ID_MSI);
	find_pci_capability(dev, "PCI_CAP_ID_CHSWP",    PCI_CAP_ID_CHSWP);
	find_pci_capability(dev, "PCI_CAP_ID_PCIX",     PCI_CAP_ID_PCIX);
	find_pci_capability(dev, "PCI_CAP_ID_HT",       PCI_CAP_ID_HT);
	find_pci_capability(dev, "PCI_CAP_ID_VNDR",     PCI_CAP_ID_VNDR);
	find_pci_capability(dev, "PCI_CAP_ID_DBG",      PCI_CAP_ID_DBG);
	find_pci_capability(dev, "PCI_CAP_ID_CCRC",     PCI_CAP_ID_CCRC);
	find_pci_capability(dev, "PCI_CAP_ID_SHPC",     PCI_CAP_ID_SHPC);
	find_pci_capability(dev, "PCI_CAP_ID_SSVID",    PCI_CAP_ID_SSVID);
	find_pci_capability(dev, "PCI_CAP_ID_AGP3",     PCI_CAP_ID_AGP3);
	find_pci_capability(dev, "PCI_CAP_ID_SECDEV",   PCI_CAP_ID_SECDEV);
	find_pci_capability(dev, "PCI_CAP_ID_EXP",      PCI_CAP_ID_EXP);
	find_pci_capability(dev, "PCI_CAP_ID_MSIX",     PCI_CAP_ID_MSIX);
	find_pci_capability(dev, "PCI_CAP_ID_SATA",     PCI_CAP_ID_SATA);
	find_pci_capability(dev, "PCI_CAP_ID_AF",       PCI_CAP_ID_AF);
	find_pci_capability(dev, "PCI_CAP_ID_EA",       PCI_CAP_ID_EA);
}
#endif

void print_pcie_cap_power_management(struct pci_dev *dev, uint16_t cap_pm)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "PCI_CAP_ID_PM : 0x%X", cap_pm);

	DEV_VDBG(dev, "\t- REG_VERSION : 0x%X",
		get_pcie_cap_pm_version(cap_pm));

	DEV_VDBG(dev, "\t- REG_PME_CLK : 0x%X",
		get_pcie_cap_pm_pme_clk(cap_pm));

	DEV_VDBG(dev, "\t- REG_DSI     : 0x%X",
		get_pcie_cap_pm_dsi(cap_pm));

	DEV_VDBG(dev, "\t- REG_AUX_CUR : 0x%X",
		get_pcie_cap_pm_aux_cur(cap_pm));

	DEV_VDBG(dev, "\t- D1_SUPPORT  : 0x%X",
		get_pcie_cap_pm_d1_support(cap_pm));

	DEV_VDBG(dev, "\t- D2_SUPPORT  : 0x%X",
		get_pcie_cap_pm_d2_support(cap_pm));

	DEV_VDBG(dev, "\t- PME_SUPPORT : 0x%X",
		get_pcie_cap_pm_pme_support(cap_pm));

	print_pcie_cap_pm_pme_support(dev, cap_pm);
}

void print_pcie_cap_pm_pme_support(struct pci_dev *dev, uint16_t cap_pm_pme_support)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t\t- D0_ASSERT             : 0x%X",
		get_pcie_cap_pm_pme_support_d0_assert(cap_pm_pme_support));

	DEV_VDBG(dev, "\t\t- D1_ASSERT             : 0x%X",
		get_pcie_cap_pm_pme_support_d1_assert(cap_pm_pme_support));

	DEV_VDBG(dev, "\t\t- D2_ASSERT             : 0x%X",
		get_pcie_cap_pm_pme_support_d2_assert(cap_pm_pme_support));

	DEV_VDBG(dev, "\t\t- D3_HOT_ASSERT         : 0x%X",
		get_pcie_cap_pm_pme_support_d3_hot_assert(cap_pm_pme_support));

	DEV_VDBG(dev, "\t\t- D3_COLD_ASSERT_OFFSET : 0x%X",
		get_pcie_cap_pm_pme_support_d3_cold_assert(cap_pm_pme_support));
}

void print_pcie_cap_pm_status_cntrl(struct pci_dev *dev, uint16_t pm_status_cntrl)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "PCIE_CAP_PM_STATUS_CNTRL : 0x%X", pm_status_cntrl);

	DEV_VDBG(dev, "\t- PMCSR_POWER_STATE   : 0x%X", 
		get_pcie_cap_pm_pmcsr_power_state(pm_status_cntrl));

	DEV_VDBG(dev, "\t- PMCSR_NO_SOFT_RESET : 0x%X", 
		get_pcie_cap_pm_pmcsr_no_soft_reset(pm_status_cntrl));

	DEV_VDBG(dev, "\t- PMCSR_PME_ENABLE    : 0x%X", 
		get_pcie_cap_pm_pmcsr_pme_enable(pm_status_cntrl));

	DEV_VDBG(dev, "\t- PMCSR_DATA_SEL      : 0x%X", 
		get_pcie_cap_pm_pmcsr_data_sel(pm_status_cntrl));

	DEV_VDBG(dev, "\t- PMCSR_DATA_SCALE    : 0x%X", 
		get_pcie_cap_pm_pmcsr_data_scale(pm_status_cntrl));
	
	DEV_VDBG(dev, "\t- PMCSR_PME_STATUS    : 0x%X", 
		get_pcie_cap_pm_pmcsr_pme_status(pm_status_cntrl));
}

void print_pcie_cap_pm_cntrl_bridge_ext(struct pci_dev *dev, uint16_t pm_cntrl_bridge_ext)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "PCIE_CAP_PM_CNTRL_BRIDGE_EXT: 0x%X",
		pm_cntrl_bridge_ext);

	DEV_VDBG(dev, "\t- PMCSR_BSE_B2_B3  : 0x%X",
		get_pcie_cap_pm_pmcsr_bse_b2_b3(pm_cntrl_bridge_ext));

	DEV_VDBG(dev, "\t- PMCSR_BSE_BPCC_EN: 0x%X",
		get_pcie_cap_pm_pmcsr_bse_bpcc_en(pm_cntrl_bridge_ext));
}

void print_pcie_cap_pm_data(struct pci_dev *dev, uint16_t pm_data)
{
	if (dev)
		DEV_VDBG(dev, "PCIE_CAP_PM_DATA: 0x%X", pm_data);
}

void print_pcie_cap_msi( struct pci_dev *dev, uint16_t msi_config_status,
	uint16_t msi_msg_addr, uint16_t msi_msg_data, uint32_t msi_msg_uppr_addr)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "PCIE_CAP_MSI: 0x%X", msi_config_status);

	DEV_VDBG(dev, "\t- MSI_MSG_CTRL_CONFIG_EN    : 0x%X",
		get_pcie_cap_msi_msg_ctrl_config_en(msi_config_status));

	DEV_VDBG(dev, "\t- MSI_MSG_CTRL_MUL_MSG_CAP  : 0x%X",
		get_pcie_cap_msi_msg_ctrl_mul_msg_cap(msi_config_status));

	DEV_VDBG(dev, "\t- MSI_MSG_CTRL_MUL_MSG_EN   : 0x%X",
		get_pcie_cap_msi_msg_ctrl_mul_msg_en(msi_config_status));

	DEV_VDBG(dev, "\t- MSI_MSG_CTRL_64_BIT_ADDR  : 0x%X",
		get_pcie_cap_msi_msg_ctrl_64_bit_addr(msi_config_status));

	DEV_VDBG(dev, "\t- MSI_MSG_CTRL_PER_VECT_MSK : 0x%X",
		get_pcie_cap_msi_msg_ctrl_per_vect_msk(msi_config_status));

	DEV_VDBG(dev, "\t- MSI_MESSAGE_ADDR          : 0x%X",
		msi_msg_addr);

	if (get_pcie_cap_msi_msg_ctrl_64_bit_addr(msi_config_status) == MSI_32_BIT_ADDRESSING) {
		DEV_VDBG(dev, "\t- MSI_MESSAGE_32_BIT_DATA   : 0x%X ",
			msi_msg_data);
	} else {
		DEV_VDBG(dev, "\t- MSI_MESSAGE_UPPR_ADDR     : 0x%X", 
			msi_msg_uppr_addr);

		DEV_VDBG(dev, "\t- MSI_MESSAGE_64_BIT_DATA   : 0x%X ",
			msi_msg_data);
	}
}

void print_pcie_cap_cap(struct pci_dev *dev, uint16_t cap)
{
	if (!dev)
		return;
	
	DEV_VDBG(dev, "PCIE_EXP_CAPABILITIES: 0x%X", cap);
	DEV_VDBG(dev, "\t- VERSION       : 0x%X", get_pcie_cap_version(cap));
	DEV_VDBG(dev, "\t- DEV_TYPE      : 0x%X", get_pcie_cap_dev_type(cap));
	DEV_VDBG(dev, "\t- SLOT_IMP      : 0x%X", get_pcie_cap_slot_imp(cap));
	DEV_VDBG(dev, "\t- INT_MSG_NUM   : 0x%X", get_pcie_cap_int_msg_num(cap));
}

void print_pcie_cap_dev(struct pci_dev *dev, uint32_t dev_cap)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- MAX_PAYLOAD               : 0x%X",
		get_pcie_cap_dev_max_payload(dev_cap));

	DEV_VDBG(dev, "\t- PHANTOM_FN_SUP            : 0x%X",
		get_pcie_cap_dev_phantom_fn_sup(dev_cap));

	DEV_VDBG(dev, "\t- EXT_TAG_FIELD_SUP         : 0x%X",
		get_pcie_cap_dev_ext_tag_field_sup(dev_cap));

	DEV_VDBG(dev, "\t- ENDPT_L0_ACCPT_LAT        : 0x%X",
		get_pcie_cap_dev_l0_accpt_lat(dev_cap));

	DEV_VDBG(dev, "\t- ENDPT_L1_ACCPT_LAT        : 0x%X",
		get_pcie_cap_dev_endpt_l1_accpt_lat(dev_cap));

	DEV_VDBG(dev, "\t- ROLE_BASED_ERR_REP        : 0x%X",
		get_pcie_cap_dev_role_based_err_rep(dev_cap));

	DEV_VDBG(dev, "\t- CAP_SLOT_POW_LIMIT_VAL    : 0x%X",
		get_pcie_cap_dev_slot_pow_limit_val(dev_cap));

	DEV_VDBG(dev, "\t- CAP_SLOT_POW_LIMIT_SCALE  : 0x%X",
		get_pcie_cap_dev_slot_pow_limit_scale(dev_cap));

	DEV_VDBG(dev, "\t- FN_LEVEL_RESET_CAP        : 0x%X",
		get_pcie_cap_dev_fn_level_reset_cap(dev_cap));
}

void print_pcie_cap_dev_ctrl(struct pci_dev *dev, uint16_t dev_ctrl)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- CORRECT_ERR_REPORT_EN     : 0x%X",
		get_pcie_cap_dev_ctrl_correct_err_report_en(dev_ctrl));

	DEV_VDBG(dev, "\t- NON_FATAL_ERR_REPORT_EN   : 0x%X",
		get_pcie_cap_dev_ctrl_non_fatal_err_report_en(dev_ctrl));

	DEV_VDBG(dev, "\t- FATAL_ERR_REPORT_EN       : 0x%X",
		get_pcie_cap_dev_ctrl_fatal_err_report_en(dev_ctrl));

	DEV_VDBG(dev, "\t- UNSPPRT_REQ_REPORT_EN     : 0x%X",
		get_pcie_cap_dev_ctrl_unspprt_req_report_en(dev_ctrl));

	DEV_VDBG(dev, "\t- EN_RELAXED_ORD            : 0x%X",
		get_pcie_cap_dev_ctrl_en_relaxed_ord(dev_ctrl));

	DEV_VDBG(dev, "\t- PCIE_CAP_MAX_PAYLOAD_SIZE : 0x%X",
		get_pcie_cap_dev_ctrl_cap_max_payload_size(dev_ctrl));

	DEV_VDBG(dev, "\t- EXT_TAG_FILED_EN          : 0x%X",
		get_pcie_cap_dev_ctrl_ext_tag_filed_en(dev_ctrl));

	DEV_VDBG(dev, "\t- PHANTOM_FN_EN             : 0x%X",
		get_pcie_cap_dev_ctrl_phantom_fn_en(dev_ctrl));

	DEV_VDBG(dev, "\t- AUX_PWR_PM_EN             : 0x%X",
		get_pcie_cap_dev_ctrl_aux_pwr_pm_en(dev_ctrl));

	DEV_VDBG(dev, "\t- EN_NO_SNOOP               : 0x%X",
		get_pcie_cap_dev_ctrl_en_no_snoop(dev_ctrl));

	DEV_VDBG(dev, "\t- MAX_READ_REQ_SIZE         : 0x%X",
		get_pcie_cap_dev_ctrl_max_read_req_size(dev_ctrl));
}

void print_pcie_cap_dev_status(struct pci_dev *dev, uint16_t dev_stat)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- CORRECTABLE_ERR_DET : 0x%X",
		get_pcie_cap_dev_status_correctable_err_det(dev_stat));

	DEV_VDBG(dev, "\t- NONFATAL_ERR_DET    : 0x%X",
		get_pcie_cap_dev_status_nonfatal_err_det(dev_stat));

	DEV_VDBG(dev, "\t- FATAL_ERR_DET       : 0x%X",
		get_pcie_cap_dev_status_fatal_err_det(dev_stat));

	DEV_VDBG(dev, "\t- UNSUP_REQ_DET       : 0x%X",
		get_pcie_cap_dev_status_unsup_req_det(dev_stat));

	DEV_VDBG(dev, "\t- AUX_PWR_DET         : 0x%X",
		get_pcie_cap_dev_status_aux_pwr_det(dev_stat));

	DEV_VDBG(dev, "\t- TRANS_PEND          : 0x%X",
		get_pcie_cap_dev_status_trans_pend(dev_stat));
}

void print_pcie_cap_link_cap( struct pci_dev *dev, uint32_t link_cap)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- MAX_LINK_SPEED                    : 0x%X",
		get_pcie_cap_link_cap_max_link_speed(link_cap));

	DEV_VDBG(dev, "\t- MAX_PCIE_CAP_LINK_WIDTH           : 0x%X",
		get_pcie_cap_link_cap_max_pcie_cap_link_width(link_cap));

	DEV_VDBG(dev, "\t- PCIE_CAP_ASPM_SUPPORT             : 0x%X",
		get_pcie_cap_link_cap_pcie_cap_aspm_support(link_cap));

	DEV_VDBG(dev, "\t- L0_EXIT_LATENCY                   : 0x%X",
		get_pcie_cap_link_cap_l0_exit_latency(link_cap));

	DEV_VDBG(dev, "\t- L1_EXIT_LATENCY                   : 0x%X",
		get_pcie_cap_link_cap_l1_exit_latency(link_cap));

	DEV_VDBG(dev, "\t- CLK_PWR_MGMNT                     : 0x%X",
		get_pcie_cap_link_cap_clk_pwr_mgmnt(link_cap));

	DEV_VDBG(dev, "\t- SURP_DOWN_ERR_REPORT_CAP          : 0x%X",
		get_pcie_cap_link_cap_surp_down_err_report_cap(link_cap));

	DEV_VDBG(dev, "\t- DLLL_ACTIVE_REPORT_CAP            : 0x%X",
		get_pcie_cap_link_cap_dlll_active_report_cap(link_cap));

	DEV_VDBG(dev, "\t- LINK_BW_NOT_CAP                   : 0x%X",
		get_pcie_cap_link_cap_link_bw_not_cap(link_cap));

	DEV_VDBG(dev, "\t- PCIE_CAP_ASPM_OPTIONAL_COMPLIANCE : 0x%X",
		get_pcie_cap_link_cap_pcie_cap_aspm_optional_compliance(link_cap));

	DEV_VDBG(dev, "\t- PORT_NUM                          : 0x%X",
		get_pcie_cap_link_cap_port_num(link_cap));
}

void print_pcie_cap_link_ctrl(struct pci_dev *dev, uint16_t link_ctrl)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- PCIE_CAP_ASPM_CNTRL     : 0x%X",
		get_pcie_cap_link_ctrl_pcie_cap_aspm_cntrl(link_ctrl));

	DEV_VDBG(dev, "\t- RCB                     : 0x%X",
		get_pcie_cap_link_ctrl_rcb(link_ctrl));

	DEV_VDBG(dev, "\t- LINK_DISABLE            : 0x%X",
		get_pcie_cap_link_ctrl_link_disable(link_ctrl));

	DEV_VDBG(dev, "\t- RETRAIN_LINK            : 0x%X",
		get_pcie_cap_link_ctrl_retrain_link(link_ctrl));

	DEV_VDBG(dev, "\t- COMMON_CLK_CONFIG       : 0x%X",
		get_pcie_cap_link_ctrl_common_clk_config(link_ctrl));

	DEV_VDBG(dev, "\t- EXTENDED_SYNCH          : 0x%X",
		get_pcie_cap_link_ctrl_extended_synch(link_ctrl));

	DEV_VDBG(dev, "\t- EN_CLK_PWR_MGMT         : 0x%X",
		get_pcie_cap_link_ctrl_en_clk_pwr_mgmt(link_ctrl));

	DEV_VDBG(dev, "\t- HARDWARE_AUTO_WIDTH_DIS : 0x%X",
		get_pcie_cap_link_ctrl_hardware_auto_width_dis(link_ctrl));

	DEV_VDBG(dev, "\t- BW_MGMT_INT_EN          : 0x%X",
		get_pcie_cap_link_ctrl_bw_mgmt_int_en(link_ctrl));

	DEV_VDBG(dev, "\t- AUTO_BW_INT_EN          : 0x%X",
		get_pcie_cap_link_ctrl_auto_bw_int_en(link_ctrl));

	DEV_VDBG(dev, "\t- DRS_SIGNAL_CNTRL        : 0x%X",
		get_pcie_cap_link_ctrl_drs_signal_cntrl(link_ctrl));

}

void print_pcie_cap_link_status(struct pci_dev *dev, uint16_t link_stat)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- CUR_LINK_SPEED          : 0x%X",
		get_pcie_cap_link_status_cur_link_speed(link_stat));

	DEV_VDBG(dev, "\t- NEG_PCIE_CAP_LINK_WIDTH : 0x%X",
		get_pcie_cap_link_status_neg_pcie_cap_link_width(link_stat));

	DEV_VDBG(dev, "\t- LINK_TRAINING           : 0x%X",
		get_pcie_cap_link_status_link_training(link_stat));

	DEV_VDBG(dev, "\t- SLOT_CLK_CONFIG         : 0x%X",
		get_pcie_cap_link_status_slot_clk_config(link_stat));

	DEV_VDBG(dev, "\t- DLL_ACTIVE              : 0x%X",
		get_pcie_cap_link_status_dll_active(link_stat));

	DEV_VDBG(dev, "\t- LINK_BW_MGMT_STATUS     : 0x%X",
		get_pcie_cap_link_status_link_bw_mgmt_status(link_stat));

	DEV_VDBG(dev, "\t- LINK_AUTO_BW_STATUS     : 0x%X",
		get_pcie_cap_link_status_link_auto_bw_status(link_stat));
}

void print_pcie_cap_slot_cap(struct pci_dev *dev, uint32_t slot_cap)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- ATTENTION_BUTTON_PRESENT    : 0x%X",
		get_pcie_cap_slot_cap_attention_button_present(slot_cap));

	DEV_VDBG(dev, "\t- POWER_CONTROLLER_PRESENT    : 0x%X",
		get_pcie_cap_slot_cap_power_controller_present(slot_cap));

	DEV_VDBG(dev, "\t- MRL_SENSOR_PRESENT          : 0x%X",
		get_pcie_cap_slot_cap_mrl_sensor_present(slot_cap));

	DEV_VDBG(dev, "\t- ATTENTION_INDICATOR_PRESENT : 0x%X",
		get_pcie_cap_slot_cap_attention_indicator_present(slot_cap));

	DEV_VDBG(dev, "\t- POWER_INDICATOR_PRESENT     : 0x%X",
		get_pcie_cap_slot_cap_power_indicator_present(slot_cap));

	DEV_VDBG(dev, "\t- HOT_PLUG_SURPRISE           : 0x%X",
		get_pcie_cap_slot_cap_hot_plug_surprise(slot_cap));

	DEV_VDBG(dev, "\t- HOT_PLUG_CAPABLE            : 0x%X",
		get_pcie_cap_slot_cap_hot_plug_capable(slot_cap));

	DEV_VDBG(dev, "\t- SLOT_POWER_LIMIT_VAL        : 0x%X",
		get_pcie_cap_slot_cap_slot_power_limit_val(slot_cap));

	DEV_VDBG(dev, "\t- SLOT_POWER_LIMIT_SCALE      : 0x%X",
		get_pcie_cap_slot_cap_slot_power_limit_scale(slot_cap));

	DEV_VDBG(dev, "\t- ELECTROMAG_INTRLCK_PRESENT  : 0x%X",
		get_pcie_cap_slot_cap_electromag_intrlck_present(slot_cap));

	DEV_VDBG(dev, "\t- NO_CMD_COMP_SUPPORT         : 0x%X",
		get_pcie_cap_slot_cap_no_cmd_comp_support(slot_cap));
}

void print_pcie_cap_slot_ctrl(struct pci_dev *dev, uint16_t slot_ctrl)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- ATTENTION_BUTTON_PRESSED_EN: 0x%X",
		get_pcie_cap_slot_ctrl_attention_button_pressed_en(slot_ctrl));

	DEV_VDBG(dev, "\t- PWR_FAULT_DET_EN           : 0x%X",
		get_pcie_cap_slot_ctrl_pwr_fault_det_en(slot_ctrl));

	DEV_VDBG(dev, "\t- MRL_SENSOR_CHANGED_EN      : 0x%X",
		get_pcie_cap_slot_ctrl_mrl_sensor_changed_en(slot_ctrl));

	DEV_VDBG(dev, "\t- PRESENT_DET_CHANGE_EN      : 0x%X",
		get_pcie_cap_slot_ctrl_present_det_change_en(slot_ctrl));

	DEV_VDBG(dev, "\t- CMD_COMP_INT_EN            : 0x%X",
		get_pcie_cap_slot_ctrl_cmd_comp_int_en(slot_ctrl));

	DEV_VDBG(dev, "\t- HOT_PLUG_INT_EN            : 0x%X",
		get_pcie_cap_slot_ctrl_hot_plug_int_en(slot_ctrl));

	DEV_VDBG(dev, "\t- ATTN_INDICATOR_CNTRL       : 0x%X",
		get_pcie_cap_slot_ctrl_cntrl_attn_indicator_cntrl(slot_ctrl));

	DEV_VDBG(dev, "\t- PWR_INDICATOR_CNTRL        : 0x%X",
		get_pcie_cap_slot_ctrl_pwr_indicator_cntrl(slot_ctrl));

	DEV_VDBG(dev, "\t- PWR_CONTROLLER_CNTRL       : 0x%X",
		get_pcie_cap_slot_ctrl_pwr_controller_cntrl(slot_ctrl));

	DEV_VDBG(dev, "\t- ELECTROMEC_INTERLOCK_CNTRL : 0x%X",
		get_pcie_cap_slot_ctrl_electromec_interlock_cntrl(slot_ctrl));

	DEV_VDBG(dev, "\t- DLLS_CHANGED_EN            : 0x%X",
		get_pcie_cap_slot_ctrl_dlls_changed_en(slot_ctrl));
}

void print_pcie_cap_slot_status(struct pci_dev *dev, uint16_t slot_stat)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- ATTENTION_BUT_PRESSED       : 0x%X",
		get_pcie_cap_slot_status_attention_but_pressed(slot_stat));

	DEV_VDBG(dev, "\t- PWR_FAULT_DET               : 0x%X",
		get_pcie_cap_slot_status_status_pwr_fault_det(slot_stat));

	DEV_VDBG(dev, "\t- MRL_SENSOR_CHNG             : 0x%X",
		get_pcie_cap_slot_status_mrl_sensor_chng(slot_stat));

	DEV_VDBG(dev, "\t- PRESENCE_DET_CHNG           : 0x%X",
		get_pcie_cap_slot_status_presence_det_chng(slot_stat));

	DEV_VDBG(dev, "\t- CMD_COMP                    : 0x%X",
		get_pcie_cap_slot_status_cmd_comp(slot_stat));

	DEV_VDBG(dev, "\t- MRL_SENSOR_STATE            : 0x%X",
		get_pcie_cap_slot_status_mrl_sensor_state(slot_stat));

	DEV_VDBG(dev, "\t- PRESENCE_DETECT_STATE       : 0x%X",
		get_pcie_cap_slot_status_presence_detect_state(slot_stat));

	DEV_VDBG(dev, "\t- ELECTROMEC_INTERLOCK_STATUS : 0x%X",
		get_pcie_cap_slot_status_electromec_interlock_status(slot_stat));

	DEV_VDBG(dev, "\t- DLL_STATE_CHANGED           : 0x%X",
		get_pcie_cap_slot_status_dll_state_changed(slot_stat));
}

void print_pcie_cap_root_ctrl(struct pci_dev *dev, uint16_t root_ctrl)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- ERR_CORR_ERR_EN            : 0x%X",
		get_pcie_cap_root_ctrl_err_corr_err_en(root_ctrl));

	DEV_VDBG(dev, "\t- ERR_NON_FATAL_ERR_EN       : 0x%X",
		get_pcie_cap_root_ctrl_err_non_fatal_err(root_ctrl));

	DEV_VDBG(dev, "\t- ERR_FATAL_ERR_EN           : 0x%X",
		get_pcie_cap_root_ctrl_err_fatal_err_en(root_ctrl));

	DEV_VDBG(dev, "\t- PME_INT_EN                 : 0x%X",
		get_pcie_cap_root_ctrl_pme_int_en(root_ctrl));

	DEV_VDBG(dev, "\t- CRS_SOFTWARE_VISIBILITY_EN : 0x%X",
		get_pcie_cap_root_ctrl_crs_software_visibility_en(root_ctrl));
}

void print_pcie_cap_root_cap(struct pci_dev *dev, uint16_t root_cap)
{
	if (dev)
		DEV_VDBG(dev, "\t- CAPABILITIES_CRS_SOFT_VIS: 0x%X",
			get_pcie_cap_root_cap_crs_soft_vis(root_cap));
}

void print_pcie_cap_root_status(struct pci_dev *dev, uint32_t root_stat)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- PME_STATUS : 0x%X",
		get_pcie_cap_root_status_pme_status(root_stat));

	if (get_pcie_cap_root_status_pme_status(root_stat) == 0x01)
		DEV_VDBG(dev, "\t- PME_REQ_ID : 0x%X",
			get_pcie_cap_root_status_pme_req_i(root_stat));

	DEV_VDBG(dev, "\t- PME_PEND   : 0x%X",
		get_pcie_cap_root_status_pme_pend(root_stat));
}

void print_pcie_cap_dev_cap_2(struct pci_dev *dev, uint32_t dev_cap)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- COMP_TIMEOUT_RANGE_SUP   : 0x%X",
		get_pcie_cap_dev_cap_2_timeout_range_sup(dev_cap));

	DEV_VDBG(dev, "\t- COMP_TIMEOUT_DISABLE_SUP : 0x%X",
		get_pcie_cap_dev_cap_2_comp_timeout_disable_sup(dev_cap));

	DEV_VDBG(dev, "\t- ARI_FORWARD_SUP          : 0x%X",
		get_pcie_cap_dev_cap_2_ari_forward_sup(dev_cap));

	DEV_VDBG(dev, "\t- ATOMIC_OP_ROUTING_SUP    : 0x%X",
		get_pcie_cap_dev_cap_2_atomic_op_routing_sup(dev_cap));

	DEV_VDBG(dev, "\t- 32_BIT_ATOMIC_OP_COMP_SUP: 0x%X",
		get_pcie_cap_dev_cap_2_32_bit_atomic_op_comp_sup(dev_cap));

	DEV_VDBG(dev, "\t- 64_BIT_ATOMIC_OP_COMP_SUP: 0x%X",
		get_pcie_cap_dev_cap_2_64_bit_atomic_op_comp_sup(dev_cap));

	DEV_VDBG(dev, "\t- 128_BIT_CAS_COMP_SUP     : 0x%X",
		get_pcie_cap_dev_cap_2_128_bit_cas_comp_sup(dev_cap));

	DEV_VDBG(dev, "\t- NO_RO_EN_PR_PR_PASS      : 0x%X",
		get_pcie_cap_dev_cap_2_no_ro_en_pr_pas(dev_cap));

	DEV_VDBG(dev, "\t- LTR_MECH_SUP             : 0x%X",
		get_pcie_cap_dev_cap_2_ltr_mech_sup(dev_cap));

	DEV_VDBG(dev, "\t- TPH_COMP_SUP             : 0x%X",
		get_pcie_cap_dev_cap_2_tph_comp_sup(dev_cap));

	DEV_VDBG(dev, "\t- OBFF_SUP                 : 0x%X",
		get_pcie_cap_dev_cap_2_obff_sup(dev_cap));

	DEV_VDBG(dev, "\t- EXT_FMT_FIELD_SUP        : 0x%X",
		get_pcie_cap_dev_cap_2_ext_fmt_field_sup(dev_cap));

	DEV_VDBG(dev, "\t- END_END_TLP_PREFIX_SUP   : 0x%X",
		get_pcie_cap_dev_cap_2_end_tlp_prefix_sup(dev_cap));

	DEV_VDBG(dev, "\t- MAX_END_END_TLP_PREFIXES : 0x%X",
		get_pcie_cap_dev_cap_2_max_end_tlp_prefixes(dev_cap));
}

void print_pcie_cap_dev_ctrl_2(struct pci_dev *dev, uint16_t dev_ctrl)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- COMP_TIMEOUT_VAL         : 0x%X", 
		get_pcie_cap_dev_ctrl_2_comp_timeout_val(dev_ctrl));

	DEV_VDBG(dev, "\t- COMP_TIMEOUT_DISABLE     : 0x%X", 
		get_pcie_cap_dev_ctrl_2_comp_timeout_disable(dev_ctrl));

	DEV_VDBG(dev, "\t- ARI_FRWD_EN              : 0x%X", 
		get_pcie_cap_dev_ctrl_2_ari_frwd_en(dev_ctrl));

	DEV_VDBG(dev, "\t- ATOMIC_OP_REQ_EN         : 0x%X", 
		get_pcie_cap_dev_ctrl_2_atomic_op_req_en(dev_ctrl));

	DEV_VDBG(dev, "\t- ATOMIC_OP_EGRESS_BLOCK   : 0x%X", 
		get_pcie_cap_dev_ctrl_2_atomic_op_egress_block(dev_ctrl));

	DEV_VDBG(dev, "\t- IDO_REQ_EN               : 0x%X", 
		get_pcie_cap_dev_ctrl_2_ido_req_en(dev_ctrl));

	DEV_VDBG(dev, "\t- IDO_COMP_EN              : 0x%X", 
		get_pcie_cap_dev_ctrl_2_ido_comp_en(dev_ctrl));

	DEV_VDBG(dev, "\t- LTR_MECH_EN_OFFSE        : 0x%X", 
		get_pcie_cap_dev_ctrl_2_ltr_mech_en(dev_ctrl));

	DEV_VDBG(dev, "\t- OBFF_EN_OFFSET)          : 0x%X", 
		get_pcie_cap_dev_ctrl_2_obff_en(dev_ctrl));

	DEV_VDBG(dev, "\t- END_TLP_PREFIX_BLOCK : 0x%X", 
		get_pcie_cap_dev_ctrl_2_end_tlp_prefix_block(dev_ctrl));
}

void print_pcie_cap_link_cap_2(struct pci_dev *dev, uint32_t link_cap)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- SUPP_LINK_SPEED_VECTOR : 0x%X", 
		get_pcie_cap_link_cap_2_supp_link_speed_vector(link_cap));

	DEV_VDBG(dev, "\t- CROSSLINK_SUP          : 0x%X", 
		get_pcie_cap_link_cap_2_crosslink_sup(link_cap));
}

void print_pcie_cap_link_ctrl_2(struct pci_dev *dev, uint16_t link_ctrl)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- TARGET_LINK_SPEED    : 0x%X",
		get_pcie_cap_link_ctrl_2_target_link_speed(link_ctrl));

	DEV_VDBG(dev, "\t- ENTER_COMPLIANCE     : 0x%X",
		get_pcie_cap_link_ctrl_2_enter_complianc(link_ctrl));

	DEV_VDBG(dev, "\t- HW_AUTO_SPEED_DISABLE: 0x%X",
		get_pcie_cap_link_ctrl_2_hw_auto_speed_disable(link_ctrl));

	DEV_VDBG(dev, "\t- SEL_DE_EMPHASIS      : 0x%X",
		get_pcie_cap_link_ctrl_2_sel_de_emphasis(link_ctrl));

	DEV_VDBG(dev, "\t- TRANSMIT_MARGIN      : 0x%X",
		get_pcie_cap_link_ctrl_2_transmit_margin(link_ctrl));

	DEV_VDBG(dev, "\t- ENTER_MODIFIED_COMP  : 0x%X",
		get_pcie_cap_link_ctrl_2_enter_modified_comp(link_ctrl));

	DEV_VDBG(dev, "\t- COMPLIANCE_SOS       : 0x%X",
		get_pcie_cap_link_ctrl_2_compliance_sos(link_ctrl));

	DEV_VDBG(dev, "\t- COMPLIANCE_PRESET    : 0x%X",
		get_pcie_cap_link_ctrl_2_compliance_preset(link_ctrl));
}

void print_pcie_cap_link_status_2(struct pci_dev *dev, uint16_t link_stat)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "\t- REG_CUR_DEEMPHASIS_LEVEL : 0x%X",
		get_pcie_cap_link_status_2_cur_deemphasis_level(link_stat));

	DEV_VDBG(dev, "\t- REG_EQUALIZATION_COMP    : 0x%X",
		get_pcie_cap_link_status_2_equalization_comp(link_stat));

	DEV_VDBG(dev, "\t- EQUALIZATION_P1_SUC      : 0x%X",
		get_pcie_cap_link_status_2_equalization_p1_suc(link_stat));

	DEV_VDBG(dev, "\t- EQUALIZATION_P2_SUC      : 0x%X",
		get_pcie_cap_link_status_2_equalization_p2_suc(link_stat));

	DEV_VDBG(dev, "\t- EQUALIZATION_P3_SUC      : 0x%X",
		get_pcie_cap_link_status_2_equalization_p3_suc(link_stat));

	DEV_VDBG(dev, "\t- LINK_EQUALIZATION_REQ    : 0x%X",
		get_pcie_cap_link_status_2_link_equalization_req(link_stat));
}

#ifdef VERBOSE_DEBUG
void find_pci_ext_capability(struct pci_dev *dev, char *cap_name, int cap)
{
	int ret = 0;

	if (!dev || !cap_name)
		return;
	
	ret = pci_find_ext_capability(dev, PCI_CAP_ID_PM);
	if (ret == CAP_NOT_FOUND)
		DEV_VDBG(dev, "PCIe extended capability not found: %s", cap_name);
	else
		DEV_VDBG(dev, "Found PCIe extended capability: %s", cap_name);
}

void pcie_find_supported_ext_caps(struct pci_dev *dev)
{
	if (!dev)
		return;

	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_ERR", PCI_EXT_CAP_ID_ERR);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_VC", PCI_EXT_CAP_ID_VC);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_DSN", PCI_EXT_CAP_ID_DSN);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_PWR", PCI_EXT_CAP_ID_PWR);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_RCLD", PCI_EXT_CAP_ID_RCLD);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_RCILC", PCI_EXT_CAP_ID_RCILC);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_RCEC", PCI_EXT_CAP_ID_RCEC);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_MFVC", PCI_EXT_CAP_ID_MFVC);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_VC9", PCI_EXT_CAP_ID_VC9);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_RCRB", PCI_EXT_CAP_ID_RCRB);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_VNDR", PCI_EXT_CAP_ID_VNDR);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_CAC", PCI_EXT_CAP_ID_CAC);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_ACS", PCI_EXT_CAP_ID_ACS);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_ARI", PCI_EXT_CAP_ID_ARI);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_ATS", PCI_EXT_CAP_ID_ATS);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_SRIOV", PCI_EXT_CAP_ID_SRIOV);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_MRIOV", PCI_EXT_CAP_ID_MRIOV);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_MCAST", PCI_EXT_CAP_ID_MCAST);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_PRI", PCI_EXT_CAP_ID_PRI);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_AMD_XXX", PCI_EXT_CAP_ID_AMD_XXX);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_REBAR", PCI_EXT_CAP_ID_REBAR);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_DPA", PCI_EXT_CAP_ID_DPA);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_TPH", PCI_EXT_CAP_ID_TPH);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_LTR", PCI_EXT_CAP_ID_LTR);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_SECPCI", PCI_EXT_CAP_ID_SECPCI);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_PMUX", PCI_EXT_CAP_ID_PMUX);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_PASID", PCI_EXT_CAP_ID_PASID);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_DPC", PCI_EXT_CAP_ID_DPC);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_L1SS", PCI_EXT_CAP_ID_L1SS);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_PTM", PCI_EXT_CAP_ID_PTM);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_DLF", PCI_EXT_CAP_ID_DLF);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_PL_16GT", PCI_EXT_CAP_ID_PL_16GT);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_LANE_MERGE_REC", PCI_EXT_CAP_ID_LANE_MERGE_REC);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_HIERARCHY_ID", PCI_EXT_CAP_ID_HIERARCHY_ID);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_NPEM",  PCI_EXT_CAP_ID_NPEM);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_PHY_LAYER_32_GTS", PCI_EXT_CAP_ID_PHY_LAYER_32_GTS);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_ID_ALT_PROTOCOL", PCI_EXT_CAP_ID_ALT_PROTOCOL);
	find_pci_ext_capability(dev, "PCI_EXT_CAP_SFI", PCI_EXT_CAP_SFI);
}
#endif

void print_pcie_stat(struct pci_dev *dev, pcie_cap_struct *cap)
{
	if (!dev || !cap)
		return;

	DEV_VDBG(dev, "\t- PCIe Status:");

	DEV_VDBG(dev, "\t- Expected PCIe link speed : %d",
		cap->expected_pcie_link_speed);

	DEV_VDBG(dev, "\t- Expected PCIe link width : %d",
		cap->expected_pcie_link_width);

	DEV_VDBG(dev, "\t- Current PCIe link speed : %d",
		cap->current_pcie_link_speed);

	DEV_VDBG(dev, "\t- Current PCIe link width : %d",
		cap->current_pcie_link_width);
}

/* VSEC DEBUG */

void print_endpoint_info(struct pci_dev *dev, endpoint_info_struct endpoint_info)
{
	if (!dev)
		return;

	DEV_VDBG(dev, "%s endpoint:", endpoint_info.name);
	DEV_VDBG(dev, "\t - BAR index     : 0x%X",   endpoint_info.bar_num);
	DEV_VDBG(dev, "\t - Start address : 0x%llX", endpoint_info.start_addr);
	DEV_VDBG(dev, "\t - End address   : 0x%llX", endpoint_info.end_addr);
	DEV_VDBG(dev, "\t - Length        : 0x%llX", endpoint_info.bar_len);
}
