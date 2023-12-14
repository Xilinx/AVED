// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_ami_program.c - Unit test file for ami_program.c
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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/* External includes */
#include "cmocka.h"

/* Test includes */
#include "test_harness.h"

/* AMI API includes */
#include "ami_internal.h"
#include "ami_device_internal.h"
#include "ami_program.h"

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/* Wrappers */
static struct wrapper w_fopen  = { REAL, REAL, 0, 0 };
static struct wrapper w_fseek  = { REAL, REAL, 0, 0 };
static struct wrapper w_ftell  = { REAL, REAL, 0, 0 };
static struct wrapper w_malloc = { REAL, REAL, 0, 0 };
static struct wrapper w_fread  = { REAL, REAL, 0, 0 };
static struct wrapper w_ferror = { REAL, REAL, 0, 0 };
static struct wrapper w_fclose = { REAL, REAL, 0, 0 };

/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

int __wrap_ami_dev_hot_reset(ami_device **dev)
{
	return AMI_STATUS_OK;
}

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

int __wrap_ami_dev_pci_reload(ami_device **dev, const char *bdf)
{
	return AMI_STATUS_OK;
}

extern int __real_ioctl(int fd, unsigned long request, ...);

int __wrap_ioctl(int fd, unsigned long request, ...)
{
	return (int)mock();
}

extern FILE *__real_fopen(const char *filename, const char *mode);

FILE *__wrap_fopen(const char *filename, const char *mode)
{
	FILE *ret = NULL;
	static FILE dummy = { 0 };

	switch (w_fopen.current) {
	case OK:
		ret = &dummy;
		break;
	
	case REAL:
		ret = __real_fopen(filename, mode);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(fopen);
	return ret;
}

extern int __real_fclose(FILE *stream);

int __wrap_fclose(FILE *stream)
{
	int ret = AMI_LINUX_STATUS_ERROR;

	switch (w_fclose.current) {
	case OK:
		ret = AMI_LINUX_STATUS_OK;
		break;
	
	case REAL:
		ret = __real_fclose(stream);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(fclose);
	return ret;
}

extern void *__real_malloc(size_t size);

void *__wrap_malloc(size_t size)
{
	void *ret = NULL;

	switch (w_malloc.current) {
	/* case OK not implemented */

	case REAL:
		ret = __real_malloc(size);
		break;
	
	default:
		break;
	}
	
	WRAPPER_DONE(malloc);
	return ret;
}

extern size_t __real_fread(void *restrict buffer, size_t size, size_t count,
	FILE *restrict stream);

size_t __wrap_fread(void *restrict buffer, size_t size, size_t count,
	FILE *restrict stream)
{
	ssize_t ret = AMI_LINUX_STATUS_ERROR;

	switch (w_fread.current) {
	case OK:
	{
		/* Must use `will_return` if behaviour is set to `OK` */
		char *str = mock_ptr_type(char*);
		memcpy(buffer, str, strlen(str));
		ret = strlen(str);
		break;
	}
	
	case REAL:
		ret = __real_fread(buffer, size, count, stream);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(fread);
	return ret;
}

extern int __real_fseek(FILE *stream, long int offset, int whence);

int __wrap_fseek(FILE *stream, long int offset, int whence)
{
	int ret = AMI_LINUX_STATUS_ERROR;

	switch (w_fseek.current) {
	case OK:
		ret = AMI_LINUX_STATUS_OK;
		break;
	
	case REAL:
		ret = __real_fseek(stream, offset, whence);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(fseek);
	return ret;
}

extern long int __real_ftell(FILE *stream);

long int __wrap_ftell(FILE *stream)
{
	int ret = AMI_LINUX_STATUS_ERROR;

	switch (w_ftell.current) {
	case OK:
		ret = AMI_LINUX_STATUS_OK;
		break;
	
	case REAL:
		ret = __real_ftell(stream);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(ftell);
	return ret;
}

extern int __real_ferror(FILE *stream);

int __wrap_ferror(FILE *stream)
{
	int ret = AMI_LINUX_STATUS_ERROR;

	switch (w_ferror.current) {
	case OK:
		ret = AMI_LINUX_STATUS_OK;
		break;
	
	case REAL:
		ret = __real_ferror(stream);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(ferror);
	return ret;
}

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_ami_prog_download_pdi(void **state)
{
	ami_device dev = { 0 };

	/* Happy path - read_file and ioctl succeed */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(OK, fopen);
	WRAPPER_ACTION_C(OK, fseek, 2);
	WRAPPER_ACTION(OK, ftell);
	WRAPPER_ACTION(OK, fread);
	will_return(__wrap_fread, "a");
	WRAPPER_ACTION(OK, ferror);
	WRAPPER_ACTION(OK, fclose);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_OK
	);
}

void test_fail_ami_prog_download_pdi(void **state)
{
	ami_device dev = { 0 };
	enum wrapper_behaviour fseek_orig = w_fseek.orig;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_download_pdi(NULL, "a", 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `path` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_download_pdi(&dev, NULL, 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(OK, fopen);
	WRAPPER_ACTION_C(OK, fseek, 2);
	WRAPPER_ACTION(OK, ftell);
	WRAPPER_ACTION(OK, fread);
	will_return(__wrap_fread, "a");
	WRAPPER_ACTION(OK, ferror);
	WRAPPER_ACTION(OK, fclose);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - fopen fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(FAIL, fopen);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - first fseek fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(OK, fopen);
	WRAPPER_ACTION(FAIL, fseek);
	WRAPPER_ACTION(OK, fclose);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ftell fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(OK, fopen);
	WRAPPER_ACTION(OK, fseek);
	WRAPPER_ACTION(FAIL, ftell);
	WRAPPER_ACTION(OK, fclose);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - malloc fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(OK, fopen);
	WRAPPER_ACTION(OK, fseek);
	WRAPPER_ACTION(OK, ftell);
	WRAPPER_ACTION(FAIL, malloc);
	WRAPPER_ACTION(OK, fclose);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ENOMEM);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - second fseek fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(OK, fopen);
	w_fseek.orig = FAIL;
	WRAPPER_ACTION(OK, fseek);
	WRAPPER_ACTION(OK, ftell);
	WRAPPER_ACTION(OK, fclose);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);
	w_fseek.current = fseek_orig;
	w_fseek.orig = fseek_orig;

	/* Failure path - ferror fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	WRAPPER_ACTION(OK, fopen);
	WRAPPER_ACTION_C(OK, fseek, 2);
	WRAPPER_ACTION(OK, ftell);
	WRAPPER_ACTION(OK, fread);
	will_return(__wrap_fread, "a");
	WRAPPER_ACTION(FAIL, ferror);
	WRAPPER_ACTION(OK, fclose);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_download_pdi(&dev, "a", 0),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_prog_device_boot(void **state)
{
	ami_device dev = { 0 };
	ami_device *dev_ptr = &dev;

	/* Happy path */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_prog_device_boot(&dev_ptr, 0),
		AMI_STATUS_OK
	);
}

void test_fail_ami_prog_device_boot(void **state)
{
	ami_device dev = { 0 };
	ami_device *dev_ptr = &dev;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_device_boot(NULL, 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_device_boot(&dev_ptr, 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_prog_device_boot(&dev_ptr, 0),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_prog_copy_partition(void **state)
{
	ami_device dev = { 0 };

	/* Happy path */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_prog_copy_partition(&dev, 0, 1),
		AMI_STATUS_OK
	);
}

void test_fail_ami_prog_copy_partition(void **state)
{
	ami_device dev = { 0 };

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_copy_partition(NULL, 0, 1),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_copy_partition(&dev, 0, 1),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_prog_copy_partition(&dev, 0, 1),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_prog_get_fpt_header(void **state)
{
	ami_device dev = { 0 };
	struct ami_fpt_header header = { 0 };

	/* Happy path */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_prog_get_fpt_header(&dev, &header),
		AMI_STATUS_OK
	);
}

void test_fail_ami_prog_get_fpt_header(void **state)
{
	ami_device dev = { 0 };
	struct ami_fpt_header header = { 0 };

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_get_fpt_header(NULL, &header),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `header` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_get_fpt_header(&dev, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_get_fpt_header(&dev, &header),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_prog_get_fpt_header(&dev, &header),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_prog_get_fpt_partition(void **state)
{
	ami_device dev = { 0 };
	struct ami_fpt_partition part = { 0 };

	/* Happy path */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_prog_get_fpt_partition(&dev, 0, &part),
		AMI_STATUS_OK
	);
}

void test_fail_ami_prog_get_fpt_partition(void **state)
{
	ami_device dev = { 0 };
	struct ami_fpt_partition part = { 0 };

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_get_fpt_partition(NULL, 0, &part),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `header` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_prog_get_fpt_partition(&dev, 0, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_prog_get_fpt_partition(&dev, 0, &part),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_prog_get_fpt_partition(&dev, 0, &part),
		AMI_STATUS_ERROR
	);
}

/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_ami_prog_download_pdi),
		cmocka_unit_test(test_fail_ami_prog_download_pdi),
		cmocka_unit_test(test_happy_ami_prog_device_boot),
		cmocka_unit_test(test_fail_ami_prog_device_boot),
		cmocka_unit_test(test_happy_ami_prog_copy_partition),
		cmocka_unit_test(test_fail_ami_prog_copy_partition),
		cmocka_unit_test(test_happy_ami_prog_get_fpt_header),
		cmocka_unit_test(test_fail_ami_prog_get_fpt_header),
		cmocka_unit_test(test_happy_ami_prog_get_fpt_partition),
		cmocka_unit_test(test_fail_ami_prog_get_fpt_partition),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
