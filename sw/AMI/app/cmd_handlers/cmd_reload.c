// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_reload.c - This file contains the implementation for the command "reload"
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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* API includes */
#include "ami.h"
#include "ami_device.h"

/* App includes */
#include "commands.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define RMMOD    "rmmod ami"
#define INSMOD   "modprobe ami"

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum reload_type - different mechanisms for reloading a device/devices
 * @RELOAD_TYPE_INVALID: Invalid reload type
 * @RELOAD_TYPE_DRIVER: Reload the entire driver.
 * @RELOAD_TYPE_PCI: Remove a PCI device and force a bus rescan.
 * @RELOAD_TYPE_SBR: Trigger a secondary bus reset.
 */
enum reload_type {
	RELOAD_TYPE_INVALID = -1,
	RELOAD_TYPE_DRIVER,
	RELOAD_TYPE_PCI,
	RELOAD_TYPE_SBR,
};

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_reload() - "reload" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_reload(struct app_option *options, int num_args, char **args);

/**
 * parse_reload_type() - Parse the reload type option string into an enum value.
 * @reload_type: The human readable string to parse.
 * 
 * Return: equivalent enum value or RELOAD_TYPE_INVALID if unknown
 */
static enum reload_type parse_reload_type(const char *reload_type);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * t: Reload type
*/
static const char short_options[] = "hd:t:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"reload - reload a device/devices\r\n"
	"\r\nThis command requires root/sudo permissions.\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " reload -t <type> [-d <bdf>]\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help            Show this screen\r\n"
	"\t-d <b>:[d].[f]       Specify the device BDF\r\n"
	"\t-t <type>            Specify reload type\r\n"
	"\t                     Possible values are:\r\n"
	"\t                       driver=reload the entire driver\r\n"
	"\t                       pci=force a pci removal and bus rescan\r\n"
	"\t                       sbr=trigger a secondary bus reset\r\n"
;

struct app_cmd cmd_reload = {
	.callback      = &do_cmd_reload,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = true,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * Parse the reload type option.
 */
static enum reload_type parse_reload_type(const char *reload_type)
{
	if (strcmp(reload_type, "driver") == 0)
		return RELOAD_TYPE_DRIVER;
	else if (strcmp(reload_type, "pci") == 0)
		return RELOAD_TYPE_PCI;
	else if (strcmp(reload_type, "sbr") == 0)
		return RELOAD_TYPE_SBR;
	
	return RELOAD_TYPE_INVALID;
}

/*
 * "reload" command callback.
 */
static int do_cmd_reload(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *opt = NULL;
	enum reload_type type = RELOAD_TYPE_INVALID;
	ami_device *dev = NULL;
	uint16_t bdf = 0;
	const char *bdf_string = NULL;
	char dev_state[AMI_DEV_STATE_SIZE] = { 0 };

	/* Must have at least a device. */
	if (!options) {
		APP_USER_ERROR("not enough options", help_msg);
		return EXIT_FAILURE;
	}

	if (NULL != (opt = find_app_option('t', options))) {
		type = parse_reload_type(opt->arg);
	} else {
		APP_USER_ERROR("reload type not specified", help_msg);
		return EXIT_FAILURE;
	}

	/* Find device */
	switch (type) {
	case RELOAD_TYPE_SBR:
	case RELOAD_TYPE_PCI:
		/* device option is required for these reload types */
		opt = find_app_option('d', options);

		if (!opt) {
			APP_USER_ERROR("device not specified", help_msg);
			return EXIT_FAILURE;
		}

		bdf_string = opt->arg;

		/*
		 * The PCI reload works for *any* PCI device - even those
		 * which are not attached to the AMI driver. We will use the
		 * raw BDF string instead of a device handle.
		 */
		if (type != RELOAD_TYPE_PCI) {
			/* Find device */
			if (ami_dev_find(bdf_string, &dev) != AMI_STATUS_OK) {
				APP_API_ERROR("could not find the requested device");
				return EXIT_FAILURE;
			}

			ami_dev_get_pci_bdf(dev, &bdf);
			ami_dev_get_state(dev, dev_state);
		} else {
			bdf = ami_parse_bdf(bdf_string);
		}
		
		break;
	
	default:
		break;
	}

	switch (type) {
	case RELOAD_TYPE_DRIVER:
		printf("Unloading AMI driver...\r\n");
		ret = system(RMMOD);

		if (ret == AMI_LINUX_STATUS_OK) {
			printf("Done. Re-inserting driver module...\r\n");
			ret = system(INSMOD);

			if (ret == AMI_LINUX_STATUS_OK)
				printf("OK. Driver has been reloaded.\r\n");
			else
				APP_ERROR("could not insert module");
		} else {
			APP_ERROR("could not unload driver");
		}
		break;
	
	case RELOAD_TYPE_PCI:
	{
		printf("Removing PCI device and rescanning bus...\r\n");

		if ((ret = ami_dev_pci_reload(NULL, bdf_string)) == AMI_STATUS_OK) 
			printf("Done.\r\n");
		else
			APP_API_ERROR("could not perform PCI reset");

		break;
	}

	case RELOAD_TYPE_SBR:
	{
		printf("Will trigger a secondary bus reset. This may take a minute...\r\n");

		if ((ret = ami_dev_hot_reset(&dev)) == AMI_STATUS_OK) {
			printf("Done.\r\n");
			ami_dev_delete(&dev);
		} else {
			APP_API_ERROR("could not perform reset");
		}
		break;
	}
	
	default:
		APP_ERROR("invalid reload type");
		break;
	}

	return ret;
}
