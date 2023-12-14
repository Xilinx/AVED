// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_pcie.h - This file contains PCI reading/writing definitions.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_PCIE_H
#define AMI_PCIE_H

#include <linux/types.h>
#include <linux/pci.h>

#define DEV_ERR(pcie_dev, fmt, arg...)       dev_err(&(pcie_dev->dev),      "ERROR           : " fmt "\n", ##arg)
#define DEV_ERR_ONCE(pcie_dev, fmt, arg...)  dev_err_once(&(pcie_dev->dev), "ERROR           : " fmt "\n", ##arg)
#define DEV_INFO(pcie_dev, fmt, arg...)      dev_info(&(pcie_dev->dev),     "INFO            : " fmt "\n", ##arg)
#define DEV_WARN(pcie_dev, fmt, arg...)      dev_warn(&(pcie_dev->dev),     "WARNING         : " fmt "\n", ##arg)
#define DEV_CRIT_WARN(pcie_dev, fmt, arg...) dev_warn(&(pcie_dev->dev),     "CRITICAL WARNING: " fmt "\n", ##arg)
#define DEV_VDBG(pcie_dev, fmt, arg...)      dev_vdbg(&(pcie_dev->dev),     "DEBUG           : " fmt "\n", ##arg)

#define PCIE_VENDOR_ID_XILINX 0x10ee
#define PCIE_SUBSYSTEM_DEVICE_ID_XILINX 0x000e

/**
 * enum AMI_PCIE_DEVICE_ID - List of supported PCIe ID's
 * @AMI_PCIE_DEVICE_ID_V70_PF0: V70 PF0
 * @AMI_PCIE_DEVICE_ID_V70_PF1: V70 PF1
 * @AMI_PCIE_DEVICE_ID_VCK5000_PF0: VCK5000 PF0
 * @AMI_PCIE_DEVICE_ID_VCK5000_PF1: VCK5000 PF1
 * @AMI_PCIE_DEVICE_ID_V80: V80
 * @AMI_PCIE_DEVICE_ID_V80P: V80P
 *
 * Note, if you want the driver to pick up these ID's they must be added to
 * the array at the top of `ami_pcie.c`.
 */
enum AMI_PCIE_DEVICE_ID {
	AMI_PCIE_DEVICE_ID_V70_PF0     = 0x5094,
	AMI_PCIE_DEVICE_ID_V70_PF1     = 0x5095,
	AMI_PCIE_DEVICE_ID_VCK5000_PF0 = 0x5048,
	AMI_PCIE_DEVICE_ID_VCK5000_PF1 = 0x5049,
	AMI_PCIE_DEVICE_ID_V80         = 0x50B4,
	AMI_PCIE_DEVICE_ID_V80P        = 0x50BC,
};

#define PCIE_VENDOR_ID          PCIE_VENDOR_ID_XILINX
#define PCIE_SUBVENDOR_ID       PCIE_VENDOR_ID_XILINX

#define PCIE_DEVICE_ID          PCI_ANY_ID
#define PCIE_SUBDEVICE_ID       PCIE_SUBSYSTEM_DEVICE_ID_XILINX
#define PCIE_CLASS_ID           PCI_ANY_ID

#define MSI_32_BIT_ADDRESSING 0x00
#define MSI_64_BIT_ADDRESSING 0x01

#define CAP_NOT_FOUND 0

#define PCI_GPIO_RESET_OFFSET               (0x1040000)
#define PCI_GPIO_RESET_BAR                  (0)

/* Command Register Offsets */
#define  PCI_COMMAND_IO_OFFSET              0x0
#define  PCI_COMMAND_MEMORY_OFFSET          0x1
#define  PCI_COMMAND_MASTER_OFFSET          0x2
#define  PCI_COMMAND_SPECIAL_OFFSET         0x3
#define  PCI_COMMAND_INVALIDATE_OFFSET      0x4
#define  PCI_COMMAND_VGA_PALETTE_OFFSET     0x5
#define  PCI_COMMAND_PARITY_OFFSET          0x6
#define  PCI_COMMAND_WAIT_OFFSET            0x7
#define  PCI_COMMAND_SERR_OFFSET            0x8
#define  PCI_COMMAND_FAST_BACK_OFFSET       0x9
#define  PCI_COMMAND_INTX_DISABLE_OFFSET    0xA

/* Status Register Offsets */
#define  PCI_STATUS_IMM_READY_OFFSET        0x0
#define  PCI_STATUS_INTERRUPT_OFFSET        0x3
#define  PCI_STATUS_CAP_LIST_OFFSET         0x4
#define  PCI_STATUS_66MHZ_OFFSET            0x5
#define  PCI_STATUS_FAST_BACK_OFFSET        0x7
#define  PCI_STATUS_PARITY_OFFSET           0x8
#define  PCI_STATUS_DEVSEL_MASK_OFFSET      0x9
#define  PCI_STATUS_SIG_TARGET_ABORT_OFFSET 0xB
#define  PCI_STATUS_REC_TARGET_ABORT_OFFSET 0xC
#define  PCI_STATUS_REC_MASTER_ABORT_OFFSET 0xD
#define  PCI_STATUS_SIG_SYSTEM_ERROR_OFFSET 0xE
#define  PCI_STATUS_DETECTED_PARITY_OFFSET  0xF

/* Class Revision Register Offsets */
#define PCI_CLASS_REVISION_ID           0x0
#define PCI_CLASS_REVISION_PROG_IF      0x8
#define PCI_CLASS_REVISION_SUBCLASS     0x10
#define PCI_CLASS_REVISION_CLASSCODE    0x18

#define PCI_CLASS_REVISION_ID_MASK              0xFF
#define PCI_CLASS_REVISION_PROG_IF_MASK         0xFF
#define PCI_CLASS_REVISION_SUBCLASS_MASK        0xFF
#define PCI_CLASS_REVISION_CLASSCODE_MASK       0xFF

#define CLASS_CODE_PROCESSING_ACCELERATOR   0x12

/* Capabilities */
#define PCIE_CAP_ID_OFFSET              0x0
#define PCIE_CAP_NEXT_CAP_PTR_OFFSET    0x8

#define PCIE_CAP_ID_MASK                0xFF
#define PCIE_CAP_NEXT_CAP_PTR_MASK      0xFF

#define PCIE_CAP_PM_OFFSET      0x10
#define PCIE_CAP_PM_REG_MASK    0xFFFF

#define PCIE_CAP_PM_STATUS_CNTRL_OFFSET 0x0
#define PCIE_CAP_PM_STATUS_CNTRL_MASK   0xFFFF


#define PCIE_CAP_PM_CNTRL_BRIDGE_EXT_OFFSET 0x10
#define PCIE_CAP_PM_CNTRL_BRIDGE_EXT_MASK   0xFF

#define PCIE_CAP_PM_DATA_OFFSET 0x18
#define PCIE_CAP_PM_DATA_MASK   0xFF

#define PCIE_CAP_PM_REG_VERSION_OFFSET  0x0
#define PCIE_CAP_PM_REG_PME_CLK_OFFSET  0x3
#define PCIE_CAP_PM_REG_DSI_OFFSET      0x5
#define PCIE_CAP_PM_REG_AUX_CUR_OFFSET  0x6
#define PCIE_CAP_PM_D1_SUPPORT_OFFSET   0x9
#define PCIE_CAP_PM_D2_SUPPORT_OFFSET   0xA
#define PCIE_CAP_PM_PME_SUPPORT_OFFSET  0xB

#define PCIE_CAP_PM_REG_VERSION_MASK   0x07
#define PCIE_CAP_PM_REG_PME_CLK_MASK   0x01
#define PCIE_CAP_PM_REG_DSI_MASK       0x01
#define PCIE_CAP_PM_REG_AUX_CUR_MASK   0x07
#define PCIE_CAP_PM_D1_SUPPORT_MASK    0x01
#define PCIE_CAP_PM_D2_SUPPORT_MASK    0x01
#define PCIE_CAP_PM_PME_SUPPORT_MASK   0x1F

#define PCIE_CAP_PM_PME_SUPPORT_D0_ASSERT_OFFSET        0x0
#define PCIE_CAP_PM_PME_SUPPORT_D1_ASSERT_OFFSET        0x1
#define PCIE_CAP_PM_PME_SUPPORT_D2_ASSERT_OFFSET        0x2
#define PCIE_CAP_PM_PME_SUPPORT_D3_HOT_ASSERT_OFFSET    0x3
#define PCIE_CAP_PM_PME_SUPPORT_D3_COLD_ASSERT_OFFSET   0x4

#define PCIE_CAP_PM_PME_SUPPORT_D0_ASSERT_MASK       0x01
#define PCIE_CAP_PM_PME_SUPPORT_D1_ASSERT_MASK       0x01
#define PCIE_CAP_PM_PME_SUPPORT_D2_ASSERT_MASK       0x01
#define PCIE_CAP_PM_PME_SUPPORT_D3_HOT_ASSERT_MASK   0x01
#define PCIE_CAP_PM_PME_SUPPORT_D3_COLD_ASSERT_MASK  0x01

#define PCIE_CAP_PM_PMCSR_POWER_STATE_OFFSET   0x0
#define PCIE_CAP_PM_PMCSR_NO_SOFT_RESET_OFFSET 0x3
#define PCIE_CAP_PM_PMCSR_PME_ENABLE_OFFSET    0x8
#define PCIE_CAP_PM_PMCSR_DATA_SEL_OFFSET      0x9
#define PCIE_CAP_PM_PMCSR_DATA_SCALE_OFFSET    0xD
#define PCIE_CAP_PM_PMCSR_PME_STATUS_OFFSET    0xF

#define PCIE_CAP_PM_PMCSR_POWER_STATE_MASK   0x03
#define PCIE_CAP_PM_PMCSR_NO_SOFT_RESET_MASK 0x01
#define PCIE_CAP_PM_PMCSR_PME_ENABLE_MASK    0x01
#define PCIE_CAP_PM_PMCSR_DATA_SEL_MASK      0x0F
#define PCIE_CAP_PM_PMCSR_DATA_SCALE_MASK    0x03
#define PCIE_CAP_PM_PMCSR_PME_STATUS_MASK    0x01

#define PCIE_CAP_PM_PMCSR_BSE_B2_B3_OFFSET      0x6
#define PCIE_CAP_PM_PMCSR_BSE_BPCC_EN_OFFSET    0x7

#define PCIE_CAP_PM_PMCSR_BSE_B2_B3_MASK        0x01
#define PCIE_CAP_PM_PMCSR_BSE_BPCC_EN_MASK      0x01

#define PCIE_CAP_MSI_OFFSET 0x10
#define PCIE_CAP_MSI_REG_MASK   0xFFFF


#define PCIE_CAP_MSI_MSG_CTRL_CONFIG_EN_OFFSET       0x0
#define PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_CAP_OFFSET     0x1
#define PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_EN_OFFSET      0x4
#define PCIE_CAP_MSI_MSG_CTRL_64_BIT_ADDR_OFFSET     0x7
#define PCIE_CAP_MSI_MSG_CTRL_PER_VECT_MSK_OFFSET    0x8

#define PCIE_CAP_MSI_MSG_CTRL_CONFIG_EN_MASK        0x01
#define PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_CAP_MASK      0x07
#define PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_EN_MASK       0x07
#define PCIE_CAP_MSI_MSG_CTRL_64_BIT_ADDR_MASK      0x01
#define PCIE_CAP_MSI_MSG_CTRL_PER_VECT_MSK_MASK     0x01

#define MSI_MESSAGE_ADDR_OFFSET             0x4
#define MSI_MESSAGE_UPPR_ADDR_OFFSET        0x8
#define MSI_MESSAGE_32_BIT_DATA_OFFSET      0x8
#define MSI_MESSAGE_64_BIT_DATA_OFFSET      0xC

#define PCIE_EXP_CAPABILITIES_OFFSET    0x10
#define PCIE_EXP_CAPABILITIES_MASK      0xFFFF

#define PCIE_EXP_CAPABILITIES_VERSION_OFFSET     0x00
#define PCIE_EXP_CAPABILITIES_DEV_TYPE_OFFSET    0x04
#define PCIE_EXP_CAPABILITIES_SLOT_IMP_OFFSET    0x08
#define PCIE_EXP_CAPABILITIES_INT_MSG_NUM_OFFSET 0x09

#define PCIE_EXP_CAPABILITIES_VERSION_MASK      0x0F
#define PCIE_EXP_CAPABILITIES_DEV_TYPE_MASK     0x0F
#define PCIE_EXP_CAPABILITIES_SLOT_IMP_MASK     0x01
#define PCIE_EXP_CAPABILITIES_INT_MSG_NUM_MASK  0x1F

#define PCIE_CAP_DEV_CAPABILITIES_OFFSET    0x04
#define PCIE_CAP_DEV_CNTRL_OFFSET           0x08
#define PCIE_CAP_DEV_STATUS_OFFSET          0x0A
#define PCIE_CAP_LINK_CAPABILITIES_OFFSET   0x0C
#define PCIE_CAP_LINK_CNTRL_OFFSET          0x10
#define PCIE_CAP_LINK_STATUS_OFFSET         0x12
#define PCIE_CAP_SLOT_CAPABILITIES_OFFSET   0x14
#define PCIE_CAP_SLOT_CNTRL_OFFSET          0x18
#define PCIE_CAP_SLOT_STATUS_OFFSET         0x1A
#define PCIE_CAP_ROOT_CNTRL_OFFSET          0x1C
#define PCIE_CAP_ROOT_CAPABILITIES_OFFSET   0x1E
#define PCIE_CAP_ROOT_STATUS_OFFSET         0x20
#define PCIE_CAP_DEV_CAPABILITIES_2_OFFSET  0x24
#define PCIE_CAP_DEV_CNTRL_2_OFFSET         0x28
#define PCIE_CAP_DEV_STATUS_2_OFFSET        0x2A
#define PCIE_CAP_LINK_CAPABILITIES_2_OFFSET 0x2C
#define PCIE_CAP_LINK_CNTRL_2_OFFSET        0x30
#define PCIE_CAP_LINK_STATUS_2_OFFSET       0x32
#define PCIE_CAP_SLOT_CAPABILITIES_2_OFFSET 0x34
#define PCIE_CAP_SLOT_CNTRL_2_OFFSET        0x38
#define PCIE_CAP_SLOT_STATUS_2_OFFSET       0x3A

#define PCIE_CAP_DEV_STATUS_CORRECTABLE_ERR_DET_OFFSET  0x00
#define PCIE_CAP_DEV_STATUS_NONFATAL_ERR_DET_OFFSET     0x01
#define PCIE_CAP_DEV_STATUS_FATAL_ERR_DET_OFFSET        0x02
#define PCIE_CAP_DEV_STATUS_UNSUP_REQ_DET_OFFSET        0x03
#define PCIE_CAP_DEV_STATUS_AUX_PWR_DET_OFFSET          0x04
#define PCIE_CAP_DEV_STATUS_TRANS_PEND_OFFSET           0x05

#define PCIE_CAP_DEV_STATUS_CORRECTABLE_ERR_DET_MASK  0x01
#define PCIE_CAP_DEV_STATUS_NONFATAL_ERR_DET_MASK     0x01
#define PCIE_CAP_DEV_STATUS_FATAL_ERR_DET_MASK        0x01
#define PCIE_CAP_DEV_STATUS_UNSUP_REQ_DET_MASK        0x01
#define PCIE_CAP_DEV_STATUS_AUX_PWR_DET_MASK          0x01
#define PCIE_CAP_DEV_STATUS_TRANS_PEND_MASK           0x01

#define PCIE_CAP_DEV_CNTRL_CORRECT_ERR_REPORT_EN_OFFSET     0x00
#define PCIE_CAP_DEV_CNTRL_NON_FATAL_ERR_REPORT_EN_OFFSET   0x01
#define PCIE_CAP_DEV_CNTRL_FATAL_ERR_REPORT_EN_OFFSET       0x02
#define PCIE_CAP_DEV_CNTRL_UNSPPRT_REQ_REPORT_EN_OFFSET     0x03
#define PCIE_CAP_DEV_CNTRL_EN_RELAXED_ORD_OFFSET            0x04
#define PCIE_CAP_DEV_CNTRL_PCIE_CAP_MAX_PAYLOAD_SIZE_OFFSET 0x05
#define PCIE_CAP_DEV_CNTRL_EXT_TAG_FILED_EN_OFFSET          0x08
#define PCIE_CAP_DEV_CNTRL_PHANTOM_FN_EN_OFFSET             0x09
#define PCIE_CAP_DEV_CNTRL_AUX_PWR_PM_EN_OFFSET             0x0A
#define PCIE_CAP_DEV_CNTRL_EN_NO_SNOOP_OFFSET               0x0B
#define PCIE_CAP_DEV_CNTRL_MAX_READ_REQ_SIZE_OFFSET         0x0C

#define PCIE_CAP_DEV_CNTRL_CORRECT_ERR_REPORT_EN_MASK       0x01
#define PCIE_CAP_DEV_CNTRL_NON_FATAL_ERR_REPORT_EN_MASK     0x01
#define PCIE_CAP_DEV_CNTRL_FATAL_ERR_REPORT_EN_MASK         0x01
#define PCIE_CAP_DEV_CNTRL_UNSPPRT_REQ_REPORT_EN_MASK       0x01
#define PCIE_CAP_DEV_CNTRL_EN_RELAXED_ORD_MASK              0x01
#define PCIE_CAP_DEV_CNTRL_PCIE_CAP_MAX_PAYLOAD_SIZE_MASK   0x07
#define PCIE_CAP_DEV_CNTRL_EXT_TAG_FILED_EN_MASK            0x01
#define PCIE_CAP_DEV_CNTRL_PHANTOM_FN_EN_MASK               0x01
#define PCIE_CAP_DEV_CNTRL_AUX_PWR_PM_EN_MASK               0x01
#define PCIE_CAP_DEV_CNTRL_EN_NO_SNOOP_MASK                 0x01
#define PCIE_CAP_DEV_CNTRL_MAX_READ_REQ_SIZE_MASK           0x07

#define PCIE_CAP_DEV_CAPABILITIES_MAX_PAYLOAD_OFFSET                 0x00
#define PCIE_CAP_DEV_CAPABILITIES_PHANTOM_FN_SUP_OFFSET              0x03
#define PCIE_CAP_DEV_CAPABILITIES_EXT_TAG_FIELD_SUP_OFFSET           0x05
#define PCIE_CAP_DEV_CAPABILITIES_ENDPT_L0_ACCPT_LAT_OFFSET          0x06
#define PCIE_CAP_DEV_CAPABILITIES_ENDPT_L1_ACCPT_LAT_OFFSET          0x09
#define PCIE_CAP_DEV_CAPABILITIES_ROLE_BASED_ERR_REP_OFFSET          0x0F
#define PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_VAL_OFFSET      0x12
#define PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_SCALE_OFFSET    0x1A
#define PCIE_CAP_DEV_CAPABILITIES_FN_LEVEL_RESET_CAP_OFFSET          0x1C

#define PCIE_CAP_DEV_CAPABILITIES_MAX_PAYLOAD_MASK               0x07
#define PCIE_CAP_DEV_CAPABILITIES_PHANTOM_FN_SUP_MASK            0x03
#define PCIE_CAP_DEV_CAPABILITIES_EXT_TAG_FIELD_SUP_MASK         0x01
#define PCIE_CAP_DEV_CAPABILITIES_ENDPT_L0_ACCPT_LAT_MASK        0x07
#define PCIE_CAP_DEV_CAPABILITIES_ENDPT_L1_ACCPT_LAT_MASK        0x07
#define PCIE_CAP_DEV_CAPABILITIES_ROLE_BASED_ERR_REP_MASK        0x01
#define PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_VAL_MASK    0xFF
#define PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_SCALE_MASK  0x03
#define PCIE_CAP_DEV_CAPABILITIES_FN_LEVEL_RESET_CAP_MASK        0x01

#define PCIE_CAP_LINK_CAPABILITIES_MAX_LINK_SPEED_OFFSET            0x00
#define PCIE_CAP_LINK_CAPABILITIES_MAX_PCIE_CAP_LINK_WIDTH_OFFSET   0x04
#define PCIE_CAP_LINK_CAPABILITIES_PCIE_CAP_ASPM_SUPPORT_OFFSET     0x0A
#define PCIE_CAP_LINK_CAPABILITIES_L0_EXIT_LATENCY_OFFSET           0x0C
#define PCIE_CAP_LINK_CAPABILITIES_L1_EXIT_LATENCY_OFFSET           0x0F
#define PCIE_CAP_LINK_CAPABILITIES_CLK_PWR_MGMNT_OFFSET             0x12
#define PCIE_CAP_LINK_CAPABILITIES_SURP_DOWN_ERR_REPORT_CAP_OFFSET  0x13
#define PCIE_CAP_LINK_CAPABILITIES_DLLL_ACTIVE_REPORT_CAP_OFFSET    0x14
#define PCIE_CAP_LINK_CAPABILITIES_LINK_BW_NOT_CAP_OFFSET           0x15
#define PCIE_CAP_LINK_CAPABILITIES_ASPM_OPTIONAL_COMPLIANCE_OFFSET  0x16
#define PCIE_CAP_LINK_CAPABILITIES_PORT_NUM_OFFSET                  0x18

#define PCIE_CAP_LINK_CAPABILITIES_MAX_LINK_SPEED_MASK                     0x0F
#define PCIE_CAP_LINK_CAPABILITIES_MAX_PCIE_CAP_LINK_WIDTH_MASK            0x3F
#define PCIE_CAP_LINK_CAPABILITIES_PCIE_CAP_ASPM_SUPPORT_MASK              0x03
#define PCIE_CAP_LINK_CAPABILITIES_L0_EXIT_LATENCY_MASK                    0x07
#define PCIE_CAP_LINK_CAPABILITIES_L1_EXIT_LATENCY_MASK                    0x07
#define PCIE_CAP_LINK_CAPABILITIES_CLK_PWR_MGMNT_MASK                      0x01
#define PCIE_CAP_LINK_CAPABILITIES_SURP_DOWN_ERR_REPORT_CAP_MASK           0x01
#define PCIE_CAP_LINK_CAPABILITIES_DLLL_ACTIVE_REPORT_CAP_MASK             0x01
#define PCIE_CAP_LINK_CAPABILITIES_LINK_BW_NOT_CAP_MASK                    0x01
#define PCIE_CAP_LINK_CAPABILITIES_ASPM_OPTIONAL_COMPLIANCE_MASK           0x01
#define PCIE_CAP_LINK_CAPABILITIES_PORT_NUM_MASK                           0xFF

#define PCIE_CAP_LINK_STATUS_CUR_LINK_SPEED_OFFSET              0x00
#define PCIE_CAP_LINK_STATUS_NEG_PCIE_CAP_LINK_WIDTH_OFFSET     0x04
#define PCIE_CAP_LINK_STATUS_LINK_TRAINING_OFFSET               0x0B
#define PCIE_CAP_LINK_STATUS_SLOT_CLK_CONFIG_OFFSET             0x0C
#define PCIE_CAP_LINK_STATUS_DLL_ACTIVE_OFFSET                  0x0D
#define PCIE_CAP_LINK_STATUS_LINK_BW_MGMT_STATUS_OFFSET         0x0E
#define PCIE_CAP_LINK_STATUS_LINK_AUTO_BW_STATUS_OFFSET         0x0F


#define PCIE_CAP_LINK_STATUS_CUR_LINK_SPEED_MASK           0x0F
#define PCIE_CAP_LINK_STATUS_NEG_PCIE_CAP_LINK_WIDTH_MASK  0x3F
#define PCIE_CAP_LINK_STATUS_LINK_TRAINING_MASK            0x01
#define PCIE_CAP_LINK_STATUS_SLOT_CLK_CONFIG_MASK          0x01
#define PCIE_CAP_LINK_STATUS_DLL_ACTIVE_MASK               0x01
#define PCIE_CAP_LINK_STATUS_LINK_BW_MGMT_STATUS_MASK      0x01
#define PCIE_CAP_LINK_STATUS_LINK_AUTO_BW_STATUS_MASK      0x01

#define PCIE_CAP_LINK_CNTRL_PCIE_CAP_ASPM_CNTRL_OFFSET      0x00
#define PCIE_CAP_LINK_CNTRL_RCB_OFFSET                      0x03
#define PCIE_CAP_LINK_CNTRL_LINK_DISABLE_OFFSET             0x04
#define PCIE_CAP_LINK_CNTRL_RETRAIN_LINK_OFFSET             0x05
#define PCIE_CAP_LINK_CNTRL_COMMON_CLK_CONFIG_OFFSET        0x06
#define PCIE_CAP_LINK_CNTRL_EXTENDED_SYNCH_OFFSET           0x07
#define PCIE_CAP_LINK_CNTRL_EN_CLK_PWR_MGMT_OFFSET          0x08
#define PCIE_CAP_LINK_CNTRL_HARDWARE_AUTO_WIDTH_DIS_OFFSET  0x09
#define PCIE_CAP_LINK_CNTRL_BW_MGMT_INT_EN_OFFSET           0x0A
#define PCIE_CAP_LINK_CNTRL_AUTO_BW_INT_EN_OFFSET           0x0B
#define PCIE_CAP_LINK_CNTRL_DRS_SIGNAL_CNTRL_OFFSET         0x0E

#define PCIE_CAP_LINK_CNTRL_PCIE_CAP_ASPM_CNTRL_MASK      0x03
#define PCIE_CAP_LINK_CNTRL_RCB_MASK                      0x01
#define PCIE_CAP_LINK_CNTRL_LINK_DISABLE_MASK             0x01
#define PCIE_CAP_LINK_CNTRL_RETRAIN_LINK_MASK             0x01
#define PCIE_CAP_LINK_CNTRL_COMMON_CLK_CONFIG_MASK        0x01
#define PCIE_CAP_LINK_CNTRL_EXTENDED_SYNCH_MASK           0x01
#define PCIE_CAP_LINK_CNTRL_EN_CLK_PWR_MGMT_MASK          0x01
#define PCIE_CAP_LINK_CNTRL_HARDWARE_AUTO_WIDTH_DIS_MASK  0x01
#define PCIE_CAP_LINK_CNTRL_BW_MGMT_INT_EN_MASK           0x01
#define PCIE_CAP_LINK_CNTRL_AUTO_BW_INT_EN_MASK           0x01
#define PCIE_CAP_LINK_CNTRL_DRS_SIGNAL_CNTRL_MASK         0x03

#define PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_BUTTON_PRESENT_OFFSET       0x00
#define PCIE_CAP_SLOT_CAPABILITIES_POWER_CONTROLLER_PRESENT_OFFSET       0x01
#define PCIE_CAP_SLOT_CAPABILITIES_MRL_SENSOR_PRESENT_OFFSET             0x02
#define PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_INDICATOR_PRESENT_OFFSET    0x03
#define PCIE_CAP_SLOT_CAPABILITIES_POWER_INDICATOR_PRESENT_OFFSET        0x04
#define PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_SURPRISE_OFFSET              0x05
#define PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_CAPABLE_OFFSET               0x06
#define PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_VAL_OFFSET           0x07
#define PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_SCALE_OFFSET         0x0F
#define PCIE_CAP_SLOT_CAPABILITIES_ELECTROMAG_INTRLCK_PRESENT_OFFSET     0x11
#define PCIE_CAP_SLOT_CAPABILITIES_NO_CMD_COMP_SUPPORT_OFFSET            0x12

#define PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_BUTTON_PRESENT_MASK     0x01
#define PCIE_CAP_SLOT_CAPABILITIES_POWER_CONTROLLER_PRESENT_MASK     0x01
#define PCIE_CAP_SLOT_CAPABILITIES_MRL_SENSOR_PRESENT_MASK           0x01
#define PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_INDICATOR_PRESENT_MASK  0x01
#define PCIE_CAP_SLOT_CAPABILITIES_POWER_INDICATOR_PRESENT_MASK      0x01
#define PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_SURPRISE_MASK            0x01
#define PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_CAPABLE_MASK             0x01
#define PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_VAL_MASK         0xFF
#define PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_SCALE_MASK       0x03
#define PCIE_CAP_SLOT_CAPABILITIES_ELECTROMAG_INTRLCK_PRESENT_MASK   0x01
#define PCIE_CAP_SLOT_CAPABILITIES_NO_CMD_COMP_SUPPORT_MASK          0x01

#define PCIE_CAP_SLOT_CNTRL_ATTENTION_BUTTON_PRESSED_EN_OFFSET    0x00
#define PCIE_CAP_SLOT_CNTRL_PWR_FAULT_DET_EN_OFFSET               0x01
#define PCIE_CAP_SLOT_CNTRL_MRL_SENSOR_CHANGED_EN_OFFSET          0x02
#define PCIE_CAP_SLOT_CNTRL_PRESENT_DET_CHANGE_EN_OFFSET          0x03
#define PCIE_CAP_SLOT_CNTRL_CMD_COMP_INT_EN_OFFSET                0x04
#define PCIE_CAP_SLOT_CNTRL_HOT_PLUG_INT_EN_OFFSET                0x05
#define PCIE_CAP_SLOT_CNTRL_ATTN_INDICATOR_CNTRL_OFFSET           0x06
#define PCIE_CAP_SLOT_CNTRL_PWR_INDICATOR_CNTRL_OFFSET            0x08
#define PCIE_CAP_SLOT_CNTRL_PWR_CONTROLLER_CNTRL_OFFSET           0x0A
#define PCIE_CAP_SLOT_CNTRL_ELECTROMEC_INTERLOCK_CNTRL_OFFSET     0x0B
#define PCIE_CAP_SLOT_CNTRL_DLLS_CHANGED_EN_OFFSET                0x0C

#define PCIE_CAP_SLOT_CNTRL_ATTENTION_BUTTON_PRESSED_EN_MASK  0x01
#define PCIE_CAP_SLOT_CNTRL_PWR_FAULT_DET_EN_MASK             0x01
#define PCIE_CAP_SLOT_CNTRL_MRL_SENSOR_CHANGED_EN_MASK        0x01
#define PCIE_CAP_SLOT_CNTRL_PRESENT_DET_CHANGE_EN_MASK        0x01
#define PCIE_CAP_SLOT_CNTRL_CMD_COMP_INT_EN_MASK              0x01
#define PCIE_CAP_SLOT_CNTRL_HOT_PLUG_INT_EN_MASK              0x01
#define PCIE_CAP_SLOT_CNTRL_ATTN_INDICATOR_CNTRL_MASK         0x03
#define PCIE_CAP_SLOT_CNTRL_PWR_INDICATOR_CNTRL_MASK          0x03
#define PCIE_CAP_SLOT_CNTRL_PWR_CONTROLLER_CNTRL_MASK         0x01
#define PCIE_CAP_SLOT_CNTRL_ELECTROMEC_INTERLOCK_CNTRL_MASK   0x01
#define PCIE_CAP_SLOT_CNTRL_DLLS_CHANGED_EN_MASK              0x01

#define PCIE_CAP_SLOT_STATUS_ATTENTION_BUT_PRESSED_OFFSET         0x00
#define PCIE_CAP_SLOT_STATUS_PWR_FAULT_DET_OFFSET                 0x01
#define PCIE_CAP_SLOT_STATUS_MRL_SENSOR_CHNG_OFFSET               0x02
#define PCIE_CAP_SLOT_STATUS_PRESENCE_DET_CHNG_OFFSET             0x03
#define PCIE_CAP_SLOT_STATUS_CMD_COMP_OFFSET                      0x04
#define PCIE_CAP_SLOT_STATUS_MRL_SENSOR_STATE_OFFSET              0x05
#define PCIE_CAP_SLOT_STATUS_PRESENCE_DETECT_STATE_OFFSET         0x06
#define PCIE_CAP_SLOT_STATUS_ELECTROMEC_INTERLOCK_STATUS_OFFSET   0x07
#define PCIE_CAP_SLOT_STATUS_DLL_STATE_CHANGED_OFFSET             0x08

#define PCIE_CAP_SLOT_STATUS_ATTENTION_BUT_PRESSED_MASK        0x01
#define PCIE_CAP_SLOT_STATUS_PWR_FAULT_DET_MASK                0x01
#define PCIE_CAP_SLOT_STATUS_MRL_SENSOR_CHNG_MASK              0x01
#define PCIE_CAP_SLOT_STATUS_PRESENCE_DET_CHNG_MASK            0x01
#define PCIE_CAP_SLOT_STATUS_CMD_COMP_MASK                     0x01
#define PCIE_CAP_SLOT_STATUS_MRL_SENSOR_STATE_MASK             0x01
#define PCIE_CAP_SLOT_STATUS_PRESENCE_DETECT_STATE_MASK        0x01
#define PCIE_CAP_SLOT_STATUS_ELECTROMEC_INTERLOCK_STATUS_MASK  0x01
#define PCIE_CAP_SLOT_STATUS_DLL_STATE_CHANGED_MASK            0x01

#define PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_VAL_OFFSET        0x00
#define PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_DISABLE_OFFSET    0x04
#define PCIE_CAP_DEV_CNTRL_2_ARI_FRWD_EN_OFFSET             0x05
#define PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_REQ_EN_OFFSET        0x06
#define PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_EGRESS_BLOCK_OFFSET  0x07
#define PCIE_CAP_DEV_CNTRL_2_IDO_REQ_EN_OFFSET              0x08
#define PCIE_CAP_DEV_CNTRL_2_IDO_COMP_EN_OFFSET             0x09
#define PCIE_CAP_DEV_CNTRL_2_LTR_MECH_EN_OFFSET             0x0A
#define PCIE_CAP_DEV_CNTRL_2_OBFF_EN_OFFSET                 0x0D
#define PCIE_CAP_DEV_CNTRL_2_END_TLP_PREFIX_BLOCK_OFFSET    0x0F

#define PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_VAL_MASK          0x0F
#define PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_DISABLE_MASK      0x01
#define PCIE_CAP_DEV_CNTRL_2_ARI_FRWD_EN_MASK               0x01
#define PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_REQ_EN_MASK          0x01
#define PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_EGRESS_BLOCK_MASK    0x01
#define PCIE_CAP_DEV_CNTRL_2_IDO_REQ_EN_MASK                0x01
#define PCIE_CAP_DEV_CNTRL_2_IDO_COMP_EN_MASK               0x01
#define PCIE_CAP_DEV_CNTRL_2_LTR_MECH_EN_MASK               0x01
#define PCIE_CAP_DEV_CNTRL_2_OBFF_EN_MASK                   0x03
#define PCIE_CAP_DEV_CNTRL_2_END_TLP_PREFIX_BLOCK_MASK      0x01

#define PCIE_CAP_ROOT_CNTRL_ERR_CORR_ERR_EN_OFFSET              0x00
#define PCIE_CAP_ROOT_CNTRL_ERR_NON_FATAL_ERR_EN_OFFSET         0x01
#define PCIE_CAP_ROOT_CNTRL_ERR_FATAL_ERR_EN_OFFSET             0x02
#define PCIE_CAP_ROOT_CNTRL_PME_INT_EN_OFFSET                   0x03
#define PCIE_CAP_ROOT_CNTRL_CRS_SOFTWARE_VISIBILITY_EN_OFFSET   0x04

#define PCIE_CAP_ROOT_CNTRL_ERR_CORR_ERR_EN_MASK             0x01
#define PCIE_CAP_ROOT_CNTRL_ERR_NON_FATAL_ERR_EN_MASK        0x01
#define PCIE_CAP_ROOT_CNTRL_ERR_FATAL_ERR_EN_MASK            0x01
#define PCIE_CAP_ROOT_CNTRL_PME_INT_EN_MASK                  0x01
#define PCIE_CAP_ROOT_CNTRL_CRS_SOFTWARE_VISIBILITY_EN_MASK  0x01

#define PCIE_CAP_ROOT_CAPABILITIES_CRS_SOFT_VIS_OFFSET  0x00
#define PCIE_CAP_ROOT_CAPABILITIES_CRS_SOFT_VIS_MASK    0x01

#define PCIE_CAP_ROOT_STATUS_PME_REQ_ID_OFFSET  0x00
#define PCIE_CAP_ROOT_STATUS_PME_STATUS_OFFSET  0x10
#define PCIE_CAP_ROOT_STATUS_PME_PEND_OFFSET    0x11

#define PCIE_CAP_ROOT_STATUS_PME_REQ_ID_MASK    0xFFFF
#define PCIE_CAP_ROOT_STATUS_PME_STATUS_MASK    0x01
#define PCIE_CAP_ROOT_STATUS_PME_PEND_MASK      0x01

#define PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_RANGE_SUP_OFFSET       0x00
#define PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_DISABLE_SUP_OFFSET     0x04
#define PCIE_CAP_DEV_CAPABILITIES_2_ARI_FORWARD_SUP_OFFSET              0x05
#define PCIE_CAP_DEV_CAPABILITIES_2_ATOMIC_OP_ROUTING_SUP_OFFSET        0x06
#define PCIE_CAP_DEV_CAPABILITIES_2_32_BIT_ATOMIC_OP_COMP_SUP_OFFSET    0x07
#define PCIE_CAP_DEV_CAPABILITIES_2_64_BIT_ATOMIC_OP_COMP_SUP_OFFSET    0x08
#define PCIE_CAP_DEV_CAPABILITIES_2_128_BIT_CAS_COMP_SUP_OFFSET         0x09
#define PCIE_CAP_DEV_CAPABILITIES_2_NO_RO_EN_PR_PASS_OFFSET             0x0A
#define PCIE_CAP_DEV_CAPABILITIES_2_LTR_MECH_SUP_OFFSET                 0x0B
#define PCIE_CAP_DEV_CAPABILITIES_2_TPH_COMP_SUP_OFFSET                 0x0C
#define PCIE_CAP_DEV_CAPABILITIES_2_OBFF_SUP_OFFSET                     0x12
#define PCIE_CAP_DEV_CAPABILITIES_2_EXT_FMT_FIELD_SUP_OFFSET            0x14
#define PCIE_CAP_DEV_CAPABILITIES_2_END_TLP_PREFIX_SUP_OFFSET           0x15
#define PCIE_CAP_DEV_CAPABILITIES_2_MAX_END_TLP_PREFIXES_OFFSET         0x16

#define PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_RANGE_SUP_MASK     0x0F
#define PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_DISABLE_SUP_MASK   0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_ARI_FORWARD_SUP_MASK            0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_ATOMIC_OP_ROUTING_SUP_MASK      0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_32_BIT_ATOMIC_OP_COMP_SUP_MASK  0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_64_BIT_ATOMIC_OP_COMP_SUP_MASK  0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_128_BIT_CAS_COMP_SUP_MASK       0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_NO_RO_EN_PR_PR_PASS_MASK        0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_LTR_MECH_SUP_MASK               0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_TPH_COMP_SUP_MASK               0x03
#define PCIE_CAP_DEV_CAPABILITIES_2_OBFF_SUP_MASK                   0x03
#define PCIE_CAP_DEV_CAPABILITIES_2_EXT_FMT_FIELD_SUP_MASK          0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_END_TLP_PREFIX_SUP_MASK         0x01
#define PCIE_CAP_DEV_CAPABILITIES_2_MAX_END_END_TLP_PREFIXES_MASK   0x03

#define PCIE_CAP_LINK_CAPABILITIES_2_SUPP_LINK_SPEED_VECTOR_OFFSET  0x01
#define PCIE_CAP_LINK_CAPABILITIES_2_CROSSLINK_SUP_OFFSET           0x08

#define PCIE_CAP_LINK_CAPABILITIES_2_SUPP_LINK_SPEED_VECTOR_MASK    0x7F
#define PCIE_CAP_LINK_CAPABILITIES_2_CROSSLINK_SUP_MASK             0x01

#define PCIE_CAP_LINK_CNTRL_2_TARGET_LINK_SPEED_OFFSET            0x00
#define PCIE_CAP_LINK_CNTRL_2_ENTER_COMPLIANCE_OFFSET             0x04
#define PCIE_CAP_LINK_CNTRL_2_HW_AUTO_SPEED_DISABLE_OFFSET        0x05
#define PCIE_CAP_LINK_CNTRL_2_SEL_DE_EMPHASIS_OFFSET              0x06
#define PCIE_CAP_LINK_CNTRL_2_TRANSMIT_MARGIN_OFFSET              0x07
#define PCIE_CAP_LINK_CNTRL_2_ENTER_MODIFIED_COMP_OFFSET          0x0A
#define PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_SOS_OFFSET               0x0B
#define PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_PRESET_OFFSET            0x0C

#define PCIE_CAP_LINK_CNTRL_2_TARGET_LINK_SPEED_MASK      0x0F
#define PCIE_CAP_LINK_CNTRL_2_ENTER_COMPLIANCE_MASK       0x01
#define PCIE_CAP_LINK_CNTRL_2_HW_AUTO_SPEED_DISABLE_MASK  0x01
#define PCIE_CAP_LINK_CNTRL_2_SEL_DE_EMPHASIS_MASK        0x01
#define PCIE_CAP_LINK_CNTRL_2_TRANSMIT_MARGIN_MASK        0x07
#define PCIE_CAP_LINK_CNTRL_2_ENTER_MODIFIED_COMP_MASK    0x01
#define PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_SOS_MASK         0x01
#define PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_PRESET_MASK      0x0F

#define PCIE_CAP_LINK_STATUS2_CUR_DEEMPHASIS_LEVEL_OFFSET   0x00
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_COMP_OFFSET      0x01
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_P1_SUC_OFFSET    0x02
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_P2_SUC_OFFSET    0x03
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_P3_SUC_OFFSET    0x04
#define PCIE_CAP_LINK_STATUS2_LINK_EQUALIZATION_REQ_OFFSET  0x05

#define PCIE_CAP_LINK_STATUS2_CUR_DEEMPHASIS_LEVEL_MASK     0x01
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_COMP_MASK        0x01
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_P1_SUC_MASK      0x01
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_P2_SUC_MASK      0x01
#define PCIE_CAP_LINK_STATUS2_EQUALIZATION_P3_SUC_MASK      0x01
#define PCIE_CAP_LINK_STATUS2_LINK_EQUALIZATION_REQ_MASK    0x01

#define PCIE_EXT_CAP_BASE_ADDR                0x100

#define PCIE_EXT_CAP_ID_OFFSET          0x00
#define PCIE_EXT_CAP_VERSION_OFFSET     0x10
#define PCIE_EXT_CAP_NEXT_OFFSET        0x14

#define PCIE_EXT_CAP_ID_MASK            0xFFFF
#define PCIE_EXT_CAP_VERSION_MASK       0xF
#define PCIE_EXT_CAP_NEXT_MASK          0xFFC

#define PCIE_EXT_AER_CAP_HEADER_OFFSET                0x00
#define PCIE_EXT_AER_UNCOR_ERR_STATUS_OFFSET          0x04
#define PCIE_EXT_AER_UNCOR_ERR_MASK_OFFSET            0x08
#define PCIE_EXT_AER_UNCOR_ERR_SEV_OFFSET             0x0C
#define PCIE_EXT_AER_COR_ERR_STATUS_OFFSET            0x10
#define PCIE_EXT_AER_COR_ERR_MASK_OFFSET              0x14
#define PCIE_EXT_AER_ADV_ERR_CAP_CNTRL_OFFSET         0x18
#define PCIE_EXT_AER_HEADER_LOG_REG_DW1_OFFSET        0x1C
#define PCIE_EXT_AER_HEADER_LOG_REG_DW2_OFFSET        0x20
#define PCIE_EXT_AER_HEADER_LOG_REG_DW3_OFFSET        0x24
#define PCIE_EXT_AER_HEADER_LOG_REG_DW4_OFFSET        0x28
#define PCIE_EXT_AER_ROOT_ERR_CMD_OFFSET              0x2C
#define PCIE_EXT_AER_ROOT_ERR_STATUS_OFFSET           0x30
#define PCIE_EXT_AER_ERR_SRC_ID_OFFSET                0x34
#define PCIE_EXT_AER_FIRST_TLP_PREFIX_LOG_OFFSET      0x38
#define PCIE_EXT_AER_SECOND_TLP_PREFIX_LOG_OFFSET     0x3C
#define PCIE_EXT_AER_THIRD_TLP_PREFIX_LOG_OFFSET      0x40
#define PCIE_EXT_AER_FOURTH_TLP_PREFIX_LOG_OFFSET     0x44

#define PCIE_EXT_ARI_CAP_OFFSET                       0x04
#define PCIE_EXT_ARI_CNTRL_OFFSET                     0x06

#define PCIE_EXT_SEC_LINK_CNTRL3_OFFSET               0x04
#define PCIE_EXT_SEC_LINK_ERR_STATUS_OFFSET           0x08

#define PCIE_EXT_DLF_CAP_OFFSET                       0x04

#define PCIE_EXT_PHY_16_GTS_CAP_OFFSET                              0x04
#define PCIE_EXT_PHY_16_GTS_CNTRL_OFFSET                            0x08
#define PCIE_EXT_PHY_16_GTS_STATUS_OFFSET                           0x0C
#define PCIE_EXT_PHY_16_GTS_LOCAL_PARITY_MISMATCH_STATUS_OFFSET     0x10
#define PCIE_EXT_PHY_16_GTS_FIRST_PARITY_MISMATCH_STATUS_OFFSET     0x14
#define PCIE_EXT_PHY_16_GTS_SECOND_PARITY_MISMATCH_STATUS_OFFSET    0x18

#define PCIE_LANE_MARGINING_AT_REC_ID                    0x27
/*
 * PCI Code and ID Assignment Specification as per Rev 1.1 (Codes not included in pci_regs.h)
 */
#define PCI_EXT_CAP_ID_LANE_MERGE_REC          0x27    /* Lane merging at receiver */
#define PCI_EXT_CAP_ID_HIERARCHY_ID            0x28    /* Hierarchy ID */
#define PCI_EXT_CAP_ID_NPEM                    0x29    /* Native PCIe enclosure management */
#define PCI_EXT_CAP_ID_PHY_LAYER_32_GTS        0x2A    /* Physical Layer 32.0 GT/s */
#define PCI_EXT_CAP_ID_ALT_PROTOCOL            0x2B    /* Alternate Protocol */
#define PCI_EXT_CAP_SFI                        0x2C    /* System Firmware Intermediary */

/* CG added */

#define PCI_STATUS_IMM_READY    0x01    /* Immediate Readiness */

#define PCI_EXT_CAP_ID_L1SS                   0x1E    /* L1 PM Substates */
#define PCI_EXT_CAP_ID_PTM                    0x1F    /* Precision Time Measurement */
#define PCI_EXT_CAP_ID_DLF                    0x25    /* Data Link Feature */
#define PCI_EXT_CAP_ID_PL_16GT                0x26    /* Physical Layer 16.0 GT/s */

/* Config */

#define PCIE_MAX_READ_BYTES 512

enum {
	PCIE_BAR0 = 0,
	PCIE_BAR1,
	PCIE_BAR2,
	PCIE_BAR3,
	PCIE_BAR4,
	PCIE_BAR5,
	NUM_PCIE_BAR,
};

static const char PCIE_BAR_NAME[NUM_PCIE_BAR][13] = {
	"bar region 0",
	"bar region 1",
	"bar region 2",
	"bar region 3",
	"bar region 4",
	"bar region 5"
};

/* header struct */

/**
 * struct bar_header_struct - Represents a PCI BAR
 * @requested: Has this region been requested for the current device
 * @start_addr: BAR start address.
 * @end_addr: BAR end address.
 * @flags: BAR flags.
 * @len: Bar size.
 */
 struct bar_header_struct {
	bool     requested;
	uint64_t start_addr;
	uint64_t end_addr;
	uint64_t flags;
	uint64_t len;
};

typedef struct {
	uint16_t            vendor_id;
	uint16_t            device_id;
	uint16_t            command;
	uint16_t            status;
	uint32_t            class_revision;
	uint8_t             cache_line_size;
	uint8_t             latency_timer;
	uint8_t             header_type;
	uint8_t             bist;
	struct bar_header_struct   bar[NUM_PCIE_BAR];
	uint32_t            cardbus_cis;
	uint16_t            subsystem_vendor_id;
	uint16_t            subsystem_id;
	uint32_t            rom_addr;
	uint8_t             capability_list;
	uint8_t             interrupt_line;
	uint8_t             interrupt_pin;
	uint8_t             min_gnt;
	uint8_t             max_lat;
} pcie_header_struct;

/* Cap struct */

typedef struct {
	uint8_t  power_mgt_cap_base_addr;
	uint8_t  cap_id;
	uint8_t  next_cap;
	uint16_t power_mgt_cap_reg;
	uint16_t power_mgt_status_ctrl;
	uint8_t  power_mgt_ctrl_status_bridge_ext;
	uint8_t  power_mgt_data;
} pcie_pwr_mgt_cap_struct;

typedef struct {
	uint8_t  msi_msg_cap_base_addr;
	uint8_t  cap_id;
	uint8_t  next_cap;
	uint16_t msi_config_status_reg;
	uint32_t msi_msg_addr;
	uint32_t msi_msg_uppr_addr;
	uint16_t msi_msg_data;
} pcie_msi_cap_struct;

typedef struct {
	uint8_t  pcie_exp_cap_base_addr;
	uint8_t  pcie_exp_cap_id;
	uint8_t  next_cap;
	uint16_t pcie_exp_cap_reg;
	uint32_t pcie_exp_dev_cap;
	uint16_t pcie_exp_dev_ctrl;
	uint16_t pcie_exp_dev_status;
	uint32_t pcie_exp_link_cap;
	uint16_t pcie_exp_link_ctrl;
	uint16_t pcie_exp_link_status;
	uint32_t pcie_exp_slot_cap;
	uint16_t pcie_exp_slot_ctrl;
	uint16_t pcie_exp_slot_status;
	uint16_t pcie_exp_root_ctrl;
	uint16_t pcie_exp_root_cap;
	uint32_t pcie_exp_root_status;
	uint32_t pcie_exp_dev_cap_2;
	uint16_t pcie_exp_dev_ctrl_2;
	uint16_t pcie_exp_dev_status_2;
	uint32_t pcie_exp_link_cap_2;
	uint16_t pcie_exp_link_ctrl_2;
	uint16_t pcie_exp_link_status_2;
} pcie_exp_cap_struct;


/********
 * Total Capabilities
 * pm_capability
 * agp_capability
 * vpd_capability
 * slotid_capability
 * msi_capability
 * cpt_pcie_hotswap_capability
 * pcix_capability
 * hyper_transport_capability
 * vendor_specific_capability
 * debug_port_capability
 * compact_pcie_central_res_cntrl_capability
 * pcie_std_hotplug_controller_capability
 * bridge_subsystm_dev_id_capability
 * agp_trgt_pci_pci_bridge_capability
 * secure_device_capability
 * pcie_express_capability
 * msix_capability
 * sata_capability
 * pcie_adv_features_capability
 * pcie_enhanced_allocation
 ********/

typedef struct {
	pcie_pwr_mgt_cap_struct     *pwr_mgt;
	pcie_msi_cap_struct         *msi;
	pcie_exp_cap_struct         *exp;
	uint8_t                     cap_count;
	uint8_t                     expected_pcie_link_speed;
	uint8_t                     expected_pcie_link_width;
	uint8_t                     current_pcie_link_speed;
	uint8_t                     current_pcie_link_width;
} pcie_cap_struct;

/* Ext cap struct */

typedef struct {
	uint16_t    id;
	uint8_t     version;
	uint16_t    next;
} pcie_ext_header_struct;

typedef struct {
	pcie_ext_header_struct  aer_header;
	uint32_t                aer_uncorrectable_err_status_reg;
	uint32_t                aer_uncorrectable_err_mask_reg;
	uint32_t                aer_uncorrectable_err_sev_reg;
	uint32_t                aer_correctable_err_status_reg;
	uint32_t                aer_correctable_err_mask_reg;
	uint32_t                aer_advanced_err_cap_cntrl_reg;
	uint32_t                aer_header_log_reg[4];
	uint32_t                aer_root_err_cmd_reg;
	uint32_t                aer_root_err_status_reg;
	uint32_t                aer_err_source_id_reg;
	uint32_t                aer_first_tlp_prefix_log_reg;
	uint32_t                aer_second_tlp_prefix_log_reg;
	uint32_t                aer_third_tlp_prefix_log_reg;
	uint32_t                aer_fourth_tlp_prefix_log_reg;
} pcie_ext_cap_aer_struct;

typedef struct {
	pcie_ext_header_struct ari_header;
	uint16_t               ari_cap_reg;
	uint16_t               ari_cntrl_reg;
} pcie_ext_cap_ari_struct;

typedef struct {
	pcie_ext_header_struct  secondary_pci_header;
	uint32_t                secondary_pci_link_cntrl3_reg;
	uint32_t                secondary_pci_lane_err_status_reg;
} pcie_ext_cap_secondary_pci_struct;

typedef struct {
	pcie_ext_header_struct  dlf_header;
	uint32_t                dlf_cap_reg;
} pcie_ext_cap_dlf_struct;

typedef struct {
	pcie_ext_header_struct  phy_16_gts_header;
	uint32_t                phy_16_gts_cap_reg;
	uint32_t                phy_16_gts_cntrl_reg;
	uint32_t                phy_16_gts_status_reg;
	uint32_t                phy_16_gts_local_parity_mismatch_status_reg;
	uint32_t                phy_16_gts_first_data_parity_mismatch_status_reg;
	uint32_t                phy_16_gts_second_data_parity_mismatch_status_reg;
} pcie_ext_cap_phy_16_gts_struct;

typedef struct {
	pcie_ext_header_struct    lane_mar_rec_header;
} pcie_ext_cap_lane_mar_rec_struct;

typedef struct {
	pcie_ext_cap_aer_struct             *aer;
	pcie_ext_cap_ari_struct             *ari;
	pcie_ext_cap_secondary_pci_struct   *secondary_pci;
	pcie_ext_cap_dlf_struct             *dlf;
	pcie_ext_cap_phy_16_gts_struct      *phy_16_gts;
	pcie_ext_cap_lane_mar_rec_struct    *lane_mar_rec;
	bool                                vsec_base_addr_found;
	uint32_t                            vsec_base_addr;
} pcie_ext_cap_struct;

typedef struct {
	pcie_header_struct      *header;
	pcie_cap_struct         *cap;
	pcie_ext_cap_struct     *ext_cap;
} pcie_config_struct;


#define get_pcie_command_io(command)            ((command & PCI_COMMAND_IO)              >> PCI_COMMAND_IO_OFFSET)
#define get_pcie_command_memory(command)        ((command & PCI_COMMAND_MEMORY)          >> PCI_COMMAND_MEMORY_OFFSET)
#define get_pcie_command_master(command)        ((command & PCI_COMMAND_MASTER)          >> PCI_COMMAND_MASTER_OFFSET)
#define get_pcie_command_special(command)       ((command & PCI_COMMAND_SPECIAL)         >> PCI_COMMAND_SPECIAL_OFFSET)
#define get_pcie_command_invalidate(command)    ((command & PCI_COMMAND_INVALIDATE)      >> PCI_COMMAND_INVALIDATE_OFFSET)
#define get_pcie_command_palette(command)       ((command & PCI_COMMAND_VGA_PALETTE)     >> PCI_COMMAND_VGA_PALETTE_OFFSET)
#define get_pcie_command_parity(command)        ((command & PCI_COMMAND_PARITY)          >> PCI_COMMAND_PARITY_OFFSET)
#define get_pcie_command_wait(command)          ((command & PCI_COMMAND_WAIT)            >> PCI_COMMAND_WAIT_OFFSET)
#define get_pcie_command_serr(command)          ((command & PCI_COMMAND_SERR)            >> PCI_COMMAND_SERR_OFFSET)
#define get_pcie_command_fast_back(command)     ((command & PCI_COMMAND_FAST_BACK)       >> PCI_COMMAND_FAST_BACK_OFFSET)
#define get_pcie_command_intx_disable(command)  ((command & PCI_COMMAND_INTX_DISABLE)    >> PCI_COMMAND_INTX_DISABLE_OFFSET)

#define get_pcie_status_imm_ready(status)        ((status & PCI_STATUS_IMM_READY)         >> PCI_STATUS_IMM_READY_OFFSET)
#define get_pcie_status_interrupt(status)        ((status & PCI_STATUS_INTERRUPT)         >> PCI_STATUS_INTERRUPT_OFFSET)
#define get_pcie_status_cap_list(status)         ((status & PCI_STATUS_CAP_LIST)          >> PCI_STATUS_CAP_LIST_OFFSET)
#define get_pcie_status_66mhz(status)            ((status & PCI_STATUS_66MHZ)             >> PCI_STATUS_66MHZ_OFFSET)
#define get_pcie_status_fast_back(status)        ((status & PCI_STATUS_FAST_BACK)         >> PCI_STATUS_FAST_BACK_OFFSET)
#define get_pcie_status_parity(status)           ((status & PCI_STATUS_PARITY)            >> PCI_STATUS_PARITY_OFFSET)
#define get_pcie_status_devsel_mask(status)      ((status & PCI_STATUS_DEVSEL_MASK)       >> PCI_STATUS_DEVSEL_MASK_OFFSET)
#define get_pcie_status_sig_target_abort(status) ((status & PCI_STATUS_SIG_TARGET_ABORT)  >> PCI_STATUS_SIG_TARGET_ABORT_OFFSET)
#define get_pcie_status_rec_target_abort(status) ((status & PCI_STATUS_REC_TARGET_ABORT)  >> PCI_STATUS_REC_TARGET_ABORT_OFFSET)
#define get_pcie_status_rec_master_abort(status) ((status & PCI_STATUS_REC_MASTER_ABORT)  >> PCI_STATUS_REC_MASTER_ABORT_OFFSET)
#define get_pcie_status_sig_system_error(status) ((status & PCI_STATUS_SIG_SYSTEM_ERROR)  >> PCI_STATUS_SIG_SYSTEM_ERROR_OFFSET)
#define get_pcie_status_detected_parity(status)  ((status & PCI_STATUS_DETECTED_PARITY)   >> PCI_STATUS_DETECTED_PARITY_OFFSET)

#define get_pcie_class_revision_id(class_revisio )           ((class_revision >> PCI_CLASS_REVISION_ID)           & PCI_CLASS_REVISION_ID_MASK)
#define get_pcie_class_revision_prg_int(class_revisio )      ((class_revision >> PCI_CLASS_REVISION_PROG_IF)      & PCI_CLASS_REVISION_PROG_IF_MASK)
#define get_pcie_class_revision_subclass(class_revisio )     ((class_revision >> PCI_CLASS_REVISION_SUBCLASS)     & PCI_CLASS_REVISION_SUBCLASS_MASK)
#define get_pcie_class_revision_classcode(class_revisio )    ((class_revision >> PCI_CLASS_REVISION_CLASSCODE)    & PCI_CLASS_REVISION_CLASSCODE_MASK)

void get_pcie_bar_info(struct pci_dev *dev, int bar_idx, struct bar_header_struct *bar);

int read_pcie_config_header(struct pci_dev *dev, pcie_header_struct **header);

/* Generic BAR access functions */

/**
 * read_pcie_bar() - Read data from a PCI bar.
 * @dev: Device handle.
 * @bar_idx: Bar number.
 * @offset: Offset within BAR.
 * @num: Number of 32-bit values to read.
 * @val: Data buffer to store data (32-bit values).
 *
 * Return: 0 on success, negative error code otherwise
 */
int read_pcie_bar(struct pci_dev *dev, uint8_t bar_idx, uint64_t offset,
	uint32_t num, uint32_t *val);

/**
 * write_pcie_bar() - Write data to a PCI bar.
 * @dev: Device handle.
 * @bar_idx: Bar number.
 * @offset: Offset within BAR.
 * @num: Number of 32-bit values to write.
 * @val: Data buffer to write (32-bit values).
 *
 * Return: 0 on success, negative error code otherwise
 */
int write_pcie_bar(struct pci_dev *dev, uint8_t bar_idx, uint64_t offset,
	uint32_t num, uint32_t *val);

/* Capabilities */

#define get_pcie_capability_cap_id(capability_read_buf)      ((capability_read_buf >> PCIE_CAP_ID_OFFSET)             & PCIE_CAP_ID_MASK)
#define get_pcie_capability_next_cap(capability_read_buf)    ((capability_read_buf >> PCIE_CAP_NEXT_CAP_PTR_OFFSET)   & PCIE_CAP_NEXT_CAP_PTR_MASK)

#define get_pcie_cap_pm(capability_read_buf) ((capability_read_buf >> PCIE_CAP_PM_OFFSET) & PCIE_CAP_PM_REG_MASK)
#define get_pcie_cap_pm_version(cap_pm)      ((cap_pm >> PCIE_CAP_PM_REG_VERSION_OFFSET)  & PCIE_CAP_PM_REG_VERSION_MASK)
#define get_pcie_cap_pm_pme_clk(cap_pm)      ((cap_pm >> PCIE_CAP_PM_REG_PME_CLK_OFFSET)  & PCIE_CAP_PM_REG_PME_CLK_MASK)
#define get_pcie_cap_pm_dsi(cap_pm)          ((cap_pm >> PCIE_CAP_PM_REG_DSI_OFFSET)      & PCIE_CAP_PM_REG_DSI_MASK)
#define get_pcie_cap_pm_aux_cur(cap_pm)      ((cap_pm >> PCIE_CAP_PM_REG_AUX_CUR_OFFSET)  & PCIE_CAP_PM_REG_AUX_CUR_MASK)
#define get_pcie_cap_pm_d1_support(cap_pm)   ((cap_pm >> PCIE_CAP_PM_D1_SUPPORT_OFFSET)   & PCIE_CAP_PM_D1_SUPPORT_MASK)
#define get_pcie_cap_pm_d2_support(cap_pm)   ((cap_pm >> PCIE_CAP_PM_D2_SUPPORT_OFFSET)   & PCIE_CAP_PM_D2_SUPPORT_MASK)
#define get_pcie_cap_pm_pme_support(cap_pm)  ((cap_pm >> PCIE_CAP_PM_PME_SUPPORT_OFFSET)  & PCIE_CAP_PM_PME_SUPPORT_MASK)

#define get_pcie_cap_pm_pme_support_d0_assert(cap_pm_pme_support)       ((cap_pm_pme_support >> PCIE_CAP_PM_PME_SUPPORT_D0_ASSERT_OFFSET)         & PCIE_CAP_PM_PME_SUPPORT_D0_ASSERT_MASK)
#define get_pcie_cap_pm_pme_support_d1_assert(cap_pm_pme_support)       ((cap_pm_pme_support >> PCIE_CAP_PM_PME_SUPPORT_D1_ASSERT_OFFSET)         & PCIE_CAP_PM_PME_SUPPORT_D1_ASSERT_MASK)
#define get_pcie_cap_pm_pme_support_d2_assert(cap_pm_pme_support)       ((cap_pm_pme_support >> PCIE_CAP_PM_PME_SUPPORT_D2_ASSERT_OFFSET)         & PCIE_CAP_PM_PME_SUPPORT_D2_ASSERT_MASK)
#define get_pcie_cap_pm_pme_support_d3_hot_assert(cap_pm_pme_support)   ((cap_pm_pme_support >> PCIE_CAP_PM_PME_SUPPORT_D3_HOT_ASSERT_OFFSET)     & PCIE_CAP_PM_PME_SUPPORT_D3_HOT_ASSERT_MASK)
#define get_pcie_cap_pm_pme_support_d3_cold_assert(cap_pm_pme_support)  ((cap_pm_pme_support >> PCIE_CAP_PM_PME_SUPPORT_D3_COLD_ASSERT_OFFSET)    & PCIE_CAP_PM_PME_SUPPORT_D3_COLD_ASSERT_MASK)

#define get_pcie_cap_pm_status_cntrl(read_buf)               ((read_buf >> PCIE_CAP_PM_STATUS_CNTRL_OFFSET) & PCIE_CAP_PM_STATUS_CNTRL_MASK)
#define get_pcie_cap_pm_pmcsr_power_state(pm_status_cntrl)   ((pm_status_cntrl >> PCIE_CAP_PM_PMCSR_POWER_STATE_OFFSET)   & PCIE_CAP_PM_PMCSR_POWER_STATE_MASK)
#define get_pcie_cap_pm_pmcsr_no_soft_reset(pm_status_cntrl) ((pm_status_cntrl >> PCIE_CAP_PM_PMCSR_NO_SOFT_RESET_OFFSET) & PCIE_CAP_PM_PMCSR_NO_SOFT_RESET_MASK)
#define get_pcie_cap_pm_pmcsr_pme_enable(pm_status_cntrl)    ((pm_status_cntrl >> PCIE_CAP_PM_PMCSR_PME_ENABLE_OFFSET)    & PCIE_CAP_PM_PMCSR_PME_ENABLE_MASK)
#define get_pcie_cap_pm_pmcsr_data_sel(pm_status_cntrl)      ((pm_status_cntrl >> PCIE_CAP_PM_PMCSR_DATA_SEL_OFFSET)      & PCIE_CAP_PM_PMCSR_DATA_SEL_MASK)
#define get_pcie_cap_pm_pmcsr_data_scale(pm_status_cntrl)    ((pm_status_cntrl >> PCIE_CAP_PM_PMCSR_DATA_SCALE_OFFSET)    & PCIE_CAP_PM_PMCSR_DATA_SCALE_MASK)
#define get_pcie_cap_pm_pmcsr_pme_status(pm_status_cntrl)    ((pm_status_cntrl >> PCIE_CAP_PM_PMCSR_PME_STATUS_OFFSET)    & PCIE_CAP_PM_PMCSR_PME_STATUS_MASK)

#define get_pcie_cap_pm_cntrl_bridge_ext(read_buf)               ((read_buf >> PCIE_CAP_PM_CNTRL_BRIDGE_EXT_OFFSET) & PCIE_CAP_PM_CNTRL_BRIDGE_EXT_MASK)
#define get_pcie_cap_pm_pmcsr_bse_b2_b3(pm_cntrl_bridge_ext)     ((pm_cntrl_bridge_ext >> PCIE_CAP_PM_PMCSR_BSE_B2_B3_OFFSET)     & 0x01)
#define get_pcie_cap_pm_pmcsr_bse_bpcc_en(pm_cntrl_bridge_ext)   ((pm_cntrl_bridge_ext >> PCIE_CAP_PM_PMCSR_BSE_BPCC_EN_OFFSET)   & 0x01)

#define get_pcie_cap_pm_data(read_buf) ((read_buf >> PCIE_CAP_PM_DATA_OFFSET) & PCIE_CAP_PM_CNTRL_BRIDGE_EXT_MASK)

#define get_pcie_cap_msi_config_status(read_buf)                     ((read_buf >> PCIE_CAP_MSI_OFFSET) & PCIE_CAP_MSI_REG_MASK)
#define get_pcie_cap_msi_msg_ctrl_config_en(msi_config_status)       ((msi_config_status >> PCIE_CAP_MSI_MSG_CTRL_CONFIG_EN_OFFSET)    & PCIE_CAP_MSI_MSG_CTRL_CONFIG_EN_MASK)
#define get_pcie_cap_msi_msg_ctrl_mul_msg_cap(msi_config_status)     ((msi_config_status >> PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_CAP_OFFSET)  & PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_CAP_MASK) /* Max interrupt vectors the function is capable of supporting */
#define get_pcie_cap_msi_msg_ctrl_mul_msg_en(msi_config_status)      ((msi_config_status >> PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_EN_OFFSET)   & PCIE_CAP_MSI_MSG_CTRL_MUL_MSG_EN_MASK) /* Num of Interrupt vectors the function is supporting */
#define get_pcie_cap_msi_msg_ctrl_64_bit_addr(msi_config_status)     ((msi_config_status >> PCIE_CAP_MSI_MSG_CTRL_64_BIT_ADDR_OFFSET)  & PCIE_CAP_MSI_MSG_CTRL_64_BIT_ADDR_MASK)
#define get_pcie_cap_msi_msg_ctrl_per_vect_msk(msi_config_status)    ((msi_config_status >> PCIE_CAP_MSI_MSG_CTRL_PER_VECT_MSK_OFFSET) & PCIE_CAP_MSI_MSG_CTRL_PER_VECT_MSK_MASK)

#define get_pcie_exp_cap(read_buf)       ((read_buf >> PCIE_EXP_CAPABILITIES_OFFSET) & PCIE_EXP_CAPABILITIES_MASK)
#define get_pcie_cap_version(cap)        ((cap >> PCIE_EXP_CAPABILITIES_VERSION_OFFSET)       & PCIE_EXP_CAPABILITIES_VERSION_MASK)
#define get_pcie_cap_dev_type(cap)       ((cap >> PCIE_EXP_CAPABILITIES_DEV_TYPE_OFFSET)      & PCIE_EXP_CAPABILITIES_DEV_TYPE_MASK)
#define get_pcie_cap_slot_imp(cap)       ((cap >> PCIE_EXP_CAPABILITIES_SLOT_IMP_OFFSET)      & PCIE_EXP_CAPABILITIES_SLOT_IMP_MASK)
#define get_pcie_cap_int_msg_num(cap)    ((cap >> PCIE_EXP_CAPABILITIES_INT_MSG_NUM_OFFSET)   & PCIE_EXP_CAPABILITIES_INT_MSG_NUM_MASK)

#define get_pcie_cap_dev_max_payload(dev_cap)            ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_MAX_PAYLOAD_OFFSET)               & PCIE_CAP_DEV_CAPABILITIES_MAX_PAYLOAD_MASK)
#define get_pcie_cap_dev_phantom_fn_sup(dev_cap)         ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_PHANTOM_FN_SUP_OFFSET)            & PCIE_CAP_DEV_CAPABILITIES_PHANTOM_FN_SUP_MASK)
#define get_pcie_cap_dev_ext_tag_field_sup(dev_cap)      ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_EXT_TAG_FIELD_SUP_OFFSET)         & PCIE_CAP_DEV_CAPABILITIES_EXT_TAG_FIELD_SUP_MASK)
#define get_pcie_cap_dev_l0_accpt_lat(dev_cap)           ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_ENDPT_L0_ACCPT_LAT_OFFSET)        & PCIE_CAP_DEV_CAPABILITIES_ENDPT_L0_ACCPT_LAT_MASK)
#define get_pcie_cap_dev_endpt_l1_accpt_lat(dev_cap)     ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_ENDPT_L1_ACCPT_LAT_OFFSET)        & PCIE_CAP_DEV_CAPABILITIES_ENDPT_L1_ACCPT_LAT_MASK)
#define get_pcie_cap_dev_role_based_err_rep(dev_cap)     ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_ROLE_BASED_ERR_REP_OFFSET)        & PCIE_CAP_DEV_CAPABILITIES_ROLE_BASED_ERR_REP_MASK)
#define get_pcie_cap_dev_slot_pow_limit_val(dev_cap)     ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_VAL_OFFSET)    & PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_VAL_MASK)
#define get_pcie_cap_dev_slot_pow_limit_scale(dev_cap)   ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_SCALE_OFFSET)  & PCIE_CAP_DEV_CAPABILITIES_CAP_SLOT_POW_LIMIT_SCALE_MASK)
#define get_pcie_cap_dev_fn_level_reset_cap(dev_cap)     ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_FN_LEVEL_RESET_CAP_OFFSET)        & PCIE_CAP_DEV_CAPABILITIES_FN_LEVEL_RESET_CAP_MASK)

#define get_pcie_cap_dev_ctrl_correct_err_report_en(dev_ctrl)    ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_CORRECT_ERR_REPORT_EN_OFFSET)       & PCIE_CAP_DEV_CNTRL_CORRECT_ERR_REPORT_EN_MASK)
#define get_pcie_cap_dev_ctrl_non_fatal_err_report_en(dev_ctrl)  ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_NON_FATAL_ERR_REPORT_EN_OFFSET)     & PCIE_CAP_DEV_CNTRL_NON_FATAL_ERR_REPORT_EN_MASK)
#define get_pcie_cap_dev_ctrl_fatal_err_report_en(dev_ctrl)      ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_FATAL_ERR_REPORT_EN_OFFSET)         & PCIE_CAP_DEV_CNTRL_FATAL_ERR_REPORT_EN_MASK)
#define get_pcie_cap_dev_ctrl_unspprt_req_report_en(dev_ctrl)    ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_UNSPPRT_REQ_REPORT_EN_OFFSET)       & PCIE_CAP_DEV_CNTRL_UNSPPRT_REQ_REPORT_EN_MASK)
#define get_pcie_cap_dev_ctrl_en_relaxed_ord(dev_ctrl)           ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_EN_RELAXED_ORD_OFFSET)              & PCIE_CAP_DEV_CNTRL_EN_RELAXED_ORD_MASK)
#define get_pcie_cap_dev_ctrl_cap_max_payload_size(dev_ctrl)     ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_PCIE_CAP_MAX_PAYLOAD_SIZE_OFFSET)   & PCIE_CAP_DEV_CNTRL_PCIE_CAP_MAX_PAYLOAD_SIZE_MASK)
#define get_pcie_cap_dev_ctrl_ext_tag_filed_en(dev_ctrl)         ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_EXT_TAG_FILED_EN_OFFSET)            & PCIE_CAP_DEV_CNTRL_EXT_TAG_FILED_EN_MASK)
#define get_pcie_cap_dev_ctrl_phantom_fn_en(dev_ctrl)            ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_PHANTOM_FN_EN_OFFSET)               & PCIE_CAP_DEV_CNTRL_PHANTOM_FN_EN_MASK)
#define get_pcie_cap_dev_ctrl_aux_pwr_pm_en(dev_ctrl)            ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_AUX_PWR_PM_EN_OFFSET)               & PCIE_CAP_DEV_CNTRL_AUX_PWR_PM_EN_MASK)
#define get_pcie_cap_dev_ctrl_en_no_snoop(dev_ctrl)              ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_EN_NO_SNOOP_OFFSET)                 & PCIE_CAP_DEV_CNTRL_EN_NO_SNOOP_MASK)
#define get_pcie_cap_dev_ctrl_max_read_req_size(dev_ctrl)        ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_MAX_READ_REQ_SIZE_OFFSET)           & PCIE_CAP_DEV_CNTRL_MAX_READ_REQ_SIZE_MASK)

#define get_pcie_cap_dev_status_correctable_err_det(dev_stat)    ((dev_stat >> PCIE_CAP_DEV_STATUS_CORRECTABLE_ERR_DET_OFFSET)    & PCIE_CAP_DEV_STATUS_CORRECTABLE_ERR_DET_MASK)
#define get_pcie_cap_dev_status_nonfatal_err_det(dev_stat)       ((dev_stat >> PCIE_CAP_DEV_STATUS_NONFATAL_ERR_DET_OFFSET)       & PCIE_CAP_DEV_STATUS_NONFATAL_ERR_DET_MASK)
#define get_pcie_cap_dev_status_fatal_err_det(dev_stat)          ((dev_stat >> PCIE_CAP_DEV_STATUS_FATAL_ERR_DET_OFFSET)          & PCIE_CAP_DEV_STATUS_FATAL_ERR_DET_MASK)
#define get_pcie_cap_dev_status_unsup_req_det(dev_stat)          ((dev_stat >> PCIE_CAP_DEV_STATUS_UNSUP_REQ_DET_OFFSET)          & PCIE_CAP_DEV_STATUS_UNSUP_REQ_DET_MASK)
#define get_pcie_cap_dev_status_aux_pwr_det(dev_stat)            ((dev_stat >> PCIE_CAP_DEV_STATUS_AUX_PWR_DET_OFFSET)            & PCIE_CAP_DEV_STATUS_AUX_PWR_DET_MASK)
#define get_pcie_cap_dev_status_trans_pend(dev_stat)             ((dev_stat >> PCIE_CAP_DEV_STATUS_TRANS_PEND_OFFSET)             & PCIE_CAP_DEV_STATUS_TRANS_PEND_MASK)

#define get_pcie_cap_link_cap_max_link_speed(cap_link)                       ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_MAX_LINK_SPEED_OFFSET)              & PCIE_CAP_LINK_CAPABILITIES_MAX_LINK_SPEED_MASK)
#define get_pcie_cap_link_cap_max_pcie_cap_link_width(cap_link)              ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_MAX_PCIE_CAP_LINK_WIDTH_OFFSET)     & PCIE_CAP_LINK_CAPABILITIES_MAX_PCIE_CAP_LINK_WIDTH_MASK)
#define get_pcie_cap_link_cap_pcie_cap_aspm_support(cap_link)                ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_PCIE_CAP_ASPM_SUPPORT_OFFSET)       & PCIE_CAP_LINK_CAPABILITIES_PCIE_CAP_ASPM_SUPPORT_MASK)
#define get_pcie_cap_link_cap_l0_exit_latency(cap_link)                      ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_L0_EXIT_LATENCY_OFFSET)             & PCIE_CAP_LINK_CAPABILITIES_L0_EXIT_LATENCY_MASK)
#define get_pcie_cap_link_cap_l1_exit_latency(cap_link)                      ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_L1_EXIT_LATENCY_OFFSET)             & PCIE_CAP_LINK_CAPABILITIES_L1_EXIT_LATENCY_MASK)
#define get_pcie_cap_link_cap_clk_pwr_mgmnt(cap_link)                        ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_CLK_PWR_MGMNT_OFFSET)               & PCIE_CAP_LINK_CAPABILITIES_CLK_PWR_MGMNT_MASK)
#define get_pcie_cap_link_cap_surp_down_err_report_cap(cap_link)             ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_SURP_DOWN_ERR_REPORT_CAP_OFFSET)    & PCIE_CAP_LINK_CAPABILITIES_SURP_DOWN_ERR_REPORT_CAP_MASK)
#define get_pcie_cap_link_cap_dlll_active_report_cap(cap_link)               ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_DLLL_ACTIVE_REPORT_CAP_OFFSET)      & PCIE_CAP_LINK_CAPABILITIES_DLLL_ACTIVE_REPORT_CAP_MASK)
#define get_pcie_cap_link_cap_link_bw_not_cap(cap_link)                      ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_LINK_BW_NOT_CAP_OFFSET)             & PCIE_CAP_LINK_CAPABILITIES_LINK_BW_NOT_CAP_MASK)
#define get_pcie_cap_link_cap_pcie_cap_aspm_optional_compliance(cap_link)    ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_ASPM_OPTIONAL_COMPLIANCE_OFFSET)    & PCIE_CAP_LINK_CAPABILITIES_ASPM_OPTIONAL_COMPLIANCE_MASK)
#define get_pcie_cap_link_cap_port_num(cap_link)                             ((cap_link >> PCIE_CAP_LINK_CAPABILITIES_PORT_NUM_OFFSET)                    & PCIE_CAP_LINK_CAPABILITIES_PORT_NUM_MASK)

#define get_pcie_cap_link_ctrl_pcie_cap_aspm_cntrl(link_ctrl)        ((link_ctrl >> PCIE_CAP_LINK_CNTRL_PCIE_CAP_ASPM_CNTRL_OFFSET)       & PCIE_CAP_LINK_CNTRL_PCIE_CAP_ASPM_CNTRL_MASK)
#define get_pcie_cap_link_ctrl_rcb(link_ctrl)                        ((link_ctrl >> PCIE_CAP_LINK_CNTRL_RCB_OFFSET)                       & PCIE_CAP_LINK_CNTRL_RCB_MASK)
#define get_pcie_cap_link_ctrl_link_disable(link_ctrl)               ((link_ctrl >> PCIE_CAP_LINK_CNTRL_LINK_DISABLE_OFFSET)              & PCIE_CAP_LINK_CNTRL_LINK_DISABLE_MASK)
#define get_pcie_cap_link_ctrl_retrain_link(link_ctrl)               ((link_ctrl >> PCIE_CAP_LINK_CNTRL_RETRAIN_LINK_OFFSET)              & PCIE_CAP_LINK_CNTRL_RETRAIN_LINK_MASK)
#define get_pcie_cap_link_ctrl_common_clk_config(link_ctrl)          ((link_ctrl >> PCIE_CAP_LINK_CNTRL_COMMON_CLK_CONFIG_OFFSET)         & PCIE_CAP_LINK_CNTRL_COMMON_CLK_CONFIG_MASK)
#define get_pcie_cap_link_ctrl_extended_synch(link_ctrl)             ((link_ctrl >> PCIE_CAP_LINK_CNTRL_EXTENDED_SYNCH_OFFSET)            & PCIE_CAP_LINK_CNTRL_EXTENDED_SYNCH_MASK)
#define get_pcie_cap_link_ctrl_en_clk_pwr_mgmt(link_ctrl)            ((link_ctrl >> PCIE_CAP_LINK_CNTRL_EN_CLK_PWR_MGMT_OFFSET)           & PCIE_CAP_LINK_CNTRL_EN_CLK_PWR_MGMT_MASK)
#define get_pcie_cap_link_ctrl_hardware_auto_width_dis(link_ctrl)    ((link_ctrl >> PCIE_CAP_LINK_CNTRL_HARDWARE_AUTO_WIDTH_DIS_OFFSET)   & PCIE_CAP_LINK_CNTRL_HARDWARE_AUTO_WIDTH_DIS_MASK)
#define get_pcie_cap_link_ctrl_bw_mgmt_int_en(link_ctrl)             ((link_ctrl >> PCIE_CAP_LINK_CNTRL_BW_MGMT_INT_EN_OFFSET)            & PCIE_CAP_LINK_CNTRL_BW_MGMT_INT_EN_MASK)
#define get_pcie_cap_link_ctrl_auto_bw_int_en(link_ctrl)             ((link_ctrl >> PCIE_CAP_LINK_CNTRL_AUTO_BW_INT_EN_OFFSET)            & PCIE_CAP_LINK_CNTRL_AUTO_BW_INT_EN_MASK)
#define get_pcie_cap_link_ctrl_drs_signal_cntrl(link_ctrl)           ((link_ctrl >> PCIE_CAP_LINK_CNTRL_DRS_SIGNAL_CNTRL_OFFSET)          & PCIE_CAP_LINK_CNTRL_DRS_SIGNAL_CNTRL_MASK)

#define get_pcie_cap_link_status_cur_link_speed(link_stat)           ((link_stat >> PCIE_CAP_LINK_STATUS_CUR_LINK_SPEED_OFFSET)           & PCIE_CAP_LINK_STATUS_CUR_LINK_SPEED_MASK)
#define get_pcie_cap_link_status_neg_pcie_cap_link_width(link_stat)  ((link_stat >> PCIE_CAP_LINK_STATUS_NEG_PCIE_CAP_LINK_WIDTH_OFFSET)  & PCIE_CAP_LINK_STATUS_NEG_PCIE_CAP_LINK_WIDTH_MASK)
#define get_pcie_cap_link_status_link_training(link_stat)            ((link_stat >> PCIE_CAP_LINK_STATUS_LINK_TRAINING_OFFSET)            & PCIE_CAP_LINK_STATUS_LINK_TRAINING_MASK)
#define get_pcie_cap_link_status_slot_clk_config(link_stat)          ((link_stat >> PCIE_CAP_LINK_STATUS_SLOT_CLK_CONFIG_OFFSET)          & PCIE_CAP_LINK_STATUS_SLOT_CLK_CONFIG_MASK)
#define get_pcie_cap_link_status_dll_active(link_stat)               ((link_stat >> PCIE_CAP_LINK_STATUS_DLL_ACTIVE_OFFSET)               & PCIE_CAP_LINK_STATUS_DLL_ACTIVE_MASK)
#define get_pcie_cap_link_status_link_bw_mgmt_status(link_stat)      ((link_stat >> PCIE_CAP_LINK_STATUS_LINK_BW_MGMT_STATUS_OFFSET)      & PCIE_CAP_LINK_STATUS_LINK_BW_MGMT_STATUS_MASK)
#define get_pcie_cap_link_status_link_auto_bw_status(link_stat)      ((link_stat >> PCIE_CAP_LINK_STATUS_LINK_AUTO_BW_STATUS_OFFSET)      & PCIE_CAP_LINK_STATUS_LINK_AUTO_BW_STATUS_MASK)

#define get_pcie_cap_slot_cap_attention_button_present(slot_cap)     ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_BUTTON_PRESENT_OFFSET)    & PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_BUTTON_PRESENT_MASK)
#define get_pcie_cap_slot_cap_power_controller_present(slot_cap)     ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_POWER_CONTROLLER_PRESENT_OFFSET)    & PCIE_CAP_SLOT_CAPABILITIES_POWER_CONTROLLER_PRESENT_MASK)
#define get_pcie_cap_slot_cap_mrl_sensor_present(slot_cap)           ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_MRL_SENSOR_PRESENT_OFFSET)          & PCIE_CAP_SLOT_CAPABILITIES_MRL_SENSOR_PRESENT_MASK)
#define get_pcie_cap_slot_cap_attention_indicator_present(slot_cap)  ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_INDICATOR_PRESENT_OFFSET) & PCIE_CAP_SLOT_CAPABILITIES_ATTENTION_INDICATOR_PRESENT_MASK)
#define get_pcie_cap_slot_cap_power_indicator_present(slot_cap)      ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_POWER_INDICATOR_PRESENT_OFFSET)     & PCIE_CAP_SLOT_CAPABILITIES_POWER_INDICATOR_PRESENT_MASK)
#define get_pcie_cap_slot_cap_hot_plug_surprise(slot_cap)            ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_SURPRISE_OFFSET)           & PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_SURPRISE_MASK)
#define get_pcie_cap_slot_cap_hot_plug_capable(slot_cap)             ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_CAPABLE_OFFSET)            & PCIE_CAP_SLOT_CAPABILITIES_HOT_PLUG_CAPABLE_MASK)
#define get_pcie_cap_slot_cap_slot_power_limit_val(slot_cap)         ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_VAL_OFFSET)        & PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_VAL_MASK)
#define get_pcie_cap_slot_cap_slot_power_limit_scale(slot_cap)       ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_SCALE_OFFSET)      & PCIE_CAP_SLOT_CAPABILITIES_SLOT_POWER_LIMIT_SCALE_MASK)
#define get_pcie_cap_slot_cap_electromag_intrlck_present(slot_cap)   ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_ELECTROMAG_INTRLCK_PRESENT_OFFSET)  & PCIE_CAP_SLOT_CAPABILITIES_ELECTROMAG_INTRLCK_PRESENT_MASK)
#define get_pcie_cap_slot_cap_no_cmd_comp_support(slot_cap)          ((slot_cap >> PCIE_CAP_SLOT_CAPABILITIES_NO_CMD_COMP_SUPPORT_OFFSET)         & PCIE_CAP_SLOT_CAPABILITIES_NO_CMD_COMP_SUPPORT_MASK)

#define get_pcie_cap_slot_ctrl_attention_button_pressed_en(slot_ctrl)    ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_ATTENTION_BUTTON_PRESSED_EN_OFFSET)   & PCIE_CAP_SLOT_CNTRL_ATTENTION_BUTTON_PRESSED_EN_MASK)
#define get_pcie_cap_slot_ctrl_pwr_fault_det_en(slot_ctrl)               ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_PWR_FAULT_DET_EN_OFFSET)              & PCIE_CAP_SLOT_CNTRL_PWR_FAULT_DET_EN_MASK)
#define get_pcie_cap_slot_ctrl_mrl_sensor_changed_en(slot_ctrl)          ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_MRL_SENSOR_CHANGED_EN_OFFSET)         & PCIE_CAP_SLOT_CNTRL_MRL_SENSOR_CHANGED_EN_MASK)
#define get_pcie_cap_slot_ctrl_present_det_change_en(slot_ctrl)          ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_PRESENT_DET_CHANGE_EN_OFFSET)         & PCIE_CAP_SLOT_CNTRL_PRESENT_DET_CHANGE_EN_MASK)
#define get_pcie_cap_slot_ctrl_cmd_comp_int_en(slot_ctrl)                ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_CMD_COMP_INT_EN_OFFSET)               & PCIE_CAP_SLOT_CNTRL_CMD_COMP_INT_EN_MASK)
#define get_pcie_cap_slot_ctrl_hot_plug_int_en(slot_ctrl)                ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_HOT_PLUG_INT_EN_OFFSET)               & PCIE_CAP_SLOT_CNTRL_HOT_PLUG_INT_EN_MASK)
#define get_pcie_cap_slot_ctrl_cntrl_attn_indicator_cntrl(slot_ctrl)     ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_ATTN_INDICATOR_CNTRL_OFFSET)          & PCIE_CAP_SLOT_CNTRL_ATTN_INDICATOR_CNTRL_MASK)
#define get_pcie_cap_slot_ctrl_pwr_indicator_cntrl(slot_ctrl)            ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_PWR_INDICATOR_CNTRL_OFFSET)           & PCIE_CAP_SLOT_CNTRL_PWR_INDICATOR_CNTRL_MASK)
#define get_pcie_cap_slot_ctrl_pwr_controller_cntrl(slot_ctrl)           ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_PWR_CONTROLLER_CNTRL_OFFSET)          & PCIE_CAP_SLOT_CNTRL_PWR_CONTROLLER_CNTRL_MASK)
#define get_pcie_cap_slot_ctrl_electromec_interlock_cntrl(slot_ctrl)     ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_ELECTROMEC_INTERLOCK_CNTRL_OFFSET)    & PCIE_CAP_SLOT_CNTRL_ELECTROMEC_INTERLOCK_CNTRL_MASK)
#define get_pcie_cap_slot_ctrl_dlls_changed_en(slot_ctrl)                ((slot_ctrl >> PCIE_CAP_SLOT_CNTRL_DLLS_CHANGED_EN_OFFSET)               & PCIE_CAP_SLOT_CNTRL_DLLS_CHANGED_EN_MASK)

#define get_pcie_cap_slot_status_attention_but_pressed(slot_stat)        ((slot_stat >> PCIE_CAP_SLOT_STATUS_ATTENTION_BUT_PRESSED_OFFSET)        & PCIE_CAP_SLOT_STATUS_ATTENTION_BUT_PRESSED_MASK)
#define get_pcie_cap_slot_status_status_pwr_fault_det(slot_stat)         ((slot_stat >> PCIE_CAP_SLOT_STATUS_PWR_FAULT_DET_OFFSET)                & PCIE_CAP_SLOT_STATUS_PWR_FAULT_DET_MASK)
#define get_pcie_cap_slot_status_mrl_sensor_chng(slot_stat)              ((slot_stat >> PCIE_CAP_SLOT_STATUS_MRL_SENSOR_CHNG_OFFSET)              & PCIE_CAP_SLOT_STATUS_MRL_SENSOR_CHNG_MASK)
#define get_pcie_cap_slot_status_presence_det_chng(slot_stat)            ((slot_stat >> PCIE_CAP_SLOT_STATUS_PRESENCE_DET_CHNG_OFFSET)            & PCIE_CAP_SLOT_STATUS_PRESENCE_DET_CHNG_MASK)
#define get_pcie_cap_slot_status_cmd_comp(slot_stat)                     ((slot_stat >> PCIE_CAP_SLOT_STATUS_CMD_COMP_OFFSET)                     & PCIE_CAP_SLOT_STATUS_CMD_COMP_MASK)
#define get_pcie_cap_slot_status_mrl_sensor_state(slot_stat)             ((slot_stat >> PCIE_CAP_SLOT_STATUS_MRL_SENSOR_STATE_OFFSET)             & PCIE_CAP_SLOT_STATUS_MRL_SENSOR_STATE_MASK)
#define get_pcie_cap_slot_status_presence_detect_state(slot_stat)        ((slot_stat >> PCIE_CAP_SLOT_STATUS_PRESENCE_DETECT_STATE_OFFSET)        & PCIE_CAP_SLOT_STATUS_PRESENCE_DETECT_STATE_MASK)
#define get_pcie_cap_slot_status_electromec_interlock_status(slot_stat)  ((slot_stat >> PCIE_CAP_SLOT_STATUS_ELECTROMEC_INTERLOCK_STATUS_OFFSET)  & PCIE_CAP_SLOT_STATUS_ELECTROMEC_INTERLOCK_STATUS_MASK)
#define get_pcie_cap_slot_status_dll_state_changed(slot_stat)            ((slot_stat >> PCIE_CAP_SLOT_STATUS_DLL_STATE_CHANGED_OFFSET)            & PCIE_CAP_SLOT_STATUS_DLL_STATE_CHANGED_MASK)

#define get_pcie_cap_root_ctrl_err_corr_err_en(root_ctrl)            ((root_ctrl >>  PCIE_CAP_ROOT_CNTRL_ERR_CORR_ERR_EN_OFFSET)              & PCIE_CAP_ROOT_CNTRL_ERR_CORR_ERR_EN_MASK)
#define get_pcie_cap_root_ctrl_err_non_fatal_err(root_ctrl)          ((root_ctrl >>  PCIE_CAP_ROOT_CNTRL_ERR_NON_FATAL_ERR_EN_OFFSET)         & PCIE_CAP_ROOT_CNTRL_ERR_NON_FATAL_ERR_EN_MASK)
#define get_pcie_cap_root_ctrl_err_fatal_err_en(root_ctrl)           ((root_ctrl >>  PCIE_CAP_ROOT_CNTRL_ERR_FATAL_ERR_EN_OFFSET)             & PCIE_CAP_ROOT_CNTRL_ERR_FATAL_ERR_EN_MASK)
#define get_pcie_cap_root_ctrl_pme_int_en(root_ctrl)                 ((root_ctrl >>  PCIE_CAP_ROOT_CNTRL_PME_INT_EN_OFFSET)                   & PCIE_CAP_ROOT_CNTRL_PME_INT_EN_MASK)
#define get_pcie_cap_root_ctrl_crs_software_visibility_en(root_ctrl) ((root_ctrl >>  PCIE_CAP_ROOT_CNTRL_CRS_SOFTWARE_VISIBILITY_EN_OFFSET)   & PCIE_CAP_ROOT_CNTRL_CRS_SOFTWARE_VISIBILITY_EN_MASK)

#define get_pcie_cap_root_cap_crs_soft_vis(root_cap) ((root_cap >> PCIE_CAP_ROOT_CAPABILITIES_CRS_SOFT_VIS_OFFSET) & PCIE_CAP_ROOT_CAPABILITIES_CRS_SOFT_VIS_MASK)

#define get_pcie_cap_root_status_pme_status(root_stat)   ((root_stat >> PCIE_CAP_ROOT_STATUS_PME_STATUS_OFFSET)   & PCIE_CAP_ROOT_STATUS_PME_STATUS_MASK)
#define get_pcie_cap_root_status_pme_req_i(root_stat)    ((root_stat >> PCIE_CAP_ROOT_STATUS_PME_REQ_ID_OFFSET)   & PCIE_CAP_ROOT_STATUS_PME_REQ_ID_MASK)
#define get_pcie_cap_root_status_pme_pend(root_stat)     ((root_stat >> PCIE_CAP_ROOT_STATUS_PME_PEND_OFFSET)     & PCIE_CAP_ROOT_STATUS_PME_PEND_MASK)

#define get_pcie_cap_dev_cap_2_timeout_range_sup(dev_cap)            ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_RANGE_SUP_OFFSET)      & PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_RANGE_SUP_MASK)
#define get_pcie_cap_dev_cap_2_comp_timeout_disable_sup(dev_cap)     ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_DISABLE_SUP_OFFSET)    & PCIE_CAP_DEV_CAPABILITIES_2_COMP_TIMEOUT_DISABLE_SUP_MASK)
#define get_pcie_cap_dev_cap_2_ari_forward_sup(dev_cap)              ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_ARI_FORWARD_SUP_OFFSET)             & PCIE_CAP_DEV_CAPABILITIES_2_ARI_FORWARD_SUP_MASK)
#define get_pcie_cap_dev_cap_2_atomic_op_routing_sup(dev_cap)        ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_ATOMIC_OP_ROUTING_SUP_OFFSET)       & PCIE_CAP_DEV_CAPABILITIES_2_ATOMIC_OP_ROUTING_SUP_MASK)
#define get_pcie_cap_dev_cap_2_32_bit_atomic_op_comp_sup(dev_cap)    ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_32_BIT_ATOMIC_OP_COMP_SUP_OFFSET)   & PCIE_CAP_DEV_CAPABILITIES_2_32_BIT_ATOMIC_OP_COMP_SUP_MASK)
#define get_pcie_cap_dev_cap_2_64_bit_atomic_op_comp_sup(dev_cap)    ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_64_BIT_ATOMIC_OP_COMP_SUP_OFFSET)   & PCIE_CAP_DEV_CAPABILITIES_2_64_BIT_ATOMIC_OP_COMP_SUP_MASK)
#define get_pcie_cap_dev_cap_2_128_bit_cas_comp_sup(dev_cap)         ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_128_BIT_CAS_COMP_SUP_OFFSET)        & PCIE_CAP_DEV_CAPABILITIES_2_128_BIT_CAS_COMP_SUP_MASK)
#define get_pcie_cap_dev_cap_2_no_ro_en_pr_pas(dev_cap)              ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_NO_RO_EN_PR_PASS_OFFSET)            & PCIE_CAP_DEV_CAPABILITIES_2_NO_RO_EN_PR_PR_PASS_MASK)
#define get_pcie_cap_dev_cap_2_ltr_mech_sup(dev_cap)                 ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_LTR_MECH_SUP_OFFSET)                & PCIE_CAP_DEV_CAPABILITIES_2_LTR_MECH_SUP_MASK)
#define get_pcie_cap_dev_cap_2_tph_comp_sup(dev_cap)                 ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_TPH_COMP_SUP_OFFSET)                & PCIE_CAP_DEV_CAPABILITIES_2_TPH_COMP_SUP_MASK)
#define get_pcie_cap_dev_cap_2_obff_sup(dev_cap)                     ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_OBFF_SUP_OFFSET)                    & PCIE_CAP_DEV_CAPABILITIES_2_OBFF_SUP_MASK)
#define get_pcie_cap_dev_cap_2_ext_fmt_field_sup(dev_cap)            ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_EXT_FMT_FIELD_SUP_OFFSET)           & PCIE_CAP_DEV_CAPABILITIES_2_EXT_FMT_FIELD_SUP_MASK)
#define get_pcie_cap_dev_cap_2_end_tlp_prefix_sup(dev_cap)           ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_END_TLP_PREFIX_SUP_OFFSET)          & PCIE_CAP_DEV_CAPABILITIES_2_END_TLP_PREFIX_SUP_MASK)
#define get_pcie_cap_dev_cap_2_max_end_tlp_prefixes(dev_cap)         ((dev_cap >> PCIE_CAP_DEV_CAPABILITIES_2_MAX_END_TLP_PREFIXES_OFFSET)        & PCIE_CAP_DEV_CAPABILITIES_2_MAX_END_END_TLP_PREFIXES_MASK)

#define get_pcie_cap_dev_ctrl_2_comp_timeout_val(dev_ctrl)       ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_VAL_OFFSET)          & PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_VAL_MASK)
#define get_pcie_cap_dev_ctrl_2_comp_timeout_disable(dev_ctrl)   ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_DISABLE_OFFSET)      & PCIE_CAP_DEV_CNTRL_2_COMP_TIMEOUT_DISABLE_MASK)
#define get_pcie_cap_dev_ctrl_2_ari_frwd_en(dev_ctrl)            ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_ARI_FRWD_EN_OFFSET)               & PCIE_CAP_DEV_CNTRL_2_ARI_FRWD_EN_MASK)
#define get_pcie_cap_dev_ctrl_2_atomic_op_req_en(dev_ctrl)       ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_REQ_EN_OFFSET)          & PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_REQ_EN_MASK)
#define get_pcie_cap_dev_ctrl_2_atomic_op_egress_block(dev_ctrl) ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_EGRESS_BLOCK_OFFSET)    & PCIE_CAP_DEV_CNTRL_2_ATOMIC_OP_EGRESS_BLOCK_MASK)
#define get_pcie_cap_dev_ctrl_2_ido_req_en(dev_ctrl)             ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_IDO_REQ_EN_OFFSET)                & PCIE_CAP_DEV_CNTRL_2_IDO_REQ_EN_MASK)
#define get_pcie_cap_dev_ctrl_2_ido_comp_en(dev_ctrl)            ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_IDO_COMP_EN_OFFSET)               & PCIE_CAP_DEV_CNTRL_2_IDO_COMP_EN_MASK)
#define get_pcie_cap_dev_ctrl_2_ltr_mech_en(dev_ctrl)            ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_LTR_MECH_EN_OFFSET)               & PCIE_CAP_DEV_CNTRL_2_LTR_MECH_EN_MASK)
#define get_pcie_cap_dev_ctrl_2_obff_en(dev_ctrl)                ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_OBFF_EN_OFFSET)                   & PCIE_CAP_DEV_CNTRL_2_OBFF_EN_MASK)
#define get_pcie_cap_dev_ctrl_2_end_tlp_prefix_block(dev_ctrl)   ((dev_ctrl >> PCIE_CAP_DEV_CNTRL_2_END_TLP_PREFIX_BLOCK_OFFSET)      & PCIE_CAP_DEV_CNTRL_2_END_TLP_PREFIX_BLOCK_MASK)

#define get_pcie_cap_link_cap_2_supp_link_speed_vector(link_cap) ((link_cap >> PCIE_CAP_LINK_CAPABILITIES_2_SUPP_LINK_SPEED_VECTOR_OFFSET)    & PCIE_CAP_LINK_CAPABILITIES_2_SUPP_LINK_SPEED_VECTOR_MASK)
#define get_pcie_cap_link_cap_2_crosslink_sup(link_cap)          ((link_cap >> PCIE_CAP_LINK_CAPABILITIES_2_CROSSLINK_SUP_OFFSET)             & PCIE_CAP_LINK_CAPABILITIES_2_CROSSLINK_SUP_MASK)

#define get_pcie_cap_link_ctrl_2_target_link_speed(link_ctrl)        ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_TARGET_LINK_SPEED_OFFSET)       & PCIE_CAP_LINK_CNTRL_2_TARGET_LINK_SPEED_MASK)
#define get_pcie_cap_link_ctrl_2_enter_complianc(link_ctrl)          ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_ENTER_COMPLIANCE_OFFSET)        & PCIE_CAP_LINK_CNTRL_2_ENTER_COMPLIANCE_MASK)
#define get_pcie_cap_link_ctrl_2_hw_auto_speed_disable(link_ctrl)    ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_HW_AUTO_SPEED_DISABLE_OFFSET)   & PCIE_CAP_LINK_CNTRL_2_HW_AUTO_SPEED_DISABLE_MASK)
#define get_pcie_cap_link_ctrl_2_sel_de_emphasis(link_ctrl)          ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_SEL_DE_EMPHASIS_OFFSET)         & PCIE_CAP_LINK_CNTRL_2_SEL_DE_EMPHASIS_MASK)
#define get_pcie_cap_link_ctrl_2_transmit_margin(link_ctrl)          ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_TRANSMIT_MARGIN_OFFSET)         & PCIE_CAP_LINK_CNTRL_2_TRANSMIT_MARGIN_MASK)
#define get_pcie_cap_link_ctrl_2_enter_modified_comp(link_ctrl)      ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_ENTER_MODIFIED_COMP_OFFSET)     & PCIE_CAP_LINK_CNTRL_2_ENTER_MODIFIED_COMP_MASK)
#define get_pcie_cap_link_ctrl_2_compliance_sos(link_ctrl)           ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_SOS_OFFSET)          & PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_SOS_MASK)
#define get_pcie_cap_link_ctrl_2_compliance_preset(link_ctrl)        ((link_ctrl >> PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_PRESET_OFFSET)       & PCIE_CAP_LINK_CNTRL_2_COMPLIANCE_PRESET_MASK)

#define get_pcie_cap_link_status_2_cur_deemphasis_level(link_stat)      ((link_stat >> PCIE_CAP_LINK_STATUS2_CUR_DEEMPHASIS_LEVEL_OFFSET)    & PCIE_CAP_LINK_STATUS2_CUR_DEEMPHASIS_LEVEL_MASK)
#define get_pcie_cap_link_status_2_equalization_comp(link_stat)         ((link_stat >> PCIE_CAP_LINK_STATUS2_EQUALIZATION_COMP_OFFSET)       & PCIE_CAP_LINK_STATUS2_EQUALIZATION_COMP_MASK)
#define get_pcie_cap_link_status_2_equalization_p1_suc(link_stat)       ((link_stat >> PCIE_CAP_LINK_STATUS2_EQUALIZATION_P1_SUC_OFFSET)     & PCIE_CAP_LINK_STATUS2_EQUALIZATION_P1_SUC_MASK)
#define get_pcie_cap_link_status_2_equalization_p2_suc(link_stat)       ((link_stat >> PCIE_CAP_LINK_STATUS2_EQUALIZATION_P2_SUC_OFFSET)     & PCIE_CAP_LINK_STATUS2_EQUALIZATION_P2_SUC_MASK)
#define get_pcie_cap_link_status_2_equalization_p3_suc( link_stat )     ((link_stat >> PCIE_CAP_LINK_STATUS2_EQUALIZATION_P3_SUC_OFFSET)     & PCIE_CAP_LINK_STATUS2_EQUALIZATION_P3_SUC_MASK)
#define get_pcie_cap_link_status_2_link_equalization_req(link_stat)     ((link_stat >> PCIE_CAP_LINK_STATUS2_LINK_EQUALIZATION_REQ_OFFSET)   & PCIE_CAP_LINK_STATUS2_LINK_EQUALIZATION_REQ_MASK)

int read_pcie_capabilities( struct pci_dev *dev, pcie_header_struct *header, pcie_cap_struct **cap);

/* Extended Capabilities */

#define get_pcie_capability_ext_cap_id(ext_header_read_buf)      ((ext_header_read_buf >> PCIE_EXT_CAP_ID_OFFSET)         & PCIE_EXT_CAP_ID_MASK)
#define get_pcie_capability_ext_cap_version(ext_header_read_buf) ((ext_header_read_buf >> PCIE_EXT_CAP_VERSION_OFFSET)    & PCIE_EXT_CAP_VERSION_MASK)
#define get_pcie_capability_ext_cap_next(ext_header_read_buf)    ((ext_header_read_buf >> PCIE_EXT_CAP_NEXT_OFFSET)       & PCIE_EXT_CAP_NEXT_MASK)

int read_pcie_ext_capabilities(struct pci_dev *dev, pcie_ext_cap_struct **ext_cap);

/* PCIe configuration */

int read_pcie_configuration(struct pci_dev *dev, pcie_config_struct **pcie_config);

int write_pcie_configuration(struct pci_dev *dev);
bool is_supported_pcie_device_id(uint16_t pcie_device_id);

/* Release */
void release_pcie_ext_cap_secondary_pci(pcie_ext_cap_secondary_pci_struct **secondary_pci);
void release_pcie_ext_cap_phy_16_gts(pcie_ext_cap_phy_16_gts_struct **phy_16_gts);
void release_pcie_ext_cap_lane_mar_rec(pcie_ext_cap_lane_mar_rec_struct **lane_mar_rec);
void release_pcie_ext_cap_dlf(pcie_ext_cap_dlf_struct **dlf);
void release_pcie_ext_cap_aer(pcie_ext_cap_aer_struct **aer);
void release_pcie_ext_cap_ari(pcie_ext_cap_ari_struct **ari);
void release_pcie_ext_cap(pcie_ext_cap_struct **ext_cap);
void release_pcie_cap_pwr_mgt(pcie_pwr_mgt_cap_struct **pwr_mgt);
void release_pcie_cap_msi(pcie_msi_cap_struct **msi);
void release_pcie_cap_exp(pcie_exp_cap_struct **exp);
void release_pcie_cap(pcie_cap_struct **cap);
void release_pcie_config_header(pcie_header_struct **pcie_header);
void release_pcie_config(pcie_config_struct **pcie_config);
void release_pcie_mem(pcie_config_struct **pcie_config);

#endif /* AMI_PCIE_H */
