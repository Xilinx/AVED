// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_sensor_internal.h - This file contains the private interface for sensor logic
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_SENSOR_INTERNAL_H
#define AMI_SENSOR_INTERNAL_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>
#include <stdbool.h>

/* Private API includes */
#include "ami_internal.h"

/* Public API includes*/
#include "ami_sensor.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* Max size of a hwmon attribute. */
#define AMI_HWMON_MAX_STR	(AMI_SENSOR_MAX_STR)

/* /sys/class/hwmon/hwmon<id>/<type><num>_<attr> */
#define AMI_HWMON_DIR		"/sys/class/hwmon/hwmon%d"
#define AMI_HWMON_NODE		AMI_HWMON_DIR "/%s%d_%s"
#define AMI_HWMON_SENSORS_GLOB	AMI_HWMON_DIR "/{temp,power,in,curr}[0-9]*"
#define AMI_HWMON_ATTR		AMI_HWMON_DIR "/%s"
#define AMI_HWMON_ATTR_FORMAT	"/sys/class/hwmon/hwmon%*d/%s"

#define AMI_HWMON_PATH_MAX_SIZE	(256)
#define AMI_HWMON_TYPE_MAX_SIZE	(10)
#define AMI_HWMON_ATTR_MAX_SIZE	(16)

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum ami_sensor_attr_type - list of supported sensor attributes
 * @AMI_SENSOR_ATTR_ID: sensor ID (hwmon channel/index in ASDM)
 * @AMI_SENSOR_ATTR_NAME: human readable name
 * @AMI_SENSOR_ATTR_VALUE: instantaneous value
 * @AMI_SENSOR_ATTR_MAX: max value
 * @AMI_SENSOR_ATTR_AVG: average value
 * @AMI_SENSOR_ATTR_STATUS: human readable status
 * @AMI_SENSOR_ATTR_UNIT_MOD: sensor unit modifier
 * @AMI_SENSOR_ATTR_INVALID: unrecognised attribute
 */
enum ami_sensor_attr_type {
	AMI_SENSOR_ATTR_ID,
	AMI_SENSOR_ATTR_NAME,
	AMI_SENSOR_ATTR_VALUE,
	AMI_SENSOR_ATTR_MAX,
	AMI_SENSOR_ATTR_AVG,
	AMI_SENSOR_ATTR_STATUS,
	AMI_SENSOR_ATTR_UNIT_MOD,
	AMI_SENSOR_ATTR_WARN_LIMIT,
	AMI_SENSOR_ATTR_CRIT_LIMIT,
	AMI_SENSOR_ATTR_FATAL_LIMIT,

	AMI_SENSOR_ATTR_INVALID = -1,
};

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * struct ami_sensor_attr - Represents a single sensor attribute.
 * @valid: Boolean indicating if this attribute is valid (present)
 * @type: Type of sensor attribute.
 * @hwmon: Full path to hwmon node.
 * @value_l: Attribute value as a long (only for numeric attributes)
 * @value_s: Attribute as a string (only for string attributes)
 */
struct ami_sensor_attr {
	bool valid;
	enum ami_sensor_attr_type type;
	char hwmon[AMI_HWMON_PATH_MAX_SIZE];

	union {
		long value_l;
		char value_s[AMI_HWMON_MAX_STR];
	};
};

/**
 * struct ami_sensor_data - Represents a single sensor type.
 * @sid: sensor ID
 * @type: sensor type
 * @mod: sensor unit modifier (e.g., -3 (milli))
 * @status: status string
 * @name: name string
 * @value: instantaneous value
 * @average: average value
 * @max: max value
 * @next: pointer to next sensor data struct
 * 
 * The difference between this and a top level sensor is that a single
 * `ami_sensor` type may be composed of multiple `ami_sensor_data` structs.
 * For example, a vccint sensor may have a sensor data struct for current,
 * voltage, and temperature.
 */
struct ami_sensor_data {
	int                       sid;
	enum ami_sensor_type      type;
	enum ami_sensor_unit_mod  mod;
	struct ami_sensor_attr    status;
	struct ami_sensor_attr    name;
	struct ami_sensor_attr    value;
	struct ami_sensor_attr    average;
	struct ami_sensor_attr    max;
	struct ami_sensor_attr    warn_limit;
	struct ami_sensor_attr    crit_limit;
	struct ami_sensor_attr    fatal_limit;
	struct ami_sensor_data   *next;
};

/**
 * struct ami_sensor_internal - Private sensor data.
 * @temp: temperature data
 * @current: current data
 * @voltage: voltage data
 * @power: power data
 * 
 * Note that not all data structs may be valid for a given sensor.
 */
struct ami_sensor_internal {
	struct ami_sensor_data *temp;
	struct ami_sensor_data *current;
	struct ami_sensor_data *voltage;
	struct ami_sensor_data *power;
};

#endif  /* AMI_SENSOR_INTERNAL_H */
