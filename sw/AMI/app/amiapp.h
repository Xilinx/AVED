// SPDX-License-Identifier: GPL-2.0-only
/*
 * amiapp.h - This file contains the command line application for the AMI API
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */
#ifndef AMI_APP_H
#define AMI_APP_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <getopt.h>

/* API includes */
#include "ami.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define APP_NAME		"ami_tool"

/* For user-related errors (e.g., invalid option) */
#define APP_USER_ERROR(s, help)	fprintf(stderr, "Error: %s\r\n\r\n%s\r\n", s, help)
/* For API-related errors */
#define APP_API_ERROR(s)	fprintf(stderr, "Error: %s\r\n%s", s, ami_get_last_error())

/* For generic errors and warnings */
#define APP_ERROR(s)		fprintf(stderr, "Error: %s\r\n", s)
#define APP_WARN(s)		fprintf(stderr, "Warning: %s\r\n", s)

/*****************************************************************************/
/* Typedefs                                                                  */
/*****************************************************************************/

/* Forward declaration - needed for typedef below. */
struct app_option;

/**
 * typedef app_command - Callback to perform a single CLI command
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
typedef int (*app_command)(struct app_option *options, int num_args, char **args);

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * struct app_option - Struct to hold a single command line option
 * @val: the argument "value" returned by getopt
 * @arg: argument passed in by user (may be none)
 * @long_ind: long option index returned by getopt, if applicable
 * @handled: has this option been handled by the callback
 * @next: pointer to next option
 */
struct app_option {
	int                  val;
	const char          *arg;
	int                  long_ind;
	bool                 handled;
	struct app_option   *next;
};

/**
 * struct app_cmd - Struct representing a single app command.
 * @callback: Function to call when this command is invoked.
 * @root_required: Does this command requires root/sudo permissions.
 * @short_options: List of short options that this command accepts.
 * @long_options: List of long options that this command accepts.
 * @help_msg: Message to print when -h/--help option is given.
 */
struct app_cmd {
	app_command           callback;
	bool                  root_required;
	const char           *short_options;
	const struct option  *long_options;
	const char           *help_msg;
};

/**
 * struct app_cmd_map - Map of command name to command struct
 * @name: command name
 * @command: command struct
 */
struct app_cmd_map {
	const char      *name;
	struct app_cmd  *command;
};

/*****************************************************************************/
/* Public function declarations                                              */
/*****************************************************************************/


/**
 * find_app_command() - Search for a command by its name in the global command map
 * @name: String name of the command
 *
 * Return: Index in the global map or APP_INVALID_INDEX
 */
int find_app_command(const char *name);

/**
 * find_app_option() - Check if a specific option exists in a list of options.
 * @val: Value of the option. For short options this is the char
 *   representation of the option; for long options, it is the `val` member
 *   of the `option` struct.
 * @options: Pointer to first option.
 * 
 * This function is useful for situations where certain options must be
 * evaluated before any others without having to restrict the ordering
 * on the command line (for example --help).
 *
 * Return: Pointer to the identified option or NULL.
 */
struct app_option* find_app_option(const int val, struct app_option *options);

#endif
