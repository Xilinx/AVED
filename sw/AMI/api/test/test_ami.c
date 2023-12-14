// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_ami.c -  Unit test file for ami.c
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
#include <string.h>

/* Test includes */
#include "test_harness.h"

/* External includes */
#include "cmocka.h"

/* AMI API includes */
#include "ami_internal.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* Update this when more error codes are added. */
#define MAX_AMI_ERROR (AMI_ERROR_EVER + 1)

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

static struct wrapper w_close  = { REAL, REAL, 0, 0 };
static struct wrapper w_open   = { REAL, REAL, 0, 0 };
static struct wrapper w_read   = { REAL, REAL, 0, 0 };

/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

extern int __real_open(const char *pathname, int flags, int mode);

int __wrap_open(const char *pathname, int flags, int mode)
{
	int ret = AMI_LINUX_STATUS_ERROR;

	switch (w_open.current) {
	case OK:
		ret = AMI_LINUX_STATUS_OK;
		break;
	
	case REAL:
		ret = __real_open(pathname, flags, mode);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(open);
	return ret;
}

extern int __real_close(int fd);

int __wrap_close(int fd)
{
	int ret = AMI_LINUX_STATUS_ERROR;

	switch (w_close.current) {
	case OK:
		ret = AMI_LINUX_STATUS_OK;
		break;
	
	case REAL:
		ret = __real_close(fd);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(close);
	return ret;
}

extern ssize_t __real_read(int fildes, void *buf, size_t nbyte);

ssize_t __wrap_read(int fildes, void *buf, size_t nbyte)
{
	ssize_t ret = AMI_LINUX_STATUS_ERROR;

	switch (w_read.current) {
	case OK:
	{
		/* Must use `will_return` if behaviour is set to `OK` */
		char *str = mock_ptr_type(char*);
		memcpy(buf, str, strlen(str));
		ret = strlen(str);
		break;
	}
	
	case REAL:
		ret = __real_read(fildes, buf, nbyte);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(read);
	return ret;
}

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_convert_num(void **state)
{
	long num = 0;

	/* Happy path - correct value returned (base 10) */
	assert_int_equal(
		ami_convert_num("123\n", 10, &num),
		AMI_STATUS_OK
	);
	assert_int_equal(num, 123);

	/* Happy path - correct value returned (base 16) */
	assert_int_equal(
		ami_convert_num("10\n", 16, &num),
		AMI_STATUS_OK
	);
	assert_int_equal(num, 16);

	/* Happy path - correct value returned (automatic base) */
	assert_int_equal(
		ami_convert_num("123\n", 0, &num),
		AMI_STATUS_OK
	);
	assert_int_equal(num, 123);

	/* Happy path - correct value returned (automatic base) */
	assert_int_equal(
		ami_convert_num("0x10\n", 0, &num),
		AMI_STATUS_OK
	);
	assert_int_equal(num, 16);
}

void test_fail_convert_num(void **state)
{
	long num = 0;

	/* Failure path - invalid `out` argument */
	assert_int_equal(
		ami_convert_num("0x10\n", 0, NULL),
		AMI_STATUS_ERROR
	);
	assert_int_equal(ami_last_error, AMI_ERROR_EINVAL);

	/* Failure path - invalid `buf` argument */
	assert_int_equal(
		ami_convert_num(NULL, 0, &num),
		AMI_STATUS_ERROR
	);
	assert_int_equal(ami_last_error, AMI_ERROR_EINVAL);
}

void test_happy_set_last_error(void **state)
{
	int i = AMI_ERROR_NONE;  /* This will hit the default branch (unkown error) */

	/* Happy path - all error codes set correctly */
	for (i = 0; i < MAX_AMI_ERROR; i++) {
		assert_int_equal(
			ami_set_last_error((enum ami_error)i, NULL),
			AMI_STATUS_OK
		);
		assert_int_equal(ami_last_error, i);
	}

	/* Happy path - all error codes set correctly, with additional context */
	for (i = 0; i < MAX_AMI_ERROR; i++) {
		assert_int_equal(
			ami_set_last_error((enum ami_error)i, "%s", "test"),
			AMI_STATUS_OK
		);
		assert_int_equal(ami_last_error, i);
	}
}

void test_happy_ami_get_driver_version(void **state)
{
	struct ami_version ver = { 0 };

	/* Happy path - all values parsed correctly */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "1.2.3 +4 *1");
	assert_int_equal(
		ami_get_driver_version(&ver),
		AMI_STATUS_OK
	);
	assert_int_equal(ver.major, 1);
	assert_int_equal(ver.minor, 2);
	assert_int_equal(ver.patch, 3);
	assert_int_equal(ver.dev_commits, 4);
	assert_int_equal(ver.status, 1);
}

void test_fail_ami_get_driver_version(void **state)
{
	struct ami_version ver = { 0 };

	/* Failure path - invalid `ami_version` argument */
	assert_int_equal(
		ami_get_driver_version(NULL),
		AMI_STATUS_ERROR
	);
	assert_int_equal(ami_last_error, AMI_ERROR_EINVAL);

	/* Failure path - failed to parse version string */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "invalid");
	assert_int_equal(
		ami_get_driver_version(&ver),
		AMI_STATUS_ERROR
	);
	assert_int_equal(ami_last_error, AMI_ERROR_EFMT);

	/* Failure path - read fails */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(FAIL, read);
	WRAPPER_ACTION(OK, close);
	assert_int_equal(
		ami_get_driver_version(&ver),
		AMI_STATUS_ERROR
	);
	assert_int_equal(ami_last_error, AMI_ERROR_EIO);

	/* Failure path - open fails */
	WRAPPER_ACTION(FAIL, open);
	assert_int_equal(
		ami_get_driver_version(&ver),
		AMI_STATUS_ERROR
	);
	assert_int_equal(ami_last_error, AMI_ERROR_EBADF);
}

void test_happy_ami_get_api_version(void **state)
{
	struct ami_version ver = { 0 };

	/* Happy path - return code OK */
	assert_int_equal(
		ami_get_api_version(&ver),
		AMI_STATUS_OK
	);
}

void test_fail_ami_get_api_version(void **state)
{
	/* Failure path - invalid `ami_version` argument */
	assert_int_equal(
		ami_get_api_version(NULL),
		AMI_STATUS_ERROR
	);
	assert_int_equal(ami_last_error, AMI_ERROR_EINVAL);
}

void test_happy_ami_get_last_error(void **state)
{
	/* Happy path - correct error string returned */
	ami_set_last_error(AMI_ERROR_EINVAL, "%s", "testing");
	assert_string_equal(
		ami_get_last_error(),
		"EINVAL: Invalid arguments [testing].\r\n"
	);
}

void test_happy_ami_parse_bdf(void **state)
{
	/* Happy path - bus only */
	assert_int_equal(
		ami_parse_bdf("c1"),
		AMI_MK_BDF(0xc1, 0x00, 0x00)
	);

	/* Happy path - bus and device */
	assert_int_equal(
		ami_parse_bdf("c1:ab"),
		AMI_MK_BDF(0xc1, 0xab, 0x00)
	);

	/* Happy path - bus, device, and function */
	assert_int_equal(
		ami_parse_bdf("c1:ab.1"),
		AMI_MK_BDF(0xc1, 0xab, 0x01)
	);

	/* Happy path - domain and bus */
	assert_int_equal(
		ami_parse_bdf("0000:c1"),
		AMI_MK_BDF(0xc1, 0x00, 0x00)
	);

	/* Happy path - domain, bus and device */
	assert_int_equal(
		ami_parse_bdf("0000:c1:ab"),
		AMI_MK_BDF(0xc1, 0xab, 0x00)
	);

	/* Happy path - domain, bus, device, and function */
	assert_int_equal(
		ami_parse_bdf("0000:c1:ab.1"),
		AMI_MK_BDF(0xc1, 0xab, 0x01)
	);
}

void test_fail_ami_parse_bdf(void **state)
{
	/* Failure path - invalid `bdf` argument */
	assert_int_equal(
		ami_parse_bdf(NULL),
		0
	);
}

/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_convert_num),
		cmocka_unit_test(test_fail_convert_num),
		cmocka_unit_test(test_happy_set_last_error),
		cmocka_unit_test(test_happy_ami_get_driver_version),
		cmocka_unit_test(test_fail_ami_get_driver_version),
		cmocka_unit_test(test_happy_ami_get_api_version),
		cmocka_unit_test(test_fail_ami_get_api_version),
		cmocka_unit_test(test_happy_ami_get_last_error),
		cmocka_unit_test(test_happy_ami_parse_bdf),
		cmocka_unit_test(test_fail_ami_parse_bdf),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
