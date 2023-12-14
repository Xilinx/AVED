// SPDX-License-Identifier: GPL-2.0-only
/*
 * sensors.c - This file contains sensor utilities for the AMI CLI
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard Includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* API includes */
#include "ami.h"
#include "ami_sensor.h"
#include "ami_device.h"

/* App includes */
#include "json.h"
#include "printer.h"
#include "sensors.h"
#include "apputils.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* Number of sensor fields that always get reported (name, value, status) */
#define MAX_SENSOR_FIELDS	(6)
#define DEFAULT_SENSOR_FIELDS	(3)
#define TABLE_NAME		(0)
#define TABLE_VALUE		(1)
#define TABLE_STATUS		(2)

#define UNIT_MOD_BASE		(10)
#define UNIT_STR_SIZE		(2 + 1)
#define LIMIT_STR_SIZE		(7 + 1)  /* xxx.xxx + NULL */

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * struct sensor_values - Struct to hold all available sensor data.
 * @value: Sensor value - always present.
 * @status: Sensor status - always present.
 * @mod: Sensor unit modifier.
 * @max: Sensor max value.
 * @max_r: Return status of `ami_sensor_get_xxx_max` call.
 * @avg: Sensor average value.
 * @avg_r: Return status of `ami_sensor_get_xxx_average` call.
 * @limit_w: Warning limit.
 * @limit_w_r: Return value of relevant `ami_sensor_get_xxx_limit` call.
 * @limit_c: Critical limit.
 * @limit_c_r: Return value of relevant `ami_sensor_get_xxx_limit` call.
 * @limit_f: Fatal limit.
 * @limit_f_r: Return value of relevant `ami_sensor_get_xxx_limit` call.
 */
struct sensor_values {
	double value;
	enum ami_sensor_status status;
	enum ami_sensor_unit_mod mod;
	double max;
	int    max_r;
	double avg;
	int    avg_r;
	double limit_w;
	int    limit_w_r;
	double limit_c;
	int    limit_c_r;
	double limit_f;
	int    limit_f_r;
};

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/**
 * parse_extra() - Parse the -x | --extra command line argument.
 * @options: List of command line options given by the user.
 * @extra: Pointer to bitflag which will be updated.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int parse_extra(struct app_option *options, int *extra)
{
	int ret = EXIT_SUCCESS;
	char *str = NULL;
	struct app_option *opt = options;

	if (!options || !extra)
		return EXIT_FAILURE;
	
	while (opt) {
		if (opt->val == 'x') {
			str = (char*)malloc(strlen(opt->arg));

			if (!str) {
				ret = EXIT_FAILURE;
				break;
			}

			strcpy(str, opt->arg);

			/* Split on comma */
			char *tok = strtok(str, ",");
			while (tok) {
				if (strcmp(tok, "max") == 0)
					*extra |= EXTRA_FIELDS_MAX;
				else if (strcmp(tok, "average") == 0)
					*extra |= EXTRA_FIELDS_AVG;
				else if (strcmp(tok, "limits") == 0)
					*extra |= EXTRA_FIELDS_LIMITS;
				
				tok = strtok(NULL, ",");
			}

			free(str);
		}

		opt = opt->next;
	}

	return ret;
}

/**
 * make_unit_string() - Construct a human readable sensor unit string.
 * @mod: Unit modifier.
 * @type: Sensor type (relevant bits must be extracted).
 * @unit: Variable to hold output string.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int make_unit_string(enum ami_sensor_unit_mod mod, int type, char *unit)
{
	int col = 0;
	int ret = EXIT_SUCCESS;

	if (!unit)
		return EXIT_FAILURE;
	
	switch (mod) {
	case AMI_SENSOR_UNIT_MOD_MEGA:
		unit[col++] = 'M';
		break;
	
	case AMI_SENSOR_UNIT_MOD_KILO:
		unit[col++] = 'k';
		break;
	
	case AMI_SENSOR_UNIT_MOD_MILLI:
		unit[col++] = 'm';
		break;
	
	case AMI_SENSOR_UNIT_MOD_MICRO:
		unit[col++] = 'u';
		break;

	case AMI_SENSOR_UNIT_MOD_NONE:
		break;
	
	default:
		ret = EXIT_FAILURE;
		break;
	}

	if (ret == EXIT_FAILURE)
		return ret;

	switch (type) {
	case AMI_SENSOR_TYPE_TEMP:
		unit[col++] = 'C';
		break;
	
	case AMI_SENSOR_TYPE_CURRENT:
		unit[col++] = 'A';
		break;
	
	case AMI_SENSOR_TYPE_VOLTAGE:
		unit[col++] = 'V';
		break;
	
	case AMI_SENSOR_TYPE_POWER:
		unit[col++] = 'W';
		break;
	
	default:
		ret = EXIT_FAILURE;
		break;
	}

	return ret;
}

/**
 * populate_sensor_header() - Fill in the fields of a sensor table header.
 * @dev: AMI device handle.
 * @header: Pointer to pre-allocated header.
 * @num_fields: Number of elements in header.
 * @data: Pointer to `struct app_sensor_data`.
 * 
 * Note that name, value and status are always filled in. This function
 * is only used for table formatting. JSON formatting uses different logic
 * and has no concept of a header.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_sensor_header(ami_device *dev, char **header,
	int num_fields, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;
	struct app_sensor_data *sensor_data = NULL;
	int extra = 0;

	if (!header || !data || !dev)
		return EXIT_FAILURE;
	
	sensor_data = (struct app_sensor_data*)data;
	extra = sensor_data->extra_fields;

	for (i = 0; i < num_fields; i++) {
		if (!header[i]) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (i) {
		case TABLE_NAME:
			sprintf(header[i], "%s", "Name");
			break;
		
		case TABLE_VALUE:
			sprintf(header[i], "%s", "Value");
			break;
		
		case TABLE_STATUS:
			sprintf(header[i], "%s", "Status");
			break;
		
		default:
		{
			if (extra & EXTRA_FIELDS_MAX) {
				sprintf(header[i], "%s", "Max");
				extra &= ~EXTRA_FIELDS_MAX;
			} else if (extra & EXTRA_FIELDS_AVG) {
				sprintf(header[i], "%s", "Average");
				extra &= ~EXTRA_FIELDS_AVG;
			} else if (extra & EXTRA_FIELDS_LIMITS) {
				sprintf(header[i], "%s", "Limits (Warn, Crit, Fatal)");
				extra &= ~EXTRA_FIELDS_LIMITS;
			}

			break;
		}
		}
	}

	return ret;
}

/**
 * get_all_sensor_values() - Utility function to retrieve all sensor data.
 * @dev: Device handle.
 * @sensor: Sensor name.
 * @sensor_type: Sensor type (relevant bit MUST be extracted from bitflag).
 * @extra_fields: Extra fields bitflag.
 * @values: Struct to hold all relevant sensor data.
 * @convert_units: Boolean indicating whether sensor values should be converted
 *   based on the sensor unit modifier.
 * 
 * Note, if `convert_units` is NULL, all values will be converted to their
 * non-modified version (as if the mod was AMI_SENSOR_UNIT_MOD_NONE).
 * 
 * Return: None.
 */
static void get_all_sensor_values(ami_device *dev, const char *sensor,
	int sensor_type, int extra_fields, struct sensor_values *values, bool convert_units)
{
	long v = 0, a = 0, m = 0;
	long lw = 0, lc = 0, lf = 0;
	enum ami_sensor_unit_mod modifier = AMI_SENSOR_UNIT_MOD_NONE;

	if (!values) {
		APP_WARN("invalid state when fetching sensor values");
		return;
	}

	switch (sensor_type) {
	case AMI_SENSOR_TYPE_TEMP:
		ami_sensor_get_temp_value(dev, sensor, &v, &values->status);
		ami_sensor_get_temp_unit_mod(dev, sensor, &modifier);

		if (extra_fields & EXTRA_FIELDS_MAX)
			values->max_r = ami_sensor_get_temp_uptime_max(dev, sensor, &m);
		
		if (extra_fields & EXTRA_FIELDS_AVG)
			values->avg_r = ami_sensor_get_temp_uptime_average(dev, sensor, &a);

		if (extra_fields & EXTRA_FIELDS_LIMITS) {
			values->limit_w_r = ami_sensor_get_temp_limit(dev, sensor, AMI_SENSOR_LIMIT_WARN, &lw);
			values->limit_c_r = ami_sensor_get_temp_limit(dev, sensor, AMI_SENSOR_LIMIT_CRIT, &lc);
			values->limit_f_r = ami_sensor_get_temp_limit(dev, sensor, AMI_SENSOR_LIMIT_FATAL, &lf);
		}

		break;

	case AMI_SENSOR_TYPE_POWER:
		ami_sensor_get_power_value(dev, sensor, &v, &values->status);
		ami_sensor_get_power_unit_mod(dev, sensor, &modifier);

		if (extra_fields & EXTRA_FIELDS_MAX)
			values->max_r = ami_sensor_get_power_uptime_max(dev, sensor, &m);
		
		if (extra_fields & EXTRA_FIELDS_AVG)
			values->avg_r = ami_sensor_get_power_uptime_average(dev, sensor, &a);

		if (extra_fields & EXTRA_FIELDS_LIMITS) {
			values->limit_w_r = ami_sensor_get_power_limit(dev, sensor, AMI_SENSOR_LIMIT_WARN, &lw);
			values->limit_c_r = ami_sensor_get_power_limit(dev, sensor, AMI_SENSOR_LIMIT_CRIT, &lc);
			values->limit_f_r = ami_sensor_get_power_limit(dev, sensor, AMI_SENSOR_LIMIT_FATAL, &lf);
		}

		break;

	case AMI_SENSOR_TYPE_CURRENT:
		ami_sensor_get_current_value(dev, sensor, &v, &values->status);
		ami_sensor_get_current_unit_mod(dev, sensor, &modifier);

		if (extra_fields & EXTRA_FIELDS_MAX)
			values->max_r = ami_sensor_get_current_uptime_max(dev, sensor, &m);
		
		if (extra_fields & EXTRA_FIELDS_AVG)
			values->avg_r = ami_sensor_get_current_uptime_average(dev, sensor, &a);
			
		if (extra_fields & EXTRA_FIELDS_LIMITS) {
			values->limit_w_r = ami_sensor_get_current_limit(dev, sensor, AMI_SENSOR_LIMIT_WARN, &lw);
			values->limit_c_r = ami_sensor_get_current_limit(dev, sensor, AMI_SENSOR_LIMIT_CRIT, &lc);
			values->limit_f_r = ami_sensor_get_current_limit(dev, sensor, AMI_SENSOR_LIMIT_FATAL, &lf);
		}

		break;

	case AMI_SENSOR_TYPE_VOLTAGE:
		ami_sensor_get_voltage_value(dev, sensor, &v, &values->status);
		ami_sensor_get_voltage_unit_mod(dev, sensor, &modifier);

		if (extra_fields & EXTRA_FIELDS_MAX)
			values->max_r = ami_sensor_get_voltage_uptime_max(dev, sensor, &m);
		
		if (extra_fields & EXTRA_FIELDS_AVG)
			values->avg_r = ami_sensor_get_voltage_uptime_average(dev, sensor, &a);
		
		if (extra_fields & EXTRA_FIELDS_LIMITS) {
			values->limit_w_r = ami_sensor_get_voltage_limit(dev, sensor, AMI_SENSOR_LIMIT_WARN, &lw);
			values->limit_c_r = ami_sensor_get_voltage_limit(dev, sensor, AMI_SENSOR_LIMIT_CRIT, &lc);
			values->limit_f_r = ami_sensor_get_voltage_limit(dev, sensor, AMI_SENSOR_LIMIT_FATAL, &lf);
		}

		break;
	
	default:
		break;
	}

	if (convert_units) {
		/* Convert units */
		values->value = (double)v * pow(UNIT_MOD_BASE, (double)modifier);
		
		if ((extra_fields & EXTRA_FIELDS_MAX) && ((values->max_r) == AMI_STATUS_OK)) 
			values->max = (double)m * pow(UNIT_MOD_BASE, (double)modifier);
		
		if ((extra_fields & EXTRA_FIELDS_AVG) && ((values->max_r) == AMI_STATUS_OK))
			values->avg = (double)a * pow(UNIT_MOD_BASE, (double)modifier);

		if (extra_fields & EXTRA_FIELDS_LIMITS) {
			if (values->limit_w_r == AMI_STATUS_OK)
				values->limit_w = (double)lw * pow(UNIT_MOD_BASE, (double)modifier);

			if (values->limit_c_r == AMI_STATUS_OK)
				values->limit_c = (double)lc * pow(UNIT_MOD_BASE, (double)modifier);

			if (values->limit_f_r == AMI_STATUS_OK)
				values->limit_f = (double)lf * pow(UNIT_MOD_BASE, (double)modifier);
		}

		values->mod = AMI_SENSOR_UNIT_MOD_NONE;
	} else {
		/* Store original values */
		values->mod = modifier;
		values->value = (double)v;
		values->max = (double)m;
		values->avg = (double)a;
		values->limit_w = (double)lw;
		values->limit_c = (double)lc;
		values->limit_f = (double)lf;
	}
}

/**
 * mk_sensor_row() - Construct a single table row for a sensor.
 * @dev: Device handle.
 * @sensor: Sensor name.
 * @sensor_type: Sensor type (relevant bits MUST be extracted).
 * @n_row: Current row number (for this sensor only).
 * @extra_fields: Extra fields bitflag.
 * @row: Parent row.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int mk_sensor_row(ami_device *dev, const char *sensor,
	int sensor_type, int n_row, int extra_fields, char **row)
{
	int col = 0;

	/* Sensor values */
	struct sensor_values values = { 0 };
	char unit[UNIT_STR_SIZE] = { 0 };

	if (!dev || !sensor || !row)
		return EXIT_FAILURE;
	
	get_all_sensor_values(
		dev, sensor, sensor_type, extra_fields, &values, true
	);

	if (make_unit_string(AMI_SENSOR_UNIT_MOD_NONE, sensor_type, unit) == EXIT_FAILURE)
		memset(unit, 0x00, UNIT_STR_SIZE);
	
	/* Print name - always valid. */
	if (n_row == 0)
		sprintf(
			row[col++],
			"%s",
			sensor
		);
	else
		col++;

	/* Print value - always valid. */
	sprintf(
		row[col++],
		"%.3f %s",
		values.value,
		unit
	);

	/* Print status - always valid. */
	sprintf(
		row[col++],
		"%s",
		(((values.status == AMI_SENSOR_STATUS_OK) || (values.status == AMI_SENSOR_STATUS_OK_CACHED)) ?
			((values.status == AMI_SENSOR_STATUS_OK_CACHED) ? ("valid*") : ("valid")) :
			("invalid"))
	);

	/* Extra attributes. */
	if (extra_fields & EXTRA_FIELDS_MAX) {
		if (values.max_r == AMI_STATUS_OK)
			sprintf(
				row[col++],
				"%.3f %s",
				values.max,
				unit
			);
		else
			sprintf(
				row[col++],
				"%s",
				"N/A"
			);
	}

	if (extra_fields & EXTRA_FIELDS_AVG) {
		if (values.avg_r == AMI_STATUS_OK)
			sprintf(
				row[col++],
				"%.3f %s",
				values.avg,
				unit
			);
		else
			sprintf(
				row[col++],
				"%s",
				"N/A"
			);
	}

	if (extra_fields & EXTRA_FIELDS_LIMITS) {
		/* If all limits are unavailable, print a single "N/A" */
		if ((values.limit_w_r != AMI_STATUS_OK) && (values.limit_c_r != AMI_STATUS_OK) &&
					(values.limit_f_r != AMI_STATUS_OK)) {
			sprintf(
				row[col++],
				"%s",
				"N/A"
			);
		} else {
			char limit_w_str[LIMIT_STR_SIZE] = { 0 };
			char limit_c_str[LIMIT_STR_SIZE] = { 0 };
			char limit_f_str[LIMIT_STR_SIZE] = { 0 };

			/* Warning limit */
			if (values.limit_w_r == AMI_STATUS_OK)
				sprintf(
					limit_w_str,
					"%.3f",
					values.limit_w
				);
			else
				sprintf(
					limit_w_str,
					"%s",
					"N/A"
				);
			
			/* Critical limit */
			if (values.limit_c_r == AMI_STATUS_OK)
				sprintf(
					limit_c_str,
					"%.3f",
					values.limit_c
				);
			else
				sprintf(
					limit_c_str,
					"%s",
					"N/A"
				);

			/* Fatal limit */
			if (values.limit_f_r == AMI_STATUS_OK)
				sprintf(
					limit_f_str,
					"%.3f",
					values.limit_f
				);
			else
				sprintf(
					limit_f_str,
					"%s",
					"N/A"
				);
			
			sprintf(
				row[col++],
				"%*s, %*s, %*s",
				LIMIT_STR_SIZE - 1,
				limit_w_str,
				LIMIT_STR_SIZE - 1,
				limit_c_str,
				LIMIT_STR_SIZE - 1,
				limit_f_str
			);
		}
	}

	return EXIT_SUCCESS;
}

/**
 * construct_sensor_table() - Callback for the `populate_sensor_values` function.
 * @dev: AMI device handle.
 * @rows: Pre-allocated pointer to table rows.
 * @sensor: Populate data for this sensor.
 * @extra_fields: Extra fields bitflag.
 * @j: Current row. This should be incremented by the function for each row.
 *
 * This function populates a variable number of rows in a table according
 * to a predefined format (it must be in the same order as the header),
 * and for each row filled in, it increments `j`.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int construct_sensor_table(ami_device *dev, char ***rows,
	const char *sensor, int extra_fields, int *j)
{
	int i = 0;
	int ret = EXIT_SUCCESS;
	int group_row = 0;
	uint32_t sensor_type = 0;

	if (!j || !rows[*j] || !dev)
		return EXIT_FAILURE;

	if (ami_sensor_get_type(dev, sensor, &sensor_type) != AMI_STATUS_OK)
		return EXIT_FAILURE;
	
	for (i = 0; i < AMI_SENSOR_TYPE_MAX; i++) {
		if ((1U << i) & sensor_type) {
			if (mk_sensor_row(dev, sensor, (1U << i), group_row,
					extra_fields, rows[*j]) == EXIT_FAILURE) {
				ret = EXIT_FAILURE;
				break;
			}

			(*j)++;
			group_row++;
		}
	}

	return ret;
}

/**
 * mk_sensor_node() - Construct a single Json node for a sensor.
 * @dev: Device handle.
 * @sensor: Sensor name.
 * @sensor_type: Sensor type (relevant bits MUST be extracted).
 * @extra_fields: Extra fields bitflag.
 * @parent: Parent node.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int mk_sensor_node(ami_device *dev, const char *sensor,
	int sensor_type, int extra_fields, JsonNode *parent)
{
	int ret = EXIT_SUCCESS;
	struct sensor_values values = { 0 };
	
	JsonNode *row = NULL;  /* sensor object belonging to the group */
	/* Object attributes */
	JsonNode *unit_node = NULL;
	JsonNode *value_node = NULL;
	JsonNode *status_node = NULL;
	JsonNode *max_node = NULL;
	JsonNode *avg_node = NULL;
	JsonNode *limits_node = NULL;

	if (!dev || !sensor || !parent)
		return EXIT_FAILURE;
	
	get_all_sensor_values(
		dev, sensor, sensor_type, extra_fields, &values, false
	);
	
	row = json_mkobject();

	/* All objects have value, status, and unit. */
	value_node = json_mknumber(values.value);
	unit_node = json_mknumber(values.mod);
	status_node = json_mknumber(values.status);

	json_append_member(row, "unit_mod", unit_node);
	json_append_member(row, "value", value_node);
	json_append_member(row, "status", status_node);
	
	/* Extra attributes. */
	if (extra_fields & EXTRA_FIELDS_MAX) {
		if (values.max_r == AMI_STATUS_OK)
			max_node = json_mknumber(values.max);
		else
			max_node = json_mknull();
		
		json_append_member(row, "max", max_node);
	}
	
	if (extra_fields & EXTRA_FIELDS_AVG) {
		if (values.avg_r == AMI_STATUS_OK)
			avg_node = json_mknumber(values.avg);
		else
			avg_node = json_mknull();
		
		json_append_member(row, "average", avg_node);
	}

	if (extra_fields & EXTRA_FIELDS_LIMITS) {
		JsonNode *limit_w = NULL;
		JsonNode *limit_c = NULL;
		JsonNode *limit_f = NULL;

		/* Warning */
		if (values.limit_w_r == AMI_STATUS_OK)
			limit_w = json_mknumber(values.limit_w);
		else
			limit_w = json_mknull();

		/* Critical */
		if (values.limit_c_r == AMI_STATUS_OK)
			limit_c = json_mknumber(values.limit_c);
		else
			limit_c = json_mknull();

		/* Fatal */
		if (values.limit_f_r == AMI_STATUS_OK)
			limit_f = json_mknumber(values.limit_f);
		else
			limit_f = json_mknull();

		limits_node = json_mkobject();
		json_append_member(limits_node, "warning", limit_w);
		json_append_member(limits_node, "critical", limit_c);
		json_append_member(limits_node, "fatal", limit_f);
		json_append_member(row, "limits", limits_node);
	}

	switch (sensor_type) {
	case AMI_SENSOR_TYPE_TEMP:
		json_append_member(parent, "temp", row);
		break;
	
	case AMI_SENSOR_TYPE_CURRENT:
		json_append_member(parent, "current", row);
		break;
	
	case AMI_SENSOR_TYPE_VOLTAGE:
		json_append_member(parent, "voltage", row);
		break;
	
	case AMI_SENSOR_TYPE_POWER:
		json_append_member(parent, "power", row);
		break;
	
	default:
		ret = EXIT_FAILURE;
		break;
	}

	return ret;
}

/**
 * construct_sensor_json() - Callback for the `populate_sensor_values` function.
 * @dev: Device handle.
 * @parent: Pre-allocated pointer to topmost JSON object.
 * @sensor: Populate data for this sensor.
 * @extra_fields: Extra fields bitflag.
 * @j: Current row (a row is a single sensor object like `"voltage": {...}`)
 *
 * This function creates a variable number of JSON nodes and appends them to
 * the parent in a predefined format, and for each object, it increments `j`.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int construct_sensor_json(ami_device *dev, JsonNode *parent,
	const char *sensor, int extra_fields, int *j)
{
	int i = 0;
	int ret = EXIT_SUCCESS;
	JsonNode *current_group = NULL;
	uint32_t sensor_type = 0;

	if (!j || !parent || !dev || !sensor)
		return EXIT_FAILURE;
	
	if (ami_sensor_get_type(dev, sensor, &sensor_type) != AMI_STATUS_OK)
		return EXIT_FAILURE;
	
	current_group = json_mkobject();

	for (i = 0; i < AMI_SENSOR_TYPE_MAX; i++) {
		if ((1U << i) & sensor_type) {
			if (mk_sensor_node(dev, sensor, (1U << i),
					extra_fields, current_group) == EXIT_FAILURE) {
				ret = EXIT_FAILURE;
				break;
			}
			
			(*j)++;
		}
	}

	json_append_member(parent, sensor, current_group);
	return ret;
}

/**
 * populate_sensor_values() - Populate an arbitrary data structure with sensor
 *                            data for printing.
 * @dev: Device handle.
 * @values: Pointer to data structure
 * @n_rows: Pointer to number of rows (records) in data structure.
 * @n_fields: Pointer to number of elements in each row.
 * @fmt: Format of data structure. Used to determine type of `values`.
 * @data: Pointer to `struct app_sensor_data`.
 * 
 * Note that this function is used for any generic data structure
 * (JSON and tables, in this case).
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_sensor_values(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data)
{
	int ret = EXIT_SUCCESS;
	int j = 0;
	int n_groups = 0;
	struct ami_sensor *sensors = NULL;
	struct ami_sensor *current_sensor = NULL;
	struct app_sensor_data *sensor_data = NULL;

	if (!values || !n_rows || !n_fields || !data || !dev)
		return EXIT_FAILURE;
	
	sensor_data = (struct app_sensor_data*)data;
	
	if (sensor_data->sensor) {
		sensors = (struct ami_sensor*)calloc(1, sizeof(struct ami_sensor));

		if (!sensors)
			return EXIT_FAILURE;

		strcpy(sensors->name, sensor_data->sensor);
		sensors->next = NULL;
		n_groups = 1;
	} else {
		if (ami_sensor_get_sensors(dev, &sensors, &n_groups) != AMI_STATUS_OK)
			return EXIT_FAILURE;
	}

	current_sensor = sensors;

	while (current_sensor && (j < *n_rows)) {
		switch (fmt) {
		case APP_OUT_FORMAT_JSON:
			ret = construct_sensor_json(
				dev,
				(JsonNode*)values,
				current_sensor->name,
				sensor_data->extra_fields,
				&j
			);
			break;

		case APP_OUT_FORMAT_TABLE:
			ret = construct_sensor_table(
				dev,
				(char***)values,
				current_sensor->name,
				sensor_data->extra_fields,
				&j
			);
			break;

		default:
			ret = EXIT_FAILURE;
			break;
		}

		if (ret == EXIT_FAILURE)
			break;

		current_sensor = current_sensor->next;
	}

	if (sensor_data->sensor)
		free(sensors);

	return ret;
}

/**
 * print_sensor_data() - Generic function to print sensor data in an
 *                       arbitrary format (JSON/table).
 * @dev: Device handle.
 * @extra_fields: Extra fields bitflag.
 * @sensor: Print out data for this sensor only (NULL for all sensors).
 * @stream: Optional output stream (defaults to stdout).
 * @fmt: Output format.
 * @json_out: Optional variable to store generated JSON data instead of printing.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int print_sensor_data(ami_device *dev, int extra_fields,
	const char *sensor, FILE *stream, enum app_out_format fmt,
	JsonNode **json_out)
{
	int i = 0;
	int ret = EXIT_FAILURE;
	int n_rows = 0;
	int n_fields = DEFAULT_SENSOR_FIELDS;
	struct app_sensor_data data = { 0 };
	int col_align[MAX_SENSOR_FIELDS] = { 0 };

	if (!dev)
		return EXIT_FAILURE;
	
	if (sensor) {
		/* Figure out number of data entires for this sensor. */
		uint32_t type = 0;

		if (ami_sensor_get_type(dev, sensor, &type) != AMI_STATUS_OK)
			return EXIT_FAILURE;
		
		for (i = 0; i < AMI_SENSOR_TYPE_MAX; i++) {
			if ((1U << i) & type) {
				n_rows++;
			}
		}
	} else {
		int num = 0;

		if (ami_sensor_get_num_total(dev, &num) != AMI_STATUS_OK)
			return EXIT_FAILURE;
		
		n_rows = num;
	}

	if (n_rows == 0)
		return EXIT_FAILURE;
	
	/* All values except name and status are right aligned */
	for (i = 0; i < MAX_SENSOR_FIELDS; i++) {
		switch (i) {
		case TABLE_NAME:
		case TABLE_STATUS:
			col_align[i] = TABLE_ALIGN_LEFT;
			break;
		
		default:
			col_align[i] = TABLE_ALIGN_RIGHT;
			break;
		}
	}
	
	if (extra_fields & EXTRA_FIELDS_MAX)
		n_fields++;
	
	if (extra_fields & EXTRA_FIELDS_AVG)
		n_fields++;

	if (extra_fields & EXTRA_FIELDS_LIMITS)
		n_fields++;
	
	data.extra_fields = extra_fields;
	data.sensor = sensor;
	
	/*
	 * Only the table is ever printed to stdout.
	 * The format is used only for the output file.
	 */
	ret = print_table_data(
		dev,
		n_fields,
		n_rows,
		(fmt == APP_OUT_FORMAT_TABLE) ? (stream) : (NULL),
		TABLE_DIVIDER_GROUPS,
		&populate_sensor_values,
		&populate_sensor_header,
		&data,
		col_align
	);
	
	/* Write to file. */
	if (stream && (ret != EXIT_FAILURE) && (fmt != APP_OUT_FORMAT_TABLE)) {
		switch (fmt) {
		case APP_OUT_FORMAT_JSON:
			if (!json_out)
				ret = print_json_data(
					dev,
					n_fields,
					n_rows,
					stream,
					&populate_sensor_values,
					&data
				);
			else
				ret = gen_json_data(
					dev,
					n_fields,
					n_rows,
					&populate_sensor_values,
					&data,
					json_out
				);
			
			break;

		default:
			break;
		}
	}

	return ret;
}

/*****************************************************************************/
/* Public function definitions                                               */
/*****************************************************************************/

/*
 * Primary callback for printing sensor information with the "sensors" command.
 */
int report_sensors(struct app_option *options)
{
	int ret = EXIT_FAILURE;
	const char *sensor_filter = NULL;  /* default: all sensors */

	bool verbose = false;
	int extra_fields = EXTRA_FIELDS_NONE;  /* default: no extra fields */
	enum app_out_format format = APP_OUT_FORMAT_TABLE;  /* default: table */
	FILE *stream = NULL;

	struct app_option *opt = NULL;
	bool output_given = false, fmt_given = false;

	/* options may be NULL */

	if (parse_output_options(options, &format, &verbose, &stream,
			&fmt_given, &output_given) == EXIT_FAILURE)
		return EXIT_FAILURE;

	/* Verbose option takes precedence over -x */
	if (verbose) {
		/* TODO: This will output more information in the future. */
		extra_fields |= EXTRA_FIELDS_ALL;
	} else {
		/* Check if user requested extra fields. */
		parse_extra(options, &extra_fields);
	}
	
	/* Check if user specified a sensor. */
	if (NULL != (opt = find_app_option('n', options))) {
		sensor_filter = opt->arg;
	}
	
	/* Check for -d | --device */
	if (NULL != (opt = find_app_option('d', options))) {
		ami_device *dev = NULL;

		/* Search for device. */
		if (ami_dev_find(opt->arg, &dev) == AMI_STATUS_OK) {
			/* Find device sensors. */
			if(AMI_STATUS_OK != ami_sensor_discover(dev)) {
				APP_API_ERROR("device has no sensor data");
			} else {
				/* Print sensor information. */
				ret = print_sensor_data(
					dev,
					extra_fields,
					sensor_filter,
					stream,
					format,
					NULL
				);
			}

			ami_dev_delete(&dev);
		} else {
			APP_API_ERROR("could not find the requested device");
		}
	} else {
		ami_device *dev = NULL;
		ami_device *prev = NULL;
		JsonNode *parent = NULL;

		if (fmt_given && output_given && (format == APP_OUT_FORMAT_JSON))
			parent = json_mkobject();

		APP_WARN("enumerating all devices");

		while (ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, 0, prev) == AMI_STATUS_OK) {
			uint16_t bdf = 0;
			char bdf_str[AMI_BDF_STR_LEN] = { 0 };
			JsonNode *child = NULL;

			if (ami_dev_get_pci_bdf(dev, &bdf) == AMI_STATUS_OK) {
				snprintf(
					bdf_str,
					AMI_BDF_STR_LEN,
					"%02x:%02x.%01x",
					AMI_PCI_BUS(bdf),
					AMI_PCI_DEV(bdf),
					AMI_PCI_FUNC(bdf)
				);

				printf(
					"\r\n%s:\r\n\r\n",
					bdf_str
				);
			} else {
				APP_WARN("could not retrieve device BDF");

				snprintf(
					bdf_str,
					AMI_BDF_STR_LEN,
					"%02x:%02x.%01x",
					0, 0, 0
				);
			}

			ret = ami_sensor_discover(dev);

			if (ret != AMI_STATUS_OK) {
				APP_API_ERROR("device has no sensor data");
				ami_dev_delete(&dev);
				break;
			}

			ret = print_sensor_data(
				dev,
				extra_fields,
				sensor_filter,
				stream,
				format,
				(parent == NULL) ? (NULL) : (&child)
			);

			if (ret != EXIT_SUCCESS) {
				APP_ERROR("could not print sensor data");
				ami_dev_delete(&dev);
				break;
			}

			if ((parent != NULL) && (child != NULL))
				json_append_member(parent, bdf_str, child);

			/* Move to next device. */
			ami_dev_delete(&prev);
			prev = dev;
			dev = NULL;
		}

		if ((ret == EXIT_SUCCESS) && (parent != NULL))
			print_json_obj(parent, stream);

		/* Delete final device. */
		ami_dev_delete(&prev);

		/* Delete JSON */
		if (parent != NULL)
			json_delete(parent);
	}

	if (stream)
		fclose(stream);

	return ret;
}
