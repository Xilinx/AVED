// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_sensors.c - This file contains the implementation for the command "sensors"
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

/* App includes */
#include "commands.h"
#include "apputils.h"
#include "sensors.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_sensors() - "sensors" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_sensors(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * v: Verbose
 * f: Output format
 * o: Output file
 * n: Sensor name
 * x: Extra attributes
 * 
 * `x` can be specified multiple times or passed in as a comma-separated list
 * `f` must be specified together with `o`
 */
static const char short_options[] = "hd:vf:o:n:x:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"sensors - view device sensor information\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " sensors [-d <bdf>] [options...]\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help             Show this screen.\r\n"
	"\t-d <b>:[d].[f]        Specify the device BDF\r\n"
	"\t-f <table|json>       Set the output format\r\n"
	"\t-o <file>             Specify output file\r\n"
	"\t-n <sensor>           Fetch specific sensor\r\n"
	"\t-x <field,...>        Print extra fields\r\n"
	"\t                      Possible values are:\r\n"
	"\t                        {max, average, limits}\r\n"
	"\t-v                    Print all extra fields\r\n"
;

struct app_cmd cmd_sensors = {
	.callback      = &do_cmd_sensors,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "sensors" command callback.
 */
static int do_cmd_sensors(struct app_option *options, int num_args, char **args)
{
	/* options are not required */
	return report_sensors(options);
}
