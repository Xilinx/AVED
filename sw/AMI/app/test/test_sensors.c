// SPDX-License-Identifier: GPL-2.0-only
/*
 * test_sensors.c - Unit test file for sensors.c
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
#include "sensors.c"  /* Including .c file to test static functions. */
#include "printer.h"
#include "amiapp.h"

/* API includes */
#include "ami_device.h"
#include "ami_sensor.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define ASSERT_EQ_AND_SET(a, b) do { assert_int_equal(a, b); a = b; } while (0)
#define ASSERT_NE_AND_SET(a, b) do { assert_int_not_equal(a, b); a = b; } while (0)

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

static struct wrapper w_fclose = { OK, OK, 0, 0 };
static struct wrapper w_malloc = { REAL, REAL, 0, 0 };
static struct wrapper w_calloc = { REAL, REAL, 0, 0 };

/*****************************************************************************/
/* Redefinitions/Wrapping                                                    */
/*****************************************************************************/

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

extern void *__real_calloc(size_t nmemb, size_t size);

void *__wrap_calloc(size_t nmemb, size_t size)
{
	void *ret = NULL;

	switch (w_calloc.current) {
	/* case OK not implemented */

	case REAL:
		ret = __real_calloc(nmemb, size);
		break;
	
	default:
		break;
	}
	
	WRAPPER_DONE(calloc);
	return ret;
}

/* Status getters */

int __wrap_ami_sensor_get_temp_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	*val = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_voltage_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	*val = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_current_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	*val = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_power_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	*val = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

/* Value getters */

int __wrap_ami_sensor_get_temp_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	*val = 1;
	*sensor_status = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_voltage_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	*val = 1;
	*sensor_status = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_current_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	*val = 1;
	*sensor_status = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_power_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	*val = 1;
	*sensor_status = AMI_SENSOR_STATUS_OK;
	return AMI_STATUS_OK;
}

/* Max getters */

int __wrap_ami_sensor_get_temp_uptime_max(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_voltage_uptime_max(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_current_uptime_max(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_power_uptime_max(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

/* Average getters */

int __wrap_ami_sensor_get_temp_uptime_average(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_voltage_uptime_average(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_current_uptime_average(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_power_uptime_average(ami_device *dev, const char *sensor_name,
	long *val)
{
	*val = 1;
	return AMI_STATUS_OK;
}

/* Unit mod getters */

int __wrap_ami_sensor_get_temp_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	*mod = AMI_SENSOR_UNIT_MOD_MEGA;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_voltage_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	*mod = AMI_SENSOR_UNIT_MOD_MEGA;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_current_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	*mod = AMI_SENSOR_UNIT_MOD_MEGA;
	return AMI_STATUS_OK;
}

int __wrap_ami_sensor_get_power_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	*mod = AMI_SENSOR_UNIT_MOD_MEGA;
	return AMI_STATUS_OK;
}

/* Type getter */

int __wrap_ami_sensor_get_type(ami_device *dev, const char *sensor_name, uint32_t *type)
{
	*type = (int)mock();
	return (int)mock();
}

/* Other API functions */

int __wrap_ami_dev_find(const char *bdf, ami_device **dev)
{
	if ((int)mock() == AMI_STATUS_OK) {
		*dev = (ami_device *)1;  /* DO NOT DEREFERENCE (obviously) */
		return AMI_STATUS_OK;
	}

	return AMI_STATUS_ERROR;
}

int __wrap_ami_dev_find_next(ami_device **dev, int b, int d, int f, ami_device *prev)
{
	if ((int)mock() == AMI_STATUS_OK) {
		*dev = (ami_device *)1;  /* DO NOT DEREFERENCE (obviously) */
		return AMI_STATUS_OK;
	}

	return AMI_STATUS_ERROR;
}

void __wrap_ami_dev_delete(ami_device **dev)
{

}

int __wrap_ami_dev_get_pci_bdf(ami_device *dev, uint16_t *bdf)
{
	return (int)mock();
}

int __wrap_ami_sensor_discover(ami_device *dev)
{
	return (int)mock();
}

int __wrap_ami_sensor_get_sensors(ami_device *dev, struct ami_sensor **sensors, int *num)
{
	static struct ami_sensor sensor = {
		"foo",
		NULL,
		NULL
	};

	*sensors = &sensor;
	return (int)mock();
}

int __wrap_ami_sensor_get_num_total(ami_device *dev, int *num)
{
	*num = (int)mock();
	return (int)mock();
}

/* Json lib */

JsonNode *__wrap_json_mknull(void)
{

}

JsonNode *__wrap_json_mknumber(double n)
{

}

JsonNode *__wrap_json_mkobject(void)
{
	static JsonNode node = { 0 };
	return &node;
}

void __wrap_json_append_member(JsonNode *object, const char *key, JsonNode *value)
{

}

void __wrap_json_delete(JsonNode *node)
{

}

/* App functions */

int __wrap_print_table_data(ami_device *dev, int n_fields, int n_rows, FILE *stream,
	enum table_divider_format divider_fmt, app_value_builder populate_values,
	app_header_builder populate_header, void *data, int *col_align)
{
	return (int)mock();
}

int __wrap_print_json_data(ami_device *dev, int n_fields, int n_rows, FILE *stream,
	app_value_builder populate_values, void *data)
{
	return 0;
}

int __wrap_print_json_obj(JsonNode *obj, FILE *stream)
{
	return 0;
}

int __wrap_gen_json_data(ami_device *dev, int n_fields, int n_rows,
	app_value_builder populate_values, void *data, JsonNode **out)
{
	static JsonNode node = { 0 };
	*out = &node;
	return 0;
}

struct app_option* __wrap_find_app_option(const int val, struct app_option *options)
{
	return (struct app_option*)mock();
}

int __wrap_parse_output_options(struct app_option *options, enum app_out_format *fmt,
	bool *verbose, FILE **stream, bool *fmt_given, bool *output_given)
{
	*fmt = (enum app_out_format)mock();
	*verbose = (bool)mock();
	*stream = (FILE*)mock();
	*fmt_given = (bool)mock();
	*output_given = (bool)mock();
	return (int)mock();
}

/*****************************************************************************/
/* Tests                                                                     */
/*****************************************************************************/

void test_happy_report_sensors(void **state)
{
	struct app_option opt = { 0 };
	struct app_option extra1 = { 'x', "max", 0, false, NULL };
	struct app_option extra2 = { 'x', "average", 0, false, NULL };
	struct app_option extra_csv = { 'x', "average,max", 0, false, NULL };
	struct app_option sensor = { 'n', "vccint", 0, false, NULL };
	FILE dummy_file = { 0 };

	extra1.next = &extra2;

	/* Happy path - single device, all sensors, table format */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - single device, all sensors, table format, multiple extra fields */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(&extra1),
		EXIT_SUCCESS
	);

	/* Happy path - single device, all sensors, table format, multiple extra fields (csv) */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(&extra_csv),
		EXIT_SUCCESS
	);

	/* Happy path - sensor name specified */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, &sensor);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_type, 1);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - format option given as table */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, true);                 /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - format option given as JSON */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_JSON);  /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, true);                 /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - invalid format specified, output given */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_INVALID); /* fmt */
	will_return(__wrap_parse_output_options, false);                  /* verbose */
	will_return(__wrap_parse_output_options, &dummy_file);            /* stream */
	will_return(__wrap_parse_output_options, true);                   /* format_given */
	will_return(__wrap_parse_output_options, true);                   /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);           /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - format given as JSON and output specified */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_JSON);  /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, &dummy_file);          /* stream */
	will_return(__wrap_parse_output_options, true);                 /* format_given */
	will_return(__wrap_parse_output_options, true);                 /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);
	
	/* Happy path - verbose option specified */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, true);                 /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - no device specified (iterating 2 devices) */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return_count(__wrap_find_app_option, NULL, 2);
	will_return_count(__wrap_ami_dev_get_pci_bdf, AMI_STATUS_OK, 2);
	will_return_count(__wrap_ami_dev_find_next, AMI_STATUS_OK, 2);
	will_return(__wrap_ami_dev_find_next, AMI_STATUS_ERROR);
	will_return_count(__wrap_ami_sensor_discover, AMI_STATUS_OK, 2);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return_count(__wrap_print_table_data, EXIT_SUCCESS, 2);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - no device specified (ami_dev_get_pci_bdf fails) */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return_count(__wrap_find_app_option, NULL, 2);
	will_return(__wrap_ami_dev_get_pci_bdf, AMI_STATUS_ERROR);
	will_return(__wrap_ami_dev_find_next, AMI_STATUS_OK);
	will_return(__wrap_ami_dev_find_next, AMI_STATUS_ERROR);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_print_table_data, EXIT_SUCCESS);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Happy path - no device specified, format JSON, output specified */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_JSON);  /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, &dummy_file);          /* stream */
	will_return(__wrap_parse_output_options, true);                 /* format_given */
	will_return(__wrap_parse_output_options, true);                 /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return_count(__wrap_find_app_option, NULL, 2);
	will_return_count(__wrap_ami_dev_get_pci_bdf, AMI_STATUS_OK, 2);
	will_return_count(__wrap_ami_dev_find_next, AMI_STATUS_OK, 2);
	will_return(__wrap_ami_dev_find_next, AMI_STATUS_ERROR);
	will_return_count(__wrap_ami_sensor_discover, AMI_STATUS_OK, 2);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	will_return_count(__wrap_print_table_data, EXIT_SUCCESS, 2);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);
}

void test_fail_report_sensors(void **state)
{
	struct app_option opt = { 0 };
	struct app_option fmt_json = { 'f', "json", 0, false, NULL };
	struct app_option outfile = { 'o', "foo.json", 0, false, NULL };

	/* Failure path - ami_dev_find fails */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_ERROR);
	assert_int_not_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Failure path - ami_sensor_discover fails */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_ERROR);
	assert_int_not_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Failure path - ami_sensor_discover fails when iterating devices */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return_count(__wrap_find_app_option, NULL, 2);
	will_return(__wrap_ami_dev_get_pci_bdf, AMI_STATUS_OK);
	will_return(__wrap_ami_dev_find_next, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_ERROR);
	assert_int_not_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Failure path - print_sensor_data fails */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, false);                /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return_count(__wrap_find_app_option, NULL, 2);
	will_return(__wrap_ami_dev_get_pci_bdf, AMI_STATUS_OK);
	will_return(__wrap_ami_dev_find_next, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 1);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_ERROR);
	assert_int_not_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);

	/* Failure path - `parse_output_options` fails */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, true);                 /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_FAILURE);         /* return */
	assert_int_not_equal(
		report_sensors(NULL),
		EXIT_SUCCESS
	);
}

/* Using `report_sensors` to test `print_sensor_data` */

void test_fail_print_sensor_data(void **state)
{
	struct app_option opt = { 0 };
	struct app_option sensor = { 'n', "vccint", 0, false, NULL };

	/* Failure path - 0 rows */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, true);                 /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, NULL);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_num_total, 0);
	will_return(__wrap_ami_sensor_get_num_total, AMI_STATUS_OK);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_FAILURE
	);

	/* Failure path - ami_sensor_get_type fails */
	will_return(__wrap_parse_output_options, APP_OUT_FORMAT_TABLE); /* fmt */
	will_return(__wrap_parse_output_options, false);                /* verbose */
	will_return(__wrap_parse_output_options, NULL);                 /* stream */
	will_return(__wrap_parse_output_options, true);                 /* format_given */
	will_return(__wrap_parse_output_options, false);                /* output_given */
	will_return(__wrap_parse_output_options, EXIT_SUCCESS);         /* return */
	will_return(__wrap_find_app_option, &sensor);
	will_return(__wrap_find_app_option, &opt);
	will_return(__wrap_ami_dev_find, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_discover, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_type, 1);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_ERROR);
	assert_int_equal(
		report_sensors(NULL),
		EXIT_FAILURE
	);
}

/* Static function tests (these rely on the .c file being #included) */

void test_fail_static_print_sensor_data(void **state)
{
	ami_device *dev = (ami_device*)1;
	JsonNode **out = (JsonNode**)1;

	/* Failure path - invalid `dev` argument */
	assert_int_equal(
		print_sensor_data(
			NULL,
			EXTRA_FIELDS_ALL,
			"foo",
			NULL,
			APP_OUT_FORMAT_TABLE,
			out
		),
		EXIT_FAILURE
	);
}

void test_fail_static_parse_extra(void **state)
{
	int extra = 0;
	struct app_option extra1 = { 'x', "max", 0, false, NULL };

	/* Failure path - malloc fails */
	WRAPPER_ACTION(FAIL, malloc);
	assert_int_equal(
		parse_extra(&extra1, &extra),
		EXIT_FAILURE
	);
}

void test_happy_static_make_unit_string(void **state)
{
	char unit[3] = { 0 };

	/* Temperature tests */

	/* Happy path - MEGA, temperature */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MEGA, AMI_SENSOR_TYPE_TEMP, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "MC");
	memset(unit, 0x00, 3);

	/* Happy path - KILO, temperature */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_KILO, AMI_SENSOR_TYPE_TEMP, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "kC");
	memset(unit, 0x00, 3);

	/* Happy path - MILLI, temperature */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MILLI, AMI_SENSOR_TYPE_TEMP, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "mC");
	memset(unit, 0x00, 3);

	/* Happy path - MICRO, temperature */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MICRO, AMI_SENSOR_TYPE_TEMP, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "uC");
	memset(unit, 0x00, 3);

	/* Happy path - NONE, temperature */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_NONE, AMI_SENSOR_TYPE_TEMP, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "C");
	memset(unit, 0x00, 3);

	/* Voltage tests */

	/* Happy path - MEGA, voltage */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MEGA, AMI_SENSOR_TYPE_VOLTAGE, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "MV");
	memset(unit, 0x00, 3);

	/* Happy path - KILO, voltage */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_KILO, AMI_SENSOR_TYPE_VOLTAGE, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "kV");
	memset(unit, 0x00, 3);

	/* Happy path - MILLI, voltage */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MILLI, AMI_SENSOR_TYPE_VOLTAGE, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "mV");
	memset(unit, 0x00, 3);

	/* Happy path - MICRO, voltage */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MICRO, AMI_SENSOR_TYPE_VOLTAGE, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "uV");
	memset(unit, 0x00, 3);

	/* Happy path - NONE, voltage */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_NONE, AMI_SENSOR_TYPE_VOLTAGE, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "V");
	memset(unit, 0x00, 3);

	/* Current tests */

	/* Happy path - MEGA, current */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MEGA, AMI_SENSOR_TYPE_CURRENT, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "MA");
	memset(unit, 0x00, 3);

	/* Happy path - KILO, current */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_KILO, AMI_SENSOR_TYPE_CURRENT, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "kA");
	memset(unit, 0x00, 3);

	/* Happy path - MILLI, current */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MILLI, AMI_SENSOR_TYPE_CURRENT, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "mA");
	memset(unit, 0x00, 3);

	/* Happy path - MICRO, current */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MICRO, AMI_SENSOR_TYPE_CURRENT, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "uA");
	memset(unit, 0x00, 3);

	/* Happy path - NONE, current */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_NONE, AMI_SENSOR_TYPE_CURRENT, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "A");
	memset(unit, 0x00, 3);

	/* Power tests */

	/* Happy path - MEGA, power */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MEGA, AMI_SENSOR_TYPE_POWER, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "MW");
	memset(unit, 0x00, 3);

	/* Happy path - KILO, power */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_KILO, AMI_SENSOR_TYPE_POWER, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "kW");
	memset(unit, 0x00, 3);

	/* Happy path - MILLI, power */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MILLI, AMI_SENSOR_TYPE_POWER, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "mW");
	memset(unit, 0x00, 3);

	/* Happy path - MICRO, power */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MICRO, AMI_SENSOR_TYPE_POWER, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "uW");
	memset(unit, 0x00, 3);

	/* Happy path - NONE, power */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_NONE, AMI_SENSOR_TYPE_POWER, unit),
		EXIT_SUCCESS
	);
	assert_string_equal(unit, "W");
	memset(unit, 0x00, 3);
}

void test_fail_static_make_unit_string(void **state)
{
	char unit[3] = { 0 };

	/* Failure path - invalid `unit` argument */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MEGA, AMI_SENSOR_TYPE_POWER, NULL),
		EXIT_FAILURE
	);

	/* Failure path - invalid unit mod */
	assert_int_equal(
		make_unit_string(-1, AMI_SENSOR_TYPE_POWER, unit),
		EXIT_FAILURE
	);

	/* Failure path - invalid sensor type */
	assert_int_equal(
		make_unit_string(AMI_SENSOR_UNIT_MOD_MEGA, -1, unit),
		EXIT_FAILURE
	);
}

void test_happy_static_populate_sensor_header(void **state)
{
	int i = 0;
	char *header[5] = { 0 };
	ami_device *dev = (ami_device*)1;
	struct app_sensor_data data = { 0 };

	/* Happy path - all header fields populated */
	data.extra_fields = EXTRA_FIELDS_ALL;

	for (i = 0; i < 5; i++) {
		header[i] = calloc(10, sizeof(char));
		assert_non_null(header[i]);
	}

	assert_int_equal(
		populate_sensor_header(
			dev, header, 5, &data
		),
		EXIT_SUCCESS
	);

	for (i = 0; i < 5; i++) {
		assert_int_not_equal(header[i][0], 0);
		free(header[i]);
	}
}

void test_fail_static_populate_sensor_header(void **state)
{
	char *header[] = { NULL, "b", "c" };
	ami_device *dev = (ami_device*)1;
	struct app_sensor_data data = { 0 };

	/* Failure path - invalid `header` argument */
	assert_int_equal(
		populate_sensor_header(
			dev, NULL, 0, &data
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `data` argument */
	assert_int_equal(
		populate_sensor_header(
			dev, header, 0, NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `dev` argument */
	assert_int_equal(
		populate_sensor_header(
			NULL, header, 0, &data
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid header element */
	assert_int_equal(
		populate_sensor_header(
			dev, header, 3, &data
		),
		EXIT_FAILURE
	);
}

void test_happy_static_get_all_sensor_values(void **state)
{
	ami_device *dev = (ami_device*)1;
	const char *sensor = "foo";
	double value = 0, max = 0, avg = 0;
	enum ami_sensor_unit_mod mod = 0;
	enum ami_sensor_status status = 0;
	int max_ret = 1, avg_ret = 1;

	/* Happy path - temperature */
	get_all_sensor_values(
		dev, sensor,
		AMI_SENSOR_TYPE_TEMP, EXTRA_FIELDS_ALL,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);
	ASSERT_NE_AND_SET(value, 0);
	ASSERT_NE_AND_SET(status, 0);
	ASSERT_NE_AND_SET(mod, 0);
	ASSERT_NE_AND_SET(max_ret, 1);
	ASSERT_NE_AND_SET(max, 0);
	ASSERT_NE_AND_SET(avg_ret, 1);
	ASSERT_NE_AND_SET(avg, 0);

	/* Happy path - power */
	get_all_sensor_values(
		dev, sensor,
		AMI_SENSOR_TYPE_POWER, EXTRA_FIELDS_ALL,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);
	ASSERT_NE_AND_SET(value, 0);
	ASSERT_NE_AND_SET(status, 0);
	ASSERT_NE_AND_SET(mod, 0);
	ASSERT_NE_AND_SET(max_ret, 1);
	ASSERT_NE_AND_SET(max, 0);
	ASSERT_NE_AND_SET(avg_ret, 1);
	ASSERT_NE_AND_SET(avg, 0);

	/* Happy path - current */
	get_all_sensor_values(
		dev, sensor,
		AMI_SENSOR_TYPE_CURRENT, EXTRA_FIELDS_ALL,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);
	ASSERT_NE_AND_SET(value, 0);
	ASSERT_NE_AND_SET(status, 0);
	ASSERT_NE_AND_SET(mod, 0);
	ASSERT_NE_AND_SET(max_ret, 1);
	ASSERT_NE_AND_SET(max, 0);
	ASSERT_NE_AND_SET(avg_ret, 1);
	ASSERT_NE_AND_SET(avg, 0);

	/* Happy path - voltage */
	get_all_sensor_values(
		dev, sensor,
		AMI_SENSOR_TYPE_VOLTAGE, EXTRA_FIELDS_ALL,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);
	ASSERT_NE_AND_SET(value, 0);
	ASSERT_NE_AND_SET(status, 0);
	ASSERT_NE_AND_SET(mod, 0);
	ASSERT_NE_AND_SET(max_ret, 1);
	ASSERT_NE_AND_SET(max, 0);
	ASSERT_NE_AND_SET(avg_ret, 1);
	ASSERT_NE_AND_SET(avg, 0);

	/* Happy path - omit `mod` argument */
	get_all_sensor_values(
		dev, sensor,
		AMI_SENSOR_TYPE_VOLTAGE, EXTRA_FIELDS_ALL,
		&value, &status,
		NULL,
		&max_ret, &max,
		&avg_ret, &avg
	);
	ASSERT_NE_AND_SET(value, 0);
	ASSERT_NE_AND_SET(status, 0);
	ASSERT_NE_AND_SET(max_ret, 1);
	ASSERT_NE_AND_SET(max, 0);
	ASSERT_NE_AND_SET(avg_ret, 1);
	ASSERT_NE_AND_SET(avg, 0);
}

void test_fail_static_get_all_sensor_values(void **state)
{
	/* Note: this is a void function so no errors are returned. */
	ami_device *dev = (ami_device*)1;
	const char *sensor = "foo";
	double value = 0, max = 0, avg = 0;
	enum ami_sensor_unit_mod mod = 0;
	enum ami_sensor_status status = 0;
	int max_ret = 0, avg_ret = 0;

	/* Failure path - invalid `dev` argument */
	get_all_sensor_values(
		NULL, sensor,
		0, 0,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);

	/* Failure path - invalid `sensor` argument */
	get_all_sensor_values(
		dev, NULL,
		0, 0,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);

	/* Failure path - invalid `value` argument */
	get_all_sensor_values(
		dev, sensor,
		0, 0,
		NULL, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);

	/* Failure path - invalid `status` argument */
	get_all_sensor_values(
		dev, sensor,
		0, 0,
		&value, NULL,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);

	/* Failure path - invalid `max_ret` argument */
	get_all_sensor_values(
		dev, sensor,
		0, 0,
		&value, &status,
		&mod,
		NULL, &max,
		&avg_ret, &avg
	);

	/* Failure path - invalid `max` argument */
	get_all_sensor_values(
		dev, sensor,
		0, 0,
		&value, &status,
		&mod,
		&max_ret, NULL,
		&avg_ret, &avg
	);

	/* Failure path - invalid `avg_ret` argument */
	get_all_sensor_values(
		dev, sensor,
		0, 0,
		&value, &status,
		&mod,
		&max_ret, &max,
		NULL, &avg
	);

	/* Failure path - invalid `avg` argument */
	get_all_sensor_values(
		dev, sensor,
		0, 0,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, NULL
	);

	/* Failure path - invalid sensor type */
	get_all_sensor_values(
		dev, sensor,
		-1, 0,
		&value, &status,
		&mod,
		&max_ret, &max,
		&avg_ret, &avg
	);
}

void test_happy_static_mk_sensor_row(void **state)
{
	int i = 0;
	ami_device *dev = (ami_device*)1;
	char *row[5] = { 0 };

	for (i = 0; i < 5; i++) {
		row[i] = calloc(10, sizeof(char));
		assert_non_null(row[i]);
	}

	/* Happy path - row 0 */
	mk_sensor_row(dev, "foo", AMI_SENSOR_TYPE_TEMP, 0, EXTRA_FIELDS_ALL, row);

	/* Happy path - row 1 */
	mk_sensor_row(dev, "foo", AMI_SENSOR_TYPE_TEMP, 1, EXTRA_FIELDS_ALL, row);

	/* Happy path - make_unit_string fails, get_all_sensor_values fails */
	mk_sensor_row(dev, "foo", -1, 0, EXTRA_FIELDS_ALL, row);

	for (i = 0; i < 5; i++)
		free(row[i]);
}

void test_fail_static_mk_sensor_row(void **state)
{
	ami_device *dev = (ami_device*)1;
	char *row[] = { "a", "b", "c" };

	/* Failure path - invalid `dev` argument */
	assert_int_equal(
		mk_sensor_row(NULL, "foo", 0, 0, 0, row),
		EXIT_FAILURE
	);

	/* Failure path - invalid `sensor` argument */
	assert_int_equal(
		mk_sensor_row(dev, NULL, 0, 0, 0, row),
		EXIT_FAILURE
	);

	/* Failure path - invalid `row` argument */
	assert_int_equal(
		mk_sensor_row(dev, "foo", 0, 0, 0, NULL),
		EXIT_FAILURE
	);
}

void test_fail_static_construct_sensor_table(void **state)
{
	int i = 0;
	ami_device *dev = (ami_device*)1;
	char *row[5] = { 0 };
	char ***values = NULL;
	int j = 0;

	/* One less - last element is NULL */
	for (i = 0; i < 4; i++) {
		row[i] = calloc(10, sizeof(char));
		assert_non_null(row[i]);
	}

	values = calloc(5, sizeof(values));
	assert_non_null(values);
	values[0] = row;

	/* Failure path - invalid `j` argument */
	assert_int_equal(
		construct_sensor_table(
			dev,
			values,
			"foo",
			EXTRA_FIELDS_ALL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - `rows[*j]` invalid */
	j = 4;
	assert_int_equal(
		construct_sensor_table(
			dev,
			values,
			"foo",
			EXTRA_FIELDS_ALL,
			&j
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `dev` argument */
	j = 0;
	assert_int_equal(
		construct_sensor_table(
			NULL,
			values,
			"foo",
			EXTRA_FIELDS_ALL,
			&j
		),
		EXIT_FAILURE
	);

	/* Failure path - ami_sensor_get_type fails */
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_TEMP);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_ERROR);
	assert_int_equal(
		construct_sensor_table(
			dev,
			values,
			"foo",
			EXTRA_FIELDS_ALL,
			&j
		),
		EXIT_FAILURE
	);

	for (i = 0; i < 5; i++)
		free(row[i]);
	
	free(values);
}

void test_fail_static_mk_sensor_node(void **state)
{
	ami_device *dev = (ami_device*)1;
	JsonNode node = { 0 };

	/* Failure path - invalid `dev` argument */
	assert_int_equal(
		mk_sensor_node(
			NULL,
			"foo",
			AMI_SENSOR_TYPE_TEMP,
			EXTRA_FIELDS_ALL,
			&node
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `sensor` argument */
	assert_int_equal(
		mk_sensor_node(
			dev,
			NULL,
			AMI_SENSOR_TYPE_TEMP,
			EXTRA_FIELDS_ALL,
			&node
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `parent` argument */
	assert_int_equal(
		mk_sensor_node(
			dev,
			"foo",
			AMI_SENSOR_TYPE_TEMP,
			EXTRA_FIELDS_ALL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid sensor type */
	assert_int_equal(
		mk_sensor_node(
			dev,
			"foo",
			-1,
			EXTRA_FIELDS_ALL,
			&node
		),
		EXIT_FAILURE
	);
}

void test_fail_static_construct_sensor_json(void **state)
{
	ami_device *dev = (ami_device*)1;
	int j = 0;
	JsonNode node = { 0 };

	/* Failure path - invalid `j` argument */
	assert_int_equal(
		construct_sensor_json(
			dev,
			&node,
			"foo",
			EXTRA_FIELDS_ALL,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `parent` argument */
	assert_int_equal(
		construct_sensor_json(
			dev,
			NULL,
			"foo",
			EXTRA_FIELDS_ALL,
			&j
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `dev` argument */
	assert_int_equal(
		construct_sensor_json(
			NULL,
			&node,
			"foo",
			EXTRA_FIELDS_ALL,
			&j
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `sensor` argument */
	assert_int_equal(
		construct_sensor_json(
			dev,
			&node,
			NULL,
			EXTRA_FIELDS_ALL,
			&j
		),
		EXIT_FAILURE
	);

	/* Failure path - ami_sensor_get_type fails */
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_TEMP);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_ERROR);
	assert_int_equal(
		construct_sensor_json(
			dev,
			&node,
			"foo",
			EXTRA_FIELDS_ALL,
			&j
		),
		EXIT_FAILURE
	);
}

void test_happy_static_populate_sensor_values(void **state)
{
	int i = 0;
	ami_device *dev = (ami_device*)1;
	struct app_sensor_data data = { EXTRA_FIELDS_ALL, "foo" };
	struct app_sensor_data data_no_name = { EXTRA_FIELDS_ALL, NULL };
	/* Table data */
	char *row[5] = { 0 };
	char ***values = NULL;
	/* JSON data */
	JsonNode node = { 0 };

	for (i = 0; i < 5; i++) {
		row[i] = calloc(10, sizeof(char));
		assert_non_null(row[i]);
	}

	values = calloc(1, sizeof(values));
	assert_non_null(values);
	values[0] = row;
	
	/* Happy path - table format, all fields */
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_TEMP);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_OK);
	populate_sensor_values(
		dev,
		values,
		1,
		5,
		APP_OUT_FORMAT_TABLE,
		(void*)&data
	);

	/* Happy path - JSON format, all fields */
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_TEMP);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_OK);
	populate_sensor_values(
		dev,
		values,
		1,
		5,
		APP_OUT_FORMAT_JSON,
		(void*)&data
	);

	/* Happy path - JSON format, all fields (current) */
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_CURRENT);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_OK);
	populate_sensor_values(
		dev,
		values,
		1,
		5,
		APP_OUT_FORMAT_JSON,
		(void*)&data
	);

	/* Happy path - JSON format, all fields (voltage) */
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_VOLTAGE);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_OK);
	populate_sensor_values(
		dev,
		values,
		1,
		5,
		APP_OUT_FORMAT_JSON,
		(void*)&data
	);

	/* Happy path - JSON format, all fields (power) */
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_POWER);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_OK);
	populate_sensor_values(
		dev,
		values,
		1,
		5,
		APP_OUT_FORMAT_JSON,
		(void*)&data
	);

	/* Happy path - table format, sensor not specified */
	will_return(__wrap_ami_sensor_get_sensors, AMI_STATUS_OK);
	will_return(__wrap_ami_sensor_get_type, AMI_SENSOR_TYPE_TEMP);
	will_return(__wrap_ami_sensor_get_type, AMI_STATUS_OK);
	populate_sensor_values(
		dev,
		values,
		1,
		5,
		APP_OUT_FORMAT_TABLE,
		(void*)&data_no_name
	);

	for (i = 0; i < 5; i++)
		free(row[i]);
	
	free(values);
}

void test_fail_static_populate_sensor_values(void **state)
{
	int i = 0;
	ami_device *dev = (ami_device*)1;
	char *placeholder = "foo";
	struct app_sensor_data data = { EXTRA_FIELDS_ALL, "foo" };
	struct app_sensor_data data_no_name = { EXTRA_FIELDS_ALL, NULL };

	/* Failure path - invalid `values` argument */
	assert_int_equal(
		populate_sensor_values(
			dev,
			NULL,
			0,
			0,
			APP_OUT_FORMAT_TABLE,
			&data
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `data` argument */
	assert_int_equal(
		populate_sensor_values(
			dev,
			(void*)placeholder,
			0,
			0,
			APP_OUT_FORMAT_TABLE,
			NULL
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid `dev` argument */
	assert_int_equal(
		populate_sensor_values(
			NULL,
			(void*)placeholder,
			0,
			0,
			APP_OUT_FORMAT_TABLE,
			&data
		),
		EXIT_FAILURE
	);

	/* Failure path - calloc fails */
	WRAPPER_ACTION(FAIL, calloc);
	assert_int_equal(
		populate_sensor_values(
			dev,
			(void*)placeholder,
			0,
			0,
			APP_OUT_FORMAT_TABLE,
			&data
		),
		EXIT_FAILURE
	);

	/* Failure path - ami_sensor_get_sensors fails */
	will_return(__wrap_ami_sensor_get_sensors, AMI_STATUS_ERROR);
	assert_int_equal(
		populate_sensor_values(
			dev,
			(void*)placeholder,
			0,
			0,
			APP_OUT_FORMAT_TABLE,
			&data_no_name
		),
		EXIT_FAILURE
	);

	/* Failure path - invalid output format */
	will_return(__wrap_ami_sensor_get_sensors, AMI_STATUS_OK);
	assert_int_equal(
		populate_sensor_values(
			dev,
			(void*)placeholder,
			1,
			1,
			-1,
			&data_no_name
		),
		EXIT_FAILURE
	);
}


/*****************************************************************************/

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_happy_report_sensors),
		cmocka_unit_test(test_fail_report_sensors),
		cmocka_unit_test(test_fail_print_sensor_data),
		cmocka_unit_test(test_fail_static_print_sensor_data),
		cmocka_unit_test(test_fail_static_parse_extra),
		cmocka_unit_test(test_happy_static_make_unit_string),
		cmocka_unit_test(test_fail_static_make_unit_string),
		cmocka_unit_test(test_happy_static_populate_sensor_header),
		cmocka_unit_test(test_fail_static_populate_sensor_header),
		cmocka_unit_test(test_happy_static_get_all_sensor_values),
		cmocka_unit_test(test_fail_static_get_all_sensor_values),
		cmocka_unit_test(test_happy_static_mk_sensor_row),
		cmocka_unit_test(test_fail_static_mk_sensor_row),
		cmocka_unit_test(test_fail_static_construct_sensor_table),
		cmocka_unit_test(test_happy_static_populate_sensor_values),
		cmocka_unit_test(test_fail_static_populate_sensor_values),
		cmocka_unit_test(test_fail_static_mk_sensor_node),
		cmocka_unit_test(test_fail_static_construct_sensor_json),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
