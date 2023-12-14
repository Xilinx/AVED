// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_mfg_info.c - This file contains the public interface for board info logic.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* Public API includes */
#include "ami_mfg_info.h"

/* Private API includes */
#include "ami_ioctl.h"
#include "ami_internal.h"
#include "ami_device_internal.h"

/*****************************************************************************/
/* Private functions                                                         */
/*****************************************************************************/

/**
 * get_name_for_mfg_field() - Get the name of a SYSFS EEPROM node.
 * @field: EEPROM field type to lookup.
 *
 * Return: The SYSFS name or empty string.
 */
static char *get_name_for_mfg_field(enum ami_mfg_field field)
{
	switch (field) {
	case AMI_MFG_EEPROM_VERSION: return "eeprom_version";
	case AMI_MFG_PRODUCT_NAME:   return "product_name";
	case AMI_MFG_BOARD_REV:      return "board_rev";
	case AMI_MFG_BOARD_SERIAL:   return "board_serial";
	case AMI_MFG_MEMORY_SIZE:    return "memory_size";
	case AMI_MFG_PART_NUM:       return "part_num";
	case AMI_MFG_M_PART_NUM:     return "mfg_part_num";
	case AMI_MFG_MAC_ADDR_C:     return "mac_addr_count";
	case AMI_MFG_MAC_ADDR:       return "mac_addr";
	case AMI_MFG_MAC_ADDR_N:     return "mac_addr_n";
	case AMI_MFG_ACTIVE_STATE:   return "active_state";
	case AMI_MFG_CONFIG_MODE:    return "fpga_config_mode";
	case AMI_MFG_M_DATE:         return "mfg_date";
	case AMI_MFG_UUID:           return "mfg_uuid";
	case AMI_MFG_PCIE_ID:        return "pcie_id";
	case AMI_MFG_POWER_MODE:     return "power_mode";
	case AMI_MFG_OEM_ID:         return "oem_id";
	case AMI_MFG_CAPABILITY:     return "mfg_capability";
	default:                     break;
	}

	return "";
}

/*****************************************************************************/
/* Public API function definitions                                           */
/*****************************************************************************/

/*
 * Get the manufacturing info.
 */
int ami_mfg_get_info(ami_device *dev, enum ami_mfg_field field,
	char buf[AMI_MFG_INFO_MAX_STR])
{
	int ret = AMI_STATUS_ERROR;
	char *field_name = NULL;
	char raw_buf[AMI_SYSFS_STR_MAX] = { 0 };

	if (!dev || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	/* Get the sysfs field name */
	field_name = get_name_for_mfg_field(field);
	if (!field_name || !strlen(field_name))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, field_name, raw_buf) == AMI_STATUS_OK) {
		/* Strip newline */
		raw_buf[strcspn(raw_buf, "\r\n")] = 0;
		memset(buf, 0x00, AMI_MFG_INFO_MAX_STR);
		strncpy(buf, raw_buf, AMI_MFG_INFO_MAX_STR - 1);
		ret = AMI_STATUS_OK;
	}

	return ret;
}
