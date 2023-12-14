// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_module_access.h - This file contains the QSFP module access interface.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_MODULE_ACCESS_H
#define AMI_MODULE_ACCESS_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>

/* Public API includes */
#include "ami_device.h"

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/**
 * ami_module_read() - Read one or more bytes of data from a QSFP module.
 * @dev: Device handle.
 * @device_id: Module ID.
 * @page: Page number to access.
 * @offset: Byte offset within page.
 * @num: Number of values to read.
 * @val: Buffer to store the values read.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_module_read(ami_device *dev, uint8_t device_id, uint8_t page,
	uint8_t offset, uint8_t num, uint8_t *val);

/**
 * ami_module_write() - Write one or more bytes of data to a QSFP module.
 * @dev: Device handle.
 * @device_id: Module ID.
 * @page: Page number to access.
 * @offset: Byte offset within page.
 * @num: Number of values to write.
 * @val: Register values to write.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_module_write(ami_device *dev, uint8_t device_id, uint8_t page,
	uint8_t offset, uint8_t num, uint8_t *val);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_MODULE_ACCESS_H */
