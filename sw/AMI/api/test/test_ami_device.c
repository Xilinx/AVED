// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_ami_device.c - Unit test file for ami_device.c
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

/* Shared test code */
#include "test_harness.h"

/* AMI API includes */
#include "ami_internal.h"
#include "ami_device.h"
#include "ami_device_internal.h"
#include "ami_sensor_internal.h"
#include "ami_version.h"

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * Variables to keep track of wrappers around built in functions.
 * CMocka's `will_return`/`mock` functions are not a good fit for standard
 * library functions as it can break the behaviour of other (non-test) code.
 */
static struct wrapper w_fopen  = { OK,   OK,   0, 0 };
static struct wrapper w_fclose = { OK,   OK,   0, 0 };
static struct wrapper w_calloc = { REAL, REAL, 0, 0 };
static struct wrapper w_close  = { REAL, REAL, 0, 0 };
static struct wrapper w_open   = { REAL, REAL, 0, 0 };
static struct wrapper w_read   = { REAL, REAL, 0, 0 };
static struct wrapper w_write  = { REAL, REAL, 0, 0 };

/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

int __wrap_ami_get_driver_version(struct ami_version *ami_version)
{
	ami_version->major = (uint8_t)mock();
	ami_version->minor = (uint8_t)mock();
	return (int)mock();
}

int __wrap_ami_set_last_error(enum ami_error err, const char *ctxt, ...)
{
	check_expected(err);
	function_called();
	return AMI_STATUS_OK;
}

uint16_t __wrap_ami_parse_bdf(const char *bdf)
{
	return (uint16_t)mock();
}

int __wrap_ami_sensor_discover(ami_device *dev)
{
	function_called();

	if ((int)mock() == AMI_STATUS_OK) {
		/* Create dummy data */
		dev->num_sensors = 1;
		dev->num_total_sensors = 4;

		dev->sensors = malloc(sizeof(struct ami_sensor));

		dev->sensors[0].next = NULL;
		dev->sensors[0].sensor_data = malloc(sizeof(struct ami_sensor_internal));

		dev->sensors[0].sensor_data->temp = malloc(sizeof(struct ami_sensor_data));
		dev->sensors[0].sensor_data->temp->next = NULL;

		dev->sensors[0].sensor_data->power = malloc(sizeof(struct ami_sensor_data));
		dev->sensors[0].sensor_data->power->next = NULL;

		dev->sensors[0].sensor_data->current = malloc(sizeof(struct ami_sensor_data));
		dev->sensors[0].sensor_data->current->next = NULL;

		dev->sensors[0].sensor_data->voltage = malloc(sizeof(struct ami_sensor_data));
		dev->sensors[0].sensor_data->voltage->next = NULL;

		return AMI_STATUS_OK;
	}

	return AMI_STATUS_ERROR;
}

int __wrap_ami_mem_bar_write(ami_device *dev, uint8_t idx, uint64_t offset, uint32_t val)
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

extern int __real_open(const char *pathname, int flags, int mode);

int __wrap_open(const char *pathname, int flags, int mode)
{
	int ret = AMI_LINUX_STATUS_ERROR;
	enum wrapper_behaviour b = w_open.current;

	if (b == CMOCKA)
		b = (enum wrapper_behaviour)mock();

	switch (b) {
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
	enum wrapper_behaviour b = w_close.current;

	if (b == CMOCKA)
		b = (enum wrapper_behaviour)mock();

	switch (b) {
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

extern ssize_t __real_write(int fildes, const void *buf, size_t nbyte);

ssize_t __wrap_write(int fildes, const void *buf, size_t nbyte)
{
	ssize_t ret = AMI_LINUX_STATUS_ERROR;
	enum wrapper_behaviour b = w_write.current;

	if (b == CMOCKA)
		b = (enum wrapper_behaviour)mock();

	switch (b) {
	case OK:
	{
		ret = AMI_LINUX_STATUS_OK;
		break;
	}
	
	case REAL:
		ret = __real_write(fildes, buf, nbyte);
		break;
	
	default:
		break;
	}

	WRAPPER_DONE(write);
	return ret;
}

ssize_t __wrap_getline(char **restrict lineptr, size_t *restrict n,
	FILE *restrict stream)
{
	char *str  = mock_ptr_type(char*);

	if (strcmp(str, "EOF") == 0)
		return AMI_LINUX_STATUS_ERROR;

	if (!lineptr) {
		*lineptr = __real_calloc(strlen(str) + 1, sizeof(char*));
	} else {
		*lineptr = realloc(*lineptr, strlen(str) + 1);
		memset(*lineptr, 0x00, strlen(str));
	}

	memcpy(*lineptr, str, strlen(str));
	return strlen(str);
}

extern int __real_ioctl(int fd, unsigned long request, ...);

int __wrap_ioctl(int fd, unsigned long request, ...)
{
	return (int)mock();
}

ssize_t __wrap_readlink(const char *restrict pathname, char *restrict buf,
		size_t bufsiz)
{
	return (ssize_t)mock();
}

char *__wrap_dirname(char *path)
{
	return "";
}

char *__wrap_basename(char *path)
{
	return "";
}

unsigned int __wrap_sleep(unsigned int seconds)
{
	return 0;
}

off_t __wrap_lseek(int fd, off_t offset, int whence)
{
	return (off_t)mock();
}

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_ami_dev_find_next(void **state)
{
	ami_device *dev = NULL;

	/* Happy path - specify PF */
	will_return(__wrap_getline, "2");
	will_return(__wrap_getline, "c1:00.1 1 2");
	will_return(__wrap_getline, "c1:00.0 2 3");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, 0, NULL),
		AMI_STATUS_OK
	);
	assert_non_null(dev);
	assert_int_equal(dev->cdev_num, 2);
	assert_int_equal(dev->hwmon_num, 3);
	assert_int_equal(dev->bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));

	ami_dev_delete(&dev);
	assert_null(dev);

	/* Happy path - specify bus */
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 1 2");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, 0xC1, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_OK
	);
	assert_non_null(dev);
	assert_int_equal(dev->cdev_num, 1);
	assert_int_equal(dev->hwmon_num, 2);
	assert_int_equal(dev->bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));

	ami_dev_delete(&dev);
	assert_null(dev);

	/* Happy path - specify device */
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 1 2");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, 0x00, AMI_ANY_DEV, NULL),
		AMI_STATUS_OK
	);
	assert_non_null(dev);
	assert_int_equal(dev->cdev_num, 1);
	assert_int_equal(dev->hwmon_num, 2);
	assert_int_equal(dev->bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));

	ami_dev_delete(&dev);
	assert_null(dev);

	/* Happy path - specify bus, device, and function */
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 1 2");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, 0xC1, 0x00, 0x00, NULL),
		AMI_STATUS_OK
	);
	assert_non_null(dev);
	assert_int_equal(dev->cdev_num, 1);
	assert_int_equal(dev->hwmon_num, 2);
	assert_int_equal(dev->bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));

	ami_dev_delete(&dev);
	assert_null(dev);
}

void test_fail_ami_dev_find_next(void **state)
{
	ami_device *dev = NULL;
	ami_device *dev2 = NULL;

	/* Failure path - could not allocate memory for device */
	WRAPPER_ACTION(FAIL, calloc);
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 1 2");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ENOMEM);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);

	/* Failure path - could not open devices file */
	WRAPPER_ACTION(FAIL, fopen);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);

	/* Failure path - valid pointer to non-NULL device */
	dev2 = malloc(sizeof(ami_device));
	assert_non_null(dev2);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_find_next(&dev2, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	free(dev2);

	/* Failure path - NULL device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_find_next(NULL, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);

	/* Failure path - no device found */
	will_return(__wrap_getline, "EOF");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ENODEV);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);

	/* Failure path - bad devices file format */
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "invalid");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EFMT);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);

	/* Failure path - ami_get_driver_version fails */
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);

	/* Failure path - driver major version number mismatch */
	will_return(__wrap_ami_get_driver_version, 99);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EVER);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);

	/* Failure path - driver minor version number mismatch */
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, 99);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EVER);
	assert_int_equal(
		ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
}

void test_happy_ami_dev_find(void **state)
{
	ami_device *dev = NULL;

	/* Happy path - find a device with the previously identified BDF */
	will_return(__wrap_ami_parse_bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 1 2");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_find("", &dev),
		AMI_STATUS_OK
	);
	assert_non_null(dev);

	ami_dev_delete(&dev);
	assert_null(dev);
}

void test_fail_ami_dev_find(void **state)
{
	ami_device *dev = NULL;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_find("", NULL),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
}

void test_happy_ami_dev_bringup(void **state)
{
	ami_device *dev = NULL;

	/* Happy path - find a device and setup sensors */
	expect_function_call(__wrap_ami_sensor_discover);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_parse_bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 1 2");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_bringup("", &dev),
		AMI_STATUS_OK
	);
	assert_non_null(dev);

	ami_dev_delete(&dev);
	assert_null(dev);
}

void test_fail_ami_dev_bringup(void **state)
{
	ami_device *dev = NULL;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_bringup("", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `bdf` argument */
	dev = malloc(sizeof(ami_device));
	assert_non_null(dev);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_bringup(NULL, &dev),
		AMI_STATUS_ERROR
	);
	free(dev);
}

void test_happy_ami_dev_request_access(void **state)
{
	ami_device dev = { 0 };

	/* Happy path */
	assert_int_equal(
		ami_dev_request_access(&dev),
		AMI_STATUS_OK
	);
}

void test_fail_ami_dev_request_access(void **state)
{
	ami_device dev = { 0 };
	dev.cdev = -1;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_request_access(NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_pci_reload(void **state)
{
	ami_device *dev = NULL;

	dev = calloc(1, sizeof(ami_device));
	assert_non_null(dev);
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);

	/* Happy path - Device handle given */
	dev->cdev = AMI_INVALID_FD;
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION_C(OK, write, 2);
	/* Set values for ami_dev_find_next */
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 2 3");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_pci_reload(&dev, NULL),
		AMI_STATUS_OK
	);
	assert_non_null(dev);
	assert_int_equal(dev->cdev_num, 2);
	assert_int_equal(dev->hwmon_num, 3);
	assert_int_equal(dev->bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));

	/* Happy path - BDF string given */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION_C(OK, write, 2);
	will_return(__wrap_ami_parse_bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));
	assert_int_equal(
		ami_dev_pci_reload(NULL, "c1:00.0"),
		AMI_STATUS_OK
	);
	
	free(dev);
}

void test_fail_ami_dev_pci_reload(void **state)
{
	ami_device *dev = NULL;
	ami_device *dev2 = NULL;

	dev = calloc(1, sizeof(ami_device));
	assert_non_null(dev);
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);

	/* Failure path - both device and bdf given */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_pci_reload(&dev, "c1:00.0"),
		AMI_STATUS_ERROR
	);

	/* Failure path - valid pointer to NULL handle */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_pci_reload(&dev2, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - no valid argument given */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_pci_reload(NULL, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_hot_reset(void **state)
{
	ami_device *dev = NULL;

	dev = calloc(1, sizeof(ami_device));
	assert_non_null(dev);
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/*
	 * open+close for PCI config
	 * open+close for PCI remove
	 * open+close for PCI rescan
	 * 2 writes for PCI remove+rescan
	 * 1 read + 2 writes for reset
	 */

	/* Happy path */
	WRAPPER_ACTION_C(OK, open, 3);
	WRAPPER_ACTION_C(OK, close, 3);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "");
	WRAPPER_ACTION_C(OK, write, 4);
	will_return_count(__wrap_lseek, 0, 3);
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	/* Set values for ami_dev_find_next */
	will_return(__wrap_getline, "1");
	will_return(__wrap_getline, "c1:00.0 2 3");
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MAJOR);
	will_return(__wrap_ami_get_driver_version, GIT_TAG_VER_MINOR);
	will_return(__wrap_ami_get_driver_version, AMI_STATUS_OK);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_OK
	);

	free(dev);
}

void test_fail_ami_dev_hot_reset(void **state)
{
	ami_device *dev = NULL;

	dev = calloc(1, sizeof(ami_device));
	assert_non_null(dev);
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/* Invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_hot_reset(NULL),
		AMI_STATUS_ERROR
	);

	/* ami_dev_get_pci_port fails */
	will_return(__wrap_readlink, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);

	/* config open fails */
	WRAPPER_ACTION(FAIL, open);
	will_return(__wrap_readlink, 0);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);

	/* ami_mem_bar_write fails */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);

	/* pci_remove fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION(FAIL, write);
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
	dev = calloc(1, sizeof(ami_device));
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/* first lseek fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION(OK, write);
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	will_return(__wrap_lseek, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
	dev = calloc(1, sizeof(ami_device));
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/* first read fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION(OK, write);
	WRAPPER_ACTION(FAIL, read);
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	will_return(__wrap_lseek, AMI_LINUX_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
	dev = calloc(1, sizeof(ami_device));
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/* second lseek fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION(OK, write);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "");
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	will_return(__wrap_lseek, AMI_LINUX_STATUS_OK);
	will_return(__wrap_lseek, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
	dev = calloc(1, sizeof(ami_device));
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/* first write fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION_C(CMOCKA, write, 2);
	will_return(__wrap_write, OK);
	will_return(__wrap_write, FAIL);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "");
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	will_return_count(__wrap_lseek, AMI_LINUX_STATUS_OK, 2);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
	dev = calloc(1, sizeof(ami_device));
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/* third lseek fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION_C(OK, write, 2);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "");
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	will_return_count(__wrap_lseek, AMI_LINUX_STATUS_OK, 2);
	will_return(__wrap_lseek, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
	dev = calloc(1, sizeof(ami_device));
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	/* second write fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION_C(CMOCKA, write, 3);
	will_return_count(__wrap_write, OK, 2);
	will_return(__wrap_write, FAIL);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "");
	will_return(__wrap_readlink, 0);
	will_return(__wrap_ami_mem_bar_write, AMI_STATUS_OK);
	will_return_count(__wrap_lseek, AMI_LINUX_STATUS_OK, 3);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_hot_reset(&dev),
		AMI_STATUS_ERROR
	);
	assert_null(dev);
	dev = calloc(1, sizeof(ami_device));
	dev->bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);
	dev->cdev = AMI_INVALID_FD;

	free(dev);
}

void test_happy_ami_dev_read_uuid(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_LOGIC_UUID_SIZE] = { 0 };

	/* Happy path - return status ok and uuid matches */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "84b455ddf4566d48b0118c3f375e4cbc");
	assert_int_equal(
		ami_dev_read_uuid(&dev, buf),
		AMI_STATUS_OK
	);
	assert_string_equal(buf, "84b455ddf4566d48b0118c3f375e4cbc");
}

void test_fail_ami_dev_read_uuid(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_LOGIC_UUID_SIZE] = { 0 };

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_read_uuid(NULL, buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `buf` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_read_uuid(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_num_devices(void **state)
{
	uint16_t num = 0;

	/* Happy path - return status ok and value matches */
	will_return(__wrap_getline, "1");
	assert_int_equal(
		ami_dev_get_num_devices(&num),
		AMI_STATUS_OK
	);
	assert_int_equal(num, 1);
}

void test_fail_ami_dev_get_num_devices(void **state)
{
	uint16_t num = 0;

	/* Failure path - invalid `num` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_num_devices(NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - could not open file */
	WRAPPER_ACTION(FAIL, fopen);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_dev_get_num_devices(&num),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid file format */
	will_return(__wrap_getline, "invalid");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_num_devices(&num),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_link_speed(void **state)
{
	ami_device dev = { 0 };
	uint8_t current = 0, max = 0;

	/* Happy path - return status ok and values match */
	WRAPPER_ACTION_C(OK, read, 2);
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	will_return(__wrap_read, "5");
	will_return(__wrap_read, "3");
	assert_int_equal(
		ami_dev_get_pci_link_speed(&dev, &current, &max),
		AMI_STATUS_OK
	);
	assert_int_equal(current, 3);
	assert_int_equal(max, 5);
}

void test_fail_ami_dev_get_pci_link_speed(void **state)
{
	ami_device dev = { 0 };
	uint8_t current = 0, max = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_link_speed(NULL, &current, &max),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `current` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_link_speed(&dev, NULL, &max),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `max` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_link_speed(&dev, &current, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - parsing max value fails */
	WRAPPER_ACTION_C(OK, read, 2);
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	will_return(__wrap_read, "invalid");
	will_return(__wrap_read, "1");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_pci_link_speed(&dev, &current, &max),
		AMI_STATUS_ERROR
	);

	/* Failure path - parsing current value fails */
	WRAPPER_ACTION_C(OK, read, 2);
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	will_return(__wrap_read, "1");
	will_return(__wrap_read, "invalid");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_pci_link_speed(&dev, &current, &max),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_link_width(void **state)
{
	ami_device dev = { 0 };
	uint8_t current = 0, max = 0;

	/* Happy path - return status ok and values match */
	WRAPPER_ACTION_C(OK, read, 2);
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	will_return(__wrap_read, "16");
	will_return(__wrap_read, "8");
	assert_int_equal(
		ami_dev_get_pci_link_width(&dev, &current, &max),
		AMI_STATUS_OK
	);
	assert_int_equal(current, 8);
	assert_int_equal(max, 16);
}

void test_fail_ami_dev_get_pci_link_width(void **state)
{
	ami_device dev = { 0 };
	uint8_t current = 0, max = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_link_width(NULL, &current, &max),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `current` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_link_width(&dev, NULL, &max),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `max` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_link_width(&dev, &current, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - parsing max value fails */
	WRAPPER_ACTION_C(OK, read, 2);
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	will_return(__wrap_read, "invalid");
	will_return(__wrap_read, "1");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_pci_link_width(&dev, &current, &max),
		AMI_STATUS_ERROR
	);

	/* Failure path - parsing current value fails */
	WRAPPER_ACTION_C(OK, read, 2);
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	will_return(__wrap_read, "1");
	will_return(__wrap_read, "invalid");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_pci_link_width(&dev, &current, &max),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_vendor(void **state)
{
	ami_device dev = { 0 };
	uint16_t vendor = 0;

	/* Happy path - correct vendor number returned */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "0x10ee");
	assert_int_equal(
		ami_dev_get_pci_vendor(&dev, &vendor),
		AMI_STATUS_OK
	);
	assert_int_equal(vendor, 0x10ee);
}

void test_fail_ami_dev_get_pci_vendor(void **state)
{
	ami_device dev = { 0 };
	uint16_t vendor = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_vendor(NULL, &vendor),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `vendor` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_vendor(&dev, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - parsing vendor value fails */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "invalid");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_pci_vendor(&dev, &vendor),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_device(void **state)
{
	ami_device dev = { 0 };
	uint16_t device = 0;

	/* Happy path - valid device number returned */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "0x5094");
	assert_int_equal(
		ami_dev_get_pci_device(&dev, &device),
		AMI_STATUS_OK
	);
	assert_int_equal(device, 0x5094);
}

void test_fail_ami_dev_get_pci_device(void **state)
{
	ami_device dev = { 0 };
	uint16_t device = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_device(NULL, &device),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `device` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_device(&dev, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - parsing device value fails */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "invalid");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_pci_device(&dev, &device),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_numa_node(void **state)
{
	ami_device dev = { 0 };
	uint8_t node = 0;

	/* Happy path - valid NUMA node returned */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "5");
	assert_int_equal(
		ami_dev_get_pci_numa_node(&dev, &node),
		AMI_STATUS_OK
	);
	assert_int_equal(node, 5);
}

void test_fail_ami_dev_get_pci_numa_node(void **state)
{
	ami_device dev = { 0 };
	uint8_t node = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_numa_node(NULL, &node),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `node` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_numa_node(&dev, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - parsing node value fails */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "invalid");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_dev_get_pci_numa_node(&dev, &node),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_cpulist(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_PCI_CPULIST_SIZE] = { 0 };

	/* Happy path - valid cpulist string returned */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "10-11,26-27");
	assert_int_equal(
		ami_dev_get_pci_cpulist(&dev, buf),
		AMI_STATUS_OK
	);
	assert_string_equal("10-11,26-27", buf);
}

void test_fail_ami_dev_get_pci_cpulist(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_PCI_CPULIST_SIZE] = { 0 };

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_cpulist(NULL, buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `buf` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_cpulist(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_state(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_DEV_STATE_SIZE] = { 0 };

	/* Happy path - valid cpulist string returned */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "READY");
	assert_int_equal(
		ami_dev_get_state(&dev, buf),
		AMI_STATUS_OK
	);
	assert_string_equal(AMI_DEV_READY_STR, buf);
}

void test_fail_ami_dev_get_state(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_DEV_STATE_SIZE] = { 0 };

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_state(NULL, buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `buf` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_state(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_name(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_DEV_NAME_SIZE] = { 0 };

	/* Happy path - valid cpulist string returned */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "ALVEO V80");
	assert_int_equal(
		ami_dev_get_name(&dev, buf),
		AMI_STATUS_OK
	);
	assert_string_equal("ALVEO V80", buf);
}

void test_fail_ami_dev_get_name(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_DEV_NAME_SIZE] = { 0 };

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_name(NULL, buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `buf` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_name(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_amc_version(void **state)
{
	ami_device dev = { 0 };
	struct amc_version ver = { 0 };

	/* Happy path - version string read and parsed */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "1.2.3 +4 *1");
	assert_int_equal(
		ami_dev_get_amc_version(&dev, &ver),
		AMI_STATUS_OK
	);
	assert_int_equal(ver.major, 1);
	assert_int_equal(ver.minor, 2);
	assert_int_equal(ver.patch, 3);
	assert_int_equal(ver.dev_commits, 4);
	assert_int_equal(ver.local_changes, 1);
}

void test_fail_ami_dev_get_amc_version(void **state)
{
	ami_device dev = { 0 };
	struct amc_version ver = { 0 };

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_amc_version(NULL, &ver),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `ver` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_amc_version(&dev, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid version string format */
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_read, "foobar");
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EFMT);
	assert_int_equal(
		ami_dev_get_amc_version(&dev, &ver),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_port(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_DEV_PCI_PORT_SIZE] = { 0 };

	/* Happy path */
	will_return(__wrap_readlink, AMI_LINUX_STATUS_OK);
	assert_int_equal(
		ami_dev_get_pci_port(&dev, buf),
		AMI_STATUS_OK
	);
}

void test_fail_ami_dev_get_pci_port(void **state)
{
	ami_device dev = { 0 };
	char buf[AMI_DEV_PCI_PORT_SIZE] = { 0 };

	/* Invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_port(NULL, buf),
		AMI_STATUS_ERROR
	);

	/* Invalid `buf` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_port(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_pci_bdf(void **state)
{
	ami_device dev = { 0 };
	uint16_t bdf = 0;

	dev.bdf = AMI_MK_BDF(0xC1, 0x00, 0x00);

	/* Happy path - ok return code and value matches expected */
	assert_int_equal(
		ami_dev_get_pci_bdf(&dev, &bdf),
		AMI_STATUS_OK
	);
	assert_int_equal(bdf, dev.bdf);
}

void test_fail_ami_dev_get_pci_bdf(void **state)
{
	ami_device dev = { 0 };
	uint16_t bdf = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_bdf(NULL, &bdf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `bdf` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_pci_bdf(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_cdev_num(void **state)
{
	ami_device dev = { 0 };
	int cdev = 0;

	dev.cdev = 1;

	/* Happy path - ok return code and value matches expected */
	assert_int_equal(
		ami_dev_get_cdev_num(&dev, &cdev),
		AMI_STATUS_OK
	);
	assert_int_equal(cdev, dev.cdev_num);
}

void test_fail_ami_dev_get_cdev_num(void **state)
{
	ami_device dev = { 0 };
	int cdev = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_cdev_num(NULL, &cdev),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `cdev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_cdev_num(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_dev_get_hwmon_num(void **state)
{
	ami_device dev = { 0 };
	int hwmon = 0;

	dev.hwmon_num = 2;

	/* Happy path - ok return code and value matches expected */
	assert_int_equal(
		ami_dev_get_hwmon_num(&dev, &hwmon),
		AMI_STATUS_OK
	);
	assert_int_equal(hwmon, dev.hwmon_num);
}

void test_fail_ami_dev_get_hwmon_num(void **state)
{
	ami_device dev = { 0 };
	int hwmon = 0;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_hwmon_num(NULL, &hwmon),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `hwmon` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_dev_get_hwmon_num(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

/* Private API functions */

void test_fail_open_cdev(void **state)
{
	ami_device dev = { 0 };

	dev.cdev = AMI_INVALID_FD;

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_open_cdev(NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - open fails */
	WRAPPER_ACTION(FAIL, open);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_open_cdev(&dev),
		AMI_STATUS_ERROR
	);
}

void test_happy_open_cdev(void **state)
{
	ami_device dev = { 0 };

	dev.cdev = AMI_INVALID_FD;

	/* Happy path - valid file descriptor returned */
	WRAPPER_ACTION(OK, open);
	assert_int_equal(
		ami_open_cdev(&dev),
		AMI_STATUS_OK
	);
	assert_int_not_equal(dev.cdev, AMI_INVALID_FD);

	/* Happy path - file already opened */
	assert_int_equal(
		ami_open_cdev(&dev),
		AMI_STATUS_OK
	);
	assert_int_not_equal(dev.cdev, AMI_INVALID_FD);
}

void test_happy_close_cdev(void **state)
{
	ami_device dev = { 0 };

	dev.cdev = 0;

	/* Happy path - close call succeeds */
	WRAPPER_ACTION(OK, close);
	assert_int_equal(
		ami_close_cdev(&dev),
		AMI_STATUS_OK
	);
}

void test_fail_close_cdev(void **state)
{
	ami_device dev = { 0 };

	/* Failure path - invalid arguments */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_close_cdev(NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - bad file descriptor */
	dev.cdev = 0;
	WRAPPER_ACTION(FAIL, close);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_close_cdev(&dev),
		AMI_STATUS_ERROR
	);
}

void test_fail_read_sysfs(void **state)
{
	char buf[AMI_SYSFS_STR_MAX] = { 0 };
	ami_device dev = { 0 };

	/* Failure path - couldn't open file */
	WRAPPER_ACTION(FAIL, open);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_read_sysfs(&dev, "invalid", buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - Could not read file */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(FAIL, read);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_read_sysfs(&dev, "logic_uuid", buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid device pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_read_sysfs(NULL, "logic_uuid", buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `attr` argument pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_read_sysfs(&dev, NULL, buf),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `buf` argument pointer */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_read_sysfs(&dev, "logic_uuid", NULL),
		AMI_STATUS_ERROR
	);
}

/* Using `ami_dev_pci_reload` to test `pci_remove` */
void test_fail_pci_remove(void **state)
{
	/* open fails */
	WRAPPER_ACTION(FAIL, open);
	will_return(__wrap_ami_parse_bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_dev_pci_reload(NULL, "c1:00.0"),
		AMI_STATUS_ERROR
	);

	/* write fails */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(FAIL, write);
	WRAPPER_ACTION(OK, close);
	will_return(__wrap_ami_parse_bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_pci_reload(NULL, "c1:00.0"),
		AMI_STATUS_ERROR
	);
}

/* Using `ami_dev_pci_reload` to test `pci_rescan` */
void test_fail_pci_rescan(void **state)
{
	/* open fails */
	WRAPPER_ACTION_C(CMOCKA, open, 2);
	will_return(__wrap_open, OK);
	will_return(__wrap_open, FAIL);
	WRAPPER_ACTION(OK, write);
	will_return(__wrap_ami_parse_bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_dev_pci_reload(NULL, "c1:00.0"),
		AMI_STATUS_ERROR
	);

	/* write fails */
	WRAPPER_ACTION_C(OK, open, 2);
	WRAPPER_ACTION_C(OK, close, 2);
	WRAPPER_ACTION_C(CMOCKA, write, 2);
	will_return(__wrap_write, OK);
	will_return(__wrap_write, FAIL);
	will_return(__wrap_ami_parse_bdf, AMI_MK_BDF(0xC1, 0x00, 0x00));
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_dev_pci_reload(NULL, "c1:00.0"),
		AMI_STATUS_ERROR
	);
}

/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_ami_dev_find_next),
		cmocka_unit_test(test_fail_ami_dev_find_next),
		cmocka_unit_test(test_happy_ami_dev_find),
		cmocka_unit_test(test_fail_ami_dev_find),
		cmocka_unit_test(test_happy_ami_dev_bringup),
		cmocka_unit_test(test_fail_ami_dev_bringup),
		cmocka_unit_test(test_happy_ami_dev_request_access),
		cmocka_unit_test(test_fail_ami_dev_request_access),
		cmocka_unit_test(test_happy_ami_dev_pci_reload),
		cmocka_unit_test(test_fail_ami_dev_pci_reload),
		cmocka_unit_test(test_happy_ami_dev_hot_reset),
		cmocka_unit_test(test_fail_ami_dev_hot_reset),
		cmocka_unit_test(test_happy_ami_dev_read_uuid),
		cmocka_unit_test(test_fail_ami_dev_read_uuid),
		cmocka_unit_test(test_happy_ami_dev_get_num_devices),
		cmocka_unit_test(test_fail_ami_dev_get_num_devices),
		cmocka_unit_test(test_happy_ami_dev_get_pci_link_speed),
		cmocka_unit_test(test_fail_ami_dev_get_pci_link_speed),
		cmocka_unit_test(test_happy_ami_dev_get_pci_link_width),
		cmocka_unit_test(test_fail_ami_dev_get_pci_link_width),
		cmocka_unit_test(test_happy_ami_dev_get_pci_vendor),
		cmocka_unit_test(test_fail_ami_dev_get_pci_vendor),
		cmocka_unit_test(test_happy_ami_dev_get_pci_device),
		cmocka_unit_test(test_fail_ami_dev_get_pci_device),
		cmocka_unit_test(test_happy_ami_dev_get_pci_numa_node),
		cmocka_unit_test(test_fail_ami_dev_get_pci_numa_node),
		cmocka_unit_test(test_happy_ami_dev_get_pci_cpulist),
		cmocka_unit_test(test_fail_ami_dev_get_pci_cpulist),
		cmocka_unit_test(test_happy_ami_dev_get_state),
		cmocka_unit_test(test_fail_ami_dev_get_state),
		cmocka_unit_test(test_happy_ami_dev_get_name),
		cmocka_unit_test(test_fail_ami_dev_get_name),
		cmocka_unit_test(test_happy_ami_dev_get_amc_version),
		cmocka_unit_test(test_fail_ami_dev_get_amc_version),
		cmocka_unit_test(test_happy_ami_dev_get_pci_port),
		cmocka_unit_test(test_fail_ami_dev_get_pci_port),
		cmocka_unit_test(test_happy_ami_dev_get_pci_bdf),
		cmocka_unit_test(test_fail_ami_dev_get_pci_bdf),
		cmocka_unit_test(test_happy_ami_dev_get_cdev_num),
		cmocka_unit_test(test_fail_ami_dev_get_cdev_num),
		cmocka_unit_test(test_happy_ami_dev_get_hwmon_num),
		cmocka_unit_test(test_fail_ami_dev_get_hwmon_num),
		cmocka_unit_test(test_happy_open_cdev),
		cmocka_unit_test(test_fail_open_cdev),
		cmocka_unit_test(test_happy_close_cdev),
		cmocka_unit_test(test_fail_close_cdev),
		cmocka_unit_test(test_fail_read_sysfs),
		cmocka_unit_test(test_fail_pci_remove),
		cmocka_unit_test(test_fail_pci_rescan),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
