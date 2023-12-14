// SPDX-License-Identifier: GPL-2.0-only
/*
 * printer.c - This file contains utilities for printing formatted output
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

/* App includes */
#include "json.h"
#include "table.h"
#include "printer.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define TABLE_FIELD_MAX		(64)
#define TABLE_HEADING_MAX	(32)

/*****************************************************************************/
/* Public function definitions                                               */
/*****************************************************************************/

/*
 * Print to stdout and write to a secondary stream.
 */
void my_fprintf(FILE *stream, const char *format, ...)
{
	va_list args_stdout, args_stream;

	/* Write to stdout. */
	va_start(args_stdout, format);
	vfprintf(stdout, format, args_stdout);
	va_end(args_stdout);

	/* Write to output stream. */
	if (stream && (stream != stdout)) {
		va_start(args_stream, format);
		vfprintf(stream, format, args_stream);
		va_end(args_stream);
	}
}

/*
 * Write a char to stdout and to a secondary stream.
 */
void my_putc(const char c, FILE *stream)
{
	/* Write to stdout. */
	putc(c, stdout);

	/* Write to output stream. */
	if (stream && (stream != stdout)) {
		putc(c, stream);
	}
}

/*
 * Print a divider.
 */
void print_divider(const char c, uint8_t len, FILE *stream)
{
	int i = 0;
	for (i = 0; i < len; i++) {
		my_putc(c, stream);
	}
	my_fprintf(stream, "\r\n");
}

/*
 * Print a uint32 buffer.
 */
void print_hexdump(uint64_t start_addr, void *values, uint32_t num_values,
	uint8_t num_groups, size_t value_size)
{
	int i = 0;

	if (!values || (num_values == 0) || (num_groups == 0))
		return;
	
	for (i = 0; i < num_values; i++) {
		if ((i % num_groups) == 0)
			printf(
				"[ " "0x%016" PRIx64 " ]\t",
				start_addr + (i * value_size)
			);

		switch (value_size) {
		case sizeof(uint8_t):
			printf(" %02x", ((uint8_t*)values)[i]);
			break;

		case sizeof(uint16_t):
			printf(" %04x", ((uint16_t*)values)[i]);
			break;

		case sizeof(uint32_t):
			printf(" %08x", ((uint32_t*)values)[i]);
			break;

		default:
			break;
		}

		/* Check if last element */
		if (((i + 1) % num_groups) == 0)
			printf("\r\n");
	}

	/* Print final new line only if it wasn't printed in the loop */
	if ((i % num_groups) != 0)
		printf("\r\n");
}

/*
 * Print data as a table.
 */
int print_table_data(ami_device *dev, int n_fields, int n_rows, FILE *stream,
	enum table_divider_format divider_fmt, app_value_builder populate_values,
	app_header_builder populate_header, void *data, int *col_align)
{
	int i = 0, j = 0;
	int ret = EXIT_SUCCESS;
	char **header = NULL;
	char ***rows = NULL;
	int n_fields_table = n_fields;
	int n_rows_table = n_rows;


	/* Note that `dev`, `stream`, and `data` may be NULL */

	/* Construct table header. */
	header = (char**)calloc(n_fields, sizeof(char*));

	if (!header)
		return EXIT_FAILURE;

	for (i = 0; i < n_fields; i++) {
		header[i] = (char*)calloc(TABLE_HEADING_MAX, sizeof(char));

		if (!header[i]) {
			ret = EXIT_FAILURE;
			goto delete_header;
		}
	}

	/*
	 * Insert data into header.
	 * The rows must be populated in the same order.
	 */
	ret = populate_header(dev, header, n_fields, data);

	if (ret != EXIT_SUCCESS)
		goto delete_header;

	/* Construct rows. */
	/* Array of pointers to each row. */
	rows = (char***)calloc(n_rows, sizeof(char**));

	if (!rows) {
		ret = EXIT_FAILURE;
		goto delete_header;
	}
	
	/* Row of pointers to each column. */
	for (i = 0; i < n_rows; i++) {
		rows[i] = (char**)calloc(n_fields, sizeof(char*));

		if (!rows[i]) {
			ret = EXIT_FAILURE;
			goto delete_rows;
		}

		for (j = 0; j < n_fields; j++) {
			/* Column. */
			rows[i][j] = (char*)calloc(1, TABLE_FIELD_MAX * sizeof(char));

			if (!rows[i][j]) {
				ret = EXIT_FAILURE;
				goto delete_rows;
			}
		}
	}

	ret = populate_values(dev, rows, &n_rows_table, &n_fields_table, APP_OUT_FORMAT_TABLE, data);

	/* Ensure new values are not bigger than the original */
	if ((n_rows < n_rows_table) || (n_fields < n_fields_table)) {
		ret = EXIT_FAILURE;
		goto delete_rows;
	}

	/* Output table. */
	if (ret == EXIT_SUCCESS) {
		print_table(
			header,
			rows,
			n_fields_table,
			n_rows_table,
			divider_fmt,
			stream,
			col_align
		);
	}

delete_rows:
	if (rows) {
		for (i = 0; i < n_rows; i++) {
			if (!rows[i])
				continue;

			for (j = 0; j < 3; j++) {
				if (rows[i][j])
					free(rows[i][j]);
			}

			free(rows[i]);
		}

		free(rows);
	}

delete_header:
	if (header) {
		for (i = 0; i < n_fields; i++) {
			if (header[i])
				free(header[i]);
		}

		free(header);
	}
	
	return ret;
}

/*
 * Generate a JSON data object.
 */
int gen_json_data(ami_device *dev, int n_fields, int n_rows,
	app_value_builder populate_values, void *data, JsonNode **out)
{
	int ret = EXIT_FAILURE;
	JsonNode *parent = json_mkobject();

	if (!out)
		return EXIT_FAILURE;

	/* Note that `dev`, and `data` may be NULL */

	/* Add data to parent. */
	ret = populate_values(dev, parent, &n_rows, &n_fields,
		APP_OUT_FORMAT_JSON, data);

	if (ret == EXIT_SUCCESS)
		*out = parent;

	return ret;
}

/*
 * Print a JSON object.
 */
int print_json_obj(JsonNode *obj, FILE *stream)
{
	char *json = NULL;

	if (!obj || !stream)
		return EXIT_FAILURE;

	/* Generate JSON string. */
	json = json_stringify(obj, "\t");
	fprintf(stream, "\r\n%s\r\n", json);

	/* Cleanup. */
	free(json);

	return EXIT_SUCCESS;
}

/*
 * Print data in JSON format.
 */
int print_json_data(ami_device *dev, int n_fields, int n_rows, FILE *stream,
	app_value_builder populate_values, void *data)
{
	int ret = EXIT_FAILURE;
	JsonNode *parent = json_mkobject();

	/* Note that `dev`, and `data` may be NULL */

	/* For JSON, `stream` must be specified. */
	if (!stream)
		return EXIT_FAILURE;

	ret = gen_json_data(
		dev,
		n_fields,
		n_rows,
		populate_values,
		data,
		&parent
	);

	if (ret == EXIT_SUCCESS)
		ret = print_json_obj(parent, stream);

	json_delete(parent);
	return ret;
}

/*
 * Print a progress bar.
 */
char print_progress_bar(uint32_t cur, uint32_t max, uint32_t width,
	char left, char right, char fill, char empty, char state)
{
	int i = 0;
	char new_state = 0;
	uint32_t progress = 0;

	if (max == 0)
		max = 1;

	progress = ((unsigned long long)cur * width) / max;

	/* Move to beginning of the line */
	putchar('\r');

	/* Print left margin */
	putchar(left);

	if (width < progress)
		progress = width;
	
	for (i = 0; i < progress; i++)
		putchar(fill);

	for (i = 0; i < (width - progress); i++)
		putchar(empty);
	
	putchar(right);
	printf(" %.0f%% ", ((double)cur / (double)max) * 100);

	switch (state) {
	case '|':
		putchar('|');
		new_state = '-';
		break;
	
	case '-':
		putchar('-');
		new_state = '|';
		break;
	
	default:
		putchar('|');
		new_state = '-';
		break;
	}

	/* Print space so cursor doesn't obstruct last character. */
	putchar(' ');

	fflush(stdout);
	return new_state;
}
