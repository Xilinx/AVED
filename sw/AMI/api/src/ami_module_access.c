// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_module_access.c - This file contains the QSFP module access interface.
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
#include "ami_module_access.h"

/* Private API includes */
#include "ami_ioctl.h"
#include "ami_internal.h"
#include "ami_device_internal.h"

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

/**
 * do_module_rw() - Perform either a read or write operation on a QSFP module.
 * @ioc: The ioctl to send - either read or write.
 * @dev: Device handle.
 * @device_id: Module ID.
 * @page: Page number to read/write.
 * @offset: Offset within page to read/write.
 * @num: Number of bytes to read/write.
 * @val: Buffer to store data read or data to be written.
 *
 *  Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int do_module_rw(unsigned long ioc, ami_device *dev, uint8_t device_id,
	uint8_t page, uint8_t offset, uint8_t num, uint8_t *val)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_module_payload data = { 0 };

	if (!dev || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR; /* last error is set by ami_open_cdev */

	data.addr = (unsigned long)val;
	data.device_id = device_id;
	data.page = page;
	data.offset = offset;
	data.len = num;

	if (ioctl(dev->cdev, ioc, &data) == AMI_LINUX_STATUS_ERROR) {
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

/*****************************************************************************/
/* Public API function definitions                                           */
/*****************************************************************************/

/*
 * Read one or more bytes of data from to a QSFP module.
 */
int ami_module_read(ami_device *dev, uint8_t device_id, uint8_t page,
	uint8_t offset, uint8_t num, uint8_t *val)
{
	if (!dev || !val || (num == 0))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_module_rw(
		AMI_IOC_READ_MODULE,
		dev,
		device_id,
		page,
		offset,
		num,
		val
	);
}

/*
 * Write one or more bytes of data to a QSFP module
 */
int ami_module_write(ami_device *dev, uint8_t device_id, uint8_t page,
	uint8_t offset, uint8_t num, uint8_t *val)
{
	if (!dev || !val || (num == 0))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_module_rw(
		AMI_IOC_WRITE_MODULE,
		dev,
		device_id,
		page,
		offset,
		num,
		val
	);
}
