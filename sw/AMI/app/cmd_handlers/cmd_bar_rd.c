// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_bar_rd.c - This file contains the implementation for the command "bar_rd"
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
#include "ami_mem_access.h"

/* App includes */
#include "commands.h"
#include "apputils.h"
#include "amiapp.h"
#include "printer.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_bar_rd() - "bar_rd" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_bar_rd(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * b: BAR
 * a: Offset
 * l: length
 * o: Output file
 */
static const char short_options[] = "hd:b:a:l:o:";

static const struct option long_options[] = {
	{ "help", no_argument,  NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"bar_rd - Read from PCI BAR memory\r\n"
	"\r\nThis command requires root/sudo permissions.\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " bar_rd -d <bdf> -b <bar> -a <addr>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help          Show this screen\r\n"
	"\t-d <b>:[d].[f]     Specify the device BDF\r\n"
	"\t-b <bar>           Specify the BAR to read\r\n"
	"\t-a <addr>          Specify the offset to read from\r\n"
	"\t-l <len>           Number of registers to read (default=1)\r\n"
	"\t-o <file>          Output file\r\n"
;

struct app_cmd cmd_bar_rd = {
	.callback      = &do_cmd_bar_rd,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = true,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "bar_rd" command callback.
 */
static int do_cmd_bar_rd(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *opt = NULL;
	struct app_option *device = NULL;
	ami_device *dev = NULL;
	uint16_t bdf = 0;

	/* Positional arguments */
	uint8_t bar = 0;
	uint64_t offset = 0;
	uint32_t num = 1;  /* Default to a single register */

	uint32_t *buf = NULL;

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

	/* Check if output file is specified */
	if (NULL != (opt = find_app_option('o', options))) {
		if (access(opt->arg, F_OK) == 0) {
			APP_ERROR("output file already exists");
			return EXIT_FAILURE;
		}
	}

	/* BAR */
	if (!(opt = find_app_option('b', options))) {
		APP_USER_ERROR("BAR not specified", help_msg);
		return EXIT_FAILURE;
	} else {
		bar = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Offset */
	if (!(opt = find_app_option('a', options))) {
		APP_USER_ERROR("Offset not specified", help_msg);
		return EXIT_FAILURE;
	} else {
		offset = (uint64_t)strtoul(opt->arg, NULL, 0);
	}

	/* Size */
	if ((opt = find_app_option('l', options)) != NULL) {
		num = (uint32_t)strtoul(opt->arg, NULL, 0);
	}

	/* Find device */
	if (ami_dev_find(device->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return EXIT_FAILURE;
	}

	ami_dev_get_pci_bdf(dev, &bdf);

	printf(
		"Reading %d register(s) from device %02x:%02x.%01x"
		" and BAR %d at offset 0x%016" PRIx64 "\r\n\r\n",
		num, AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf), bar, offset
	);

	buf = (uint32_t*)calloc(num, sizeof(uint32_t));

	if (buf) {
		if (num == 1) {
			ret = ami_mem_bar_read(
				dev, bar, offset, &buf[0]
			);
		} else {
			ret = ami_mem_bar_read_range(
				dev, bar, offset, num, buf
			);
		}

		if (ret == AMI_STATUS_OK) {
			ret = EXIT_SUCCESS;

			if ((opt = find_app_option('o', options))) {
				if (write_hex_data(opt->arg, buf, num, sizeof(uint32_t)) == EXIT_SUCCESS) {
					printf("Data written to output file.\r\n");
				} else {
					APP_ERROR("could not write data to output file");
				}
			} else {
				print_hexdump(offset, buf, num, APP_HEXDUMP_GROUPS_32, sizeof(uint32_t));
			}
		} else {
			APP_API_ERROR("could not read data");
		}

		free(buf);
	} else {
		APP_ERROR("could not allocate memory");
	}

	ami_dev_delete(&dev);
	return ret;
}
