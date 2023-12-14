// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_vsec.h - This file contains definitions to parse PCI XILINX VSEC.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_VSEC_H
#define AMI_VSEC_H

#include <linux/types.h>
#include <linux/pci.h>

#include "ami.h"
#include "ami_pcie.h"

/* Additional List of Features (ALF) VSEC Register Space */
#define ALF_VSEC_HDR_OFFSET                     0x04
#define ALF_VSEC_FIELD1_OFFSET                  0x08
#define ALF_VSEC_FIELD2_OFFSET                  0x0C

#define ALF_VSEC_HIGH_OFFSET_IMPLEMENTED        0x10
#define ALF_VSEC_HIGH_OFFSET_NOT_IMPLEMENTED    0x0C

#define XILINX_CAP_HDR_REG_LAST_CAP_OFFSET      0x1C

#define ALF_VSEC_LEN_OFFSET  20
#define ALF_VSEC_LEN_MASK    0xFFF

#define ALF_VSEC_BAR_IDX_OFFSET  0
#define ALF_VSEC_BAR_IDX_MASK    0x07

#define ALF_VSEC_OFF_LOW_OFFSET  0
#define ALF_VSEC_OFF_LOW_MASK    0xFFFFFFF0
#define ALF_VSEC_OFF_LOW_LEN     32

#define ALF_VSEC_OFF_HIGH_OFFSET  0
#define ALF_VSEC_OFF_HIGH_MASK    0xFFFFFFFF

#define XILINX_HW_DISCOVERY_LEN_OFFSET  0x4
#define XILINX_HW_DISCOVERY_LEN_MASK  0xFFFFFFFF

#define XILINX_HW_DISCOVERY_ENTRY_SIZE_MAX      0x80
#define XILINX_HW_DISCOVERY_ENTRY_SIZE_OFFSET   0x8
#define XILINX_HW_DISCOVERY_ENTRY_SIZE_MASK     0xFF

#define XILINX_HW_DISCOVERY_TABLE_OFFSET  0x10

#define XILINX_HW_DISCOVERY_TABLE_ENTRY_ROW_0_OFFSET  0x0
#define XILINX_HW_DISCOVERY_TABLE_ENTRY_ROW_1_OFFSET  0x4

#define XILINX_HW_DISCOVERY_TABLE_TYPE_OFFSET  0
#define XILINX_HW_DISCOVERY_TABLE_TYPE_MASK    0xFF

#define XILINX_HW_DISCOVERY_TABLE_EP_BAR_NUM_OFFSET  13
#define XILINX_HW_DISCOVERY_TABLE_EP_BAR_IDX_MASK    0x07

#define XILINX_HW_DISCOVERY_TABLE_OFF_LOW_OFFSET  16
#define XILINX_HW_DISCOVERY_TABLE_OFF_LOW_MASK    0xFFFF
#define XILINX_HW_DISCOVERY_TABLE_OFF_LOW_LEN     16 /* TODO get from XILINX_HW_DISCOVERY_TABLE_OFF_LOW_MASK */

#define XILINX_HW_DISCOVERY_TABLE_OFF_HIGH_OFFSET  0
#define XILINX_HW_DISCOVERY_TABLE_OFF_HIGH_MASK    0xFFFFFFFF

#define XILINX_HW_DISCOVERY_TABLE_INSTANCE_ID_OFFSET  8
#define XILINX_HW_DISCOVERY_TABLE_INSTANCE_ID_MASK    0x0f

/* CG TODO: Get this from hw design metadata or read from card? */
#define XILINX_ENDPOINT_BAR_LEN_HW_DISCOVERY    0x1000      /* 4k */
#define XILINX_ENDPOINT_BAR_LEN_UUID0_ROM       0x1000      /* 4k */
#define XILINX_ENDPOINT_BAR_LEN_INTERPF_MAILBOX 0x1000      /* 4k */
#define XILINX_ENDPOINT_BAR_LEN_GCQ             0x1000      /* 4k */
#define XILINX_ENDPOINT_BAR_LEN_GCQ_PAYLOAD     0x8000000   /* 128M */

/* CG TODO: Get this from hw design metadata */
#define XILINX_ENDPOINT_NAME_HW_DISCOVERY_PF0       "ep_bar_layout_mgmt_00"
#define XILINX_ENDPOINT_NAME_HW_DISCOVERY_PF1       "ep_bar_layout_user_00"
#define XILINX_ENDPOINT_NAME_UUID0_ROM              "ep_blp_rom_00"
#define XILINX_ENDPOINT_NAME_INTERPF_MAILBOX_PF0    "ep_mailbox_mgmt_00"
#define XILINX_ENDPOINT_NAME_INTERPF_MAILBOX_PF1    "ep_mailbox_user_00"
#define XILINX_ENDPOINT_NAME_GCQ                    "ep_gcq_mgmt_to_rpu_sq_pi_00"
#define XILINX_ENDPOINT_NAME_GCQ_PAYLOAD            "ep_gcq_payload_mgmt_00"

#define XILINX_LOGIC_UUID_SIZE_BYTES                16

enum xil_table_type {
	XILINX_TABLE_TYPE_RSVD              = 0x00,
	XILINX_TABLE_TYPE_PCIE_IP_VERSION,
	XILINX_TABLE_TYPE_QDMA_GLOBAL_CSR,
	XILINX_TABLE_TYPE_PCIE_MSIX_TABLE,
	XILINX_TABLE_TYPE_PCIE_MAILBOX,
	XILINX_TABLE_TYPE_QDMA_NOTIF,
	XILINX_TABLE_TYPE_VIRTIO_0_9_5_CSR  = 0x10,
	XILINX_TABLE_TYPE_VIRTIO_DEV_ICE,
	XILINX_TABLE_TYPE_VIRTIO_ISR,
	XILINX_TABLE_TYPE_VIRTIO_COM_CONFIG,
	XILINX_TABLE_TYPE_VIRTIO_NOTIF,
	XILINX_TABLE_TYPE_NIC_FN_CTRL_WIN   = 0x20,
	XILINX_TABLE_TYPE_UUID0_ROM         = 0x50,
	XILINX_TABLE_TYPE_INTER_PF_MAILBOX  = 0x53,
	XILINX_TABLE_TYPE_GCQ,
	XILINX_TABLE_TYPE_GCQ_PAYLOAD,
	XILINX_TABLE_TYPE_ADDITIONAL_CAP    = 0xFE,
	XILINX_TABLE_TYPE_END_OF_TABLE      = 0xFF,
};

typedef struct {
	endpoint_info_struct hw_discovery;       /* PF0 and PF1 */
	endpoint_info_struct uuid0_rom;          /* PF0 only */
	endpoint_info_struct interpf_mailbox;    /* PF0 and PF1 */
	endpoint_info_struct gcq;                /* PF0 only */
	endpoint_info_struct gcq_payload;        /* PF0 only */

	uint32_t logic_uuid      [XILINX_LOGIC_UUID_SIZE_BYTES/sizeof(uint32_t)];   /* PF0 only */
	char     logic_uuid_str  [XILINX_LOGIC_UUID_SIZE_BYTES*2+1];                /* PF0 only */
} endpoints_struct;

int read_logic_uuid(struct pci_dev *dev, endpoints_struct **endpoints);
int read_vsec(struct pci_dev *dev, uint32_t vsec_base_addr,
	endpoints_struct **endpoints);

void release_endpoints(endpoints_struct **endpoints);
void release_vsec_mem(endpoints_struct **endpoints);

#endif /* AMI_VSEC_H */
