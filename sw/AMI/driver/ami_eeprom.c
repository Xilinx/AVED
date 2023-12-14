// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_eeprom.c - This file contains functions to read/write from the AVED Manufacturing EEPROM.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/pci.h> 

#include "ami_top.h"
#include "ami_eeprom.h"
#include "ami_amc_control.h"

/*
 * Read one or more values from the EEPROM.
 */
int eeprom_read(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t *buf, uint8_t buf_len, uint8_t offset)
{
	int ret = SUCCESS;
	uint32_t eeprom_req_data = 0;

	if (!amc_ctrl_ctxt || !buf || (buf_len == 0))
		return -EINVAL;

	AMI_VDBG(
		amc_ctrl_ctxt,
		"Attempting to read EEPROM at offset:%d len:%d",
		offset, buf_len
	);

	eeprom_req_data = EEPROM_SET_TYPE(AMC_PROXY_CMD_RW_REQUEST_READ);
	eeprom_req_data |= EEPROM_SET_OFFSET(offset);
	ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_EEPROM_READ_WRITE, eeprom_req_data, buf,
							 buf_len);
	
	if (ret)
		AMI_ERR(amc_ctrl_ctxt, "Failed to read EEPROM");

	return ret;
}

/*
 * Write one more values to the EEPROM.
 */
int eeprom_write(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t *buf, uint8_t buf_len, uint8_t offset)
{
	int ret = SUCCESS;
	uint32_t eeprom_req_data = 0;

	if (!amc_ctrl_ctxt || !buf || (buf_len == 0))
		return -EINVAL;

	AMI_VDBG(
		amc_ctrl_ctxt,
		"Attempting to write EEPROM at offset:%d len:%d",
		offset, buf_len
	);

	eeprom_req_data = EEPROM_SET_TYPE(AMC_PROXY_CMD_RW_REQUEST_WRITE);
	eeprom_req_data |= EEPROM_SET_OFFSET(offset);
	ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_EEPROM_READ_WRITE, eeprom_req_data, buf,
							 buf_len);
	
	if (ret)
		AMI_ERR(amc_ctrl_ctxt, "Failed to write EEPROM");

	return ret;
}
