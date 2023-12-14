// SPDX-License-Identifier: GPL-2.0-only
/*
 * meta.h - This file contains utilities for printing AMI info/metadata
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_APP_META_H
#define AMI_APP_META_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* API includes */
#include "ami_device.h"

/* App includes */
#include "amiapp.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * print_overview() - Utility function to print AMI overview.
 * @options: List of command line options.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_overview(struct app_option *options);

/**
 * print_pcieinfo() - Print PCI-related info.
 * @dev: Device handle.
 * @options: List of command line options.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_pcieinfo(ami_device *dev, struct app_option *options);

/**
 * print_fpt_info() - Print FPT-related info.
 * @dev: Device handle.
 * @options: List of command line options.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_fpt_info(ami_device *dev, struct app_option *options);

/**
 * print_mfg_info() - Print Manufacturing Information.
 * @dev: Device handle.
 * @options: List of command line options.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_mfg_info(ami_device *dev, struct app_option *options);

#endif
