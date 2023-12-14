// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_pcie.c - This file contains PCI reading/writing logic.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/device.h>

#include "ami_top.h"
#include "ami_pcie.h"
#include "ami_utils.h"
#include "ami.h"
#include "ami_pci_dbg.h"


/* List of supported PCI devices */
static enum AMI_PCIE_DEVICE_ID SUPPORTED_PCIE_DEVICE_ID[] = {
	AMI_PCIE_DEVICE_ID_V70_PF0,
	AMI_PCIE_DEVICE_ID_V70_PF1,
	AMI_PCIE_DEVICE_ID_VCK5000_PF0,
	AMI_PCIE_DEVICE_ID_VCK5000_PF1,
	AMI_PCIE_DEVICE_ID_V80,
	AMI_PCIE_DEVICE_ID_V80P,
};


void get_pcie_bar_info(struct pci_dev *dev, int bar_idx, struct bar_header_struct *bar)
{
	if (!dev || !bar)
		return;

	bar->start_addr = pci_resource_start(dev, bar_idx);
	bar->end_addr   = pci_resource_end(dev, bar_idx);
	bar->flags      = pci_resource_flags(dev, bar_idx);
	bar->len        = pci_resource_len(dev, bar_idx);
}

int read_pcie_config_header(struct pci_dev *dev, pcie_header_struct **header)
{
	int ret = 0;
	int i = 0;

	if (!dev || !header)
		return -EINVAL;

	DEV_VDBG(dev, "Getting PCIe configuration header");

	(*header) = kzalloc(sizeof(pcie_header_struct), GFP_KERNEL);
	if (!((*header))) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}

	ret = pci_read_config_word(dev, PCI_VENDOR_ID , &((*header)->vendor_id));
	if (ret)
		goto fail;
	print_pcie_vendor_id(dev, (*header)->vendor_id);

	ret = pci_read_config_word(dev, PCI_DEVICE_ID ,&((*header)->device_id));
	if (ret)
		goto fail;
	print_pcie_device_id(dev, (*header)->device_id);

	ret = pci_read_config_word(dev, PCI_COMMAND, &((*header)->command));
	if (ret)
		goto fail;
	print_pcie_command (dev, (*header)->command);

	ret = pci_read_config_word(dev, PCI_STATUS, &((*header)->status));
	if (ret)
		goto fail;
	print_pcie_status(dev, (*header)->status);

	ret = pci_read_config_dword(dev, PCI_CLASS_REVISION, &((*header)->class_revision));
	if (ret)
		goto fail;
	print_pcie_class_revision(dev, (*header)->class_revision);

	ret = pci_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &((*header)->cache_line_size));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev, PCI_LATENCY_TIMER, &((*header)->latency_timer));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev, PCI_HEADER_TYPE, &((*header)->header_type));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev, PCI_BIST, &((*header)->bist));
	if (ret)
		goto fail;

	/* Read Bar Info: Start and End Address */
	for (i = PCIE_BAR0; i < NUM_PCIE_BAR; i++) {
		get_pcie_bar_info(dev, i, &((*header)->bar[i]));
		print_pcie_bar_info(dev, i, (*header)->bar[i]);
	}

	ret = pci_read_config_dword(dev, PCI_CARDBUS_CIS, &((*header)->cardbus_cis));
	if (ret)
		goto fail;

	ret = pci_read_config_word(dev,PCI_SUBSYSTEM_VENDOR_ID, &((*header)->subsystem_vendor_id));
	if (ret)
		goto fail;

	ret = pci_read_config_word(dev, PCI_SUBSYSTEM_ID, &((*header)->subsystem_id));
	if (ret)
		goto fail;

	ret = pci_read_config_dword(dev, PCI_ROM_ADDRESS, &((*header)->rom_addr));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev, PCI_CAPABILITY_LIST, &((*header)->capability_list));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &((*header)->interrupt_line));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &((*header)->interrupt_pin));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev,PCI_MIN_GNT, &((*header)->min_gnt));
	if (ret)
		goto fail;

	ret = pci_read_config_byte(dev, PCI_MAX_LAT, &((*header)->max_lat));
	if (ret)
		goto fail;

	DEV_VDBG(dev, "Successfully read configuration header");
	return SUCCESS;

fail:
	release_pcie_config_header(header);
	DEV_ERR(dev, "Failed to read configuration header");
	return ret;
}

int read_pcie_capabilities(struct pci_dev *dev, pcie_header_struct *header,
	pcie_cap_struct **cap)
{
	int ret = 0;
	uint8_t current_cap_index = 0;
	uint32_t read_buf = 0;
	uint32_t capability_read_buf = 0;

	if (!dev || !header || !cap)
		return -EINVAL;

	(*cap) = (pcie_cap_struct *)kzalloc(sizeof(pcie_cap_struct), GFP_KERNEL);
	if (!((*cap))) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*cap)->cap_count = 0;

	(*cap)->exp = kmalloc(sizeof(pcie_exp_cap_struct), GFP_KERNEL);
	if (!((*cap)->exp)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}

	(*cap)->msi = kmalloc(sizeof(pcie_msi_cap_struct), GFP_KERNEL);
	if (!((*cap)->msi)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}

	(*cap)->pwr_mgt = kmalloc(sizeof(pcie_pwr_mgt_cap_struct), GFP_KERNEL);
	if (!((*cap)->pwr_mgt)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}

#ifdef VERBOSE_DEBUG
	pcie_find_supported_cap(dev);
#endif

	current_cap_index = header->capability_list;

	do {
		DEV_VDBG(dev, "current_cap_index : 0x%X", current_cap_index);
		ret = pci_read_config_dword(dev, current_cap_index,
			&capability_read_buf);
		if (ret)
			goto fail;

		switch(get_pcie_capability_cap_id(capability_read_buf)) {
		case PCI_CAP_ID_PM:
			DEV_VDBG(dev, "********************************* PCI_CAP_ID_PM *********************************");

			(*cap)->pwr_mgt->power_mgt_cap_base_addr = \
				current_cap_index;
			(*cap)->pwr_mgt->cap_id = \
				get_pcie_capability_cap_id(capability_read_buf);
			(*cap)->pwr_mgt->next_cap = \
				get_pcie_capability_next_cap(capability_read_buf);

			DEV_VDBG(dev, "Capability ID : 0x%X",
				(*cap)->pwr_mgt->cap_id);
			DEV_VDBG(dev, "Next Item Ptr : 0x%X",
				(*cap)->pwr_mgt->next_cap);

			(*cap)->pwr_mgt->power_mgt_cap_reg = \
				get_pcie_cap_pm(capability_read_buf);
			print_pcie_cap_power_management(dev,
				(*cap)->pwr_mgt->power_mgt_cap_reg);

			ret = pci_read_config_dword(dev,
				current_cap_index + 0x04, &read_buf);
			if (ret)
				goto fail;

			(*cap)->pwr_mgt->power_mgt_status_ctrl = \
				get_pcie_cap_pm_status_cntrl(read_buf);
			print_pcie_cap_pm_status_cntrl(dev,
				(*cap)->pwr_mgt->power_mgt_status_ctrl);

			(*cap)->pwr_mgt->power_mgt_ctrl_status_bridge_ext = \
				get_pcie_cap_pm_cntrl_bridge_ext(read_buf);
			print_pcie_cap_pm_cntrl_bridge_ext(dev,
				(*cap)->pwr_mgt->power_mgt_ctrl_status_bridge_ext);

			(*cap)->pwr_mgt->power_mgt_data = \
				get_pcie_cap_pm_data(read_buf);
			print_pcie_cap_pm_data(dev,
				(*cap)->pwr_mgt->power_mgt_data);

			break;

		case PCI_CAP_ID_MSI:
			DEV_VDBG(dev, "********************************* PCI_CAP_ID_MSI *********************************");
			(*cap)->msi->msi_msg_cap_base_addr = \
				current_cap_index;
			(*cap)->msi->cap_id = \
				get_pcie_capability_cap_id (capability_read_buf);
			(*cap)->msi->next_cap = \
				get_pcie_capability_next_cap(capability_read_buf);

			DEV_VDBG(dev, "Capability ID : 0x%X",
				(*cap)->msi->cap_id);
			DEV_VDBG(dev, "Next Item ptr : 0x%X",
				(*cap)->msi->next_cap);

			(*cap)->msi->msi_config_status_reg = \
				get_pcie_cap_msi_config_status(capability_read_buf);

			ret = pci_read_config_dword(dev,
				(*cap)->msi->msi_msg_cap_base_addr + MSI_MESSAGE_ADDR_OFFSET, 
				&((*cap)->msi->msi_msg_addr));
			if (ret)
				goto fail;

			if (get_pcie_cap_msi_msg_ctrl_64_bit_addr(
					(*cap)->msi->msi_config_status_reg) == \
					MSI_32_BIT_ADDRESSING) {

				ret = pci_read_config_word(dev, 
						(*cap)->msi->msi_msg_cap_base_addr + \
						MSI_MESSAGE_32_BIT_DATA_OFFSET,
						&((*cap)->msi->msi_msg_data));
				if (ret)
					goto fail;
			} else {
				ret = pci_read_config_dword(dev,
						(*cap)->msi->msi_msg_cap_base_addr + \
						MSI_MESSAGE_UPPR_ADDR_OFFSET,
						&((*cap)->msi->msi_msg_uppr_addr));
				if (ret)
					goto fail;

				ret = pci_read_config_word(dev,
						(*cap)->msi->msi_msg_cap_base_addr + \
						MSI_MESSAGE_64_BIT_DATA_OFFSET,
						&((*cap)->msi->msi_msg_data));
				if (ret)
					goto fail;
			}

			print_pcie_cap_msi(dev,
				(*cap)->msi->msi_config_status_reg,
				(*cap)->msi->msi_msg_addr,
				(*cap)->msi->msi_msg_data,
				(*cap)->msi->msi_msg_uppr_addr);

			break;

		case PCI_CAP_ID_MSIX:
			DEV_VDBG(dev, "********************************* PCI_CAP_ID_MSIX *********************************");
			break;

		case PCI_CAP_ID_EXP:
			DEV_VDBG(dev, "********************************* PCI_CAP_ID_EXP *********************************");
			(*cap)->exp->pcie_exp_cap_base_addr = \
				current_cap_index;
			(*cap)->exp->pcie_exp_cap_id = \
				get_pcie_capability_cap_id(capability_read_buf);
			(*cap)->exp->next_cap = \
				get_pcie_capability_next_cap(capability_read_buf);

			DEV_VDBG(dev, "Capability ID : 0x%X",
				(*cap)->exp->pcie_exp_cap_id);
			DEV_VDBG(dev, "Next Item ptr : 0x%X",
				(*cap)->exp->next_cap);

			(*cap)->exp->pcie_exp_cap_reg = \
				get_pcie_exp_cap(capability_read_buf);
			print_pcie_cap_cap(dev, (*cap)->exp->pcie_exp_cap_reg);

			ret = pci_read_config_dword(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_DEV_CAPABILITIES_OFFSET,
					&((*cap)->exp->pcie_exp_dev_cap));
			if (ret)
				goto fail;
			print_pcie_cap_dev(dev, (*cap)->exp->pcie_exp_dev_cap);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_DEV_CNTRL_OFFSET,
					&((*cap)->exp->pcie_exp_dev_ctrl));
			if (ret)
				goto fail;
			print_pcie_cap_dev_ctrl(dev, (*cap)->exp->pcie_exp_dev_ctrl);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_DEV_STATUS_OFFSET,
					&((*cap)->exp->pcie_exp_dev_status));
			if (ret)
				goto fail;
			print_pcie_cap_dev_status(dev,
				(*cap)->exp->pcie_exp_dev_status);

			ret = pci_read_config_dword(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_LINK_CAPABILITIES_OFFSET,
					&((*cap)->exp->pcie_exp_link_cap));
			if (ret)
				goto fail;

			(*cap)->expected_pcie_link_speed = \
				get_pcie_cap_link_cap_max_link_speed(
					(*cap)->exp->pcie_exp_link_cap);
			(*cap)->expected_pcie_link_width = \
				get_pcie_cap_link_cap_max_pcie_cap_link_width(
					(*cap)->exp->pcie_exp_link_cap);

			print_pcie_cap_link_cap(dev,
				(*cap)->exp->pcie_exp_link_cap);

			ret = pci_read_config_word(dev, 
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_LINK_CNTRL_OFFSET,
					&((*cap)->exp->pcie_exp_link_ctrl));
			if (ret)
				goto fail;
			print_pcie_cap_link_ctrl(dev,
				(*cap)->exp->pcie_exp_link_ctrl);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_LINK_STATUS_OFFSET,
					&((*cap)->exp->pcie_exp_link_status));
			if (ret)
				goto fail;

			(*cap)->current_pcie_link_speed = \
				get_pcie_cap_link_status_cur_link_speed(
					(*cap)->exp->pcie_exp_link_status);

			(*cap)->current_pcie_link_width = \
				get_pcie_cap_link_status_neg_pcie_cap_link_width(
					(*cap)->exp->pcie_exp_link_status);

			print_pcie_cap_link_status(dev,
				(*cap)->exp->pcie_exp_link_status);

			ret = pci_read_config_dword(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_SLOT_CAPABILITIES_OFFSET,
					&((*cap)->exp->pcie_exp_slot_cap));
			if (ret)
				goto fail;
			print_pcie_cap_slot_cap(dev,
				(*cap)->exp->pcie_exp_slot_cap);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_SLOT_CNTRL_OFFSET,
					&((*cap)->exp->pcie_exp_slot_ctrl));
			if (ret)
				goto fail;
			print_pcie_cap_slot_ctrl(dev,
				(*cap)->exp->pcie_exp_slot_ctrl);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_SLOT_STATUS_OFFSET,
					&((*cap)->exp->pcie_exp_slot_status));
			if (ret)
				goto fail;
			print_pcie_cap_slot_status(dev,
				(*cap)->exp->pcie_exp_slot_status);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_ROOT_CNTRL_OFFSET,
					&((*cap)->exp->pcie_exp_root_ctrl));
			if (ret)
				goto fail;
			print_pcie_cap_root_ctrl(dev,
				(*cap)->exp->pcie_exp_root_ctrl);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_ROOT_CAPABILITIES_OFFSET,
					&((*cap)->exp->pcie_exp_root_cap));
			if (ret)
				goto fail;
			print_pcie_cap_root_cap (dev,
			(*cap)->exp->pcie_exp_root_cap);

			ret = pci_read_config_dword(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_ROOT_STATUS_OFFSET, 
					&((*cap)->exp->pcie_exp_root_status));
			if (ret)
				goto fail;
			print_pcie_cap_root_status(dev,
				(*cap)->exp->pcie_exp_root_status);

			ret = pci_read_config_dword(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_DEV_CAPABILITIES_2_OFFSET, 
					&((*cap)->exp->pcie_exp_dev_cap_2));
			if (ret)
				goto fail;
			print_pcie_cap_dev_cap_2(dev,
				(*cap)->exp->pcie_exp_dev_cap_2);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_DEV_CNTRL_2_OFFSET, 
					&((*cap)->exp->pcie_exp_dev_ctrl_2));
			if (ret)
				goto fail;
			print_pcie_cap_dev_ctrl_2(dev,
				(*cap)->exp->pcie_exp_dev_ctrl_2);

			ret = pci_read_config_dword(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_LINK_CAPABILITIES_2_OFFSET, 
					&((*cap)->exp->pcie_exp_link_cap_2));
			if (ret)
				goto fail;
			print_pcie_cap_link_cap_2 (dev,
				(*cap)->exp->pcie_exp_link_cap_2);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_LINK_CNTRL_2_OFFSET,
					&((*cap)->exp->pcie_exp_link_ctrl_2));
			if (ret)
				goto fail;
			print_pcie_cap_link_ctrl_2(dev,
				(*cap)->exp->pcie_exp_link_ctrl_2);

			ret = pci_read_config_word(dev,
					(*cap)->exp->pcie_exp_cap_base_addr + \
					PCIE_CAP_LINK_STATUS_2_OFFSET,
					 &((*cap)->exp->pcie_exp_link_status_2));
			if (ret)
				goto fail;
			print_pcie_cap_link_status_2(dev,
				(*cap)->exp->pcie_exp_link_status_2);
			break;

		default:
			DEV_VDBG(dev, "Capabilities not defined - ID : 0x%X",
				capability_read_buf & 0xFF);
			break;

		}
		current_cap_index = get_pcie_capability_next_cap(
			capability_read_buf);

	} while(current_cap_index != 0);

	DEV_VDBG(dev, "Successfully read PCI Capabilities");
	return SUCCESS;

fail:
	release_pcie_cap_pwr_mgt(&((*cap)->pwr_mgt));
	release_pcie_cap_msi(&((*cap)->msi));
	release_pcie_cap_exp(&((*cap)->exp));
	release_pcie_cap(cap);
	DEV_ERR(dev, "Failed to read PCI Capabilities");
	return ret;
}

int read_pcie_ext_capabilities(struct pci_dev *dev, pcie_ext_cap_struct **ext_cap)
{
	int ret = 0;
	uint32_t ext_header_read_buf = 0;
	uint16_t ext_cap_ptr = PCIE_EXT_CAP_BASE_ADDR;
	pcie_ext_header_struct current_ext_header = { 0 };

	if (!dev || !ext_cap)
		return -EINVAL;

	(*ext_cap) = kzalloc(sizeof(pcie_ext_cap_struct), GFP_KERNEL);
	if (!((*ext_cap))) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*ext_cap)->ari = kmalloc(sizeof(pcie_ext_cap_ari_struct), GFP_KERNEL);
	if (!((*ext_cap)->ari)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*ext_cap)->aer = kmalloc(sizeof(pcie_ext_cap_aer_struct), GFP_KERNEL);
	if (!((*ext_cap)->aer)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*ext_cap)->dlf = kmalloc(sizeof(pcie_ext_cap_dlf_struct), GFP_KERNEL);
	if (!((*ext_cap)->dlf)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*ext_cap)->lane_mar_rec = kmalloc(sizeof(pcie_ext_cap_lane_mar_rec_struct),
		GFP_KERNEL);
	if (!((*ext_cap)->lane_mar_rec)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*ext_cap)->phy_16_gts = kmalloc(sizeof(pcie_ext_cap_phy_16_gts_struct),
		GFP_KERNEL);
	if (!((*ext_cap)->phy_16_gts)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*ext_cap)->secondary_pci = kmalloc(sizeof(pcie_ext_cap_secondary_pci_struct),
		GFP_KERNEL);
	if (!((*ext_cap)->secondary_pci)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}
	(*ext_cap)->vsec_base_addr_found = false;

#ifdef VERBOSE_DEBUG
	pcie_find_supported_ext_caps(dev);
#endif

	do {
		ret = pci_read_config_dword(dev, ext_cap_ptr , &ext_header_read_buf);
		if (ret)
			goto fail;

		current_ext_header.id = \
			get_pcie_capability_ext_cap_id(ext_header_read_buf);
		current_ext_header.version = \
			get_pcie_capability_ext_cap_version(ext_header_read_buf);
		current_ext_header.next = \
			get_pcie_capability_ext_cap_next(ext_header_read_buf);\

		DEV_VDBG(dev, "\t - ID      : 0x%X", current_ext_header.id);
		DEV_VDBG(dev, "\t - VERSION : 0x%X", current_ext_header.version);
		DEV_VDBG(dev, "\t - NEXT    : 0x%X", current_ext_header.next);

		switch (get_pcie_capability_ext_cap_id(ext_header_read_buf)) {
		case PCI_EXT_CAP_ID_ERR:

			(*ext_cap)->aer->aer_header.id  = \
				current_ext_header.id;
			(*ext_cap)->aer->aer_header.version = \
				current_ext_header.version;
			(*ext_cap)->aer->aer_header.next  = \
				current_ext_header.next;

			DEV_VDBG(dev, "PCI Express Extended Capability - Advanced Error Reporting");

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_UNCOR_ERR_STATUS_OFFSET,
				&((*ext_cap)->aer->aer_uncorrectable_err_status_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_UNCOR_ERR_MASK_OFFSET,
				&((*ext_cap)->aer->aer_uncorrectable_err_mask_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_UNCOR_ERR_SEV_OFFSET,
				&((*ext_cap)->aer->aer_uncorrectable_err_sev_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_COR_ERR_STATUS_OFFSET,
				&((*ext_cap)->aer->aer_correctable_err_status_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_COR_ERR_MASK_OFFSET,
				&((*ext_cap)->aer->aer_correctable_err_mask_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev,  ext_cap_ptr +
				PCIE_EXT_AER_ADV_ERR_CAP_CNTRL_OFFSET,
				&((*ext_cap)->aer->aer_advanced_err_cap_cntrl_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_HEADER_LOG_REG_DW1_OFFSET,
				&((*ext_cap)->aer->aer_header_log_reg[0]));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_HEADER_LOG_REG_DW2_OFFSET,
				&((*ext_cap)->aer->aer_header_log_reg[1]));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_HEADER_LOG_REG_DW3_OFFSET,
				&((*ext_cap)->aer->aer_header_log_reg[2]));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_HEADER_LOG_REG_DW4_OFFSET,
				&((*ext_cap)->aer->aer_header_log_reg[3]));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_ROOT_ERR_CMD_OFFSET,
				&((*ext_cap)->aer->aer_root_err_cmd_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_ROOT_ERR_STATUS_OFFSET,
				&((*ext_cap)->aer->aer_root_err_status_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_ERR_SRC_ID_OFFSET,
				&((*ext_cap)->aer->aer_err_source_id_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_FIRST_TLP_PREFIX_LOG_OFFSET,
				&((*ext_cap)->aer->aer_first_tlp_prefix_log_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_SECOND_TLP_PREFIX_LOG_OFFSET,
				&((*ext_cap)->aer->aer_second_tlp_prefix_log_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_THIRD_TLP_PREFIX_LOG_OFFSET,
				&((*ext_cap)->aer->aer_third_tlp_prefix_log_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_AER_FOURTH_TLP_PREFIX_LOG_OFFSET,
				&((*ext_cap)->aer->aer_fourth_tlp_prefix_log_reg));
			if (ret)
				goto fail;

			break;

		case PCI_EXT_CAP_ID_VNDR:
			(*ext_cap)->vsec_base_addr_found = true;
			(*ext_cap)->vsec_base_addr  = ext_cap_ptr;
			DEV_VDBG(dev, "VSEC base address: 0x%X",
				(*ext_cap)->vsec_base_addr );
			break;

		case PCI_EXT_CAP_ID_ARI:

			(*ext_cap)->ari->ari_header.id = current_ext_header.id;
			(*ext_cap)->ari->ari_header.version = current_ext_header.version;
			(*ext_cap)->ari->ari_header.next = current_ext_header.next;

			ret = pci_read_config_word(dev, ext_cap_ptr +
				PCIE_EXT_ARI_CAP_OFFSET,
				&((*ext_cap)->ari->ari_cap_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_word(dev, ext_cap_ptr +
				PCIE_EXT_ARI_CNTRL_OFFSET,
				&((*ext_cap)->ari->ari_cntrl_reg));
			if (ret)
				goto fail;

			break;

		case PCI_EXT_CAP_ID_SECPCI:
			(*ext_cap)->secondary_pci->secondary_pci_header.id = \
				current_ext_header.id;
			(*ext_cap)->secondary_pci->secondary_pci_header.version = \
				current_ext_header.version;
			(*ext_cap)->secondary_pci->secondary_pci_header.next = \
				current_ext_header.next;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_SEC_LINK_CNTRL3_OFFSET,
				&((*ext_cap)->secondary_pci->secondary_pci_link_cntrl3_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_SEC_LINK_ERR_STATUS_OFFSET,
				&((*ext_cap)->secondary_pci->secondary_pci_lane_err_status_reg));
			if (ret)
				goto fail;

			break;

		case PCI_EXT_CAP_ID_DLF:
			(*ext_cap)->dlf->dlf_header.id = current_ext_header.id;
			(*ext_cap)->dlf->dlf_header.version = current_ext_header.version;
			(*ext_cap)->dlf->dlf_header.next = current_ext_header.next;

			ret = pci_read_config_dword(dev, ext_cap_ptr +
				PCIE_EXT_DLF_CAP_OFFSET, &((*ext_cap)->dlf->dlf_cap_reg));
			if (ret)
				goto fail;

			break;

		case PCI_EXT_CAP_ID_PL_16GT:
			(*ext_cap)->phy_16_gts->phy_16_gts_header.id =\
				current_ext_header.id;
			(*ext_cap)->phy_16_gts->phy_16_gts_header.version =\
				current_ext_header.version;
			(*ext_cap)->phy_16_gts->phy_16_gts_header.next =\
				current_ext_header.next;

			ret = pci_read_config_dword(dev,
				ext_cap_ptr + PCIE_EXT_PHY_16_GTS_CAP_OFFSET,
				&((*ext_cap)->phy_16_gts->phy_16_gts_cap_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev,
				ext_cap_ptr + PCIE_EXT_PHY_16_GTS_CNTRL_OFFSET,
				&((*ext_cap)->phy_16_gts->phy_16_gts_cntrl_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev,
				ext_cap_ptr + PCIE_EXT_PHY_16_GTS_STATUS_OFFSET,
				&((*ext_cap)->phy_16_gts->phy_16_gts_status_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev,
				ext_cap_ptr + PCIE_EXT_PHY_16_GTS_LOCAL_PARITY_MISMATCH_STATUS_OFFSET,
				&((*ext_cap)->phy_16_gts->phy_16_gts_local_parity_mismatch_status_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev,
				ext_cap_ptr + PCIE_EXT_PHY_16_GTS_FIRST_PARITY_MISMATCH_STATUS_OFFSET,
				&((*ext_cap)->phy_16_gts->phy_16_gts_first_data_parity_mismatch_status_reg));
			if (ret)
				goto fail;

			ret = pci_read_config_dword(dev,
				ext_cap_ptr + PCIE_EXT_PHY_16_GTS_SECOND_PARITY_MISMATCH_STATUS_OFFSET,
				&((*ext_cap)->phy_16_gts->phy_16_gts_second_data_parity_mismatch_status_reg));
			if (ret)
				goto fail;

			break;

		case PCIE_LANE_MARGINING_AT_REC_ID:
			(*ext_cap)->lane_mar_rec->lane_mar_rec_header.id = \
				current_ext_header.id;
			(*ext_cap)->lane_mar_rec->lane_mar_rec_header.version = \
				current_ext_header.version;
			(*ext_cap)->lane_mar_rec->lane_mar_rec_header.next = \
				current_ext_header.next;
			break;

		default:
			DEV_VDBG(dev, "Extended Capabilities not used - ID : 0x%X",
				get_pcie_capability_ext_cap_id(ext_header_read_buf));
			break;
		}

		ext_cap_ptr = current_ext_header.next;
	} while(ext_cap_ptr != 0x000);

	DEV_VDBG(dev, "Successfully read PCI Extended Capabilities");
	return SUCCESS;

fail:
	release_pcie_ext_cap_secondary_pci(&((*ext_cap)->secondary_pci));
	release_pcie_ext_cap_phy_16_gts(&((*ext_cap)->phy_16_gts));
	release_pcie_ext_cap_lane_mar_rec(&((*ext_cap)->lane_mar_rec));
	release_pcie_ext_cap_dlf(&((*ext_cap)->dlf));
	release_pcie_ext_cap_aer(&((*ext_cap)->aer));
	release_pcie_ext_cap_ari(&((*ext_cap)->ari));
	release_pcie_ext_cap(ext_cap);
	DEV_ERR(dev, "Failed to read PCI Extended Capabilities");
	return ret;
}

int read_pcie_configuration(struct pci_dev *dev, pcie_config_struct **pcie_config)
{
	int ret = 0;
	
	if (!dev || !pcie_config)
		return -EINVAL;

	DEV_VDBG(dev, "Reading PCIe configuration");

	(*pcie_config) = kzalloc(sizeof(pcie_config_struct), GFP_KERNEL);
	if (!(*pcie_config)) {
		DEV_ERR(dev, "Unable to allocate kernel memory");
		ret = -ENOMEM;
		goto fail;
	}

	(*pcie_config)->cap = NULL;
	(*pcie_config)->ext_cap = NULL;
	(*pcie_config)->header = NULL;

	ret = read_pcie_config_header(dev, &((*pcie_config)->header));
	if (ret)
		goto fail;

	ret = read_pcie_capabilities(dev, (*pcie_config)->header,
		&((*pcie_config)->cap));
	if (ret)
		goto fail;

	ret = read_pcie_ext_capabilities(dev, &((*pcie_config)->ext_cap));
	if (ret)
		goto fail;

	print_pcie_stat(dev, (*pcie_config)->cap);
	DEV_VDBG(dev, "Successfully read PCIe configuration space");
	return SUCCESS;

fail:
	release_pcie_mem(pcie_config);
	DEV_ERR(dev, "Failed to read PCIe configuration");
	return ret;
}

int write_pcie_configuration(struct pci_dev *dev)
{
	int ret = 0;
	uint32_t gpio_reset = 0;

	if (!dev)
		return -EINVAL;

	DEV_VDBG(dev, "Setting PCIe configuration");

	/* Bus-mastering */
	DEV_VDBG(dev, "Enabling bus-mastering on the device");
	pci_set_master(dev);

	/* Maximum memory read request */
	ret = pcie_get_readrq(dev);
	if (ret < 0) {
		DEV_ERR(dev, "Failed to read the maximum memory read request");
		goto fail;
	}
	DEV_VDBG(dev, "Current maximum memory read request: %d Bytes", ret);

	if (ret > PCIE_MAX_READ_BYTES) {
		DEV_VDBG(dev, "Reducing maximum memory read request to %d Bytes",
			PCIE_MAX_READ_BYTES);
		ret = pcie_set_readrq(dev, PCIE_MAX_READ_BYTES);
		if (ret) {
			DEV_ERR(dev, "Failed to set the maximum memory read request");
			goto fail;
		}
	}

	/*
	 * Identity event over GCQ will enable the hot reload so we must make
	 * sure the PMC GPIO is cleared to prevent a reboot.
	 */
	if (PCI_FUNC(dev->devfn) == 0) {
		DEV_VDBG(dev, "Clearing GPIO reset");
		ret = write_pcie_bar(
			dev,
			PCI_GPIO_RESET_BAR,
			PCI_GPIO_RESET_OFFSET,
			1,
			&gpio_reset
		);
		if (ret) {
			DEV_ERR(dev, "Failed to clear GPIO reset");
			goto fail;
		}
	}

	DEV_VDBG(dev, "Successfully set PCIe configuration");
	return SUCCESS;

fail:
	DEV_ERR(dev, "Failed to set PCIe configuration");
	return ret;
}

/**
 * do_bar_transaction() - Callback function to perform a read/write BAR transaction.
 * @dev: PCI device struct.
 * @bar_idx: Bar number.
 * @offset: Offset within BAR.
 * @num: Number of registers to read/write.
 * @val: Buffer to be written to/read from.
 * @write: Boolean indicating if we should write data.
 * 
 * If `write` is false, we will only read. If it is true, we will write data.
 * This function does not support combined transactions.
 * 
 * Return: 0 on success, negative error code otherwise. 
 */
static int do_bar_transaction(struct pci_dev *dev, uint8_t bar_idx,
	uint64_t offset, uint32_t num, uint32_t *val, bool write)
{
	int i = 0;
	int ret = 0;
	bool req = false;
	struct pf_dev_struct *pf_dev = NULL;
	struct bar_header_struct *bar = NULL;
	void __iomem *virt_addr = NULL;

	if (!dev || !val || (num == 0) || (bar_idx > NUM_PCIE_BAR))
		return -EINVAL;

	pf_dev = dev_get_drvdata(&dev->dev);

	if (!pf_dev)
		return -EINVAL;
	
	bar = &(pf_dev->pcie_config->header->bar[bar_idx]);

	if ((bar->len == 0) || ((offset + num) > bar->len))
		return -EFAULT;  /* Bad address */

	/* Try to request region if it hasn't already been requested. */
	if (!bar->requested) {
		if (pci_request_region(dev, bar_idx, PCIE_BAR_NAME[bar_idx]))
			return -EBUSY;

		req = true;
		bar->requested = true;
	}

	virt_addr = pci_iomap_range(
		dev,
		bar_idx,
		offset,
		num * sizeof(uint32_t)
	);

	if (!virt_addr) {
		ret = -EIO;
		goto release_region;
	}
	
	for (i = 0; i < num; i++) {
		if (write)
			iowrite32(val[i], virt_addr + (sizeof(uint32_t) * i));
		else
			val[i] = ioread32(virt_addr + (sizeof(uint32_t) * i));
	}

	pci_iounmap(dev, virt_addr);

release_region:
	if (req) {
		bar->requested = false;
		pci_release_region(dev, bar_idx);
	}

	return ret;
}

/*
 * Read from a PCI BAR.
 */
int read_pcie_bar(struct pci_dev *dev, uint8_t bar_idx, uint64_t offset,
	uint32_t num, uint32_t *val)
{
	if (!dev || !val || (num == 0))
		return -EINVAL;

	return do_bar_transaction(
		dev,
		bar_idx,
		offset,
		num,
		val,
		false
	);
}

/*
 * Write to a PCI BAR.
 */
int write_pcie_bar(struct pci_dev *dev, uint8_t bar_idx, uint64_t offset,
	uint32_t num, uint32_t *val)
{
	if (!dev || !val || (num == 0))
		return -EINVAL;

	return do_bar_transaction(
		dev,
		bar_idx,
		offset,
		num,
		val,
		true
	);
}

bool is_supported_pcie_device_id(uint16_t pcie_device_id)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(SUPPORTED_PCIE_DEVICE_ID); i++) {
		if (pcie_device_id == SUPPORTED_PCIE_DEVICE_ID[i])
			return true;
	}
	return false;
}

void release_pcie_ext_cap_secondary_pci(
	pcie_ext_cap_secondary_pci_struct **secondary_pci)
{
	if (secondary_pci && *secondary_pci) {
		kfree(*secondary_pci);
		*secondary_pci = NULL;
	}
}

void release_pcie_ext_cap_phy_16_gts(
	pcie_ext_cap_phy_16_gts_struct **phy_16_gts)
{
	if (phy_16_gts && *phy_16_gts) {
		kfree(*phy_16_gts);
		*phy_16_gts = NULL;
	}
}

void release_pcie_ext_cap_lane_mar_rec(
	pcie_ext_cap_lane_mar_rec_struct **lane_mar_rec)
{
	if (lane_mar_rec && *lane_mar_rec) {
		kfree(*lane_mar_rec);
		*lane_mar_rec = NULL;
	}
}

void release_pcie_ext_cap_dlf(pcie_ext_cap_dlf_struct **dlf)
{
	if (dlf && *dlf) {
		kfree(*dlf);
		*dlf = NULL;
	}
}

void release_pcie_ext_cap_aer(pcie_ext_cap_aer_struct **aer)
{
	if (aer && *aer) {
		kfree(*aer);
		*aer = NULL;
	}
}

void release_pcie_ext_cap_ari(pcie_ext_cap_ari_struct **ari)
{
	if (ari && *ari) {
		kfree(*ari);
		*ari = NULL;
	}
}

void release_pcie_ext_cap(pcie_ext_cap_struct **ext_cap)
{
	if (ext_cap && *ext_cap) {
		kfree(*ext_cap);
		*ext_cap = NULL;
	}
}

void release_pcie_cap_pwr_mgt(pcie_pwr_mgt_cap_struct **pwr_mgt)
{
	if (pwr_mgt && *pwr_mgt) {
		kfree(*pwr_mgt);
		*pwr_mgt = NULL;
	}
}

void release_pcie_cap_msi(pcie_msi_cap_struct **msi)
{
	if (msi && *msi) {
		kfree(*msi);
		*msi = NULL;
	}
}

void release_pcie_cap_exp(pcie_exp_cap_struct **exp)
{
	if (exp && *exp) {
		kfree(*exp);
		*exp = NULL;
	}
}

void release_pcie_cap(pcie_cap_struct **cap)
{
	if (cap && *cap) {
		kfree(*cap);
		*cap = NULL;
	}
}

void release_pcie_config_header(pcie_header_struct **pcie_header)
{
	if (pcie_header && *pcie_header) {
		kfree(*pcie_header);
		*pcie_header = NULL;
	}
}

void release_pcie_config(pcie_config_struct **pcie_config)
{
	if (pcie_config && *pcie_config) {
		kfree(*pcie_config);
		(*pcie_config) = NULL;
	}
}

void release_pcie_mem(pcie_config_struct **pcie_config)
{
	if (pcie_config && *pcie_config) {
		if ((*pcie_config)->ext_cap) {
			release_pcie_ext_cap_secondary_pci(
				&((*pcie_config)->ext_cap->secondary_pci));

			release_pcie_ext_cap_phy_16_gts(
				&((*pcie_config)->ext_cap->phy_16_gts));

			release_pcie_ext_cap_lane_mar_rec(
				&((*pcie_config)->ext_cap->lane_mar_rec));

			release_pcie_ext_cap_dlf(
				&((*pcie_config)->ext_cap->dlf));

			release_pcie_ext_cap_aer(
				&((*pcie_config)->ext_cap->aer));

			release_pcie_ext_cap_ari(
				&((*pcie_config)->ext_cap->ari));
		}

		release_pcie_ext_cap(&((*pcie_config)->ext_cap));

		if ((*pcie_config)->cap) {
			release_pcie_cap_pwr_mgt(&((*pcie_config)->cap->pwr_mgt));
			release_pcie_cap_msi(&((*pcie_config)->cap->msi));
			release_pcie_cap_exp(&((*pcie_config)->cap->exp));
		}

		release_pcie_cap(&((*pcie_config)->cap));
		release_pcie_config_header(&((*pcie_config)->header));
	}
	release_pcie_config(pcie_config);
}
