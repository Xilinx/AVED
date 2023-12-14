// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_module_byte_rd.c - This file contains the implementation for the command "module_byte_rd"
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
#include <getopt.h>
#include <unistd.h>
#include <inttypes.h>

/* API include */
#include "ami_module_access.h"

/* App includes */
#include "commands.h"
#include "apputils.h"
#include "amiapp.h"
#include "printer.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_module_byte_rd() - "module_byte_rd" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_module_byte_rd(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * c: Cage (module) ID
 * p: Page number
 * b: Byte offset
 */
static const char short_options[] = "hd:c:p:b:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"module_byte_rd - Read from a QSFP module\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " module_byte_rd -d <bdf> -c <n> -p <n> -b <n>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help          Show this screen\r\n"
	"\t-d <b>:[d].[f]     Specify the device BDF\r\n"
	"\t-c <cage>          Module ID to read from\r\n"
	"\t-p <page>          Page number to read\r\n"
	"\t-b <byte>          Specify the offset to read from\r\n"
;

struct app_cmd cmd_module_byte_rd = {
	.callback      = &do_cmd_module_byte_rd,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "module_byte_rd" command callback.
 */
static int do_cmd_module_byte_rd(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *opt = NULL;
	struct app_option *device = NULL;
	ami_device *dev = NULL;
	uint16_t bdf = 0;
	uint8_t val = 0;

	/* Parsed options */
	uint8_t cage = 0;
	uint8_t page = 0;
	uint8_t off = 0;

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

	/* Cage */
	if (!(opt = find_app_option('c', options))) {
		APP_USER_ERROR("cage not specified", help_msg);
		return EXIT_FAILURE;
	} else {
		cage = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Page */
	if (!(opt = find_app_option('p', options))) {
		APP_USER_ERROR("page not specified", help_msg);
		return EXIT_FAILURE;
	} else {
		page = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Offset */
	if (!(opt = find_app_option('b', options))) {
		APP_USER_ERROR("byte offset not specified", help_msg);
		return EXIT_FAILURE;
	} else {
		off = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Find device */
	if (ami_dev_find(device->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return EXIT_FAILURE;
	}

	ami_dev_get_pci_bdf(dev, &bdf);

	printf(
		"Reading byte 0x%02x from page %d (device %02x:%02x.%01x, cage %d)\r\n",
		off, page, AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf), cage
	);

	if (ami_module_read( dev, cage, page, off, 1, &val) == AMI_STATUS_OK) {
		ret = EXIT_SUCCESS;
		printf("Value 0x%02x\r\n", val);
	} else {
		APP_API_ERROR("could not read data");
	}

	ami_dev_delete(&dev);
	return ret;
}
