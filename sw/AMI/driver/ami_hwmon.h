// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_hwmon.h - This file contains definitions related to hwmon.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_HWMON_H
#define AMI_HWMON_H

#include <linux/hwmon.h>

#include "ami_top.h"

/**
 * register_hwmon() - Registers the hwmon subsystem.
 * @dev: The PCI device structure.
 * @pf_dev: The PCI device data structure.
 * 
 * This function registers ALL supported sensors/attributes
 * irrespective of whether the device supports them or not.
 * Per-device availability is verified by the `XXX_is_visible` callbacks,
 * so not all defined attributes may actually get created.
 * 
 * Return: 0 or error.
 */
int register_hwmon(struct device *dev, struct pf_dev_struct *pf_dev);

/**
 * remove_hwmon() - Unregister a hwmon device.
 * @dev: The hwmon device struct.
 * 
 * Note that this function should normally not be called as hwmon
 * is registered using managed memory.
 * 
 * Return: None.
 */
void remove_hwmon(struct device *dev);

/**
 * read_sensor_val() - Read a single sensor value.
 * @pf_dev: PCI device data structure.
 * @type: Hwmon sensor type.
 * @attr: Hwmon attribute type.
 * @channel: Hwmon channel.
 * @val: Variable to store sensor value.
 * @status: Variable to store sensor status (optional).
 * @fresh: Variable to store cache status for this value (optional).
 * 
 * Return: 0 or negative error code.
 */
int read_sensor_val(struct pf_dev_struct *pf_dev, enum hwmon_sensor_types type,
	u32 attr, int channel, long *val, char *status, bool *fresh);

#endif /* AMI_HWMON_H */
