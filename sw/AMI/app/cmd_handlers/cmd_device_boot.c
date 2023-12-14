// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_device_boot.c - This file contains the implementation for the command "device_boot"
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

/* API includes */
#include "ami.h"
#include "ami_program.h"

/* App includes */
#include "commands.h"
#include "apputils.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_device_boot() - "device_boot" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_device_boot(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * p: Partition number
 */
static const char short_options[] = "hd:p:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"device_boot - set device boot partition\r\n"
	"\r\nThis command requires root/sudo permissions.\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " device_boot -d <bdf> -p <n>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help           Show this screen\r\n"
	"\t-d <b>:[d].[f]      Specify the device BDF\r\n"
	"\t-p <partition>      Partition to select\r\n"
;

struct app_cmd cmd_device_boot = {
	.callback      = &do_cmd_device_boot,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "device_boot" command callback.
 */
static int do_cmd_device_boot(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;

	/* Required options */
	struct app_option *device = NULL;
	struct app_option *partition = NULL;

	/* Required data */
	ami_device *dev = NULL;
	uint32_t partition_number = 0;

	/* Must have at least a device and partition number. */
	if (!options) {
		APP_USER_ERROR("not enough options", help_msg);
		return EXIT_FAILURE;
	}

	/* Device and partition are required. */
	device = find_app_option('d', options);
	partition = find_app_option('p', options);

	if (!device || !partition) {
		APP_USER_ERROR("not enough arguments", help_msg);
		return AMI_STATUS_ERROR;
	}

	/* Find device */
	if (ami_dev_find(device->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return AMI_STATUS_ERROR;
	}

	/* Check compatibility mode */
	warn_compat_mode(dev);

	partition_number = (uint32_t)strtoul(partition->arg, NULL, 0);

	printf("Will do a hot reset to boot into partition %d. This may take a minute...\r\n",
		partition_number);
	
	if (ami_prog_device_boot(&dev, partition_number) == AMI_STATUS_OK) {
		ret = EXIT_SUCCESS;
		printf(
			"\r\nOK. Partition selected (%d) - quitting.\r\n"
			"***********************************************\r\n"
			"Hot reset has been performed into partition %d.\r\n"
			"***********************************************\r\n",
			partition_number, partition_number
		);
	} else {
		APP_API_ERROR("could not select partition");
	}

	ami_dev_delete(&dev);
	return ret;
}
