// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_debug_verbosity.c - This file contains the implementation for the command "debug_verbosity"
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
#include <string.h>

/* API includes */
#include "ami.h"
#include "ami_device.h"

/* App includes */
#include "commands.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_debug_verbosity() - "debug_verbosity" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_debug_verbosity(struct app_option *options, int num_args, char **args);

/**
 * parse_debug_level() - Parse debug level option string into an enum value.
 * @debug_level: The human readable string to parse.
 * @level: Enum to store the parsed debug level if valid.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int parse_debug_level(const char *debug_level, enum ami_amc_debug_level *level);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * l: Debug level
*/
static const char short_options[] = "hd:l:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"debug_verbosity - set the AMC debug level\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " debug_verbosity -d <bdf> -l <level>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help             Show this screen\r\n"
	"\t-d <b>:[d].[f]        Specify the device BDF\r\n"
	"\t-l <level>            Debug level\r\n"
	"\t                      Possible values are:\r\n"
	"\t                        {log, info, error, warning, debug}\r\n"
;

struct app_cmd cmd_debug_verbosity = {
	.callback      = &do_cmd_debug_verbosity,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * Parse the debug level option.
 */
static int parse_debug_level(const char *debug_level, enum ami_amc_debug_level *level)
{
	int ret = EXIT_SUCCESS;

	if (!debug_level || !level)
		return EXIT_FAILURE;

	if (strcmp(debug_level, "log") == 0)
		*level = AMI_AMC_DEBUG_LEVEL_LOG;
	else if (strcmp(debug_level, "info") == 0)
		*level = AMI_AMC_DEBUG_LEVEL_INFO;
	else if (strcmp(debug_level, "error") == 0)
		*level = AMI_AMC_DEBUG_LEVEL_ERROR;
	else if (strcmp(debug_level, "warning") == 0)
		*level = AMI_AMC_DEBUG_LEVEL_WARNING;
	else if (strcmp(debug_level, "debug") == 0)
		*level = AMI_AMC_DEBUG_LEVEL_DEBUG;
	else
		ret = EXIT_FAILURE;

	return ret;
}

/*
 * "debug_verbosity" command callback.
 */
static int do_cmd_debug_verbosity(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;
	struct app_option *device = NULL;
	struct app_option *level = NULL;
	ami_device *dev = NULL;
	/* using AMI_AMC_DEBUG_LEVEL_LOG as a placeholder, this will be overwritten anyway */
	enum ami_amc_debug_level debug_level = AMI_AMC_DEBUG_LEVEL_LOG;

	/* Must have at least a device. */
	if (!options) {
		APP_USER_ERROR("not enough options", help_msg);
		return EXIT_FAILURE;
	}

	/* level option is required */
	if (NULL != (level = find_app_option('l', options))) {
		if (parse_debug_level(level->arg, &debug_level) != EXIT_SUCCESS) {
			APP_USER_ERROR("invalid debug level specified", help_msg);
			return EXIT_FAILURE;
		}
	} else {
		APP_USER_ERROR("debug level not specified", help_msg);
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
	
	printf("Setting AMC debug level to '%s'.\r\n", level->arg);
	
	if (ami_dev_set_amc_debug_level(dev, debug_level) == AMI_STATUS_OK) {
		printf("OK. debug level updated!\r\n");
		ret = EXIT_SUCCESS;
	} else {
		APP_API_ERROR("could not update debug level");
	}

	ami_dev_delete(&dev);
	return ret;
}
