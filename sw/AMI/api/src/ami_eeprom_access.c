// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_eeprom_access.c - This file contains the eeprom access interface.
 *
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

/* Public API includes */
#include "ami_eeprom_access.h"

/* Private API includes */
#include "ami_ioctl.h"
#include "ami_internal.h"
#include "ami_device_internal.h"

/*****************************************************************************/
/* Public API function definitions                                           */
/*****************************************************************************/

/*
 * ami_eeprom_read() - Read one or more bytes of data from the EEPROM.
 */
int ami_eeprom_read(ami_device *dev, uint8_t offset, uint8_t num, uint8_t *val)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_eeprom_payload data = { 0 };

	if (!dev || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR; /* last error is set by ami_open_cdev */

	data.addr = (unsigned long)val;
	data.len = num;
	data.offset = offset;

	if (ioctl(dev->cdev, AMI_IOC_READ_EEPROM, &data) == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	} else {
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * ami_eeprom_write() - Write one or more bytes of data to the EEPROM.
 */
int ami_eeprom_write(ami_device *dev, uint8_t offset, uint8_t num, uint8_t *val)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_eeprom_payload data = { 0 };

	if (!dev || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR; /* last error is set by ami_open_cdev */

	data.addr = (unsigned long)val;
	data.len = num;
	data.offset = offset;

	if (ioctl(dev->cdev, AMI_IOC_WRITE_EEPROM, &data) == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	} else {
		ret = AMI_STATUS_OK;
	}

	return ret;
}
