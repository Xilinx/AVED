// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_sensor.h- This file contains the public interface for sensor logic
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_SENSOR_H
#define AMI_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>

/* Public API include */
#include "ami.h"
#include "ami_device.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define AMI_SENSOR_MAX_STR	(64)
#define AMI_SENSOR_OK_STR	"Sensor Present and Valid"

/*
 * This is used to allow easy iteration over sensor type bitflags.
 * NOTE: This must always be updated according to the sensor type flags.
 */
#define AMI_SENSOR_TYPE_MAX	(4)

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum ami_sensor_type - list of AMI sensor types
 * @AMI_SENSOR_TYPE_TEMP: temperature sensor
 * @AMI_SENSOR_TYPE_CURRENT: current sensor
 * @AMI_SENSOR_TYPE_VOLTAGE: voltage sensor
 * @AMI_SENSOR_TYPE_POWER: power sensor
 * @AMI_SENSOR_TYPE_INVALID: invalid sensor type
 */
enum ami_sensor_type {
	AMI_SENSOR_TYPE_TEMP    = (uint32_t)(1 << 0),
	AMI_SENSOR_TYPE_CURRENT = (uint32_t)(1 << 1),
	AMI_SENSOR_TYPE_VOLTAGE = (uint32_t)(1 << 2),
	AMI_SENSOR_TYPE_POWER   = (uint32_t)(1 << 3),

	AMI_SENSOR_TYPE_INVALID = (uint32_t)0,
};

/**
 * enum ami_sensor_status - list of sensor status codes
 * @AMI_SENSOR_STATUS_INVALID: Generic invalid code to be used as a placeholder.
 * @AMI_SENSOR_STATUS_NOT_PRESENT: Sensor not present.
 * @AMI_SENSOR_STATUS_OK: Sensor present and valid.
 * @AMI_SENSOR_STATUS_NO_DATA: Data not available.
 * @AMI_SENSOR_STATUS_OK_CACHED: Value is OK but not fresh.
 * @AMI_SENSOR_STATUS_NA: Not applicable or default value.
 */
enum ami_sensor_status {
	AMI_SENSOR_STATUS_INVALID     = -1,

	AMI_SENSOR_STATUS_NOT_PRESENT = 0x00,
	AMI_SENSOR_STATUS_OK          = 0x01,
	AMI_SENSOR_STATUS_NO_DATA     = 0x02,
	AMI_SENSOR_STATUS_OK_CACHED   = 0x03,  /* Not from ASDM */
	/* 0x04 - 0x7E Reserved */
	AMI_SENSOR_STATUS_NA          = 0x7F,
	/* 0x80 - 0xFF Reserved */
};

/**
 * enum ami_sensor_unit_mod - base unit modifiers for sensor readings
 * @AMI_SENSOR_UNIT_MOD_MEGA: 10^6 modifier
 * @AMI_SENSOR_UNIT_MOD_KILO: 10^3 modifier
 * @AMI_SENSOR_UNIT_MOD_NONE: 10^0 modifier (none)
 * @AMI_SENSOR_UNIT_MOD_MILLI: 10^-3 modifier
 * @AMI_SENSOR_UNIT_MOD_MICRO: 10^-6 modifier
 * 
 * Note that no enum is defined for the type of unit used as this can be
 * inferred from the getter function being called. For reference, the units used
 * are as follows:
 * 
 * Temp - Degrees Celsius
 * Power - Watts
 * Voltage - Volts
 * Current - Amperes
 */
enum ami_sensor_unit_mod {
	AMI_SENSOR_UNIT_MOD_MEGA  =  6,
	AMI_SENSOR_UNIT_MOD_KILO  =  3,
	AMI_SENSOR_UNIT_MOD_NONE  =  0,
	AMI_SENSOR_UNIT_MOD_MILLI = -3,
	AMI_SENSOR_UNIT_MOD_MICRO = -6
};

/**
 * enum ami_sensor_limit - list of supported sensor limits/thresholds
 * @AMI_SENSOR_LIMIT_WARN: Warning threshold
 * @AMI_SENSOR_LIMIT_CRIT: Critical threshold
 * @AMI_SENSOR_LIMIT_FATAL: Fatal threshold
 */
enum ami_sensor_limit {
	AMI_SENSOR_LIMIT_WARN,
	AMI_SENSOR_LIMIT_CRIT,
	AMI_SENSOR_LIMIT_FATAL,
};

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/* Opaque declaration of sensor private data. */
typedef struct ami_sensor_internal ami_sensor_internal;

/**
 * struct ami_sensor() - Represents a top-level sensor struct.
 * @name: Sensor name.
 * @next: Pointer to next sensor.
 * @sensor_data: Private sensor data. Not to be used by user.
 */
struct ami_sensor {
	char                 name[AMI_SENSOR_MAX_STR];
	struct ami_sensor   *next;
	ami_sensor_internal *sensor_data;
};

/*****************************************************************************/
/* Public API function declarations                                          */
/*****************************************************************************/

/**
 * ami_sensor_discover() - Find all sensors supported by a specific device.
 * @dev: Device handle
 * 
 * This function updates the sensor related attributes of
 * the given device handle. This should generally be called only once
 * in the lifetime of a particular device.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_sensor_discover(ami_device *dev);

/**
 * ami_sensor_set_refresh() - Set the sensor update interval.
 * @dev: Device handle.
 * @val: New update interval (ms).
 * 
 * Note that nothing needs to be done by the user to enforce the refresh
 * interval. The set/get functions are simply for updating the interval
 * or fetching it. The refresh will always be in effect under the hood
 * (unless, of course, the refresh timeout is set to 0). Every applicable
 * sensor getter will read a hwmon file, which triggers a driver callback; the
 * driver knows the last update timestamp and can figure out whether or not
 * a new value should be fetched.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_set_refresh(ami_device *dev, uint16_t val);

/**
 * ami_sensor_get_refresh() - Get the current sensor update interval.
 * @dev: Device handle.
 * @val: Variable to hold update interval (ms).
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_refresh(ami_device *dev, uint16_t *val);

/***************************** Sensor Getters *********************************/

/**
 * ami_sensor_get_type() -Get a bitmask describing the sensor types
 *   that a sensor supports.
 * @dev: Device handle.
 * @sensor_name: Name of sensor.
 * @type: Output variable to store sensor type.
 * 
 * See `enum AMI_SENSOR_TYPE` for possible values.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_type(ami_device *dev, const char *sensor_name, uint32_t *type);

/**
 * ami_sensor_get_sensors() - Get the list of sensors belonging to a device.
 * @dev: Device handle.
 * @sensors: Output variable to hold list of sensors.
 * @num: Output variable to hold number of sensors.
 * 
 * Note that sensors are grouped by name. For example, a sensor called
 * 'vccint' which is composed of a temperature, current, and voltage sensor
 * is counted as a single sensor.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_sensors(ami_device *dev, struct ami_sensor **sensors, int *num);

/**
 * ami_sensor_get_num_total() - Get the total number of sensors for a device.
 * @dev: Device handle.
 * @num: Output variable to hold number of sensors.
 * 
 * For this function, each sensor type is treated as an individual sensor.
 * For example, 'vccint temp', 'vccint power', '12v_pex temp', etc...
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_num_total(ami_device *dev, int *num);

/**
 * ami_sensor_get_temp_status() - Get the status string of a temperature sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold status code.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_temp_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val);

/**
 * ami_sensor_get_voltage_status() - Get the status string of a voltage sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold status code.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */	
int ami_sensor_get_voltage_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val);

/**
 * ami_sensor_get_current_status() - Get the status string of a current sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold status code.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_current_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val);

/**
 * ami_sensor_get_power_status() - Get the status string of a power sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold status code.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */	
int ami_sensor_get_power_status(ami_device *dev, const char *sensor_name,
	enum ami_sensor_status *val);

/**
 * ami_sensor_get_temp_value() - Get the value of a temperature sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * @sensor_status: Optional variable to hold the status of the sensor
 * 
 * Note that this function returns the instantaneous sensor value at time of
 * the last sensor refresh.
 * 
 * To check if the value returned is valid, you must check the return code
 * of the function AND the sensor status. The sensor status refers to the
 * status of the sensor as a whole, so it is possible for the function
 * to fail but the status to be OK. Note that the `sensor_status` argument is
 * optional - if not NULL, the function will call the 
 * `get_temp_status` function in addition to reading the sensor value;
 * otherwise, only the sensor value is read.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_temp_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status);

/**
 * ami_sensor_get_voltage_value() - Get the value of a voltage sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * @sensor_status: Optional variable to hold the status of the sensor
 * 
 * Note that this function returns the instantaneous sensor value at time of
 * the last sensor refresh.
 * 
 * To check if the value returned is valid, you must check the return code
 * of the function AND the sensor status. The sensor status refers to the
 * status of the sensor as a whole, so it is possible for the function
 * to fail but the status to be OK. Note that the `sensor_status` argument is
 * optional - if not NULL, the function will call the 
 * `get_voltage_status` function in addition to reading the sensor value;
 * otherwise, only the sensor value is read.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_voltage_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status);

/**
 * ami_sensor_get_current_value() - Get the value of a current sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * @sensor_status: Optional variable to hold the status of the sensor
 * 
 * Note that this function returns the instantaneous sensor value at time of
 * the last sensor refresh.
 * 
 * To check if the value returned is valid, you must check the return code
 * of the function AND the sensor status. The sensor status refers to the
 * status of the sensor as a whole, so it is possible for the function
 * to fail but the status to be OK. Note that the `sensor_status` argument is
 * optional - if not NULL, the function will call the 
 * `get_current_status` function in addition to reading the sensor value;
 * otherwise, only the sensor value is read.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_current_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status);

/**
 * ami_sensor_get_power_value() - Get the value of a power sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * @sensor_status: Optional variable to hold the status of the sensor
 * 
 * Note that this function returns the instantaneous sensor value at time of
 * the last sensor refresh.
 * 
 * To check if the value returned is valid, you must check the return code
 * of the function AND the sensor status. The sensor status refers to the
 * status of the sensor as a whole, so it is possible for the function
 * to fail but the status to be OK. Note that the `sensor_status` argument is
 * optional - if not NULL, the function will call the
 * `get_power_status` function in addition to reading the sensor value;
 * otherwise, only the sensor value is read.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */	
int ami_sensor_get_power_value(ami_device *dev, const char *sensor_name,
	long *val, enum ami_sensor_status *sensor_status);

/**
 * ami_sensor_get_temp_uptime_max() - Get the max value of a temperature sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_temp_uptime_max(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_voltage_uptime_max() - Get the max value of a voltage sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_voltage_uptime_max(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_current_uptime_max() - Get the max value of a current sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_current_uptime_max(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_power_uptime_max() - Get the max value of a power sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_power_uptime_max(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_temp_uptime_average() - Get the average value of a temperature sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_temp_uptime_average(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_voltage_uptime_average() - Get the average value of a voltage sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_voltage_uptime_average(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_current_uptime_average() - Get the average value of a current sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_current_uptime_average(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_power_uptime_average() - Get the average value of a power sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @val: Variable to hold output value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_power_uptime_average(ami_device *dev,
	const char *sensor_name, long *val);

/**
 * ami_sensor_get_temp_unit_mod() - Get the unit modifier of a temperature sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @mod: Variable to hold modifier value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_temp_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod);

/**
 * ami_sensor_get_voltage_unit_mod() - Get the unit modifier of a voltage sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @mod: Variable to hold modifier value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_voltage_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod);

/**
 * ami_sensor_get_current_unit_mod() - Get the unit modifier of a current sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @mod: Variable to hold modifier value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_current_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod);

/**
 * ami_sensor_get_power_unit_mod() - Get the unit modifier of a power sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @mod: Variable to hold modifier value.
 * 
 * These values are NOT calculated by AMI.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_power_unit_mod(ami_device *dev, const char *sensor_name,
	enum ami_sensor_unit_mod *mod);

/**
 * ami_sensor_get_temp_limit() - Get the limit/threshold of a temperature sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @limit_type: Type of limit to fetch (warn/crit/fatal/etc.)
 * @val: Variable to hold limit value.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_temp_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val);

/**
 * ami_sensor_get_voltage_limit() - Get the limit/threshold of a voltage sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @limit_type: Type of limit to fetch (warn/crit/fatal/etc.)
 * @val: Variable to hold limit value.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_voltage_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val);

/**
 * ami_sensor_get_current_limit() - Get the limit/threshold of a current sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @limit_type: Type of limit to fetch (warn/crit/fatal/etc.)
 * @val: Variable to hold limit value.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_current_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val);

/**
 * ami_sensor_get_power_limit() - Get the limit/threshold of a power sensor.
 * @dev: Device handle.
 * @sensor_name: Sensor name.
 * @limit_type: Type of limit to fetch (warn/crit/fatal/etc.)
 * @val: Variable to hold limit value.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_sensor_get_power_limit(ami_device *dev, const char *sensor_name,
	enum ami_sensor_limit limit_type, long *val);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_SENSOR_H */
