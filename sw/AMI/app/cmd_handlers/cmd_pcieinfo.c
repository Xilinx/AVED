// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_pcieinfo.c - This file contains the implementation for the command "pcieinfo"
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>

/* API includes */
#include "ami.h"
#include "ami_device.h"

/* App includes */
#include "commands.h"
#include "meta.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_pcieinfo() - "pcieinfo" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_pcieinfo(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * f: Output format
 * o: Output file
*/
static const char short_options[] = "hd:f:o:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"pcieinfo - view PCI-related information\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " pcieinfo -d <bdf> [options...]\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help             Show this screen\r\n"
	"\t-d <b>:[d].[f]        Specify the device BDF\r\n"
	"\t-f <table|json>       Set the output format\r\n"
	"\t-o <file>             Specify output file\r\n"
;

struct app_cmd cmd_pcieinfo = {
	.callback      = &do_cmd_pcieinfo,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "pcieinfo" command callback.
 */
static int do_cmd_pcieinfo(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *device = NULL;
	ami_device *dev = NULL;

	/* Must have at least a device. */
	if (!options) {
		APP_USER_ERROR("not enough options", help_msg);
		return EXIT_FAILURE;
	}

	/* device option is required */
	device = find_app_option('d', options);

	if (!device) {
		APP_USER_ERROR("device not specified", help_msg);
		return EXIT_FAILURE;
	}

	/* Find device */
	if (ami_dev_find(device->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return EXIT_FAILURE;
	}

	ret = print_pcieinfo(dev, options);
	ami_dev_delete(&dev);
	return ret;
}
