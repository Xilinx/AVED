// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_mem_access.h- This file contains the public interface for AMI memory access logic
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_MEM_ACCESS_H
#define AMI_MEM_ACCESS_H

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
 * ami_mem_bar_read() - Read a PCI bar register.
 * @dev: Device handle.
 * @idx: Bar index.
 * @offset: Register offset within BAR.
 * @val: Variable to store register value.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_mem_bar_read(ami_device *dev, uint8_t idx, uint64_t offset, uint32_t *val);

/**
 * ami_mem_bar_write() - Write to a PCI bar register.
 * @dev: Device handle.
 * @idx: Bar index.
 * @offset: Register offset within BAR.
 * @val: Value to write.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_mem_bar_write(ami_device *dev, uint8_t idx, uint64_t offset, uint32_t val);

/**
 * ami_mem_bar_read_range() - Read block of data from a PCI bar register.
 * @dev: Device handle.
 * @idx: Bar index.
 * @offset: First register offset within BAR.
 * @num: Number of registers to read.
 * @val: Buffer to store register values.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_mem_bar_read_range(ami_device *dev, uint8_t idx, uint64_t offset,
	uint32_t num, uint32_t *val);

/**
 * ami_mem_bar_write_range() - Write block of data to a PCI bar register.
 * @dev: Device handle.
 * @idx: Bar index.
 * @offset: First register offset within BAR.
 * @num: Number of registers to write to.
 * @val: Register values to write.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_mem_bar_write_range(ami_device *dev, uint8_t idx, uint64_t offset,
	uint32_t num, uint32_t *val);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_MEM_ACCESS_H */
