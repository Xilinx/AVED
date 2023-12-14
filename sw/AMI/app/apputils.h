// SPDX-License-Identifier: GPL-2.0-only
/*
 * apputils.h - Utility functions for the AMI command line
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_APP_UTILS_H
#define AMI_APP_UTILS_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <linux/limits.h>

/* API includes */
#include "ami.h"
#include "ami_device.h"

/* App includes */
#include "printer.h"
#include "amiapp.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array)	(sizeof(array) / sizeof(array[0]))
#endif

#define APP_INVALID_INDEX	(-1)
#define APP_CONFIRM_PROMPT	"Are you sure you wish to proceed? [Y/n]: "

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * read_file() - Read an entire file into a byte buffer.
 * @fname: Full path to file.
 * @buf: Pointer to byte buffer.
 * @size: Pointer to variable which will hold buffer size.
 *
 * Note that the caller is responsible for freeing the memory allocated
 * by this function.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int read_file(const char *fname, uint8_t **buf, uint32_t *size);

/**
 * confirm_action() - Ask the user for confirmation before performing an action.
 * @prompt: Prompt to display to the user.
 * @yes: Valid user response (e.g. 'y', 'Y', etc...)
 * @attempts: Number of times to ask before returning.
 *
 * Note that the user prompt will timeout after 5 minutes if no input is
 * received - this relies on a Linux-only poll API.
 * 
 * Return: true if the user accepted, false otherwise.
 */
bool confirm_action(const char *prompt, const char yes, int attempts);

/**
 * find_logic_uuid() - Find the logic UUID of a PDI file. 
 * @pdi: Full PDI file path.
 * @uuid: Variable to store UUID.
 *
 * This function expects a file 'version.json' to exist in the same
 * directory as the PDI image - this file must contain the logic UUID.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int find_logic_uuid(const char pdi[PATH_MAX], char uuid[AMI_LOGIC_UUID_SIZE]);

/**
 * read_hex_data() - Read hex data from a file (to be written).
 * @fname: Full path to data file.
 * @values: Buffer to store values read from file.
 * @num_values: Variable to store number of values read.
 * @value_size: Size of a single value in the data buffer.
 * 
 * For this function and the `write_hex_data` functions, the file format is that
 * of a single register value per line. The size of a register depends on the
 * value of the `value_size` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int read_hex_data(const char *fname, void **values, uint32_t *num_values,
	size_t value_size);

/**
 * write_hex_data() - Write hex data to a file.
 * @fname: Full path to data file.
 * @values: Buffer containing values which were read.
 * @num_values: Number of values which were read.
 * @value_size: Size of a single value in the data buffer.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int write_hex_data(const char *fname, void *values, uint32_t num_values,
	size_t value_size);

/**
 * parse_output_options() - Parse output related options.
 * @options: Options to parse.
 * @fmt: Variable to store output format.
 * @verbose: Variable to store verbose flag (optional).
 * @stream: Variable to store parsed output stream.
 * @fmt_given: Boolean indicating if user gave -f option (optional).
 * @output_given: Boolean indicating if user gave -o option (optional).
 *
 * A lot of commands make use of the same output-related options and the logic
 * to parse those options is the same every time, so this is a helper
 * function to simplify the process.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int parse_output_options(struct app_option *options, enum app_out_format *fmt,
	bool *verbose, FILE **stream, bool *fmt_given, bool *output_given);

/**
 * warn_compat_mode() - Warn the user if a device is running in compatibility mode.
 * @dev: AMI device handle.
 *
 * Return: None.
 */
void warn_compat_mode(ami_device *dev);

#endif
