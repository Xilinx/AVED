// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_eeprom.h - This file contains functions to read/write from the AVED Manufacturing EEPROM.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */
 
#ifndef AMI_EEPROM_H
#define AMI_EEPROM_H

#include <linux/types.h>
#include "ami_top.h"
#include "ami_amc_control.h"

#define	EEPROM_TYPE_POS				(0)
#define	EEPROM_TYPE_MASK			(0x01)
#define	EEPROM_OFFSET_POS			(8)
#define	EEPROM_OFFSET_MASK			(0xFF)

#define EEPROM_GET_OFFSET(data)    		((data >> EEPROM_OFFSET_POS) & EEPROM_OFFSET_MASK)
#define EEPROM_GET_TYPE(data)    		((data >> EEPROM_TYPE_POS) & EEPROM_TYPE_MASK)

#define EEPROM_SET_OFFSET(data)     	        ((data & EEPROM_OFFSET_MASK) << EEPROM_OFFSET_POS)
#define EEPROM_SET_TYPE(data)    		((data & EEPROM_TYPE_MASK) << EEPROM_TYPE_POS)

/**
 * eeprom_read() - Read one or more values from the EEPROM.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @buf: Buffer to be populated with the bytes read.
 * @buf_len: The number of bytes to be read.
 * @offset: The offset from the base address of the EEPROM.
 * 
 * Return: 0 or negative error code.
 */
int eeprom_read(struct amc_control_ctxt *amc_ctrl_ctxt,
		uint8_t *buf,
		uint8_t buf_len,
		uint8_t offset);

/**
 * eeprom_write() - Write one more values to the EEPROM.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @buf: Buffer to be populated with the bytes to write.
 * @buf_len: The number of bytes to be written.
 * @offset: The offset from the base address of the EEPROM.
 * 
 * Return: 0 or negative error code.
 */
int eeprom_write(struct amc_control_ctxt *amc_ctrl_ctxt,
		uint8_t *buf,
		uint8_t buf_len,
		uint8_t offset);

#endif  /* AMI_EEPROM_H */
