// SPDX-License-Identifier: GPL-2.0-only
/*
 * sensors.h - This file contains sensor utilities for the AMI CLI
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_APP_SENSORS_H
#define AMI_APP_SENSORS_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>

/* API includes */
#include "ami_device.h"

/* App includes */
#include "json.h"
#include "amiapp.h"

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum extra_sensor_fields - Flags for reporting additional sensor info.
 * @EXTRA_FIELDS_NONE: No additional values.
 * @EXTRA_FIELDS_MAX: Maximum sensor value.
 * @EXTRA_FIELDS_AVG: Average sensor value.
 * @EXTRA_FIELDS_LIMITS: Sensor limits.
 * @EXTRA_FIELDS_ALL: All additional values.
 * 
 * Note that "Name", "Value", and "Status" are always reported.
 */
enum extra_sensor_fields {
	EXTRA_FIELDS_NONE   = 0,
	EXTRA_FIELDS_MAX    = (uint32_t)1U << 0,
	EXTRA_FIELDS_AVG    = (uint32_t)1U << 1,
	EXTRA_FIELDS_LIMITS = (uint32_t)1U << 2,
	EXTRA_FIELDS_ALL    = EXTRA_FIELDS_MAX | EXTRA_FIELDS_AVG | EXTRA_FIELDS_LIMITS,
};

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * struct app_sensor_data - Struct to hold extra information when printing sensor data
 * @extra_fields: Bitflag indicating additional sensor values to print.
 * @sensor: If specified, print only data for this sensor.
 */
struct app_sensor_data {
	int extra_fields;
	const char *sensor;
};

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * report_sensors() - Utility function to print sensor information.
 * @options: List of command line options.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
int report_sensors(struct app_option *options);

#endif  /* AMI_APP_SENSORS_H */
