// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_table.c - Unit test file for table.c
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
#include "table.h"

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

static struct wrapper w_calloc = { REAL, REAL, 0, 0 };

/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

void __wrap_print_divider(const char c, uint8_t len, FILE *stream)
{
	function_called();
}

void __wrap_my_fprintf(FILE *stream, const char *format, ...)
{
	function_called();
}

extern void *__real_calloc(size_t num, size_t size);

void *__wrap_calloc(size_t num, size_t size)
{
	void *ret = NULL;

	switch (w_calloc.current) {
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

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_print_table_row(void **state)
{
	char *values[] = { "a", "b" };
	int col_widths[] = { 2 };
	int right_align[] = { TABLE_ALIGN_RIGHT, TABLE_ALIGN_RIGHT };
	int left_align[] = { TABLE_ALIGN_LEFT, TABLE_ALIGN_LEFT };

	/* Happy path - no divider, no alignment specified */
	expect_function_calls(__wrap_my_fprintf, 3);  /* 2 columns + newline */
	assert_int_equal(
		print_table_row(2, values, col_widths, 0, 0, false, NULL, NULL),
		EXIT_SUCCESS
	);

	/* Happy path - with divider, no alignment specified */
	expect_function_call(__wrap_print_divider);
	expect_function_calls(__wrap_my_fprintf, 3);
	assert_int_equal(
		print_table_row(2, values, col_widths, 0, 0, true, NULL, NULL),
		EXIT_SUCCESS
	);

	/* Happy path - no divider, right aligned */
	expect_function_calls(__wrap_my_fprintf, 3);
	assert_int_equal(
		print_table_row(2, values, col_widths, 0, 0, false, NULL, right_align),
		EXIT_SUCCESS
	);

	/* Happy path - no divider, left aligned */
	expect_function_calls(__wrap_my_fprintf, 3);
	assert_int_equal(
		print_table_row(2, values, col_widths, 0, 0, false, NULL, left_align),
		EXIT_SUCCESS
	);
}

void test_fail_print_table_row(void **state)
{
	char* values[] = { "a" };
	int col_widths[] = { 0 };

	/* Failure path - invalid `col_widths` argument */
	assert_int_equal(
		print_table_row(0, values, NULL, 0, 0, false, NULL, NULL),
		EXIT_FAILURE
	);

	/* Failure path - invalid `values` argument */
	assert_int_equal(
		print_table_row(0, NULL, col_widths, 0, 0, false, NULL, NULL),
		EXIT_FAILURE
	);
}

void test_happy_print_table(void **state)
{
	int i = 0, j = 0;
	char *header[] = { "a", "b" };
	char* row1[] = { "a", "ab" };
	char* row2[] = { "c", "cd" };
	char* row3[] = { "", "ef" };
	char** values[] = { row1, row2, row3 };

	/*
	 * Happy path - TABLE_DIVIDER_HEADER_ONLY
	 *
	 * Will print the following:
	 * whitespace
	 * header (2 columns)
	 * whitespace
	 * divider
	 * then, for each row (3):
	 *   each row element (2)
	 *   whitespace
	 * whitespace
	 */
	expect_function_calls(__wrap_my_fprintf, 4);
	expect_function_call(__wrap_print_divider);
	expect_function_calls(__wrap_my_fprintf, 10);
	assert_int_equal(
		print_table(header, values, 2, 3, TABLE_DIVIDER_HEADER_ONLY, NULL, NULL),
		EXIT_SUCCESS
	);

	/*
	 * Happy path - TABLE_DIVIDER_ALL
	 *
	 * Will print the following:
	 * whitespace
	 * header (2 columns)
	 * whitespace
	 * then, for each row (3):
	 *   divider
	 *   each row element (2)
	 *   whitespace
	 * whitespace
	 */
	expect_function_calls(__wrap_my_fprintf, 4);
	for (i = 0; i < 3; i++) {
		expect_function_call(__wrap_print_divider);
		expect_function_calls(__wrap_my_fprintf, 3);
	}
	expect_function_call(__wrap_my_fprintf);
	assert_int_equal(
		print_table(header, values, 2, 3, TABLE_DIVIDER_ALL, NULL, NULL),
		EXIT_SUCCESS
	);

	/*
	 * Happy path - TABLE_DIVIDER_ALL
	 * Will print same as above but without the final divider.
	 */
	expect_function_calls(__wrap_my_fprintf, 4);
	for (i = 0; i < 3; i++) {
		if (i != 2)
			expect_function_call(__wrap_print_divider);
		expect_function_calls(__wrap_my_fprintf, 3);
	}
	expect_function_call(__wrap_my_fprintf);
	assert_int_equal(
		print_table(header, values, 2, 3, TABLE_DIVIDER_GROUPS, NULL, NULL),
		EXIT_SUCCESS
	);

	/* Happy path - unknown divider format (hit default branch)
	 * Will print no dividers at all.
	 */
	expect_function_calls(__wrap_my_fprintf, 14);
	assert_int_equal(
		print_table(header, values, 2, 3, 100, NULL, NULL),
		EXIT_SUCCESS
	);
}

void test_fail_print_table(void **state)
{
	char* header[] = { "a" };
	char* row[] = { "a" };
	char** values[] = { row };

	/* Failure path - invalid `header` argument */
	assert_int_equal(
		print_table(NULL, values, 0, 0, TABLE_DIVIDER_NONE, NULL, NULL),
		EXIT_FAILURE
	);

	/* Failure path - invalid `values` argument */
	assert_int_equal(
		print_table(header, NULL, 0, 0, TABLE_DIVIDER_NONE, NULL, NULL),
		EXIT_FAILURE
	);

	/* Failure path - calloc fails */
	WRAPPER_ACTION(FAIL, calloc);
	assert_int_equal(
		print_table(header, values, 0, 0, TABLE_DIVIDER_NONE, NULL, NULL),
		EXIT_FAILURE
	);
}

/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_print_table_row),
		cmocka_unit_test(test_fail_print_table_row),
		cmocka_unit_test(test_happy_print_table),
		cmocka_unit_test(test_fail_print_table),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
