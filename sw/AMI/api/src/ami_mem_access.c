// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_mem_access.c - This file contains the implementation of memory read/write logic.
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
#include "ami_mem_access.h"

/* Private API includes */
#include "ami_ioctl.h"
#include "ami_internal.h"
#include "ami_device_internal.h"

/*****************************************************************************/
/* Local function declarations                                               */
/*****************************************************************************/

/**
 * do_bar_transaction() - Do a PCI BAR read or write.
 * @dev: Device handle.
 * @idx: Bar number.
 * @offset: Offset within BAR.
 * @num: Number of BAR registers to read/write.
 * @val: Buffer to write to the BAR or to store values read from the BAR.
 * @request: IOCTL request code (AMI_IOC_READ_BAR or AMI_IOC_WRITE_BAR).
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int do_bar_transaction(ami_device *dev, uint8_t idx, uint64_t offset,
	uint32_t num, uint32_t *val, unsigned long request);

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/*
 * Do a PCI BAR read or write.
 */
static int do_bar_transaction(ami_device *dev, uint8_t idx, uint64_t offset,
	uint32_t num, uint32_t *val, unsigned long request)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_bar_data data = { 0 };

	if (!dev || !val || (num == 0))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if ((request != AMI_IOC_READ_BAR) && (request != AMI_IOC_WRITE_BAR))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR;  /* ami_open_cdev sets the last error */

	data.num = num;
	data.addr = (unsigned long)val;
	data.bar_idx = idx;
	data.offset = offset;
	data.cap_override = dev->cap_override;

	errno = 0;

	if (ioctl(dev->cdev, request, &data) == AMI_LINUX_STATUS_ERROR)
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	else
		ret = AMI_STATUS_OK;

	return ret;
}

/*****************************************************************************/
/* Public API function definitions                                           */
/*****************************************************************************/

/*
 * Read from a PCI bar.
 */
int ami_mem_bar_read(ami_device *dev, uint8_t idx, uint64_t offset, uint32_t *val)
{
	if (!dev || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_bar_transaction(
		dev,
		idx,
		offset,
		1,
		val,
		AMI_IOC_READ_BAR
	);
}

/*
 * Write to a PCI bar.
 */
int ami_mem_bar_write(ami_device *dev, uint8_t idx, uint64_t offset, uint32_t val)
{
	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_bar_transaction(
		dev,
		idx,
		offset,
		1,
		&val,
		AMI_IOC_WRITE_BAR
	);
}

/*
 * Read block from a PCI bar.
 */
int ami_mem_bar_read_range(ami_device *dev, uint8_t idx, uint64_t offset,
	uint32_t num, uint32_t *val)
{
	if (!dev || !val || (num == 0))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_bar_transaction(
		dev,
		idx,
		offset,
		num,
		val,
		AMI_IOC_READ_BAR
	);
}

/*
 * Write block to a PCI bar.
 */
int ami_mem_bar_write_range(ami_device *dev, uint8_t idx, uint64_t offset,
	uint32_t num, uint32_t *val)
{
	if (!dev || !val || (num == 0))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_bar_transaction(
		dev,
		idx,
		offset,
		num,
		val,
		AMI_IOC_WRITE_BAR
	);
}
