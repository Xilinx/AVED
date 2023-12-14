// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_eeprom_access.h - This file contains the eeprom access interface.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_EEPROM_ACCESS_H
#define AMI_EEPROM_ACCESS_H

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
 * ami_eeprom_read() - Read one or more bytes of data from the EEPROM.
 * @dev: Device handle.
 * @offset: Offset into the EEPROM from base.
 * @num: Number of values to read.
 * @val: Buffer to store the values read.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_eeprom_read(ami_device *dev, uint8_t offset, uint8_t num, uint8_t *val);

/**
 * ami_eeprom_write() - Write one or more bytes of data to the EEPROM.
 * @dev: Device handle.
 * @offset: Offset into the EEPROM from base.
 * @num: Number of values to write.
 * @val: Register values to write.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_eeprom_write(ami_device *dev, uint8_t offset, uint8_t num, uint8_t *val);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_EEPROM_ACCESS_H */
