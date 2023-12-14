// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_printer.c - Unit test file for printer.c
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* External includes */
#include "cmocka.h"

/* Shared test code */
#include "test_harness.h"

/* App includes */
#include "printer.h"

/* API includes */
#include "ami_device.h"

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

static struct wrapper w_calloc = { REAL, REAL, 0, 0 };

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

static int populate_values(ami_device *dev, void *values,
	int n_rows, int n_fields, enum app_out_format fmt, void *data)
{
	return (int)mock();
}

static int populate_header(ami_device *dev, char **header,
	int n_fields, void *data)
{
	return (int)mock();
}


/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

char *__wrap_json_stringify(const JsonNode *node, const char *space)
{
	return (char*)mock();
}

void __wrap_json_delete(JsonNode *node)
{
	function_called();
}

JsonNode *__wrap_json_mkobject(void)
{
	return (JsonNode*)mock();
}

int __wrap_print_table(char* header[], char** values[], int num_cols, int num_rows,
	enum table_divider_format divider_fmt, FILE *stream, int *col_align)
{
	function_called();
}

int __wrap_vfprintf(FILE *stream, const char *format, va_list arg)
{
	function_called();
}

int __wrap_putc(int character, FILE *stream)
{
	function_called();
}

extern int __real_printf(const char *format, ...);

int __wrap_printf(const char *format, ...)
{
	function_called();
}

extern void *__real_calloc(size_t num, size_t size);

void *__wrap_calloc(size_t num, size_t size)
{
	void *ret = NULL;
	enum wrapper_behaviour b = w_calloc.current;

	if (b == CMOCKA)
		b = (enum wrapper_behaviour)mock();

	switch (b) {
	/* case OK not implemented */

	case REAL:
		ret = __real_calloc(num, size);
		break;
	
	default:
		break;
	}
	
	WRAPPER_DONE(calloc);
	return ret;
}

int __wrap_fprintf(FILE *stream, const char *format, ...)
{
	function_called();
}

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_my_fprintf(void **state)
{
	FILE f = { 0 };

	/* Happy path - print to stdout only */
	expect_function_call(__wrap_vfprintf);
	my_fprintf(NULL, "%s", "a");

	/* Happy path - print to a secondary stream */
	expect_function_calls(__wrap_vfprintf, 2);
	my_fprintf(&f, "%s", "a");

	/* Happy path - print to a single stream (stdout specified as secondary) */
	expect_function_call(__wrap_vfprintf);
	my_fprintf(stdout, "%s", "a");
}

void test_happy_my_putc(void **state)
{
	FILE f = { 0 };

	/* Happy path - print to stdout only */
	expect_function_call(__wrap_putc);
	my_putc('a', NULL);

	/* Happy path - print to a secondary stream */
	expect_function_calls(__wrap_putc, 2);
	my_putc('a', &f);

	/* Happy path - print to a single stream (stdout specified as secondary) */
	expect_function_call(__wrap_putc);
	my_putc('a', stdout);
}

void test_happy_print_divider(void **state)
{
	FILE f = { 0 };

	/* Happy path - print to stdout only */
	expect_function_calls(__wrap_putc, 10);
	expect_function_call(__wrap_vfprintf);
	print_divider('-', 10, NULL);

	/* Happy path - print to a secondary stream */
	expect_function_calls(__wrap_putc, 20);
	expect_function_calls(__wrap_vfprintf, 2);
	print_divider('-', 10, &f);

	/* Happy path - print to a single stream (stdout specified as secondary) */
	expect_function_calls(__wrap_putc, 10);
	expect_function_call(__wrap_vfprintf);
	print_divider('-', 10, stdout);
}

void test_happy_print_hexdump(void **state)
{
	uint32_t values[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	/* Happy path - odd number of values, 2 groups
	 * Will print the following:
	 * <addr> 1 2 <newline>    (3)
	 * <addr> 3 4 <newline>    (3)
	 * <addr> 5 6 <newline>    (3)
	 * <addr> 7 8 <newline>    (3)
	 * <addr> 9                (1)
	 * <newline>               (1)
	 */
	expect_function_calls(__wrap_printf, (3 * 4) + 2);
	print_hexdump(0, values, 9, 2);
}

void test_fail_print_hexdump(void **state)
{
	/* Note that `print_hexdump` is a void function so no error code is returned */
	uint32_t values[] = { 0 };

	/* Failure path - invalid `values` argument */
	print_hexdump(0, NULL, 1, 1);

	/* Failure path - num_values == 0 */
	print_hexdump(0, values, 0, 1);

	/* Failure path - num_groups == 0 */
	print_hexdump(0, values, 1, 0);
}

void test_happy_print_table_data(void **state)
{
	/* Happy path - all functions succeed */
	will_return(populate_values, EXIT_SUCCESS);
	will_return(populate_header, EXIT_SUCCESS);
	expect_function_call(__wrap_print_table);
	assert_int_equal(
		print_table_data(
			NULL,
			3,
			3,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_SUCCESS
	);
}

void test_fail_print_table_data(void **state)
{
	/* Failure path - calloc for `header` fails */
	WRAPPER_ACTION(FAIL, calloc);
	assert_int_equal(
		print_table_data(
			NULL,
			3,
			3,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - calloc for `header[0]` fails */
	WRAPPER_ACTION_C(CMOCKA, calloc, 2);
	will_return(__wrap_calloc, REAL);
	will_return(__wrap_calloc, FAIL);
	assert_int_equal(
		print_table_data(
			NULL,
			3,
			3,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - `populate_header` fails */
	will_return(populate_header, EXIT_FAILURE);
	assert_int_equal(
		print_table_data(
			NULL,
			3,
			3,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - calloc for `rows` fails */
	will_return(populate_header, EXIT_SUCCESS);
	WRAPPER_ACTION_C(CMOCKA, calloc, 3);
	will_return_count(__wrap_calloc, REAL, 2);
	will_return(__wrap_calloc, FAIL);
	assert_int_equal(
		print_table_data(
			NULL,
			1,
			3,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - calloc for `rows[0]` fails */
	will_return(populate_header, EXIT_SUCCESS);
	WRAPPER_ACTION_C(CMOCKA, calloc, 4);
	will_return_count(__wrap_calloc, REAL, 3);
	will_return(__wrap_calloc, FAIL);
	assert_int_equal(
		print_table_data(
			NULL,
			1,
			3,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - calloc for `rows[0][0]` fails */
	will_return(populate_header, EXIT_SUCCESS);
	WRAPPER_ACTION_C(CMOCKA, calloc, 5);
	will_return_count(__wrap_calloc, REAL, 4);
	will_return(__wrap_calloc, FAIL);
	assert_int_equal(
		print_table_data(
			NULL,
			1,
			3,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - `populate_values` fails */
	will_return(populate_header, EXIT_SUCCESS);
	will_return(populate_values, EXIT_FAILURE);
	WRAPPER_ACTION_C(REAL, calloc, 6);
	assert_int_equal(
		print_table_data(
			NULL,
			1,
			1,
			NULL,
			TABLE_DIVIDER_ALL,
			populate_values,
			populate_header,
			NULL,
			NULL
		),
		EXIT_FAILURE
	);
}

void test_happy_gen_json_data(void **state)
{
	JsonNode node = { 0 };
	JsonNode *out = NULL;

	/* Happy path - correct pointer returned */
	will_return(__wrap_json_mkobject, &node);
	will_return(populate_values, EXIT_SUCCESS);
	assert_int_equal(
		gen_json_data(
			NULL, 0, 0, populate_values, NULL, &out
		),
		EXIT_SUCCESS
	);
	assert_ptr_equal(out, &node);
}

void test_fail_gen_json_data(void **state)
{
	/* Failure path - invalid `out` argument */
	will_return(__wrap_json_mkobject, NULL);
	assert_int_equal(
		gen_json_data(
			NULL, 0, 0, populate_values, NULL, NULL
		),
		EXIT_FAILURE
	);
}

void test_happy_print_json_obj(void **state)
{
	JsonNode node = { 0 };
	char *json = calloc(3, sizeof(char));
	strcpy(json, "a");
	assert_non_null(json);

	/* Happy path - return code OK and expected functions called */
	will_return(__wrap_json_stringify, json);
	expect_function_call(__wrap_fprintf);
	assert_int_equal(
		print_json_obj(&node, stdout),
		EXIT_SUCCESS
	);
}

void test_fail_print_json_obj(void **state)
{
	JsonNode node = { 0 };

	/* Failure path - invalid `obj` argument */
	assert_int_equal(
		print_json_obj(NULL, stdout),
		EXIT_FAILURE
	);

	/* Failure path - invalid `stream` argument */
	assert_int_equal(
		print_json_obj(&node, NULL),
		EXIT_FAILURE
	);
}

void test_happy_print_json_data(void **state)
{
	/* Need to execute happy path for both `gen_json_data` and `print_json_obj`. */
	JsonNode node1 = { 0 };
	JsonNode node2 = { 0 };
	char *json = calloc(3, sizeof(char));
	strcpy(json, "a");
	assert_non_null(json);

	/* parent node inside print_json_data */
	will_return(__wrap_json_mkobject, &node1);
	/* gen_json_data setup */
	will_return(__wrap_json_mkobject, &node2);
	will_return(populate_values, EXIT_SUCCESS);
	/* print_json_obj setup */
	will_return(__wrap_json_stringify, json);
	expect_function_call(__wrap_fprintf);
	/* print_json_data setup*/
	expect_function_call(__wrap_json_delete);
	assert_int_equal(
		print_json_data(
			NULL, 0, 0, stdout, populate_values, NULL
		),
		EXIT_SUCCESS
	);
}

void test_fail_print_json_data(void **state)
{
	/* Failure path - invalid `stream` argument */
	will_return(__wrap_json_mkobject, NULL);
	assert_int_equal(
		print_json_data(
			NULL, 0, 0, NULL, populate_values, NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - gen_json_data fails */
	will_return(__wrap_json_mkobject, NULL);
	will_return(__wrap_json_mkobject, NULL);
	will_return(populate_values, EXIT_FAILURE);
	expect_function_call(__wrap_json_delete);
	assert_int_equal(
		print_json_data(
			NULL, 0, 0, stdout, populate_values, NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - print_json_obj fails */
	will_return(__wrap_json_mkobject, NULL);
	will_return(__wrap_json_mkobject, NULL);
	will_return(populate_values, EXIT_SUCCESS);
	expect_function_call(__wrap_json_delete);
	assert_int_equal(
		print_json_data(
			NULL, 0, 0, stdout, populate_values, NULL
		),
		EXIT_FAILURE
	);
}

/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_my_fprintf),
		cmocka_unit_test(test_happy_my_putc),
		cmocka_unit_test(test_happy_print_divider),
		cmocka_unit_test(test_happy_print_hexdump),
		cmocka_unit_test(test_fail_print_hexdump),
		cmocka_unit_test(test_happy_print_table_data),
		cmocka_unit_test(test_fail_print_table_data),
		cmocka_unit_test(test_happy_gen_json_data),
		cmocka_unit_test(test_fail_gen_json_data),
		cmocka_unit_test(test_happy_print_json_obj),
		cmocka_unit_test(test_fail_print_json_obj),
		cmocka_unit_test(test_happy_print_json_data),
		cmocka_unit_test(test_fail_print_json_data),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
