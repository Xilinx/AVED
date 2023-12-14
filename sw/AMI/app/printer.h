// SPDX-License-Identifier: GPL-2.0-only
/*
 * printer.h - This file contains utilities for printing formatted output
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_APP_PRINTER_H
#define AMI_APP_PRINTER_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

/* External Includes */
#include "json.h"

/* API Includes */
#include "ami_device.h"
#include "table.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define  APP_HEXDUMP_GROUPS_32 (4)
#define  APP_HEXDUMP_GROUPS_8  (16)

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum app_out_format - Output format for commands which report info.
 * @APP_OUT_FORMAT_TABLE: Format data into a table.
 * @APP_OUT_FORMAT_JSON: Format data as JSON.
 * @APP_OUT_FORMAT_INVALID: Unrecognised output format.
 */
enum app_out_format {
	APP_OUT_FORMAT_TABLE,
	APP_OUT_FORMAT_JSON,

	APP_OUT_FORMAT_INVALID = -1,
};

/*****************************************************************************/
/* Typedefs                                                                  */
/*****************************************************************************/

/**
 * typedef app_value_builder - Function to populate generic table/json data values.
 * @dev: AMI device handle.
 * @values: Pointer to implementation specific data structure holding values.
 * @n_rows: Pointer to number of rows (records) in data structure.
 * @n_fields: Pointer to number of elements in each row.
 * @fmt: Output format/format of data structure.
 * @data: Implementation specific data.
 * 
 * Fuctions of this type should be passed to the functions `print_json`
 * and `print_table` to print arbitrary data in a specific format.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
typedef int (*app_value_builder)(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data);

/**
 * typedef app_header_builder - Function to populate table headers.
 * @dev: AMI device handle.
 * @header: Pointer to pre-allocated header.
 * @n_fields: Number of columns in the table/header.
 * @data: Implementation specific data.
 * 
 * Note that functions of this type are applicable only to printing tables.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
typedef int (*app_header_builder)(ami_device *dev, char **header,
	int n_fields, void *data);

/*****************************************************************************/
/* Public function declarations                                              */
/*****************************************************************************/

/**
 * my_fprintf() - Wrapper around `fprintf` which will print to stdout and
 *                write to the given file if not NULL.
 * @stream: Output stream (should not be stdout).
 * @format: Format specifier.
 *
 * Useful for commands that need to print to the screen and potentially
 * also write to a specified output file.
 * 
 * Return: None.
 */
void my_fprintf(FILE *stream, const char *format, ...);

/**
 * my_putc() - Wrapper around `putc` which will print to stdout and
 *             write to the given file if not NULL.
 * @c: Character to write.
 * @stream: Output stream (should not be stdout).
 *
 * Useful for commands that need to print to the screen and potentially
 * also write to a specified output file.
 * 
 * Return: None.
 */
void my_putc(const char c, FILE *stream);

/**
 * print_divider() - Utility function to print a divider of variable length.
 * @c: Character to use.
 * @len: Length of divider.
 * @stream: Output stream (if NULL, print only to stdout)
 * 
 * Return: None.
 */
void print_divider(const char c, uint8_t len, FILE *stream);

/**
 * print_hexdump() - Utility function to display a data buffer in a similar
 *   format to the `hexdump` command.
 * @start_addr: Start offset of the data (this is only used for printing).
 * @values: Data buffer.
 * @num_values: Number of values in data buffer.
 * @num_groups: NUmber of values to have on a single line.
 * @value_size: Size of a single value in the data buffer.
 */
void print_hexdump(uint64_t start_addr, void *values, uint32_t num_values,
	uint8_t num_groups, size_t value_size);

/**
 * gen_json_data() - Create a JSON object from arbitrary data.
 * @dev: Device handle (optional).
 * @n_fields: Number of fields in each row (object).
 * @n_rows: Number of rows (objects).
 * @populate_values: Implementation specific function to populate JSON values.
 * @data: Implementation specific data (optional).
 * @out: Argument to store the generated JSON object.
 * 
 * This function allocates the memory for a top level JSON parent,
 * populates it with children objects, and returns the resulting object via
 * the `out` parameter. If you only want to print the resulting JSON, see the
 * function `print_json_data`.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int gen_json_data(ami_device *dev, int n_fields, int n_rows,
	app_value_builder populate_values, void *data, JsonNode **out);

/**
 * print_json_obj() - Print a JSON object as a human readable string
 * @obj: Object to print.
 * @stream: Output stream.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_json_obj(JsonNode *obj, FILE *stream);

/**
 * print_json_data() - Print arbitrary data as a JSON string.
 * @dev: Device handle (optional).
 * @n_fields: Number of fields in each row (object).
 * @n_rows: Number of rows (objects).
 * @stream: Output stream.
 * @populate_values: Implementation specific function to populate JSON values.
 * @data: Implementation specific data (optional).
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_json_data(ami_device *dev, int n_fields, int n_rows, FILE *stream,
	app_value_builder populate_values, void *data);

/**
 * print_table_data() - Format arbitrary data into a table.
 * @dev: Device handle..
 * @n_fields: Number of fields (columns) in each row.
 * @n_rows: Number of rows.
 * @stream: Output stream.
 * @divider_fmt: Row divider printing rule.
 * @populate_values: Implementation specific function to populate table values.
 * @populate_header: Implementation specific function to populate table header.
 * @data: Implementation specific data.
 * @col_align: Optional column alignment values.
 * 
 * This function allocates the memory for a table, populates the table values
 * and finally prints the table to the indicated output stream. The number of 
 * fields (columns) and number of rows might differ in the printed table, as 
 * these variables may be modified by the populate_values callback.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_table_data(ami_device *dev, int n_fields, int n_rows, FILE *stream,
	enum table_divider_format divider_fmt, app_value_builder populate_values,
	app_header_builder populate_header, void *data, int *col_align);

/**
 * print_progress_bar() - Print a progress bar.
 * @cur: Current value.
 * @max: Max posible value.
 * @width: Width of the progress bar (characters).
 * @left: Left margin character.
 * @right: Right margin character.
 * @fill: Progress fill character.
 * @empty: Remaining fill character.
 * @state: Last progress bar state; this is either '|' or '-'
 * 
 * The progress bar state is used simply to determine which character
 * gets printed after the progress bar (either '|' or '-'). This is useful
 * as it lets the user know that nothing is hanging if progress
 * updates are slow. The state can be initially set to 0 and on subsequent
 * calls set to the return value of the previous call.
 *
 * Return: The new progress bar state
 */
char print_progress_bar(uint32_t cur, uint32_t max, uint32_t width,
	char left, char right, char fill, char empty, char state);

#endif
