// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_sysfs.h - This file contains sysfs-related functions.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_SYSFS_H
#define AMI_SYSFS_H

#include "ami_top.h"

/**
 * register_sysfs() - Registers the sysfs subsystem.
 * @dev: The PCI device structure.
 * 
 * This function expects the driver data of the given device to be already
 * populated and valid.
 * 
 * Return: 0 or negative error.
 */
int register_sysfs(struct device *dev);

/**
 * remove_sysfs() - Remove all previously registered sysfs attributes.
 * @dev: The PCI device structure.
 * 
 * This function expects the driver data of the given device to be already
 * populated and valid.
 * 
 * Return: None.
 */
void remove_sysfs(struct device *dev);

#endif  /* AMI_SYSFS_H */
