// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_ami_mem_access.c - Unit test file for ami_mem_access.c
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

/* External includes */
#include "cmocka.h"

/* AMI API includes */
#include "ami_internal.h"
#include "ami_device_internal.h"
#include "ami_mem_access.h"

/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

int __wrap_ami_set_last_error(enum ami_error err, const char *ctxt, ...)
{
	check_expected(err);
	function_called();
	return AMI_STATUS_OK;
}

int __wrap_ami_open_cdev(ami_device *dev)
{
	return (int)mock();
}

extern int __real_ioctl(int fd, unsigned long request, ...);

int __wrap_ioctl(int fd, unsigned long request, ...)
{
	return (int)mock();
}

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_ami_mem_bar_read(void **state)
{
	ami_device dev = { 0 };
	uint32_t val = 0;

	/* Happy path - return status OK */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_mem_bar_read(&dev, 0, 0, &val),
		AMI_STATUS_OK
	);
}

void test_fail_ami_mem_bar_read(void **state)
{
	ami_device dev = { 0 };
	uint32_t val = 0;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_read(NULL, 0, 0, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_read(&dev, 0, 0, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_mem_bar_read(&dev, 0, 0, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_mem_bar_read(&dev, 0, 0, &val),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_mem_bar_write(void **state)
{
	ami_device dev = { 0 };

	/* Happy path - return status OK */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_mem_bar_write(&dev, 0, 0, 0),
		AMI_STATUS_OK
	);
}

void test_fail_ami_mem_bar_write(void **state)
{
	ami_device dev = { 0 };

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_write(NULL, 0, 0, 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_mem_bar_write(&dev, 0, 0, 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_mem_bar_write(&dev, 0, 0, 0),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_mem_bar_read_range(void **state)
{
	ami_device dev = { 0 };
	uint32_t val = 0;

	/* Happy path - return status OK */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_mem_bar_read_range(&dev, 0, 0, 1, &val),
		AMI_STATUS_OK
	);
}

void test_fail_ami_mem_bar_read_range(void **state)
{
	ami_device dev = { 0 };
	uint32_t val = 0;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_read_range(NULL, 0, 0, 1, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_read_range(&dev, 0, 0, 1, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `num` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_read_range(&dev, 0, 0, 0, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_mem_bar_read_range(&dev, 0, 0, 1, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_mem_bar_read_range(&dev, 0, 0, 1, &val),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_mem_bar_write_range(void **state)
{
	ami_device dev = { 0 };
	uint32_t val = 0;

	/* Happy path - return status OK */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_mem_bar_write_range(&dev, 0, 0, 1, &val),
		AMI_STATUS_OK
	);
}

void test_fail_ami_mem_bar_write_range(void **state)
{
	ami_device dev = { 0 };
	uint32_t val = 0;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_write_range(NULL, 0, 0, 1, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_write_range(&dev, 0, 0, 1, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `num` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_mem_bar_write_range(&dev, 0, 0, 0, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_mem_bar_write_range(&dev, 0, 0, 1, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_mem_bar_write_range(&dev, 0, 0, 1, &val),
		AMI_STATUS_ERROR
	);
}

/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_ami_mem_bar_read),
		cmocka_unit_test(test_fail_ami_mem_bar_read),
		cmocka_unit_test(test_happy_ami_mem_bar_write),
		cmocka_unit_test(test_fail_ami_mem_bar_write),
		cmocka_unit_test(test_happy_ami_mem_bar_read_range),
		cmocka_unit_test(test_fail_ami_mem_bar_read_range),
		cmocka_unit_test(test_happy_ami_mem_bar_write_range),
		cmocka_unit_test(test_fail_ami_mem_bar_write_range),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
