// SPDX-License-Identifier: GPL-2.0-only
/*
 * table.c - This file contains utilities for printing tables
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "table.h"
#include "printer.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define COLUMN_PADDING (2)

/*****************************************************************************/
/* Public function declarations                                              */
/*****************************************************************************/

/*
 * Print a single table row.
 */
int print_table_row(int num_cols, char* values[],
	int *col_widths, int col_padding, int table_width, bool divider,
	FILE *stream, int *col_align)
{
	int i = 0;

	/* col_align is optional */

	if (!col_widths || !values)
		return EXIT_FAILURE;

	/* Print divider first. */
	if (divider)
		print_divider('-', table_width, stream);

	/* Print columns. */
	for (i = 0; i < num_cols; i++) {
		if (i == 0) {
			if (col_align && (col_align[i] == TABLE_ALIGN_RIGHT))
				my_fprintf(
					stream,
					"%*s",
					col_widths[i] + col_padding,
					values[i]
				);
			else
				my_fprintf(
					stream,
					"%-*s",
					col_widths[i] + col_padding,
					values[i]
				);
		} else {
			if (values[i] && (values[i][0] != '\0')) {
				if (col_align && (col_align[i] == TABLE_ALIGN_RIGHT))
					my_fprintf(
						stream,
						" | %*s",
						col_widths[i] + col_padding,
						values[i]
					);
				else
					my_fprintf(
						stream,
						" | %-*s",
						col_widths[i] + col_padding,
						values[i]
					);
			}
		}
	}
	my_fprintf(stream, "\r\n");

	return EXIT_SUCCESS;
}

/*
 * Print a table.
 */
int print_table(char* header[], char** values[], int num_cols, int num_rows,
	enum table_divider_format divider_fmt, FILE *stream, int *col_align)
{
	int i = 0, j = 0;
	int table_width = 0;
	int *column_widths = NULL;
	bool print_divider = false;

	if (!header || !values)
		return EXIT_FAILURE;

	column_widths = (int*)calloc(num_cols, sizeof(int));

	if (!column_widths)
		return EXIT_FAILURE;

	/* Need to figure out max width of each column. */
	for (i = 0; i < num_cols; i++) {
		/* Start with the header size */
		int col_width = strlen(header[i]);
		int max_col = col_width;

		/* Check if any item is longer. */
		for (j = 0; j < num_rows; j++) {
			col_width = strlen(values[j][i]);

			if (col_width > max_col)
				max_col = col_width;
		}

		column_widths[i] = max_col;
	}

	/* Calculate total table width */
	for (i = 0; i < num_cols; i++) {
		table_width += column_widths[i] + COLUMN_PADDING;

		/* Account for column separator */
		if (i > 0)
			table_width += 3;
	}

	my_fprintf(stream, "\r\n");  /* whitespace padding */

	switch (divider_fmt) {
	case TABLE_DIVIDER_HEADER_ONLY:
	case TABLE_DIVIDER_ALL:
	case TABLE_DIVIDER_GROUPS:
		print_divider = true;
		break;
	
	default:
		break;
	}

	/* Print header */
	print_table_row(
		num_cols,
		header,
		column_widths,
		COLUMN_PADDING,
		table_width,
		false,
		stream,
		col_align
	);

	/* Print rows */
	for (j = 0; j < num_rows; j++) {
		switch (divider_fmt) {
		case TABLE_DIVIDER_HEADER_ONLY:
			if (j == 0)
				print_divider = true;
			else
				print_divider = false;
			break;

		case TABLE_DIVIDER_ALL:
			print_divider = true;
			break;

		case TABLE_DIVIDER_GROUPS:
			if ((j != 0) && (strlen(values[j][0]) == 0))
				print_divider = false;
			else
				print_divider = true;
			break;
		
		default:
			print_divider = false;
			break;
		}

		print_table_row(
			num_cols,
			values[j],
			column_widths,
			COLUMN_PADDING,
			table_width,
			print_divider,
			stream,
			col_align
		);
	}

	my_fprintf(stream, "\r\n");  /* whitespace padding */

	/* Cleanup. */
	free(column_widths);
	return EXIT_SUCCESS;
}
