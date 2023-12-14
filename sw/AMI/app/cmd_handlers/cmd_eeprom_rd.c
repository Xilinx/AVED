
// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_eeprom_rd.c - This file contains the implementation for the command "eeprom_rd"
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
 * do_cmd_eeprom_rd() - "eeprom_rd" command callback.
 * 
 * For parameters and return value see the definition for `app_command`.
 */
static int do_cmd_eeprom_rd(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * a: Offset
 * l: length
 * o: Output file
 */
static const char short_options[] = "hd:a:l:o:";

static const struct option long_options[] = {
	{ "help", no_argument,  NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"eeprom_rd - Read the device EEPROM\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " eeprom_rd -d <bdf> -a <addr>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help          Show this screen\r\n"
	"\t-d <b>:[d].[f]     Specify the device BDF\r\n"
	"\t-a <addr>          Specify the offset to read from\r\n"
	"\t-l <len>           Number of registers to read (default=1)\r\n"
	"\t-o <file>          Output file\r\n"
;

struct app_cmd cmd_eeprom_rd = {
	.callback      = &do_cmd_eeprom_rd,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "eeprom_rd" command callback.
 */
static int do_cmd_eeprom_rd(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *opt = NULL;
	struct app_option *device = NULL;
	ami_device *dev = NULL;
	uint16_t bdf = 0;

	/* Positional arguments */
	uint8_t offset = 0;
	uint8_t num = 1;  /* Default to a single register */

	uint8_t *buf = NULL;

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

	/* Offset */
	if (!(opt = find_app_option('a', options))) {
		APP_USER_ERROR("Offset not specified", help_msg);
		return EXIT_FAILURE;
	} else {
		offset = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Size */
	if ((opt = find_app_option('l', options)) != NULL) {
		num = (uint8_t)strtoul(opt->arg, NULL, 0);
	}

	/* Find device */
	if (ami_dev_find(device->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return EXIT_FAILURE;
	}

	ami_dev_get_pci_bdf(dev, &bdf);

	printf(
		"Reading %d byte(s) from device %02x:%02x.%01x"
		" at offset 0x%02x\r\n\r\n",
		num, AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf), offset
	);

	buf = (uint8_t*)calloc(num, sizeof(uint8_t));

	if (buf) {
		ret = ami_eeprom_read(dev, offset, num, buf);

		if (ret == AMI_STATUS_OK) {
			ret = EXIT_SUCCESS;

			if ((opt = find_app_option('o', options))) {
				if (write_hex_data(opt->arg, buf, num, sizeof(uint8_t)) == EXIT_SUCCESS) {
					printf("Data written to output file.\r\n");
				} else {
					APP_ERROR("could not write data to output file");
				}
			} else {
				print_hexdump(offset, buf, num, APP_HEXDUMP_GROUPS_8, sizeof(uint8_t));
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
