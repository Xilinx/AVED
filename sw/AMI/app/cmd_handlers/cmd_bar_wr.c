// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_bar_wr.c - This file contains the implementation for the command "bar_wr"
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
 * do_cmd_bar_wr() - "bar_wr" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_bar_wr(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * b: BAR
 * a: Offset
 * i: Input value
 * I: Input file
 * 
 * i and I are mutually exclusive.
 */
static const char short_options[] = "hd:b:a:i:I:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"bar_wr - Write to PCI BAR memory\r\n"
	"\r\nThis command requires root/sudo permissions.\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " bar_wr -d <bdf> -b <bar> -a <addr> (-i|-I) <input>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help          Show this screen\r\n"
	"\t-d <b>:[d].[f]     Specify the device BDF\r\n"
	"\t-b <bar>           Specify the BAR to write\r\n"
	"\t-a <addr>          Specify the offset to write to\r\n"
	"\t-i <value>         Register value to write\r\n"
	"\t-I <file>          File to write\r\n"
;

struct app_cmd cmd_bar_wr = {
	.callback      = &do_cmd_bar_wr,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = true,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "bar_wr" command callback.
 */
static int do_cmd_bar_wr(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *opt = NULL;
	ami_device *dev = NULL;
	uint16_t bdf = 0;

	/* Positional arguments */
	uint8_t bar = 0;
	uint64_t offset = 0;
	uint32_t num = 0;

	uint32_t *buf = NULL;

	if (!options) {
		APP_USER_ERROR("not enough options", help_msg);
		return EXIT_FAILURE;
	}

	/* device option is required */
	if (!(opt = find_app_option('d', options))) {
		APP_USER_ERROR("device not specified", help_msg);
		return EXIT_FAILURE;
	}

	/* Find device */
	if (ami_dev_find(opt->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return EXIT_FAILURE;
	}

	ami_dev_get_pci_bdf(dev, &bdf);

	/* BAR */
	if (!(opt = find_app_option('b', options))) {
		APP_USER_ERROR("BAR not specified", help_msg);
		goto done;
	} else {
		bar = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Offset */
	if (!(opt = find_app_option('a', options))) {
		APP_USER_ERROR("Offset not specified", help_msg);
		goto done;
	} else {
		offset = (uint64_t)strtoul(opt->arg, NULL, 0);
	}

	/* Input */
	if (find_app_option('i', options) && find_app_option('I', options)) {
		APP_USER_ERROR("Cannot specify -i and -I", help_msg);
		goto done;
	} else {
		if ((opt = find_app_option('i', options))) {
			num = 1;
			buf = (uint32_t*)calloc(num, sizeof(uint32_t));

			if (buf) {
				buf[0] = (uint32_t)strtoul(opt->arg, NULL, 0);
			} else {
				APP_ERROR("could not allocate memory");
				goto done;
			}
		} else if ((opt = find_app_option('I', options))) {
			if (read_hex_data(opt->arg, (void**)&buf, &num, sizeof(uint32_t)) != EXIT_SUCCESS) {
				APP_ERROR("could not read input file");
				goto done;
			}
		} else {
			APP_USER_ERROR("no input data", help_msg);
			goto done;
		}
	}

	printf(
		"Writing the following data to device %02x:%02x.%01x"
		" and BAR %d at offset 0x%016" PRIx64 "\r\n\r\n",
		AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf), bar, offset
	);
	print_hexdump(offset, buf, num, APP_HEXDUMP_GROUPS_32, sizeof(uint32_t));
	printf("\r\n");

	if ((NULL != find_app_option('F', options)) ||
			confirm_action(APP_CONFIRM_PROMPT, 'Y', 3)) {
		if (num == 1) {
			ret = ami_mem_bar_write(
				dev, bar, offset, buf[0]
			);
		} else {
			ret = ami_mem_bar_write_range(
				dev, bar, offset, num, buf
			);
		}

		if (ret == AMI_STATUS_OK) {
			ret = EXIT_SUCCESS;
			printf("Successfully wrote to %d register(s)\r\n", num);
		} else {
			APP_API_ERROR("could not write data");
		}
	} else {
		ret = EXIT_SUCCESS;
		printf("\r\nAborting...\r\n");
	}

done:
	if (buf)
		free(buf);
	
	ami_dev_delete(&dev);
	return ret;
}
