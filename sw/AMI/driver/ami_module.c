// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_module.c - This file contains functions to read/write QSFP modules
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/pci.h> 

#include "ami_top.h"
#include "ami_module.h"
#include "ami_amc_control.h"

/*
 * Read one or more values from a QSFP module.
 */
int module_read(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t device_id,
	uint8_t page, uint8_t offset, uint8_t *buf, uint8_t buf_len)
{
	int ret = SUCCESS;

	if (!amc_ctrl_ctxt || !buf || (buf_len == 0))
		return -EINVAL;
	
	AMI_VDBG(
		amc_ctrl_ctxt,
		"Attempting to read module %d at offset 0x%02x and page %d",
		device_id, offset, page
	);

	ret = submit_gcq_command(
		amc_ctrl_ctxt,
		GCQ_SUBMIT_CMD_MODULE_READ_WRITE,
		MK_MODULE_RW_FLAGS(AMC_PROXY_CMD_RW_REQUEST_READ, device_id, page, offset),
		buf,
		buf_len
	);
	
	if (ret)
		AMI_ERR(amc_ctrl_ctxt, "Failed to read module");

	return ret;
}

/*
 * Write one more values to a QSFP module.
 */
int module_write(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t device_id,
	uint8_t page, uint8_t offset, uint8_t *buf, uint8_t buf_len)
{
	int ret = SUCCESS;

	if (!amc_ctrl_ctxt || !buf || (buf_len == 0))
		return -EINVAL;
	
	AMI_VDBG(
		amc_ctrl_ctxt,
		"Attempting to write module %d at offset 0x%02x and page %d",
		device_id, offset, page
	);

	ret = submit_gcq_command(
		amc_ctrl_ctxt,
		GCQ_SUBMIT_CMD_MODULE_READ_WRITE,
		MK_MODULE_RW_FLAGS(AMC_PROXY_CMD_RW_REQUEST_WRITE, device_id, page, offset),
		buf,
		buf_len
	);
	
	if (ret)
		AMI_ERR(amc_ctrl_ctxt, "Failed to write module");

	return ret;
}
