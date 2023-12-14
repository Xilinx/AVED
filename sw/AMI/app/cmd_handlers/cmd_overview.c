// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_overview.c - This file contains the implementation for the command "overview"
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
#include "meta.h"

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_overview() - "overview" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_overview(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * f: Output format
 * o: Output file
 * v: Verbose output
 */
static const char short_options[] = "hf:o:v";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"overview - show basic AMI/device information\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " overview [options...]\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help            Show this screen\r\n"
	"\t-f <table|json>      Set the output format\r\n"
	"\t-o <file>            Specify output file\r\n"
	"\t-v                   Print verbose information\r\n"
;

struct app_cmd cmd_overview = {
	.callback      = &do_cmd_overview,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = false,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * "overview" command callback.
 */
static int do_cmd_overview(struct app_option *options, int num_args, char **args)
{
	/* options not required */
	return print_overview(options);
}
