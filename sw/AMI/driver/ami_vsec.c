// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_vsec.c - This file contains logic to parse PCI XILINX VSEC.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include "ami_vsec.h"
#include "ami_pci_dbg.h"

/* VSEC is only applicable for xilinx-vendor boards
 * No need to check the vendor ID is PCIE_VENDOR_ID_XILINX here prior to VSEC
 * discovery as only Xilinx card are used in MODULE_DEVICE_TABLE */

int read_logic_uuid(struct pci_dev *dev, endpoints_struct **endpoints)
{
	int ret = 0;
	int i = 0;
	void __iomem *virt_addr = NULL;

	if (!dev || !endpoints)
		return -EINVAL;

	if (!(*endpoints)->uuid0_rom.found) {
		DEV_ERR(dev, "Endpoint %s not found in HW discovery",
			XILINX_ENDPOINT_NAME_UUID0_ROM);
		ret = -ENODEV;
		goto fail;
	}

	ret = pci_request_region(dev, (*endpoints)->uuid0_rom.bar_num, 
			PCIE_BAR_NAME[(*endpoints)->uuid0_rom.bar_num]);
	if (ret) {
		DEV_ERR(dev, "Could not request %s region (%s)", 
			PCIE_BAR_NAME[(*endpoints)->uuid0_rom.bar_num],
			(*endpoints)->uuid0_rom.name);
		ret = -EIO;
		goto fail;
	}

	virt_addr = pci_iomap_range(dev, (*endpoints)->uuid0_rom.bar_num, 
			(*endpoints)->uuid0_rom.start_addr,
			(*endpoints)->uuid0_rom.bar_len);
	if (!virt_addr) {
		DEV_ERR(dev, "Could not map %s endpoint into virtual memory at start address 0x%llX", 
			(*endpoints)->uuid0_rom.name, (*endpoints)->uuid0_rom.start_addr);
		ret = -EIO;
		goto release_bar;
	}

	(*endpoints)->logic_uuid_str[0] = '\0';
	for (i = ARRAY_SIZE((*endpoints)->logic_uuid) - 1; i >= 0; i--) {
		(*endpoints)->logic_uuid[i] = \
			ioread32(virt_addr + sizeof(uint32_t) * i);

		sprintf((*endpoints)->logic_uuid_str + \
			strlen((*endpoints)->logic_uuid_str),
			"%08x", (*endpoints)->logic_uuid[i]);
	}

	DEV_INFO(dev, "Logic uuid = %s", (*endpoints)->logic_uuid_str);
	pci_iounmap(dev, virt_addr);
	pci_release_region(dev, (*endpoints)->uuid0_rom.bar_num);

	return SUCCESS;

release_bar:
	pci_release_region(dev, (*endpoints)->uuid0_rom.bar_num);

fail:
	DEV_ERR(dev, "Failed to read logic UUID");
	return ret;
}

int read_vsec(struct pci_dev *dev, uint32_t vsec_base_addr,
	endpoints_struct **endpoints)
{
	int ret = 0;
	int i = 0;
	uint32_t read_buf = 0;
	bool end_of_table = false;
	uint32_t table_length = 0, table_entry_size = 0;
	uint8_t ep_type = 0, ep_bar_num = 0;
	uint64_t ep_start_addr = 0;
	void * __iomem hw_discovery_virt_addr = NULL;
	uint8_t pcie_function_num = 0;
	uint16_t vsec_len = 0;

	if (!dev || !endpoints)
		return -EINVAL;

	pcie_function_num = PCI_FUNC(dev->devfn);

	DEV_VDBG(dev, "Reading vendor specific information for PF %d",
		pcie_function_num);

	(*endpoints) = kzalloc(sizeof(endpoints_struct), GFP_KERNEL);
	if (!(*endpoints)) {
		DEV_ERR(dev, "Failed to allocate memory for endpoints");
		ret = -ENOMEM;
		goto fail;
	}

	/*
	 * Additional List of Features (ALF) VSEC
	 *
	 *  ----------------------------------------------------------------------
	 * | Next Cap [31:20] | Cap Version [19:16] | PCIe Extended Cap ID [15:0] |
	 *  ----------------------------------------------------------------------
	 * | VSEC Len [31:20] | VSEC Rev [19:16]    | VSEC ID [15:0]              |
	 *  ----------------------------------------------------------------------
	 * |             Low Address [31:4]         | Rsvd [3] | Bar Index [2:0]  |
	 *  ----------------------------------------------------------------------
	 * |                    High Address [31:0] effective [63:32]             |
	 *  ----------------------------------------------------------------------
	 */
	ret = pci_read_config_dword(dev, vsec_base_addr + ALF_VSEC_HDR_OFFSET,
		&read_buf);
	if (ret) {
		ret = -EIO;
		goto fail;
	}
	vsec_len = (read_buf >> ALF_VSEC_LEN_OFFSET) & ALF_VSEC_LEN_MASK;
	DEV_VDBG(dev, "vsec len : 0x%X", vsec_len);

	ret = pci_read_config_dword(dev, vsec_base_addr + ALF_VSEC_FIELD1_OFFSET,
		&read_buf);
	if (ret) {
		ret = -EIO;
		goto fail;
	}

	(*endpoints)->hw_discovery.bar_num = \
		(read_buf >> ALF_VSEC_BAR_IDX_OFFSET) & ALF_VSEC_BAR_IDX_MASK;
	(*endpoints)->hw_discovery.start_addr = \
		(read_buf >> ALF_VSEC_OFF_LOW_OFFSET) & ALF_VSEC_OFF_LOW_MASK;

	if (vsec_len == ALF_VSEC_HIGH_OFFSET_IMPLEMENTED) {
		/* Read the offset high register */
		ret = pci_read_config_dword(dev,
			vsec_base_addr + ALF_VSEC_FIELD2_OFFSET, &read_buf);
		if (ret) {
			ret = -EIO;
			goto fail;
		}

		(*endpoints)->hw_discovery.start_addr |= \
			((uint64_t)((read_buf >> ALF_VSEC_OFF_HIGH_OFFSET) &
			ALF_VSEC_OFF_HIGH_MASK) << ALF_VSEC_OFF_LOW_LEN);
	}

	(*endpoints)->hw_discovery.bar_len = XILINX_ENDPOINT_BAR_LEN_HW_DISCOVERY;
	(*endpoints)->hw_discovery.end_addr = \
		(*endpoints)->hw_discovery.start_addr +
		(*endpoints)->hw_discovery.bar_len - 1;

	if (pcie_function_num == 0) {
		strcpy((*endpoints)->hw_discovery.name,
			XILINX_ENDPOINT_NAME_HW_DISCOVERY_PF0);
	} else if (pcie_function_num == 1) {
		strcpy((*endpoints)->hw_discovery.name,
			XILINX_ENDPOINT_NAME_HW_DISCOVERY_PF1);
	}

	print_endpoint_info(dev, (*endpoints)->hw_discovery);

	/*
	 * Traverse the Xilinx Capabilities
	 *
	 * Xilinx Capabilities Format
	 *  ------------------------------------------------------------------------------
	 * | Rsvd[31:29] | Last Capability[28] | Format Revision[27:20] | Format ID[19:0] |
	 *  ------------------------------------------------------------------------------
	 * |                        Length[31:0]                                          |
	 *  ------------------------------------------------------------------------------
	 * | Rsvd[31:8]                           |       Entry Size[7:0]                 |
	 *  ------------------------------------------------------------------------------
	 * |                        Rsvd[31:0]                                            |
	 *  ------------------------------------------------------------------------------
	 *
	 *  @Format ID -
	 *  0x0 - Reserved and should not be used
	 *  0x1 - BAR Layout Register Format (Currently, only valid value format)
	 *  0xEF100 - Host Interface (Reserved)
	 *  0xFFFFE - Continued Capabilities Address
	 *  0xFFFFF - Termination of the list of capabilities
	 *
	 *  @Format Revision - 1 (fixed)
	 *
	 *  @Entry Size -
	 *  0x08 - 8 byte entry size
	 *  0x10 - 16 byte entry size
	 *  0x20 - 32 byte entry size
	 *  0x40 - 64 byte entry size
	 *  0x80 - 128 byte entry size
	 */

	/* Map the Header Registers and Table Format */
	hw_discovery_virt_addr = pci_iomap_range(dev,
			(*endpoints)->hw_discovery.bar_num,
			(*endpoints)->hw_discovery.start_addr,
			XILINX_HW_DISCOVERY_TABLE_OFFSET);

	if (!hw_discovery_virt_addr) {
		DEV_ERR(dev, "Failed to map bar_layout into memory");
		ret = -EIO;
		goto fail;
	}

	DEV_VDBG(dev, "HW discovery Virt Addr : %p , Device Addr : %llx",
		hw_discovery_virt_addr,
		(*endpoints)->hw_discovery.start_addr);

	/* Calculate the length of all the table entries excluding header */
	read_buf = ioread32(hw_discovery_virt_addr + \
		XILINX_HW_DISCOVERY_LEN_OFFSET);

	table_length = (read_buf & XILINX_HW_DISCOVERY_LEN_MASK) - \
		XILINX_HW_DISCOVERY_TABLE_OFFSET;

	read_buf = ioread32(hw_discovery_virt_addr + \
		XILINX_HW_DISCOVERY_ENTRY_SIZE_OFFSET);

	table_entry_size = read_buf & XILINX_HW_DISCOVERY_ENTRY_SIZE_MASK;

	DEV_VDBG(dev, "table_length : 0x%X, table_entry_size : 0x%X",
		table_length, table_entry_size);
	
	/* Do some basic sanity checking */
	if ((table_length == ((uint32_t)-1)) ||
			(table_entry_size > XILINX_HW_DISCOVERY_ENTRY_SIZE_MAX) ||
			((table_length % table_entry_size) != 0)) {
		DEV_ERR(dev, "Invalid table size");
		ret = -EINVAL;
		goto fail;
	}

	/* Unmap the memory mapped BAR region */
	DEV_VDBG(dev, "Unmapping BAR Entry");
	pci_iounmap(dev, hw_discovery_virt_addr);

	/* Map the Table Entry 1 ... n */
	hw_discovery_virt_addr = pci_iomap_range(dev,
			(*endpoints)->hw_discovery.bar_num,
			(*endpoints)->hw_discovery.start_addr + \
			XILINX_HW_DISCOVERY_TABLE_OFFSET,
			table_length);

	if (!hw_discovery_virt_addr) {
		DEV_ERR(dev, "Failed to map bar table entry into memory");
		ret = -EIO;
		goto fail;
	}

	/*
	 *                                         Table Entry
	 *
	 *      -----------------------------------------------------------------------------------------
	 *     |    Low Addr Offset[31:16]   |  Bar Number[15:13]   |  Type Revision[12:8]  | Type[7:0]  |
	 *      -----------------------------------------------------------------------------------------
	 *     |                                High Addr Offset[31:0]                                   |
	 *      -----------------------------------------------------------------------------------------
	 *     | Rsvd[31:24]        |  Major[23:16]       | Minor[15:8]       | Version Type[7:0]        |
	 *      -----------------------------------------------------------------------------------------
	 *     |                                Rsvd[31:0]                                               |
	 *      -----------------------------------------------------------------------------------------
	 */

	/* Traverse all the table entry */
	for (i = 0; i < table_length; i += table_entry_size) {
		if (end_of_table)
			break;

		DEV_VDBG(dev, "Table entry device base addr : 0x%llX (virt addr = %p)",
			(*endpoints)->hw_discovery.start_addr +
			XILINX_HW_DISCOVERY_TABLE_OFFSET + i,
			hw_discovery_virt_addr + i);

		read_buf = ioread32(hw_discovery_virt_addr + i +
			XILINX_HW_DISCOVERY_TABLE_ENTRY_ROW_0_OFFSET);

		ep_type = (read_buf >> XILINX_HW_DISCOVERY_TABLE_TYPE_OFFSET) &
			XILINX_HW_DISCOVERY_TABLE_TYPE_MASK;

		ep_bar_num  = \
			(read_buf >> XILINX_HW_DISCOVERY_TABLE_EP_BAR_NUM_OFFSET) &
			XILINX_HW_DISCOVERY_TABLE_EP_BAR_IDX_MASK; /* BAR Num where the target aperture is located */

		ep_start_addr = \
			(read_buf >> XILINX_HW_DISCOVERY_TABLE_OFF_LOW_OFFSET) &
			XILINX_HW_DISCOVERY_TABLE_OFF_LOW_MASK;

		read_buf = ioread32(hw_discovery_virt_addr + i +
			XILINX_HW_DISCOVERY_TABLE_ENTRY_ROW_1_OFFSET);

		ep_start_addr |= \
			((uint64_t)((read_buf >> XILINX_HW_DISCOVERY_TABLE_OFF_HIGH_OFFSET) &
				XILINX_HW_DISCOVERY_TABLE_OFF_HIGH_MASK) <<
			XILINX_HW_DISCOVERY_TABLE_OFF_LOW_LEN);

		DEV_VDBG(dev,
			"ep_type : 0x%X, ep_bar_num : 0x%X, ep_start_addr : 0x%llX",
			ep_type, ep_bar_num, ep_start_addr);

		switch(ep_type) {
		case XILINX_TABLE_TYPE_UUID0_ROM:
			(*endpoints)->uuid0_rom.found = \
				true;

			(*endpoints)->uuid0_rom.bar_num = \
				ep_bar_num;

			(*endpoints)->uuid0_rom.start_addr = \
				ep_start_addr;

			(*endpoints)->uuid0_rom.bar_len = \
				XILINX_ENDPOINT_BAR_LEN_UUID0_ROM;

			(*endpoints)->uuid0_rom.end_addr = \
				(*endpoints)->uuid0_rom.start_addr +
				(*endpoints)->uuid0_rom.bar_len - 1;

			strcpy((*endpoints)->uuid0_rom.name,
				XILINX_ENDPOINT_NAME_UUID0_ROM);

			print_endpoint_info(dev, (*endpoints)->uuid0_rom);
			break;

		case XILINX_TABLE_TYPE_INTER_PF_MAILBOX:
			(*endpoints)->interpf_mailbox.found = \
				true;

			(*endpoints)->interpf_mailbox.bar_num = \
				ep_bar_num;

			(*endpoints)->interpf_mailbox.start_addr = \
				ep_start_addr;

			(*endpoints)->interpf_mailbox.bar_len = \
				XILINX_ENDPOINT_BAR_LEN_INTERPF_MAILBOX;

			(*endpoints)->interpf_mailbox.end_addr = \
				(*endpoints)->interpf_mailbox.start_addr +
				(*endpoints)->interpf_mailbox.bar_len - 1;

			if (pcie_function_num == 0) {
				strcpy((*endpoints)->interpf_mailbox.name,
					XILINX_ENDPOINT_NAME_INTERPF_MAILBOX_PF0);
			} else if (pcie_function_num == 1) {
				strcpy((*endpoints)->interpf_mailbox.name,
					XILINX_ENDPOINT_NAME_INTERPF_MAILBOX_PF1);
			}

			print_endpoint_info(dev, (*endpoints)->interpf_mailbox);
			break;

		case XILINX_TABLE_TYPE_GCQ:
			(*endpoints)->gcq.found = \
				true;

			(*endpoints)->gcq.bar_num = \
				ep_bar_num;

			(*endpoints)->gcq.start_addr = \
				ep_start_addr;

			(*endpoints)->gcq.bar_len = \
				XILINX_ENDPOINT_BAR_LEN_GCQ;

			(*endpoints)->gcq.end_addr = \
				(*endpoints)->gcq.start_addr +
				(*endpoints)->gcq.bar_len - 1;

			strcpy((*endpoints)->gcq.name, XILINX_ENDPOINT_NAME_GCQ);
			print_endpoint_info(dev, (*endpoints)->gcq);
			break;

		case XILINX_TABLE_TYPE_GCQ_PAYLOAD:
			(*endpoints)->gcq_payload.found = \
				true;

			(*endpoints)->gcq_payload.bar_num = \
				ep_bar_num;

			(*endpoints)->gcq_payload.start_addr = \
				ep_start_addr;

			(*endpoints)->gcq_payload.bar_len = \
				XILINX_ENDPOINT_BAR_LEN_GCQ_PAYLOAD;

			(*endpoints)->gcq_payload.end_addr = \
				(*endpoints)->gcq_payload.start_addr +
				(*endpoints)->gcq_payload.bar_len - 1;

			strcpy((*endpoints)->gcq_payload.name,
				XILINX_ENDPOINT_NAME_GCQ_PAYLOAD);
			print_endpoint_info(dev, (*endpoints)->gcq_payload);
			break;

		case XILINX_TABLE_TYPE_END_OF_TABLE:
			end_of_table = true;
			DEV_VDBG(dev, "End of table");
			break;

		default:
			DEV_ERR(dev,
				"Found Unsupported or Reserved Type Endpoint: 0x%X",
				ep_type);
			ret = -EINVAL;
			goto fail;
			break;
		}
	}

	DEV_VDBG(dev, "Unmapping HW discovery BAR memory");
	pci_iounmap(dev, hw_discovery_virt_addr);
	hw_discovery_virt_addr = 0;

	if (pcie_function_num == 0) {
		ret = read_logic_uuid(dev, endpoints);
		if (ret)
			goto fail;
	}

	DEV_VDBG(dev, "Successfully read Vendor Specific Region (VSEC)");
	return SUCCESS;

fail:
	if (hw_discovery_virt_addr)
		pci_iounmap(dev, hw_discovery_virt_addr);

	release_vsec_mem(endpoints);
	DEV_ERR(dev, "Failed to read Vendor Specific Region (VSEC)");
	return ret;
}

void release_endpoints(endpoints_struct **endpoints)
{
	if (endpoints && *endpoints) {
		kfree(*endpoints);
		*endpoints = NULL;
	}
}

void release_vsec_mem(endpoints_struct **endpoints)
{
	if (!endpoints)
		release_endpoints(endpoints);
}
