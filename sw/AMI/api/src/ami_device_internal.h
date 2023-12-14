// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_device_internal.h - This file contains the private interface for device related logic
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_DEVICE_INTERNAL_H
#define AMI_DEVICE_INTERNAL_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>

/* Public API includes */
#include "ami_device.h"

/* Private API includes */
#include "ami_internal.h"
#include "ami_sensor_internal.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define AMI_DEV_SYSFS_DIR	"/sys/bus/pci/drivers/ami/0000:%02x:%02x.%1x"
#define AMI_DEV_SYSFS_NODE	AMI_DEV_SYSFS_DIR "/%s"
#define AMI_SYSFS_PATH_MAX	(256)
#define AMI_SYSFS_STR_MAX	(256)

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * struct ami_device - represents a single PCI device
 * @bdf: device BDF
 * @cap_override: bypass permission checks when communicating with the driver
 * @cdev_num: character device number
 * @cdev: character device file handle (may be invalid)
 * @hwmon_num: hwmon device number
 * @num_sensors: number of suported sensors (eg. vccint, 12v_pex, etc...)
 * @num_total_sensors: total number of sensors  (e.g. vccint temp, vccint power, etc...)
 * @sensors: list of supported sensors (head)
 * 
 * If `cap_override` is set to true, all IOCTL's (and any other relevant API)
 * issued using this device handle will bypass any permission checks
 * and execute code that would, normally, only be reachable with root/sudo!
 */
struct ami_device {
	uint16_t            bdf;
	bool                cap_override;
	int                 cdev_num;
	int                 cdev;
	int                 hwmon_num;
	int                 num_sensors;
	int                 num_total_sensors;
	struct ami_sensor  *sensors;
};

/*****************************************************************************/
/* Private API function declarations                                         */
/*****************************************************************************/

/*
 * ami_open_cdev() - Open a character device file if not already opened.
 * @dev: AMI device handle to which the cdev file belongs.
 *
 * Return: AMI_STATUS_OK if opened, AMI_STATUS_ERROR otherwise
 */
int ami_open_cdev(ami_device *dev);

/*
 * ami_close_cdev() - Close a character device file if not already closed.
 * @dev: AMI device handle to which the cdev file belongs.
 *
 * Return: AMI_STATUS_OK if closed, AMI_STATUS_ERROR otherwise
 */
int ami_close_cdev(ami_device *dev);

/*
 * ami_read_sysfs() - Read a sysfs attribute.
 * @dev: Device handle.
 * @attr: Attribute name.
 * @buf: Buffer to store attribute value. Must be at least AMI_SYSFS_STR_MAX bytes.
 * 
 * Return: AMI_STATUS_ERROR or AMI_STATUS_OK
 */
int ami_read_sysfs(ami_device *dev, const char *attr, char *buf);

/**
 * ami_dev_register() - Register the current process with the driver.
 * @dev: Device handle.
 *
 * Return: AMI_STATUS_ERROR or AMI_STATUS_OK
 */
int ami_dev_register(ami_device *dev);

/**
 * ami_dev_deregister() - Deregister the current process from the driver.
 * @dev: Device handle.
 *
 * Return: AMI_STATUS_ERROR or AMI_STATUS_OK
 */
int ami_dev_deregister(ami_device *dev);

#endif  /* AMI_DEVICE_INTERNAL_H */
