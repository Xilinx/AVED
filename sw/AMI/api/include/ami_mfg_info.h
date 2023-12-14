// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_mfg_info.h - This file contains the public interface for board info logic.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_MFG_INFO_H
#define AMI_MFG_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define AMI_MFG_INFO_MAX_STR		(64)

#define AMI_MFG_INFO_OEM_ID_XILINX	(0x10DA)
#define AMI_MFG_INFO_OEM_ID_AMD		(0x3704)

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>

/* Public API includes */
#include "ami_device.h"

/*****************************************************************************/
/* Structs, Enums                                                            */
/*****************************************************************************/

/**
 * enum ami_mfg_field - List of device EEPROM fields.
 * @AMI_MFG_FIELD_MIN: Min enum value to allow iteration.
 * @AMI_MFG_EEPROM_VERSION: The EEPROM version.
 * @AMI_MFG_PRODUCT_NAME: the product name.
 * @AMI_MFG_BOARD_REV: the board revision.
 * @AMI_MFG_BOARD_SERIAL: the board serial number.
 * @AMI_MFG_MEMORY_SIZE:  Coveys the max memory (in GB).
 * @AMI_MFG_PART_NUM: Board part number.
 * @AMI_MFG_M_PART_NUM: Manufacturer part number.
 * @AMI_MFG_MAC_ADDR_C: Number of MAC IDs.
 * @AMI_MFG_MAC_ADDR: Primary MAC ID.
 * @AMI_MFG_MAC_ADDR_N:  Number of MAC IDs.
 * @AMI_MFG_ACTIVE_STATE: Active/Passive.
 * @AMI_MFG_CONFIG_MODE: 07: Master SPI x4 (QSPIx4), 08: OSPI.
 * @AMI_MFG_M_DATE: Manufacturing Date.
 * @AMI_MFG_UUID: Used to uniquely ID the product.
 * @AMI_MFG_PCIE_ID: Vendor ID, Device ID, SubVendor ID, SubDevice ID.
 * @AMI_MFG_POWER_MODE: Max power mode.
 * @AMI_MFG_OEM_ID: OEM ID.
 * @AMI_MFG_CAPABILITY: Capability word.
 * @AMI_MFG_FIELD_MAX: Max enum value to allow iteration.
 */
enum ami_mfg_field {
	AMI_MFG_FIELD_MIN,

	AMI_MFG_EEPROM_VERSION,
	AMI_MFG_PRODUCT_NAME,
	AMI_MFG_BOARD_REV,
	AMI_MFG_BOARD_SERIAL,
	AMI_MFG_MEMORY_SIZE,
	AMI_MFG_PART_NUM,
	AMI_MFG_M_PART_NUM,
	AMI_MFG_MAC_ADDR_C,
	AMI_MFG_MAC_ADDR,
	AMI_MFG_MAC_ADDR_N,
	AMI_MFG_ACTIVE_STATE,
	AMI_MFG_CONFIG_MODE,
	AMI_MFG_M_DATE,
	AMI_MFG_UUID,
	AMI_MFG_PCIE_ID,
	AMI_MFG_POWER_MODE,
	AMI_MFG_OEM_ID,
	AMI_MFG_CAPABILITY,

	AMI_MFG_FIELD_MAX,
};

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/**
 * ami_mfg_get_info() - Read a single EEPROM field.
 * @dev: Device handle.
 * @field: Type of field to read.
 * @buf: Buffer to store field value.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_mfg_get_info(ami_device *dev, enum ami_mfg_field field,
	char buf[AMI_MFG_INFO_MAX_STR]);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_MFG_INFO_H */
