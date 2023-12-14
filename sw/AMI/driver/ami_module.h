// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_module.h - This file contains functions to read/write QSFP modules
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */
 
#ifndef AMI_MODULE_H
#define AMI_MODULE_H

#include <linux/types.h>

#include "ami_top.h"
#include "ami_amc_control.h"

/**
 * Format of flags:
 * 0xAABBCCDD where:
 *   0xAA is the request type (read or write)
 *   0xBB is the device ID
 *   0xCC is the page number
 *   0xDD is the offset
 */
#define MK_MODULE_RW_FLAGS(req, dev, page, off)	(((uint8_t)req << 24)         | \
							((uint8_t)dev << 16)  | \
							((uint8_t)page << 8)  | \
							((uint8_t)off))
#define MODULE_RW_TYPE(flags)			((uint8_t)(flags >> 24))
#define MODULE_RW_DEVICE(flags)			((uint8_t)((flags & 0x00ff0000) >> 16))
#define MODULE_RW_PAGE(flags)			((uint8_t)((flags & 0x0000ff00) >> 8))
#define MODULE_RW_OFFSET(flags)			((uint8_t)(flags & 0x000000ff))


/**
 * module_read() - Read one or more values from a QSFP module.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @device_id: Module ID.
 * @page: Page number to access.
 * @offset: Offset within page.
 * @buf: Pointer to output buffer.
 * @buf_len: Number of bytes to read.
 * 
 * Return: 0 or negative error code.
 */
int module_read(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t device_id,
	uint8_t page, uint8_t offset, uint8_t *buf, uint8_t buf_len);

/**
 * module_write() - Write one more values to a QSFP module.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @device_id: Module ID.
 * @page: Page number to access.
 * @offset: Offset within page.
 * @buf: Pointer to data buffer to write.
 * @buf_len: Number of bytes to write.
 * 
 * Return: 0 or negative error code.
 */
int module_write(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t device_id,
	uint8_t page, uint8_t offset, uint8_t *buf, uint8_t buf_len);

#endif  /* AMI_MODULE_H */
