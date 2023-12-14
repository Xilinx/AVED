// SPDX-License-Identifier: GPL-2.0-only
/*
 * table.h - This file contains utilities for printing tables
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_APP_TABLE_H
#define AMI_APP_TABLE_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <stdbool.h>

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum table_divider_format - Specify when a row divider should be printed.
 * @TABLE_DIVIDER_NONE: Never print a divider.
 * @TABLE_DIVIDER_HEADER_ONLY: Print a divider after the header only.
 * @TABLE_DIVIDER_ALL: Print a divider for every row.
 * @TABLE_DIVIDER_GROUPS: Print a divider for every row, unless the row is
 *     missing data in the first column.
 */
enum table_divider_format {
	TABLE_DIVIDER_NONE,
	TABLE_DIVIDER_HEADER_ONLY,
	TABLE_DIVIDER_ALL,
	TABLE_DIVIDER_GROUPS,
};

/**
 * enum table_align - Specify the alignment of a table column.
 * @TABLE_ALIGN_RIGHT: Align text to the right (padding on left).
 * @TABLE_ALIGN_LEFT: Align text to the left (padding on right).
 */
enum table_align {
	TABLE_ALIGN_RIGHT,
	TABLE_ALIGN_LEFT,
};

/*****************************************************************************/
/* Public function declarations                                              */
/*****************************************************************************/

/**
 * print_table_row() - Print a single table row.
 * @num_cols: Number of columns.
 * @values: Row values.
 * @col_widths: Column widths.
 * @col_padding: Column padding.
 * @table_width: Total table width.
 * @divider: Should a row separator be printed.
 * @stream: Output stream (defaults to stdout)
 * @col_align: Optional alignment of columns (defaults to left).
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_table_row(int num_cols, char* values[],
	int *col_widths, int col_padding, int table_width, bool divider,
	FILE *stream, int *col_align);

/**
 * print_table() - Print data formatted into a table
 * @header: List of table headings.
 * @values: Table rows.
 * @num_cols: Number of columns in each row.
 * @num_rows: Number of rows in the table.
 * @divider_fmt: Row divider printing rule.
 * @stream: Output stream
 * @col_align: Optional alignment of columns (defaults to left).
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int print_table(char* header[], char** values[], int num_cols, int num_rows,
	enum table_divider_format divider_fmt, FILE *stream, int *col_align);

#endif
