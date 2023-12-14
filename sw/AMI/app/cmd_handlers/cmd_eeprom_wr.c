// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_eeprom_wr.c - This file contains the implementation for the command "eeprom_wr"
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
#include "ami_eeprom_access.h"

/* App includes */
#include "commands.h"
#include "apputils.h"
#include "amiapp.h"
#include "printer.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_eeprom_wr() - "eeprom_wr" command callback.
 * 
 * For parameters and return value see the definition for `app_command`.
 */
static int do_cmd_eeprom_wr(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * a: Offset
 * i: Input value
 * I: Input file
 * 
 * i and I are mutually exclusive.
 */
static const char short_options[] = "hd:a:i:I:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"eeprom_wr - Write to the device EEPROM\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " eeprom_wr -d <bdf> -a <addr> (-i|-I) <input>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help          Show this screen\r\n"
	"\t-d <b>:[d].[f]     Specify the device BDF\r\n"
	"\t-a <addr>          Specify the offset to write to\r\n"
	"\t-i <value>         Register value to write\r\n"
	"\t-I <file>          File to write\r\n"
;

struct app_cmd cmd_eeprom_wr = {
	.callback      = &do_cmd_eeprom_wr,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "eeprom_wr" command callback.
 */
static int do_cmd_eeprom_wr(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *opt = NULL;
	ami_device *dev = NULL;
	uint16_t bdf = 0;

	/* Positional arguments */
	uint8_t offset = 0;
	uint8_t num = 0;

	uint8_t *buf = NULL;

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

	/* Offset */
	if (!(opt = find_app_option('a', options))) {
		APP_USER_ERROR("Offset not specified", help_msg);
		goto done;
	} else {
		offset = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Input */
	if (find_app_option('i', options) && find_app_option('I', options)) {
		APP_USER_ERROR("Cannot specify -i and -I", help_msg);
		goto done;
	} else {
		if ((opt = find_app_option('i', options))) {
			num = 1;
			buf = (uint8_t*)calloc(num, sizeof(uint8_t));

			if (buf) {
				buf[0] = (uint8_t)strtoul(opt->arg, NULL, 0);
			} else {
				APP_ERROR("could not allocate memory");
				goto done;
			}
		} else if ((opt = find_app_option('I', options))) {
			uint32_t n = 0;
			if (read_hex_data(opt->arg, (void**)&buf, &n, sizeof(uint8_t)) != EXIT_SUCCESS) {
				APP_ERROR("could not read input file");
				goto done;
			}
			num = (uint8_t)n;  /* Restrict the write to a uint8 */
		} else {
			APP_USER_ERROR("no input data", help_msg);
			goto done;
		}
	}

	printf(
		"Writing the following data to device %02x:%02x.%01x"
		" at offset 0x%02x\r\n\r\n",
		AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf), offset
	);
	print_hexdump(offset, buf, num, APP_HEXDUMP_GROUPS_8, sizeof(uint8_t));
	printf("\r\n");

	if ((NULL != find_app_option('F', options)) || confirm_action(APP_CONFIRM_PROMPT, 'Y', 3)) {
		ret = ami_eeprom_write(dev, offset, num, buf);

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
