// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_ami_sensor.c - Unit test file for ami_sensor.c
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
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>

/* External includes */
#include "cmocka.h"

/* Test includes */
#include "test_harness.h"

/* AMI API includes */
#include "ami_internal.h"
#include "ami_device_internal.h"
#include "ami_sensor_internal.h"
#include "ami_ioctl.h"

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/* Wrappers */
static struct wrapper w_close    = { REAL, REAL, 0, 0 };
static struct wrapper w_open     = { REAL, REAL, 0, 0 };
static struct wrapper w_read     = { REAL, REAL, 0, 0 };
static struct wrapper w_write    = { REAL, REAL, 0, 0 };
static struct wrapper w_calloc   = { REAL, REAL, 0, 0 };
static struct wrapper w_snprintf = { REAL, REAL, 0, 0 };

/* Test data */
static struct ami_sensor_data test_temp    = { 0 };
static struct ami_sensor_data test_power   = { 0 };
static struct ami_sensor_data test_current = { 0 };
static struct ami_sensor_data test_voltage = { 0 };

static struct ami_sensor_internal test_sensor_data = { 
	&test_temp,
	&test_current,
	&test_voltage,
	&test_power
};

static struct ami_sensor test_sensor = { 0 };

/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

int __wrap_ami_set_last_error(enum ami_error err, const char *ctxt, ...)
{
	check_expected(err);
	function_called();
	return AMI_STATUS_OK;
}

int __wrap_ami_convert_num(const char *buf, int base, long *out)
{
	check_expected(buf);
	*out = (long)mock();
	return (int)mock();
}

int __wrap_glob(const char *restrict pattern, int flags,
	int (*errfunc)(const char *epath, int eerrno),
	glob_t *restrict pglob)
{
	if ((int)mock() == AMI_LINUX_STATUS_OK) {
		size_t c = (size_t)mock();
		char **files = mock_ptr_type(char**);
		pglob->gl_pathc = c;
		pglob->gl_pathv = files;
		return AMI_LINUX_STATUS_OK;
	}

	return AMI_LINUX_STATUS_ERROR;
}

int __wrap_globfree(glob_t *pglob)
{
	return AMI_LINUX_STATUS_OK;
}

int __wrap_stat(const char *path, struct stat *buf)
{
	buf->st_mode = (mode_t)mock();
	return mock();
}

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

extern ssize_t __real_write(int fildes, const void *buf, size_t nbyte);

ssize_t __wrap_write(int fildes, const void *buf, size_t nbyte)
{
	ssize_t ret = AMI_LINUX_STATUS_ERROR;

	switch (w_write.current) {
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

extern int __real_snprintf(char *str, size_t size, const char *format, ...);

int __wrap_snprintf(char *str, size_t size, const char *format, ...)
{
	int ret = AMI_LINUX_STATUS_ERROR;

	switch (w_snprintf.current) {
	/* case OK not implemented */

	case REAL:
	{
		va_list args;
		va_start(args, format);
		ret = vsnprintf(str, size, format, args);
		va_end(args);
		break;
	}
	
	default:
		break;
	}
	
	WRAPPER_DONE(snprintf);
	return ret;
}

int __wrap_ami_open_cdev(ami_device *dev)
{
	return (int)mock();
}

extern int __real_ioctl(int fd, unsigned long request, void *argp);

int __wrap_ioctl(int fd, unsigned long request, void *argp)
{
	if ((int)mock() == AMI_LINUX_STATUS_OK) {
		void *data = (void*)mock();
		size_t sz = (size_t)mock();

		if (data && argp && (sz != 0))
			memcpy(argp, data, sz);

		return AMI_LINUX_STATUS_OK;
	}

	return AMI_LINUX_STATUS_ERROR;
}

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

/**
 * Delete all device sensor data.
*/
static void delete_sensors(ami_device *dev)
{

	if (dev->sensors) {
		struct ami_sensor *sensor = dev->sensors;
		struct ami_sensor *next = NULL;

		while (sensor) {
			next = sensor->next;

			/* Free private data. */
			free(sensor->sensor_data->temp);
			free(sensor->sensor_data->power);
			free(sensor->sensor_data->current);
			free(sensor->sensor_data->voltage);
			free(sensor->sensor_data);

			/* Free sensor. */
			free(sensor);
			sensor = next;
		}

		dev->num_sensors = 0;
		dev->num_total_sensors = 0;
		dev->sensors = NULL;
	}
}

/*
 * Initialize sensor data for testing.
 */
static int setup_data(void **state)
{
	test_sensor.sensor_data = &test_sensor_data;

	strcpy(test_sensor.name, "foo");
	strcpy(test_temp.name.value_s, "foo");
	strcpy(test_power.name.value_s, "foo");
	strcpy(test_current.name.value_s, "foo");
	strcpy(test_voltage.name.value_s, "foo");

	test_temp.status.type = AMI_SENSOR_ATTR_STATUS;
	test_temp.value.type = AMI_SENSOR_ATTR_VALUE;
	test_temp.max.type = AMI_SENSOR_ATTR_MAX;
	test_temp.max.valid = true;
	test_temp.average.type = AMI_SENSOR_ATTR_AVG;
	test_temp.average.valid = true;
	strcpy(test_temp.status.hwmon, "/sys/class/hwmon/hwmon2/temp1_status");
	strcpy(test_temp.value.hwmon, "/sys/class/hwmon/hwmon2/temp1_input");
	strcpy(test_temp.max.hwmon, "/sys/class/hwmon/hwmon2/temp1_max");
	strcpy(test_temp.average.hwmon, "/sys/class/hwmon/hwmon2/temp1_average");

	test_voltage.status.type = AMI_SENSOR_ATTR_STATUS;
	test_voltage.value.type = AMI_SENSOR_ATTR_VALUE;
	test_voltage.max.type = AMI_SENSOR_ATTR_MAX;
	test_voltage.max.valid = true;
	test_voltage.average.type = AMI_SENSOR_ATTR_AVG;
	test_voltage.average.valid = true;
	strcpy(test_voltage.status.hwmon, "/sys/class/hwmon/hwmon2/in1_status");
	strcpy(test_voltage.value.hwmon, "/sys/class/hwmon/hwmon2/in1_input");
	strcpy(test_temp.max.hwmon, "/sys/class/hwmon/hwmon2/in1_max");
	strcpy(test_temp.average.hwmon, "/sys/class/hwmon/hwmon2/in1_average");

	test_current.status.type = AMI_SENSOR_ATTR_STATUS;
	test_current.value.type = AMI_SENSOR_ATTR_VALUE;
	test_current.max.type = AMI_SENSOR_ATTR_MAX;
	test_current.max.valid = true;
	test_current.average.type = AMI_SENSOR_ATTR_AVG;
	test_current.average.valid = true;
	strcpy(test_current.status.hwmon, "/sys/class/hwmon/hwmon2/curr1_status");
	strcpy(test_current.value.hwmon, "/sys/class/hwmon/hwmon2/curr1_input");
	strcpy(test_temp.max.hwmon, "/sys/class/hwmon/hwmon2/curr1_max");
	strcpy(test_temp.average.hwmon, "/sys/class/hwmon/hwmon2/curr1_average");

	test_power.status.type = AMI_SENSOR_ATTR_STATUS;
	test_power.value.type = AMI_SENSOR_ATTR_VALUE;
	test_power.max.type = AMI_SENSOR_ATTR_MAX;
	test_power.max.valid = true;
	test_power.average.type = AMI_SENSOR_ATTR_AVG;
	test_power.average.valid = true;
	strcpy(test_power.status.hwmon, "/sys/class/hwmon/hwmon2/power1_status");
	strcpy(test_power.value.hwmon, "/sys/class/hwmon/hwmon2/power1_input");
	strcpy(test_temp.max.hwmon, "/sys/class/hwmon/hwmon2/power1_max");
	strcpy(test_temp.average.hwmon, "/sys/class/hwmon/hwmon2/power1_average");

	return 0;
}

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_ami_sensor_discover(void **state)
{
	int i = 0;
	ami_device dev  = { 0 };
	const int n_files = 7;

	char *files[] = {
		"/sys/class/hwmon/hwmon2/temp1_label",  /* device */
		"/sys/class/hwmon/hwmon2/in1_label",    /* pcb */
		"/sys/class/hwmon/hwmon2/temp1_input",
		"/sys/class/hwmon/hwmon2/temp2_label",  /* pcb */
		"/sys/class/hwmon/hwmon2/temp2_input",
		"/sys/class/hwmon/hwmon2/temp2_max",
		"/sys/class/hwmon/hwmon2/in1_input",
		NULL 
	};

	char *files_temp[] = {
		"/sys/class/hwmon/hwmon2/temp1_label",
		"/sys/class/hwmon/hwmon2/temp1_input",
		NULL 
	};

	char *files_voltage[] = {
		"/sys/class/hwmon/hwmon2/in1_label",
		"/sys/class/hwmon/hwmon2/in1_status",
		NULL
	};

	char *files_current[] = {
		"/sys/class/hwmon/hwmon2/curr1_label",
		"/sys/class/hwmon/hwmon2/curr1_max",
		NULL
	};

	char *files_power[] = {
		"/sys/class/hwmon/hwmon2/power1_label",
		"/sys/class/hwmon/hwmon2/power1_avg",
		NULL
	};

	/* Happy path - multiple sensors */
	WRAPPER_ACTION_C(OK, open, 3);
	WRAPPER_ACTION_C(OK, close, 3);
	WRAPPER_ACTION_C(OK, read, 3);
	will_return(__wrap_read, "device");
	will_return_count(__wrap_read, "pcb", 2);
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, n_files);
	will_return(__wrap_glob, files);
	for (i = 0; i < n_files; i++) {
		will_return(__wrap_stat, __S_IFREG);
		will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	}
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	/* find_sensor_by_name will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_OK
	);
	assert_int_equal(dev.num_sensors, 2);
	assert_int_equal(dev.num_total_sensors, 3);
	delete_sensors(&dev);
	
	/* Happy path - single temperature sensor */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "device");
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 2);
	will_return(__wrap_glob, files_temp);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	/* find_sensor_by_name will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_OK
	);
	assert_int_equal(dev.num_sensors, 1);
	assert_int_equal(dev.num_total_sensors, 1);
	delete_sensors(&dev);

	/* Happy path - single voltage sensor */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "device");
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 2);
	will_return(__wrap_glob, files_voltage);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	/* find_sensor_by_name will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_OK
	);
	assert_int_equal(dev.num_sensors, 1);
	assert_int_equal(dev.num_total_sensors, 1);
	delete_sensors(&dev);

	/* Happy path - single current sensor */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "device");
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 2);
	will_return(__wrap_glob, files_current);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	/* find_sensor_by_name will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_OK
	);
	assert_int_equal(dev.num_sensors, 1);
	assert_int_equal(dev.num_total_sensors, 1);
	delete_sensors(&dev);

	/* Happy path - single power sensor */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "device");
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 2);
	will_return(__wrap_glob, files_power);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	/* find_sensor_by_name will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_OK
	);
	assert_int_equal(dev.num_sensors, 1);
	assert_int_equal(dev.num_total_sensors, 1);
	delete_sensors(&dev);
}

void test_fail_ami_sensor_discover(void **state)
{
	ami_device dev = { 0 };
	char *files[] = { NULL };
	char *files2[] = {
		"/sys/class/hwmon/hwmon2/temp1_input",
		NULL
	};

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - glob fails */
	will_return(__wrap_glob, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ERET);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - no sensor files */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 0);
	will_return(__wrap_glob, files);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - calloc fails */
	WRAPPER_ACTION(FAIL, calloc);
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files2);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ENOMEM);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);
}

/*
 * NOTE: Using `ami_sensor_discover` as a proxy to test the static
 * function `parse_hwmon`.
 */

void test_fail_parse_hwmon(void **state)
{
	ami_device dev = { 0 };
	char *files2[] = { "a", NULL };
	char *files3[] = { "/sys/class/hwmon/hwmon2/temp", NULL };
	char *files4[] = { "/sys/class/hwmon/hwmon2/temp1", NULL };
	char *files5[] = { "/sys/class/hwmon/hwmon2/foo1_input", NULL };
	char *files6[] = { "/sys/class/hwmon/hwmon2/temp1_foo", NULL };

	/* Failure path - stat fails */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files2);
	will_return(__wrap_stat, 0);
	will_return(__wrap_stat, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - S_ISREG fails */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files2);
	will_return(__wrap_stat, 0);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - first sscanf check fails */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files2);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EFMT);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - second sscanf check fails */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files3);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EFMT);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - third sscanf check fails */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files4);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EFMT);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid sensor */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files5);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid sensor attribute */
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files6);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_set_refresh(void **state)
{
	ami_device dev = { 0 };

	/* Happy path - sensor refresh set OK */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	will_return(__wrap_ioctl, NULL);
	will_return(__wrap_ioctl, 0);
	assert_int_equal(
		ami_sensor_set_refresh(&dev, 0),
		AMI_STATUS_OK
	);
}

void test_fail_ami_sensor_set_refresh(void **state)
{
	ami_device dev = { 0 };

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_set_refresh(NULL, 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_sensor_set_refresh(&dev, 0),
		AMI_STATUS_ERROR
	);

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_sensor_set_refresh(&dev, 0),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_refresh(void **state)
{
	ami_device dev = { 0 };
	uint16_t refresh = 0;

	/* Happy path - sensor refresh read OK */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "1000");
	expect_string(__wrap_ami_convert_num, buf, "1000");
	will_return(__wrap_ami_convert_num, 1000);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_refresh(&dev, &refresh),
		AMI_STATUS_OK
	);
	assert_int_equal(refresh, 1000);
}

void test_fail_ami_sensor_get_refresh(void **state)
{
	ami_device dev = { 0 };
	uint16_t refresh = 0;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_refresh(NULL, &refresh),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_refresh(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_type(void **state)
{
	ami_device dev = { 0 };
	uint32_t type = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	assert_int_equal(
		ami_sensor_get_type(&dev, "foo", &type),
		AMI_STATUS_OK
	);

	assert_int_equal(
		type,
		AMI_SENSOR_TYPE_TEMP |
		AMI_SENSOR_TYPE_POWER |
		AMI_SENSOR_TYPE_CURRENT |
		AMI_SENSOR_TYPE_VOLTAGE
	);
}

void test_fail_ami_sensor_get_type(void **state)
{
	ami_device dev = { 0 };
	uint32_t type = 0;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_type(NULL, "", &type),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `type` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_type(&dev, "", NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_sensors(void **state)
{
	ami_device dev = { 0 };
	struct ami_sensor sensor = { 0 };
	struct ami_sensor *sensors = NULL;
	int num_sensors = 0;

	dev.sensors = &sensor;
	dev.num_sensors = 1;

	/* Happy path - correct values returned */
	assert_int_equal(
		ami_sensor_get_sensors(&dev, &sensors, &num_sensors),
		AMI_STATUS_OK
	);
	assert_int_equal(num_sensors, 1);
	assert_non_null(sensors);
}

void test_fail_ami_sensor_get_sensors(void **state)
{
	ami_device dev = { 0 };
	struct ami_sensor *sensors = NULL;
	struct ami_sensor *sensors2 = NULL;
	int num_sensors = 0;

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_sensors(NULL, &sensors, &num_sensors),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensors` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_sensors(&dev, NULL, &num_sensors),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `num` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_sensors(&dev, &sensors, NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - valid pointer to non-NULL sensors */
	sensors2 = malloc(sizeof(struct ami_sensor));
	assert_non_null(sensors2);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_sensors(&dev, &sensors2, &num_sensors),
		AMI_STATUS_ERROR
	);
	free(sensors2);
}

void test_happy_ami_sensor_get_num_total(void **state)
{
	int num = 0;
	ami_device dev = { 0 };
	dev.num_total_sensors = 4;

	/* Happy path - correct values returned */
	assert_int_equal(
		ami_sensor_get_num_total(&dev, &num),
		AMI_STATUS_OK
	);
	assert_int_equal(num, 4);
}

void test_fail_ami_sensor_get_num_total(void **state)
{
	int num = 0;
	ami_device dev = { 0 };

	/* Failure path - invalid `dev` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_num_total(NULL, &num),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `num` argument */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_num_total(&dev, NULL),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_temp_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* IOCTL return data. */
	struct ami_ioc_sensor_value data = { 
		.status = AMI_SENSOR_OK_STR,
		.fresh = true,
		.val = 123
	};

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve value with no status */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_temp_value(&dev, "foo", &val, NULL),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);

	/* Happy path - retrieve value with status */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	will_return(__wrap_ioctl, &data);
	will_return(__wrap_ioctl, sizeof(data));
	assert_int_equal(
		ami_sensor_get_temp_value(&dev, "foo", &val, &status),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
	assert_int_equal(status, AMI_SENSOR_STATUS_OK);
}

void test_fail_ami_sensor_get_temp_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_value(NULL, "foo", &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_value(&dev, NULL, &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_value(&dev, "foo", NULL, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_value(&dev, "foo", &val, &status),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_voltage_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* IOCTL return data. */
	struct ami_ioc_sensor_value data = { 
		.status = AMI_SENSOR_OK_STR,
		.fresh = true,
		.val = 123
	};

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve value with no status */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_voltage_value(&dev, "foo", &val, NULL),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);

	/* Happy path - retrieve value with status */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	will_return(__wrap_ioctl, &data);
	will_return(__wrap_ioctl, sizeof(data));
	assert_int_equal(
		ami_sensor_get_voltage_value(&dev, "foo", &val, &status),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
	assert_int_equal(status, AMI_SENSOR_STATUS_OK);
}

void test_fail_ami_sensor_get_voltage_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_value(NULL, "foo", &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_value(&dev, NULL, &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_value(&dev, "foo", NULL, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_value(&dev, "foo", &val, &status),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_current_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* IOCTL return data. */
	struct ami_ioc_sensor_value data = { 
		.status = AMI_SENSOR_OK_STR,
		.fresh = false,
		.val = 123
	};

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve value with no status */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_current_value(&dev, "foo", &val, NULL),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);

	/* Happy path - retrieve value with status */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	will_return(__wrap_ioctl, &data);
	will_return(__wrap_ioctl, sizeof(data));
	assert_int_equal(
		ami_sensor_get_current_value(&dev, "foo", &val, &status),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
	assert_int_equal(status, AMI_SENSOR_STATUS_OK_CACHED);
}

void test_fail_ami_sensor_get_current_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_value(NULL, "foo", &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_value(&dev, NULL, &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_value(&dev, "foo", NULL, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_value(&dev, "foo", &val, &status),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_power_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* IOCTL return data. */
	struct ami_ioc_sensor_value data = { 
		.status = AMI_SENSOR_OK_STR,
		.fresh = true,
		.val = 123
	};

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve value with no status */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_power_value(&dev, "foo", &val, NULL),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);

	/* Happy path - retrieve value with status */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_OK);
	will_return(__wrap_ioctl, &data);
	will_return(__wrap_ioctl, sizeof(data));
	assert_int_equal(
		ami_sensor_get_power_value(&dev, "foo", &val, &status),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
	assert_int_equal(status, AMI_SENSOR_STATUS_OK);
}

void test_fail_ami_sensor_get_power_value(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_value(NULL, "foo", &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_value(&dev, NULL, &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_value(&dev, "foo", NULL, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_value(&dev, "foo", &val, &status),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_temp_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, AMI_SENSOR_OK_STR);
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_OK);
}

void test_fail_ami_sensor_get_temp_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_status(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_voltage_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, AMI_SENSOR_OK_STR);
	assert_int_equal(
		ami_sensor_get_voltage_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_OK);
}

void test_fail_ami_sensor_get_voltage_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_status(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_status(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_status(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_status(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_current_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, AMI_SENSOR_OK_STR);
	assert_int_equal(
		ami_sensor_get_current_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_OK);
}

void test_fail_ami_sensor_get_current_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_status(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_status(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_status(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_status(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_power_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, AMI_SENSOR_OK_STR);
	assert_int_equal(
		ami_sensor_get_power_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_OK);
}

void test_fail_ami_sensor_get_power_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_status(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_status(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_status(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_status(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_temp_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_temp_uptime_max(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_temp_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_max(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_max(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_max(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_temp.max.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_temp.max.valid = true;
}

void test_happy_ami_sensor_get_voltage_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_max(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_voltage_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_max(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_max(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_max(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_voltage.max.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_voltage.max.valid = true;
}

void test_happy_ami_sensor_get_current_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_current_uptime_max(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_current_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_max(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_max(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_max(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_current.max.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_current.max.valid = true;
}

void test_happy_ami_sensor_get_power_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_power_uptime_max(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_power_uptime_max(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_max(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_max(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_max(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_power.max.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_max(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_power.max.valid = true;
}

void test_happy_ami_sensor_get_temp_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_temp_uptime_average(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_temp_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_average(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_average(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_average(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_temp.average.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_temp.average.valid = true;
}

void test_happy_ami_sensor_get_voltage_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_average(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_voltage_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_average(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_average(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_average(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_voltage.average.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_voltage.average.valid = true;
}

void test_happy_ami_sensor_get_current_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_current_uptime_average(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_current_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_average(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_average(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_average(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_current.average.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_current.average.valid = true;
}

void test_happy_ami_sensor_get_power_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "123");
	expect_string(__wrap_ami_convert_num, buf, "123");
	will_return(__wrap_ami_convert_num, 123);
	will_return(__wrap_ami_convert_num, AMI_STATUS_OK);
	assert_int_equal(
		ami_sensor_get_power_uptime_average(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, 123);
}

void test_fail_ami_sensor_get_power_uptime_average(void **state)
{
	ami_device dev = { 0 };
	long val = 0;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_average(NULL, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_average(&dev, NULL, &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `val` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_average(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);

	/* Failure path - value not valid */
	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;
	test_power.average.valid = false;
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_uptime_average(&dev, "foo", &val),
		AMI_STATUS_ERROR
	);
	test_power.average.valid = true;
}

void test_happy_ami_sensor_get_temp_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	assert_int_equal(
		ami_sensor_get_temp_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_OK
	);
}

void test_fail_ami_sensor_get_temp_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_unit_mod(NULL, "foo", &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_unit_mod(&dev, NULL, &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `mod` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_unit_mod(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_temp_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_voltage_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	assert_int_equal(
		ami_sensor_get_voltage_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_OK
	);
}

void test_fail_ami_sensor_get_voltage_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_unit_mod(NULL, "foo", &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_unit_mod(&dev, NULL, &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `mod` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_unit_mod(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_voltage_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_current_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	assert_int_equal(
		ami_sensor_get_current_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_OK
	);
}

void test_fail_ami_sensor_get_current_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_unit_mod(NULL, "foo", &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_unit_mod(&dev, NULL, &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `mod` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_unit_mod(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_current_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_ERROR
	);
}

void test_happy_ami_sensor_get_power_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - retrieve correct value */
	assert_int_equal(
		ami_sensor_get_power_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_OK
	);
}

void test_fail_ami_sensor_get_power_unit_mod(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_unit_mod mod = AMI_SENSOR_UNIT_MOD_NONE;

	/* Failure path - invalid `dev` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_unit_mod(NULL, "foo", &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `sensor_name` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_unit_mod(&dev, NULL, &mod),
		AMI_STATUS_ERROR
	);

	/* Failure path - invalid `mod` arugment */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_unit_mod(&dev, "foo", NULL),
		AMI_STATUS_ERROR
	);

	/* Failure path - sensor not found */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	assert_int_equal(
		ami_sensor_get_power_unit_mod(&dev, "foo", &mod),
		AMI_STATUS_ERROR
	);
}

/*
* NOTE: Using `ami_sensor_get_temp_status` as a proxy to test the static
* function `parse_sensor_status`.
*/

void test_happy_parse_sensor_status(void **state)
{
	
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Happy path - sensor OK */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "Sensor Present and Valid");
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_OK);

	/* Happy path - sensor not present */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "Sensor Not Present");
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_NOT_PRESENT);

	/* Happy path - no data */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "Data Not Available");
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_NO_DATA);

	/* Happy path - N/A */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "Not Applicable or Default Value");
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_NA);
}

void test_fail_parse_sensor_status(void **state)
{
	ami_device dev = { 0 };
	enum ami_sensor_status val = AMI_SENSOR_STATUS_INVALID;

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Failure path - invalid status string */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	will_return(__wrap_read, "invalid");
	assert_int_equal(
		ami_sensor_get_temp_status(&dev, "foo", &val),
		AMI_STATUS_OK
	);
	assert_int_equal(val, AMI_SENSOR_STATUS_INVALID);
}

/* Using `ami_sensor_get_refresh` to test the read/write hwmon functions. */

void test_fail_read_hwmon(void **state)
{
	ami_device dev = { 0 };
	uint16_t refresh = 0;

	/* Failure path - open fails */
	WRAPPER_ACTION(FAIL, open);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EBADF);
	assert_int_equal(
		ami_sensor_get_refresh(&dev, &refresh),
		AMI_STATUS_ERROR
	);

	/* Failure path - read fails */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(FAIL, read);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_sensor_get_refresh(&dev, &refresh),
		AMI_STATUS_ERROR
	);
}

/* Using `ami_sensor_discover` to test `populate_device_sensors`. */

void test_fail_populate_device_sensors(void **state)
{
	ami_device dev = { 0 };

	const char *files[] = {
		"/sys/class/hwmon/hwmon2/power1_label",
		NULL
	};

	/* Failure path - first calloc fails */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION_C(CMOCKA, calloc, 2);
	will_return(__wrap_calloc, REAL);
	will_return(__wrap_calloc, FAIL);
	will_return(__wrap_read, "device");
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	/* find_sensor_by_name will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ENOMEM);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);

	/* Failure path - second calloc fails */
	WRAPPER_ACTION(OK, open);
	WRAPPER_ACTION(OK, close);
	WRAPPER_ACTION(OK, read);
	WRAPPER_ACTION_C(CMOCKA, calloc, 3);
	will_return(__wrap_calloc, REAL);
	will_return(__wrap_calloc, REAL);
	will_return(__wrap_calloc, FAIL);
	will_return(__wrap_read, "device");
	will_return(__wrap_glob, AMI_LINUX_STATUS_OK);
	will_return(__wrap_glob, 1);
	will_return(__wrap_glob, files);
	will_return(__wrap_stat, __S_IFREG);
	will_return(__wrap_stat, AMI_LINUX_STATUS_OK);
	/* find_sensor_data will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	/* find_sensor_by_name will fail once */
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EINVAL);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_ENOMEM);
	assert_int_equal(
		ami_sensor_discover(&dev),
		AMI_STATUS_ERROR
	);
	delete_sensors(&dev);
}

/* Using `ami_sensor_get_temp_value` to test `get_single_sensor_val`. */

void test_fail_get_single_sensor_val(void **state)
{
	ami_device dev = { 0 };
	long val = 0;
	enum ami_sensor_status status = AMI_SENSOR_STATUS_INVALID;

	/* IOCTL return data. */
	struct ami_ioc_sensor_value data = { 
		.status = AMI_SENSOR_OK_STR,
		.fresh = true,
		.val = 123
	};

	dev.sensors = &test_sensor;
	dev.num_sensors = 1;
	dev.num_total_sensors = 4;

	/* Failure path - ami_open_cdev fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_ERROR);
	assert_int_equal(
		ami_sensor_get_temp_value(&dev, "foo", &val, &status),
		AMI_STATUS_ERROR
	);

	/* Failure path - ioctl fails */
	will_return(__wrap_ami_open_cdev, AMI_STATUS_OK);
	will_return(__wrap_ioctl, AMI_LINUX_STATUS_ERROR);
	expect_function_call(__wrap_ami_set_last_error);
	expect_value(__wrap_ami_set_last_error, err, AMI_ERROR_EIO);
	assert_int_equal(
		ami_sensor_get_temp_value(&dev, "foo", &val, &status),
		AMI_STATUS_ERROR
	);
}

/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_ami_sensor_discover),
		cmocka_unit_test(test_fail_ami_sensor_discover),
		cmocka_unit_test(test_fail_parse_hwmon),
		cmocka_unit_test(test_happy_ami_sensor_set_refresh),
		cmocka_unit_test(test_fail_ami_sensor_set_refresh),
		cmocka_unit_test(test_happy_ami_sensor_get_refresh),
		cmocka_unit_test(test_fail_ami_sensor_get_refresh),
		cmocka_unit_test(test_happy_ami_sensor_get_type),
		cmocka_unit_test(test_fail_ami_sensor_get_type),
		cmocka_unit_test(test_happy_ami_sensor_get_sensors),
		cmocka_unit_test(test_fail_ami_sensor_get_sensors),
		cmocka_unit_test(test_happy_ami_sensor_get_num_total),
		cmocka_unit_test(test_fail_ami_sensor_get_num_total),
		cmocka_unit_test(test_happy_ami_sensor_get_temp_value),
		cmocka_unit_test(test_fail_ami_sensor_get_temp_value),
		cmocka_unit_test(test_happy_ami_sensor_get_voltage_value),
		cmocka_unit_test(test_fail_ami_sensor_get_voltage_value),
		cmocka_unit_test(test_happy_ami_sensor_get_current_value),
		cmocka_unit_test(test_fail_ami_sensor_get_current_value),
		cmocka_unit_test(test_happy_ami_sensor_get_power_value),
		cmocka_unit_test(test_fail_ami_sensor_get_power_value),
		cmocka_unit_test(test_happy_ami_sensor_get_temp_status),
		cmocka_unit_test(test_fail_ami_sensor_get_temp_status),
		cmocka_unit_test(test_happy_ami_sensor_get_voltage_status),
		cmocka_unit_test(test_fail_ami_sensor_get_voltage_status),
		cmocka_unit_test(test_happy_ami_sensor_get_current_status),
		cmocka_unit_test(test_fail_ami_sensor_get_current_status),
		cmocka_unit_test(test_happy_ami_sensor_get_power_status),
		cmocka_unit_test(test_fail_ami_sensor_get_power_status),
		cmocka_unit_test(test_happy_ami_sensor_get_temp_uptime_max),
		cmocka_unit_test(test_fail_ami_sensor_get_temp_uptime_max),
		cmocka_unit_test(test_happy_ami_sensor_get_voltage_uptime_max),
		cmocka_unit_test(test_fail_ami_sensor_get_voltage_uptime_max),
		cmocka_unit_test(test_happy_ami_sensor_get_current_uptime_max),
		cmocka_unit_test(test_fail_ami_sensor_get_current_uptime_max),
		cmocka_unit_test(test_happy_ami_sensor_get_power_uptime_max),
		cmocka_unit_test(test_fail_ami_sensor_get_power_uptime_max),
		cmocka_unit_test(test_happy_ami_sensor_get_temp_uptime_average),
		cmocka_unit_test(test_fail_ami_sensor_get_temp_uptime_average),
		cmocka_unit_test(test_happy_ami_sensor_get_voltage_uptime_average),
		cmocka_unit_test(test_fail_ami_sensor_get_voltage_uptime_average),
		cmocka_unit_test(test_happy_ami_sensor_get_current_uptime_average),
		cmocka_unit_test(test_fail_ami_sensor_get_current_uptime_average),
		cmocka_unit_test(test_happy_ami_sensor_get_power_uptime_average),
		cmocka_unit_test(test_fail_ami_sensor_get_power_uptime_average),
		cmocka_unit_test(test_happy_ami_sensor_get_temp_unit_mod),
		cmocka_unit_test(test_fail_ami_sensor_get_temp_unit_mod),
		cmocka_unit_test(test_happy_ami_sensor_get_voltage_unit_mod),
		cmocka_unit_test(test_fail_ami_sensor_get_voltage_unit_mod),
		cmocka_unit_test(test_happy_ami_sensor_get_current_unit_mod),
		cmocka_unit_test(test_fail_ami_sensor_get_current_unit_mod),
		cmocka_unit_test(test_happy_ami_sensor_get_power_unit_mod),
		cmocka_unit_test(test_fail_ami_sensor_get_power_unit_mod),
		cmocka_unit_test(test_happy_parse_sensor_status),
		cmocka_unit_test(test_fail_parse_sensor_status),
		cmocka_unit_test(test_fail_read_hwmon),
		cmocka_unit_test(test_fail_populate_device_sensors),
		cmocka_unit_test(test_fail_get_single_sensor_val),
	};

	return cmocka_run_group_tests(tests, setup_data, NULL);
}
