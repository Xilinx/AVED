// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_sensor.c - This file contains the implementation of sensor related logic
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/* Private API includes */
#include "ami_internal.h"
#include "ami_ioctl.h"
#include "ami_sensor_internal.h"
#include "ami_device_internal.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define SENSOR_REFRESH_ATTR		"update_interval"
#define SENSOR_REFRESH_MAX_STR		(8)

/* For parsing hwmon sensor status */
#define SENSOR_STATUS_NAME_NOT_PRESENT	"Sensor Not Present"
#define SENSOR_STATUS_NAME_OK		"Sensor Present and Valid"
#define SENSOR_STATUS_NAME_NO_DATA	"Data Not Available"
#define SENSOR_STATUS_NAME_NA		"Not Applicable or Default Value"

/*****************************************************************************/
/* Local function declarations                                               */
/*****************************************************************************/

/**
 * parse_hwmon() - Parse a full hwmon path into sensor attributes.
 * @path: Full path to the hwmon node
 * @parsed_sid: Pointer to store the parsed sensor ID
 * @parsed_type: Pointer to store parsed sensor type
 * @parsed_attr: Pointer to store parsed sensor attribute
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int parse_hwmon(const char *path, int *parsed_sid,
	enum ami_sensor_type *parsed_type, enum ami_sensor_attr_type *parsed_attr);

/**
 * construct_hwmon() - Construct a hwmon path.
 * @path: Variable to hold path, must have AMI_HWMON_PATH_MAX_SIZE bytes.
 * @hwmon_num: Hwmon number as visible in the filesystem.
 * @attr: Human readable name of hwmon attribute as visible in the filesystem.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int construct_hwmon(char *path, int hwmon_num, const char *attr);

/**
 * open_hwmon() - Open a hwmon node for reading or writing (or both).
 * @hwmon: Full path to the hwmon attribute (optional).
 * @hwmon_num: Hwmon node number (optional).
 * @attr: Human readable name of hwmon attribute (optional).
 * @mode: File mode (O_RDONLY, O_WRONLY, O_RDWR).
 * 
 * Note that this function can be called from 2 different contexts:
 *    - If you know the full path of the attribute, set `hwmon` to your path,
 *      `attr` to NULL, and `hwmon_num` to any arbitrary value.
 *    - If you only know the attribute number and hwmon number,
 *      set `hwmon` to NULL and pass in the other arguments as normal.
 * 
 * Return: The file descriptor (-1 on error).
 */
static int open_hwmon(const char *hwmon, int hwmon_num, const char *attr, int mode);

/**
 * read_hwmon() - Read a hwmon attribute into a buffer.
 * @hwmon: Full path to the hwmon attribute (optional).
 * @hwmon_num: Hwmon node number (optional).
 * @attr: Human readable name of hwmon attribute (optional).
 * @buf: Output buffer, must hold at least AMI_HWMON_MAX_STR bytes.
 * 
 * The `hwmon`, `hwmon_num`, and `attr` arguments are passed to the `open_hwmon`
 * function. See the definition for an explanation of how these arguments
 * are used.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
static int read_hwmon(const char *hwmon, int hwmon_num, const char *attr, char *buf);

/**
 * read_sensor_attr() - Read a specific sensor attribute.
 * @attr: Pointer to attribute struct.
 * 
 * This function automatically detects the type of the hwmon attribute
 * and sets the correct variable within the `attr->val` member. For string
 * attribute it sets the `value_s` member which is a buffer of size
 * `AMI_HWMON_MAX_STR`. For numeric attributes it sets the member `value_l` which
 * is a signed long.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int read_sensor_attr(struct ami_sensor_attr *attr);

/**
 * get_single_sensor_val() - Get a single sensor value.
 * @dev: Device handle.
 * @sensor_type: Sensor type.
 * @sid: Sensor ID.
 * @attr: Pointer to sensor attribute to populate.
 * @status_attr: Pointer to status attribute to populate.
 * @fresh: Variable to hold cache status for this value.
 * 
 * This function is similar to `read_sensor_attr`, however, it makes an IOCTL
 * call instead of reading hwmon files. This eliminates the race condition if a
 * user was interested in reading a sensor value and the status. Currently,
 * this is only supported for the instant sensor value (not max/average).
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int get_single_sensor_val(ami_device *dev, enum ami_sensor_type sensor_type,
	int sid, struct ami_sensor_attr *attr, struct ami_sensor_attr *status_attr, bool *fresh);

/**
 * find_sensor_data() - Find a specific data struct for a given sensor.
 * @sensors: List of sensor data structs. 
 * @sid: Sensor ID.
 * @type: Sensor type.
 * @data: Output variable to store sensor data.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
static int find_sensor_data(struct ami_sensor_data *sensors, int sid,
	enum ami_sensor_type type, struct ami_sensor_data **data);

/**
 * find_sensor_by_name() - Find a top-level sensor struct.
 * @dev: Device handle.
 * @name: Sensor name.
 * @sensor: Pointer to output variable.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int find_sensor_by_name(ami_device *dev, const char *name,
	struct ami_sensor **sensor);

/**
 * populate_device_sensors() - Create top level sensor structs from sensor data.
 * @dev: Device handle.
 * @data: All sensor data collected.
 * 
 * This function should only be called once as pami of the discovery step.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
static int populate_device_sensors(ami_device *dev, struct ami_sensor_data *data);

/**
 * parse_sensor_status() - Convert a sensor status string to enum.
 * @status: Status string from hwmon.
 * 
 * Return: enum ami_sensor_status (AMI_SENSOR_STATUS_INVALID on fail).
 */
static enum ami_sensor_status parse_sensor_status(const char *status);

/**
 * limit_type_to_attr() - Convert a limit type to a sensor attribute.
 * @limit: Limit type.
 *
 * Return: enum ami_sensor_attr_type (AMI_SENSOR_ATTR_INVALID on fail)
 */
static enum ami_sensor_attr_type limit_type_to_attr(enum ami_sensor_limit limit);

/**
 * get_value() - Utility function to get the value of a sensor attribute.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @attr: Attribute type.
 * @type: Sensor type.
 * @val: Output variable to store value.
 * @status: Also fetch the sensor status (optional).
 * 
 * For numeric values, val must be a `long`.
 * For string values, val must be a `char`*.
 * Sensor status is a special case and requires an `enum ami_sensor_status*`.
 * 
 * If `status` is not a NULL pointer, we make an ioctl call to fetch the data
 * instead of reading the hwmon files. This eliminates any race condition
 * when a user is interested in BOTH the sensor value and the status.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int get_value(ami_device *dev, const char *sensor_name,
	enum ami_sensor_attr_type attr, enum ami_sensor_type type, void *val,
	enum ami_sensor_status *status);

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/*
 * Construct a hwmon path.
 */
static int construct_hwmon(char *path, int hwmon_num, const char *attr)
{
	if (!path || !attr)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	snprintf(
		path,
		AMI_HWMON_PATH_MAX_SIZE,
		AMI_HWMON_ATTR,
		hwmon_num,
		attr
	);
	
	return AMI_STATUS_OK;
}

/*
 * Open a hwmon node.
 */
static int open_hwmon(const char *hwmon, int hwmon_num, const char *attr, int mode)
{
	int file = AMI_INVALID_FD;
	char path[AMI_HWMON_PATH_MAX_SIZE] = { 0 };

	if (!hwmon && !attr)
		return file;
	
	if (attr)
		construct_hwmon(path, hwmon_num, attr);
	else
		strncpy(path, hwmon, AMI_HWMON_PATH_MAX_SIZE);

	return open(path, mode);
}

/*
 * Read from a hwmon node.
 */
static int read_hwmon(const char *hwmon, int hwmon_num, const char *attr, char *buf)
{
	int file = AMI_INVALID_FD;
	int ret = AMI_STATUS_ERROR;

	if (!buf || (!hwmon && !attr))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	file = open_hwmon(hwmon, hwmon_num, attr, O_RDONLY);

	if (file != AMI_INVALID_FD) {
		if (AMI_LINUX_STATUS_ERROR != read(file, buf, AMI_HWMON_MAX_STR))
			ret = AMI_STATUS_OK;
		else
			ret = AMI_API_ERROR(AMI_ERROR_EIO);
		
		close(file);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}

	return ret;
}

/*
 * Parse a hwmon path.
 */
static int parse_hwmon(const char *path, int *parsed_sid,
	enum ami_sensor_type *parsed_type, enum ami_sensor_attr_type *parsed_attr)
{
	int ret = AMI_STATUS_ERROR;
	struct stat path_stat = { 0 };

	/* ID */
	int sid = 0;
	/* Actual attribute */
	char attr[AMI_HWMON_ATTR_MAX_SIZE] = { 0 };
	/* Full attribute string. */
	char hwmon_attr[AMI_HWMON_ATTR_MAX_SIZE + AMI_HWMON_TYPE_MAX_SIZE] = { 0 };

	if (!path)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	/* Check if this path points to a file. */
	if(stat(path, &path_stat) == AMI_LINUX_STATUS_ERROR)
		return AMI_API_ERROR(AMI_ERROR_EBADF);

	if (!S_ISREG(path_stat.st_mode))
		return AMI_API_ERROR(AMI_ERROR_EBADF);

	if ((sscanf(path, AMI_HWMON_ATTR_FORMAT, hwmon_attr) == 1) &&
			(sscanf(hwmon_attr, "%*[^0123456789]%d", &sid) == 1) &&
			(sscanf(hwmon_attr, "%*[^_]_%s", attr) == 1))  {
		/* Populate fields. */
		*parsed_sid = sid;

		/*
		* Parse sensor type and attribute type.
		* Using the first character of the string for this
		* because it's cleaner (we can us a switch). This works
		* for now, but it may be better to just compare the
		* whole string with `strncmp`.
		*/
		switch (hwmon_attr[0]) {
		case 'i':  /* Voltage */
			*parsed_type = AMI_SENSOR_TYPE_VOLTAGE;
			break;
		
		case 'p':  /* Power */
			*parsed_type = AMI_SENSOR_TYPE_POWER;
			break;
		
		case 't':  /* Temperature */
			*parsed_type = AMI_SENSOR_TYPE_TEMP;
			break;
		
		case 'c':  /* Current */
			*parsed_type = AMI_SENSOR_TYPE_CURRENT;
			break;

		default:
			*parsed_type = AMI_SENSOR_TYPE_INVALID;
			break;
		}

		/* Parse attribute - min/lowest currently not supported. */
		if (strcmp(attr, "label") == 0) {
			*parsed_attr = AMI_SENSOR_ATTR_NAME;
		} else if (strcmp(attr, "input") == 0) {
			*parsed_attr = AMI_SENSOR_ATTR_VALUE;
		} else if ((strcmp(attr, "input_highest") == 0) || (strcmp(attr, "highest") == 0)) {
			*parsed_attr = AMI_SENSOR_ATTR_MAX;
		} else if (strcmp(attr, "average") == 0) {
			*parsed_attr = AMI_SENSOR_ATTR_AVG;
		} else if (strcmp(attr, "status") == 0) {
			*parsed_attr = AMI_SENSOR_ATTR_STATUS;
		} else if (strcmp(attr, "max") == 0) {
			*parsed_attr = AMI_SENSOR_ATTR_WARN_LIMIT;
		} else if (strcmp(attr, "lcrit") == 0) {
			*parsed_attr = AMI_SENSOR_ATTR_CRIT_LIMIT;
		} else if (strcmp(attr, "crit") == 0) {
			*parsed_attr = AMI_SENSOR_ATTR_FATAL_LIMIT;
		} else {
			*parsed_attr = AMI_SENSOR_ATTR_INVALID;
		}

		if ((*parsed_type != AMI_SENSOR_TYPE_INVALID) &&
				(*parsed_attr != AMI_SENSOR_ATTR_INVALID))
			ret = AMI_STATUS_OK;
		else
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EFMT);
	}

	return ret;
}

/*
 * Read a single sensor attribute.
 */
static int read_sensor_attr(struct ami_sensor_attr *attr)
{
	char buf[AMI_HWMON_MAX_STR] = { 0 };
	int ret = AMI_STATUS_ERROR;

	if (!attr || !attr->hwmon)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (read_hwmon(attr->hwmon, 0, NULL, buf) == AMI_STATUS_OK) {
		switch (attr->type) {
		/* String */
		case AMI_SENSOR_ATTR_NAME:
		case AMI_SENSOR_ATTR_STATUS:
			/*
			 * Note that sensor status is a special case.
			 * It gets parsed as a string but the user accesses it
			 * as `enum ami_sensor_type.
			 */
			memset(attr->value_s, 0x00, AMI_HWMON_MAX_STR);
			memcpy(attr->value_s, buf, AMI_HWMON_MAX_STR);
			/* Strip newlines */
			attr->value_s[strcspn(attr->value_s, "\r\n")] = 0;
			ret = AMI_STATUS_OK;
			break;
		
		/* Number */
		case AMI_SENSOR_ATTR_MAX:
		case AMI_SENSOR_ATTR_AVG:
		case AMI_SENSOR_ATTR_VALUE:
		case AMI_SENSOR_ATTR_WARN_LIMIT:
		case AMI_SENSOR_ATTR_CRIT_LIMIT:
		case AMI_SENSOR_ATTR_FATAL_LIMIT:
			ret = ami_convert_num(buf, AMI_BASE_10, &attr->value_l);
			break;
		
		default:
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
			break;
		}
	}

	return ret;
}

/*
 * Fetch a single sensor value over IOCTL.
 */
static int get_single_sensor_val(ami_device *dev, enum ami_sensor_type sensor_type,
	int sid, struct ami_sensor_attr *attr, struct ami_sensor_attr *status_attr, bool *fresh)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_sensor_value val = { 0 };

	if (!dev || !attr || !status_attr || !fresh)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR;
	
	switch (sensor_type) {
	case AMI_SENSOR_TYPE_TEMP:
		val.sensor_type = IOC_SENSOR_TYPE_TEMP;
		break;
	
	case AMI_SENSOR_TYPE_CURRENT:
		val.sensor_type = IOC_SENSOR_TYPE_CURRENT;
		break;
	
	case AMI_SENSOR_TYPE_VOLTAGE:
		val.sensor_type = IOC_SENSOR_TYPE_VOLTAGE;
		break;
	
	case AMI_SENSOR_TYPE_POWER:
		val.sensor_type = IOC_SENSOR_TYPE_POWER;
		break;
	
	default:
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	}

	if (sensor_type == AMI_SENSOR_TYPE_VOLTAGE)
		val.hwmon_channel = sid;
	else
		val.hwmon_channel = sid - 1;

	errno = 0;
	if (ioctl(dev->cdev, AMI_IOC_GET_SENSOR_VALUE, &val) == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	} else {
		/* Set status */
		memset(status_attr->value_s, 0x00, AMI_HWMON_MAX_STR);
		memcpy(status_attr->value_s, val.status, AMI_IOC_SENSOR_STATUS_LEN);
		/* Strip newlines */
		status_attr->value_s[strcspn(status_attr->value_s, "\r\n")] = 0;

		/* Set value */
		attr->value_l = val.val;

		/* Set cache status */
		*fresh = val.fresh;

		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Find a sensor data struct.
 */
static int find_sensor_data(struct ami_sensor_data *sensors, int sid,
	enum ami_sensor_type type, struct ami_sensor_data **data)
{
	int ret = AMI_STATUS_ERROR;

	if (sensors && data) {
		struct ami_sensor_data *next = sensors;

		while (next) {
			if ((next->type == type) && (next->sid == sid)) {
				*data = next;
				ret = AMI_STATUS_OK;
				break;
			}

			next = next->next;
		}
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
	}

	return ret;
}

/*
 * Find a sensor struct.
 */
static int find_sensor_by_name(ami_device *dev, const char *name,
	struct ami_sensor **sensor)
{
	int ret = AMI_STATUS_ERROR;

	/* Cache last value */
	static ami_device *last_dev = NULL;
	static struct ami_sensor *last = NULL;

	if (!dev || !dev->sensors || !name || !sensor)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	/* Check cached value. */
	if (last && (last_dev == dev) && (strcmp(last->name, name) == 0)) {
		*sensor = last;
		ret = AMI_STATUS_OK;
	} else {
		struct ami_sensor *next = dev->sensors;

		while (next) {
			if (strcmp(next->name, name) == 0) {
				*sensor = next;
				last = next;
				last_dev = dev;
				ret = AMI_STATUS_OK;
				break;
			}

			next = next->next;
		}
	}

	return ret;
}

/*
 * Create top level sensor structs.
 */
static int populate_device_sensors(ami_device *dev, struct ami_sensor_data *data)
{
	int ret = AMI_STATUS_OK;
	struct ami_sensor *sensors_tail = NULL;
	struct ami_sensor_data *next = data;

	if (!dev || dev->sensors)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	while (next) {
		struct ami_sensor *sensor = NULL;
		find_sensor_by_name(dev, next->name.value_s, &sensor);

		if (!sensor) {
			sensor = \
				(struct ami_sensor*)calloc(1, sizeof(struct ami_sensor));

			if (!sensor) {
				ret = AMI_API_ERROR(AMI_ERROR_ENOMEM);
				break;
			}

			sensor->sensor_data = \
				(ami_sensor_internal*)calloc(1, sizeof(struct ami_sensor_internal));

			if (!sensor->sensor_data) {
				ret = AMI_API_ERROR(AMI_ERROR_ENOMEM);
				break;
			}

			strcpy(sensor->name, next->name.value_s);

			if (dev->sensors) {
				sensors_tail->next = sensor;
				sensors_tail = sensor;
			} else {
				dev->sensors = sensor;
				sensors_tail = sensor;
			}

			dev->num_sensors++;
		}

		switch (next->type) {
		case AMI_SENSOR_TYPE_TEMP:
			sensor->sensor_data->temp = next;
			break;
		
		case AMI_SENSOR_TYPE_VOLTAGE:
			sensor->sensor_data->voltage = next;
			break;
		
		case AMI_SENSOR_TYPE_POWER:
			sensor->sensor_data->power = next;
			break;
		
		case AMI_SENSOR_TYPE_CURRENT:
			sensor->sensor_data->current = next;
			break;
		
		default:
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
			break;
		}

		if (ret != AMI_STATUS_OK)
			break;
		
		next = next->next;
	}

	return ret;
}

/*
 * Convert a status string to enum value.
 */
static enum ami_sensor_status parse_sensor_status(const char *status)
{
	if (strcmp(status, SENSOR_STATUS_NAME_NOT_PRESENT) == 0)
		return AMI_SENSOR_STATUS_NOT_PRESENT;
	else if (strcmp(status, SENSOR_STATUS_NAME_OK) == 0)
		return AMI_SENSOR_STATUS_OK;
	else if (strcmp(status, SENSOR_STATUS_NAME_NO_DATA) == 0)
		return AMI_SENSOR_STATUS_NO_DATA;
	else if (strcmp(status, SENSOR_STATUS_NAME_NA) == 0)
		return AMI_SENSOR_STATUS_NA;

	return AMI_SENSOR_STATUS_INVALID;
}

/*
 * Convert a limit type to an attribute.
 */
static enum ami_sensor_attr_type limit_type_to_attr(enum ami_sensor_limit limit)
{
	switch (limit) {
	case AMI_SENSOR_LIMIT_WARN:
		return AMI_SENSOR_ATTR_WARN_LIMIT;
	
	case AMI_SENSOR_LIMIT_CRIT:
		return AMI_SENSOR_ATTR_CRIT_LIMIT;
	
	case AMI_SENSOR_LIMIT_FATAL:
		return AMI_SENSOR_ATTR_FATAL_LIMIT;
	
	default:
		break;
	}

	return AMI_SENSOR_ATTR_INVALID;
}

/*
 * Get the value of a sensor attribute.
 */
static int get_value(ami_device *dev, const char *sensor_name,
	enum ami_sensor_attr_type attr, enum ami_sensor_type type, void *val,
	enum ami_sensor_status *status)
{
	int ret = AMI_STATUS_OK;
	struct ami_sensor_data *data = NULL;
	struct ami_sensor *sensor = NULL;

	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (find_sensor_by_name(dev, sensor_name, &sensor) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR;
	
	switch (type) {
	case AMI_SENSOR_TYPE_TEMP:
		data = sensor->sensor_data->temp;
		break;
	
	case AMI_SENSOR_TYPE_POWER:
		data = sensor->sensor_data->power;
		break;
	
	case AMI_SENSOR_TYPE_VOLTAGE:
		data = sensor->sensor_data->voltage;
		break;
	
	case AMI_SENSOR_TYPE_CURRENT:
		data = sensor->sensor_data->current;
		break;
	
	default:
		break;
	}

	if (!data)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	switch (attr) {
	/* String */
	case AMI_SENSOR_ATTR_NAME:
		*((const char**)val) = data->name.value_s;
		break;
	
	/* Number */
	case AMI_SENSOR_ATTR_STATUS:
		ret = read_sensor_attr(&data->status);
		
		if (!ret)
			*((enum ami_sensor_status*)val) = \
				parse_sensor_status(data->status.value_s);

		break;

	case AMI_SENSOR_ATTR_VALUE:
		/* If user requested status, use IOCTL instead. */
		if (status) {
			bool f = false;
			ret = get_single_sensor_val(dev, type, data->sid,
				&data->value, &data->status, &f);
			
			if (!ret) {
				*status = parse_sensor_status(data->status.value_s);

				if ((*status == AMI_SENSOR_STATUS_OK) && (f == false))
					*status = AMI_SENSOR_STATUS_OK_CACHED;
			}
		} else {
			/* Read hwmon otherwise... */
			ret = read_sensor_attr(&data->value);
		}

		if (!ret)
			*((long*)val) = data->value.value_l;

		break;
	
	case AMI_SENSOR_ATTR_MAX:
		if (data->max.valid) {
			ret = read_sensor_attr(&data->max);

			if (!ret)
				*((long*)val) = data->max.value_l;
		} else {
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
		}

		break;

	case AMI_SENSOR_ATTR_AVG:
		if (data->average.valid) {
			ret = read_sensor_attr(&data->average);

			if (!ret)
				*((long*)val) = data->average.value_l;
		} else {
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
		}

		break;
	
	/*
	 * The limits don't change over the lifetime of a device so we only
	 * need to read hwmon once - assume that if the limit value is non-zero
	 * the hwmon entry has already been read.
	 */
	case AMI_SENSOR_ATTR_WARN_LIMIT:
		if (data->warn_limit.valid) {
			if (data->warn_limit.value_l == 0)
				ret = read_sensor_attr(&data->warn_limit);

			if (!ret)
				*((long*)val) = data->warn_limit.value_l;
		} else {
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
		}

		break;
	
	case AMI_SENSOR_ATTR_CRIT_LIMIT:
		if (data->crit_limit.valid) {
			if (data->crit_limit.value_l == 0)
				ret = read_sensor_attr(&data->crit_limit);

			if (!ret)
				*((long*)val) = data->crit_limit.value_l;
		} else {
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
		}

		break;
	
	case AMI_SENSOR_ATTR_FATAL_LIMIT:
		if (data->fatal_limit.valid) {
			if (data->fatal_limit.value_l == 0)
				ret = read_sensor_attr(&data->fatal_limit);

			if (!ret)
				*((long*)val) = data->fatal_limit.value_l;
		} else {
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
		}

		break;

	/* The unit is a special case. */
	case AMI_SENSOR_ATTR_UNIT_MOD:
		*((enum ami_sensor_unit_mod*)val) = data->mod;
		break;

	default:
		ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
		break;
	}

	return ret;
}

/*****************************************************************************/
/* Public API function definitions                                           */
/*****************************************************************************/

/*
 * Find all sensors supported by a device.
 */
int ami_sensor_discover(ami_device *dev)
{
	int ret = AMI_STATUS_ERROR;
	int glob_ret = AMI_LINUX_STATUS_ERROR;
	glob_t glb = { 0 };
	char **sensor_files = NULL;

	struct ami_sensor_data *data = NULL;
	struct ami_sensor_data *sensors = NULL;
	struct ami_sensor_data *sensors_tail = NULL;
	char hwmon_sensors[AMI_HWMON_PATH_MAX_SIZE] = { 0 };

	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	snprintf(
		hwmon_sensors,
		AMI_HWMON_PATH_MAX_SIZE,
		AMI_HWMON_SENSORS_GLOB,
		dev->hwmon_num
	);

	glob_ret = glob(hwmon_sensors, GLOB_ERR | GLOB_BRACE, NULL, &glb);

	if (glob_ret != AMI_LINUX_STATUS_OK)
		return AMI_API_ERROR(AMI_ERROR_ERET);

	sensor_files = glb.gl_pathv;

	while (*sensor_files) {
		/* Parsed sensor data. */
		struct ami_sensor_attr *attribute = NULL;

		/* Hwmon variables. */
		int sid = 0;
		enum ami_sensor_type type = AMI_SENSOR_TYPE_INVALID;
		enum ami_sensor_attr_type attr = AMI_SENSOR_ATTR_INVALID;

		/* Parse hwmon path. */
		ret = parse_hwmon(*sensor_files, &sid, &type, &attr);

		if (ret != AMI_STATUS_OK) {
			/*
			 * Try to continue if we fail to parse as the driver may expose
			 * attributes which the API doesn't support.
			 */
			sensor_files++;
			continue;
		}

		/*
		 * Fetch top level sensor.
		 */
		if ((!data) || (data->type != type) || (data->sid != sid)) {
			if (find_sensor_data(sensors, sid, type, &data) != AMI_STATUS_OK)
				data = NULL;
		}

		if (!data) {
			/* Create new sensor. */
			data = (struct ami_sensor_data*)calloc(1, sizeof(struct ami_sensor_data));
			if (!data) {
				ret = AMI_API_ERROR(AMI_ERROR_ENOMEM);
				break;
			}

			data->sid = sid;
			data->type = type;
			data->next = NULL;

			/* Set sensor unit. */
			switch (data->type) {
			case AMI_SENSOR_TYPE_TEMP:
			case AMI_SENSOR_TYPE_CURRENT:
			case AMI_SENSOR_TYPE_VOLTAGE:
				data->mod = AMI_SENSOR_UNIT_MOD_MILLI;
				break;
			
			case AMI_SENSOR_TYPE_POWER:
				data->mod = AMI_SENSOR_UNIT_MOD_MICRO;
				break;
			
			default:
				data->mod = AMI_SENSOR_UNIT_MOD_NONE;
				break;
			}

			if (sensors) {
				sensors_tail->next = data;
				sensors_tail = data;
			} else {
				sensors = data;
				sensors_tail = data;
			}

			dev->num_total_sensors++;
		}

		switch (attr) {
		case AMI_SENSOR_ATTR_NAME:
			attribute = &data->name;
			break;

		case AMI_SENSOR_ATTR_STATUS:
			attribute = &data->status;
			break;

		case AMI_SENSOR_ATTR_MAX:
			attribute = &data->max;
			break;

		case AMI_SENSOR_ATTR_AVG:
			attribute = &data->average;
			break;

		case AMI_SENSOR_ATTR_VALUE:
			attribute = &data->value;
			break;
		
		case AMI_SENSOR_ATTR_WARN_LIMIT:
			attribute = &data->warn_limit;
			break;
		
		case AMI_SENSOR_ATTR_CRIT_LIMIT:
			attribute = &data->crit_limit;
			break;
		
		case AMI_SENSOR_ATTR_FATAL_LIMIT:
			attribute = &data->fatal_limit;
			break;

		default:
			break;
		}

		if (!attribute) {
			ret = AMI_API_ERROR(AMI_ERROR_EINVAL);
			break;
		}

		attribute->valid = true;
		attribute->type = attr;
		memcpy(attribute->hwmon, *sensor_files, AMI_HWMON_MAX_STR);

		/*
		 * Read only the sensor name.
		 * All other values will be fetched only if a user requests them
		 * via the respective getter functions.
		 */
		if (attr == AMI_SENSOR_ATTR_NAME)
			ret = read_sensor_attr(attribute);
		else
			ret = AMI_STATUS_OK;
		

		if (ret != AMI_STATUS_OK)
			break;

		sensor_files++;
	}

	globfree(&glb);

	if (ret == AMI_STATUS_OK)
		return populate_device_sensors(dev, sensors);
	
	return ret;
}

/*
 * Set the sensor refresh timeout.
 */
int ami_sensor_set_refresh(ami_device *dev, uint16_t val)
{
	int ret = AMI_STATUS_ERROR;

	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR;

	if (ioctl(dev->cdev, AMI_IOC_SET_SENSOR_REFRESH, val) == AMI_LINUX_STATUS_ERROR)
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	else
		ret = AMI_STATUS_OK;
	
	return ret;
}

/*
 * Get the sensor refresh timeout.
 */
int ami_sensor_get_refresh(ami_device *dev, uint16_t *val)
{
	long value = 0;
	int ret = AMI_STATUS_ERROR;
	char buf[AMI_HWMON_MAX_STR] = { 0 };

	if (!dev || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (AMI_STATUS_OK == read_hwmon(NULL, dev->hwmon_num, SENSOR_REFRESH_ATTR, buf)) {
		if (AMI_STATUS_OK == ami_convert_num(buf, AMI_BASE_10, &value)) {
			ret = AMI_STATUS_OK;
			*val = value;
		}
	}

	return ret;
}

// /***************************** Sensor Getters *********************************/

/*
 * Get a bitmask describing the sensor types that a sensor supports.
 */
int ami_sensor_get_type(ami_device *dev, const char *sensor_name, uint32_t *type)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_sensor *sensor = NULL;

	if (!dev || !type)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	find_sensor_by_name(dev, sensor_name, &sensor);

	if (sensor) {
		int sensor_type = 0;

		if (sensor->sensor_data->temp) {
			sensor_type |= AMI_SENSOR_TYPE_TEMP;
		}
		
		if (sensor->sensor_data->power) {
			sensor_type |= AMI_SENSOR_TYPE_POWER;
		}
		
		if (sensor->sensor_data->current) {
			sensor_type |= AMI_SENSOR_TYPE_CURRENT;
		}
		
		if (sensor->sensor_data->voltage) {
			sensor_type |= AMI_SENSOR_TYPE_VOLTAGE;
		}
		
		ret = AMI_STATUS_OK;
		*type = sensor_type;
	}

	return ret;
}

/*
 * Get the sensors for a device.
 */
int ami_sensor_get_sensors(ami_device *dev, struct ami_sensor **sensors, int *num)
{
	if (!dev || !sensors || *sensors || !num)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	*sensors = dev->sensors;
	*num = dev->num_sensors;
	return AMI_STATUS_OK;
}

int ami_sensor_get_num_total(ami_device *dev, int *num)
{
	if (!dev || !num)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	*num = dev->num_total_sensors;
	return AMI_STATUS_OK;
}

/* Value getters */

/*
 * Get temperature sensor value.
 */
int ami_sensor_get_temp_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_VALUE,
			AMI_SENSOR_TYPE_TEMP, (void*)val, sensor_status);
}

/*
 * Get voltage sensor value.
 */
int ami_sensor_get_voltage_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_VALUE,
			AMI_SENSOR_TYPE_VOLTAGE, (void*)val, sensor_status);
}

/*
 * Get current sensor value.
 */
int ami_sensor_get_current_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_VALUE,
			AMI_SENSOR_TYPE_CURRENT, (void*)val, sensor_status);
}

/*
 * Get power sensor value.
 */
int ami_sensor_get_power_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_VALUE,
			AMI_SENSOR_TYPE_POWER, (void*)val, sensor_status);
}

/* Status getters */

/*
 * Get temperature sensor status.
 */
int ami_sensor_get_temp_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_STATUS,
		AMI_SENSOR_TYPE_TEMP, (void*)val, NULL);
}

/*
 * Get voltage sensor status.
 */
int ami_sensor_get_voltage_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_STATUS,
		AMI_SENSOR_TYPE_VOLTAGE, (void*)val, NULL);
}

/*
 * Get current sensor status.
 */
int ami_sensor_get_current_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_STATUS,
		AMI_SENSOR_TYPE_CURRENT, (void*)val, NULL);
}

/*
 * Get power sensor status.
 */
int ami_sensor_get_power_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_STATUS,
		AMI_SENSOR_TYPE_POWER, (void*)val, NULL);
}

/* Max getters */

/*
 * Get temperature sensor max.
 */
int ami_sensor_get_temp_uptime_max(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_MAX,
		AMI_SENSOR_TYPE_TEMP, (void*)val, NULL);
}

/*
 * Get voltage sensor max.
 */
int ami_sensor_get_voltage_uptime_max(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_MAX,
		AMI_SENSOR_TYPE_VOLTAGE, (void*)val, NULL);
}

/*
 * Get current sensor max.
 */
int ami_sensor_get_current_uptime_max(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_MAX,
		AMI_SENSOR_TYPE_CURRENT, (void*)val, NULL);
}

/*
 * Get power sensor max.
 */
int ami_sensor_get_power_uptime_max(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_MAX,
		AMI_SENSOR_TYPE_POWER, (void*)val, NULL);
}

/* Average getters */

/*
 * Get temperature sensor average.
 */
int ami_sensor_get_temp_uptime_average(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_AVG,
		AMI_SENSOR_TYPE_TEMP, (void*)val, NULL);
}

/*
 * Get voltage sensor average.
 */
int ami_sensor_get_voltage_uptime_average(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_AVG,
		AMI_SENSOR_TYPE_VOLTAGE, (void*)val, NULL);
}

/*
 * Get current sensor average.
 */
int ami_sensor_get_current_uptime_average(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_AVG,
		AMI_SENSOR_TYPE_CURRENT, (void*)val, NULL);
}

/*
 * Get power sensor average.
 */
int ami_sensor_get_power_uptime_average(ami_device *dev,
	const char *sensor_name, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_AVG,
		AMI_SENSOR_TYPE_POWER, (void*)val, NULL);
}

/* Unit getters */

/*
 * Get temperature sensor unit.
 */
int ami_sensor_get_temp_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	if (!dev || !sensor_name || !mod)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_UNIT_MOD,
		AMI_SENSOR_TYPE_TEMP, (void*)mod, NULL);
}

/*
 * Get voltage sensor unit.
 */
int ami_sensor_get_voltage_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	if (!dev || !sensor_name || !mod)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_UNIT_MOD,
		AMI_SENSOR_TYPE_VOLTAGE, (void*)mod, NULL);
}

/*
 * Get current sensor unit.
 */
int ami_sensor_get_current_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	if (!dev || !sensor_name || !mod)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_UNIT_MOD,
		AMI_SENSOR_TYPE_CURRENT, (void*)mod, NULL);
}

/*
 * Get power sensor unit.
 */
int ami_sensor_get_power_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod)
{
	if (!dev || !sensor_name || !mod)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, AMI_SENSOR_ATTR_UNIT_MOD,
		AMI_SENSOR_TYPE_POWER, (void*)mod, NULL);
}

/* Limit getters */

/*
 * Get the limits of a temperature sensor.
 */
int ami_sensor_get_temp_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, limit_type_to_attr(limit_type),
		AMI_SENSOR_TYPE_TEMP, (void*)val, NULL);
}

/*
 * Get the limits of a voltage sensor.
 */
int ami_sensor_get_voltage_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, limit_type_to_attr(limit_type),
		AMI_SENSOR_TYPE_VOLTAGE, (void*)val, NULL);
}

/*
 * Get the limits of a current sensor.
 */
int ami_sensor_get_current_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, limit_type_to_attr(limit_type),
		AMI_SENSOR_TYPE_CURRENT, (void*)val, NULL);
}

/*
 * Get the limits of a power sensor.
 */
int ami_sensor_get_power_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val)
{
	if (!dev || !sensor_name || !val)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return get_value(dev, sensor_name, limit_type_to_attr(limit_type),
		AMI_SENSOR_TYPE_POWER, (void*)val, NULL);
}
