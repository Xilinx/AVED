// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_hwmon.c - This file contains logic related to hwmon.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/types.h>         /* umode_t */
#include <linux/device.h>        /* struct device */
#include <linux/err.h>           /* IS_ERR, PTR_ERR */
#include <linux/string.h>        /* string functions */
#include <linux/kernel.h>        /* container_of */

#include "ami.h"
#include "ami_pcie.h"
#include "ami_hwmon.h"
#include "ami_amc_control.h"
#include "ami_sensor.h"

/*
 * NOTE: Where possible, we implement functionality for ALL sensors that may be
 * present - this is then checked against each device in the `alveo_is_visible`
 * function so that a device's hwmon tree is populated only with the sensors
 * that the particular device supports. Suported sensors are discovered
 * through the ASDM API (see `ami_amc_control.c` and `ami_sensor.c`). The ASDM API
 * does not assign globally unique sensor ID's but rather "indexes" each sensor
 * in a particular repo type. This allows us to have an almost direct mapping
 * between hwmon channel and sensor "ID" (or index). Thus, for all intents
 * and purposes "channel" and "sensor id" are used interchangeably in this file.
 */
#define ALVEO_NUM_TEMP_SENSORS		(20)
#define ALVEO_NUM_VOLTAGE_SENSORS	(20)
#define ALVEO_NUM_CURRENT_SENSORS	(20)
#define ALVEO_NUM_POWER_SENSORS		(5)

#define NONE_TO_MILLI_UNIT(x)	        (x * 1000)
#define KILO_TO_MILLI_UNIT(x)           (NONE_TO_MILLI_UNIT(x * 1000))
#define MEGA_TO_MILLI_UNIT(x)           (KILO_TO_MILLI_UNIT(x * 1000))
#define MICRO_TO_MILLI_UNIT(x)	        (x / 1000)

#define MILLI_TO_MICRO_UNIT(x)	        (x * 1000)
#define NONE_TO_MICRO_UNIT(x)	        (MILLI_TO_MICRO_UNIT(x * 1000))
#define KILO_TO_MICRO_UNIT(x)	        (NONE_TO_MICRO_UNIT(x * 1000))
#define MEGA_TO_MICRO_UNIT(x)	        (KILO_TO_MICRO_UNIT(x * 1000))

#define READ_ONLY			(0444)
#define READ_WRITE			(0644)

/* Utility macros for hwmon attributes */
#define HWMON_LIMITS(x) ( \
	HWMON_##x ## _MAX         | \
	HWMON_##x ## _LCRIT       | \
	HWMON_##x ## _CRIT        | \
	HWMON_##x ## _MAX_ALARM   | \
	HWMON_##x ## _LCRIT_ALARM | \
	HWMON_##x ## _CRIT_ALARM    \
)

/**
 * enum ami_sensor_attribute - List of supported sensor attributes.
 * @SENSOR_ATTR_INSTANT: The current value of the sensor.
 * @SENSOR_ATTR_MIN: The historical min value of the sensor - not supported currently.
 * @SENSOR_ATTR_MAX: The historical max value of the sensor.
 * @SENSOR_ATTR_AVERAGE: The average value of the sensor since boot.
 * @SENSOR_ATTR_LABEL: Human readable name of the sensor.
 * @SENSOR_ATTR_STATUS: Sensor status.
 * @SENSOR_ATTR_UNIT_MOD: The sensor unit modifier.
 * @SENSOR_ATTR_WARN: Max sensor limit.
 * @SENSOR_ATTR_WARN_A: Max sensor limit alarm.
 * @SENSOR_ATTR_CRIT: Critical sensor limit.
 * @SENSOR_ATTR_CRIT_A: Critical sensor limit alarm.
 * @SENSOR_ATTR_FATAL: Fatal sensor limit.
 * @SENSOR_ATTR_FATAL_A: Fatal sensor limit alarm.
 * @SENSOR_ATTR_INVALID: The invalid sensor attribute.
 * 
 * This enum is an analogue to `hwmon_xxx_attributes` but is not tied
 * to a specific sensor type. Not all sensors support limits and alarms.
 */
enum ami_sensor_attribute {
	SENSOR_ATTR_INSTANT,
	SENSOR_ATTR_MIN,
	SENSOR_ATTR_MAX,
	SENSOR_ATTR_AVERAGE,
	SENSOR_ATTR_LABEL,
	SENSOR_ATTR_STATUS,
	SENSOR_ATTR_UNIT_MOD,
	SENSOR_ATTR_WARN,
	SENSOR_ATTR_WARN_A,
	SENSOR_ATTR_CRIT,
	SENSOR_ATTR_CRIT_A,
	SENSOR_ATTR_FATAL,
	SENSOR_ATTR_FATAL_A,
	SENSOR_ATTR_INVALID,
};

/**
 * enum ami_sensor_unit_mod - base unit modifiers for sensor readings
 * @SENSOR_UNIT_MOD_MEGA: 10^6 modifier
 * @SENSOR_UNIT_MOD_KILO: 10^3 modifier
 * @SENSOR_UNIT_MOD_NONE: 10^0 modifier (none)
 * @SENSOR_UNIT_MOD_MILLI: 10^-3 modifier
 * @SENSOR_UNIT_MOD_MICRO: 10^-6 modifier
 */
enum ami_sensor_unit_mod {
	SENSOR_UNIT_MOD_MEGA  =  6,
	SENSOR_UNIT_MOD_KILO  =  3,
	SENSOR_UNIT_MOD_NONE  =  0,
	SENSOR_UNIT_MOD_MILLI = -3,
	SENSOR_UNIT_MOD_MICRO = -6
};

/**
 * enum ami_sensor_type - List of supported sensors.
 * @SENSOR_TYPE_INVALID: Invalid sensor type.
 * @SENSOR_TYPE_TEMP: Temperature sensor.
 * @SENSOR_TYPE_CURRENT: Current sensor.
 * @SENSOR_TYPE_VOLTAGE: Voltage sensor.
 * @SENSOR_TYPE_POWER: Device power sensor.
 * 
 * This enum is an analogue to `enum hwmon_sensor_types`.
 */
enum ami_sensor_type {
	SENSOR_TYPE_INVALID = -1,
	SENSOR_TYPE_TEMP    = SDR_TYPE_TEMP,
	SENSOR_TYPE_CURRENT = SDR_TYPE_CURRENT,
	SENSOR_TYPE_VOLTAGE = SDR_TYPE_VOLTAGE,
	SENSOR_TYPE_POWER   = SDR_POWER_TYPE,
};

/**
 * _alveo_is_visible() - Check if a sensor attribute exists for a given PCI device.
 * @pf_dev: Pointer to PCI device data.
 * @sid: Sensor ID (index/channel).
 * @type: Sensor type.
 * @attr: Sensor attribute.
 * 
 * Return: The mode of the hwmon file if the attribute exists or 0.
 */
umode_t _alveo_is_visible(struct pf_dev_struct *pf_dev, int sid,
		enum ami_sensor_type type, enum ami_sensor_attribute attr);

/* hwmon callbacks */
umode_t alveo_is_visible(const void *data, enum hwmon_sensor_types type,
		u32 attr, int channel);
int alveo_read(struct device *dev, enum hwmon_sensor_types type,
		u32 attr, int channel, long *val);
int alveo_write(struct device *dev, enum hwmon_sensor_types type,
			u32 attr, int channel, long val);


static const struct hwmon_ops alveo_ops = {
	.is_visible = alveo_is_visible,
	.read = alveo_read,
	.write = alveo_write,
};

static const u32 alveo_chip_config[] = {
	HWMON_C_UPDATE_INTERVAL,
	0
};

static const struct hwmon_channel_info alveo_chip = {
	.type = hwmon_chip,
	.config = alveo_chip_config,
};

static const u32 alveo_temp_config[ALVEO_NUM_TEMP_SENSORS + 1] = {
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),

	/* Extra attributes in case a device needs them. */
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),
	HWMON_T_INPUT | HWMON_T_HIGHEST | HWMON_LIMITS(T),

	0
};

static const struct hwmon_channel_info alveo_temp = {
	.type = hwmon_temp,
	.config = alveo_temp_config,
};

static const u32 alveo_voltage_config[ALVEO_NUM_VOLTAGE_SENSORS + 1] = {
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),

	/* Extra attributes in case a device needs them. */
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),
	HWMON_I_INPUT | HWMON_I_HIGHEST | HWMON_I_AVERAGE | HWMON_LIMITS(I),

	0
};

static const struct hwmon_channel_info alveo_voltage = {
	.type = hwmon_in,
	.config = alveo_voltage_config,
};

static const u32 alveo_current_config[ALVEO_NUM_CURRENT_SENSORS + 1] = {
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),

	/* Extra attributes in case a device needs them. */
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),
	HWMON_C_INPUT | HWMON_C_HIGHEST | HWMON_C_AVERAGE | HWMON_LIMITS(C),

	0
};

static const struct hwmon_channel_info alveo_current = {
	.type = hwmon_curr,
	.config = alveo_current_config,
};

static const u32 alveo_power_config[ALVEO_NUM_POWER_SENSORS + 1] = {
	HWMON_P_INPUT | HWMON_P_INPUT_HIGHEST | HWMON_P_AVERAGE | HWMON_LIMITS(P),

	/* Extra attributes in case a device needs them. */
	HWMON_P_INPUT | HWMON_P_INPUT_HIGHEST | HWMON_P_AVERAGE | HWMON_LIMITS(P),
	HWMON_P_INPUT | HWMON_P_INPUT_HIGHEST | HWMON_P_AVERAGE | HWMON_LIMITS(P),
	HWMON_P_INPUT | HWMON_P_INPUT_HIGHEST | HWMON_P_AVERAGE | HWMON_LIMITS(P),
	HWMON_P_INPUT | HWMON_P_INPUT_HIGHEST | HWMON_P_AVERAGE | HWMON_LIMITS(P),

	0
};

static const struct hwmon_channel_info alveo_power = {
	.type = hwmon_power,
	.config = alveo_power_config,
};

static const struct hwmon_channel_info * alveo_info[] = {
	&alveo_chip,
	&alveo_temp,
	&alveo_voltage,
	&alveo_current,
	&alveo_power,
	NULL
};

static const struct hwmon_chip_info alveo_hwmon_info = {
	.ops = &alveo_ops,
	.info = alveo_info
};

/**
 * to_ami_sensor_type() - Convert `hwmon_sensor_type` to `ami_sensor_type`.
 * @type: The `enum hwmon_sensor_type` to convert from.
 * 
 * Return: The equivalent sensor type or SENSOR_TYPE_INVALID.
 */
enum ami_sensor_type to_ami_sensor_type(enum hwmon_sensor_types type)
{
	/* The ami sensor type maps directly to the SDR repo type. */
	enum ami_sensor_type ret = SENSOR_TYPE_INVALID;

	switch (type) {
	case hwmon_temp:  ret = SENSOR_TYPE_TEMP;    break;
	case hwmon_in:    ret = SENSOR_TYPE_VOLTAGE; break;
	case hwmon_curr:  ret = SENSOR_TYPE_CURRENT; break;
	case hwmon_power: ret = SENSOR_TYPE_POWER;   break;
	default: break;
	}

	return ret;
}

/**
 * to_ami_attribute() - Convert a hwmon type/attr pair to `ami_sensor_attribute`.
 * @hwmon_type: The hwmon sensor type.
 * @hwmon_attr: The hwmon sensor attribute.
 * 
 * Return: Corresponding ami_sensor_attribute (may be INVALID).
 */
enum ami_sensor_attribute to_ami_attribute(enum hwmon_sensor_types hwmon_type,
		u32 hwmon_attr)
{
	enum ami_sensor_attribute ret = SENSOR_ATTR_INVALID;

	switch (hwmon_type) {
	case hwmon_temp:
		switch (hwmon_attr) {
		case hwmon_temp_input:          ret = SENSOR_ATTR_INSTANT; break;
		case hwmon_temp_highest:        ret = SENSOR_ATTR_MAX;     break;
		case hwmon_temp_lowest:         ret = SENSOR_ATTR_MIN;     break;
		case hwmon_temp_max_alarm:      ret = SENSOR_ATTR_WARN_A;  break;
		case hwmon_temp_max:            ret = SENSOR_ATTR_WARN;    break;
		case hwmon_temp_lcrit_alarm:    ret = SENSOR_ATTR_CRIT_A;  break;
		case hwmon_temp_lcrit:          ret = SENSOR_ATTR_CRIT;    break;
		case hwmon_temp_crit_alarm:     ret = SENSOR_ATTR_FATAL_A; break;
		case hwmon_temp_crit:           ret = SENSOR_ATTR_FATAL;   break;
		default: break;
		}
		break;
	
	case hwmon_in:
		switch (hwmon_attr) {
		case hwmon_in_input:            ret = SENSOR_ATTR_INSTANT; break;
		case hwmon_in_highest:          ret = SENSOR_ATTR_MAX;     break;
		case hwmon_in_lowest:           ret = SENSOR_ATTR_MIN;     break;
		case hwmon_in_average:          ret = SENSOR_ATTR_AVERAGE; break;
		case hwmon_in_max_alarm:        ret = SENSOR_ATTR_WARN_A;  break;
		case hwmon_in_max:              ret = SENSOR_ATTR_WARN;    break;
		case hwmon_in_lcrit_alarm:      ret = SENSOR_ATTR_CRIT_A;  break;
		case hwmon_in_lcrit:            ret = SENSOR_ATTR_CRIT;    break;
		case hwmon_in_crit_alarm:       ret = SENSOR_ATTR_FATAL_A; break;
		case hwmon_in_crit:             ret = SENSOR_ATTR_FATAL;   break;
		default: break;
		}
		break;
	
	case hwmon_curr:
		switch (hwmon_attr) {
		case hwmon_curr_input:          ret = SENSOR_ATTR_INSTANT; break;
		case hwmon_curr_highest:        ret = SENSOR_ATTR_MAX;     break;
		case hwmon_curr_lowest:         ret = SENSOR_ATTR_MIN;     break;
		case hwmon_curr_average:        ret = SENSOR_ATTR_AVERAGE; break;
		case hwmon_curr_max_alarm:      ret = SENSOR_ATTR_WARN_A;  break;
		case hwmon_curr_max:            ret = SENSOR_ATTR_WARN;    break;
		case hwmon_curr_lcrit_alarm:    ret = SENSOR_ATTR_CRIT_A;  break;
		case hwmon_curr_lcrit:          ret = SENSOR_ATTR_CRIT;    break;
		case hwmon_curr_crit_alarm:     ret = SENSOR_ATTR_FATAL_A; break;
		case hwmon_curr_crit:           ret = SENSOR_ATTR_FATAL;   break;

		default: break;
		}
		break;
	
	case hwmon_power:
		switch (hwmon_attr) {
		case hwmon_power_input:         ret = SENSOR_ATTR_INSTANT; break;
		case hwmon_power_input_highest: ret = SENSOR_ATTR_MAX;     break;
		case hwmon_power_input_lowest:  ret = SENSOR_ATTR_MIN;     break;
		case hwmon_power_average:       ret = SENSOR_ATTR_AVERAGE; break;
		case hwmon_power_max_alarm:     ret = SENSOR_ATTR_WARN_A;  break;
		case hwmon_power_max:           ret = SENSOR_ATTR_WARN;    break;
		case hwmon_power_lcrit_alarm:   ret = SENSOR_ATTR_CRIT_A;  break;
		case hwmon_power_lcrit:         ret = SENSOR_ATTR_CRIT;    break;
		case hwmon_power_crit_alarm:    ret = SENSOR_ATTR_FATAL_A; break;
		case hwmon_power_crit:          ret = SENSOR_ATTR_FATAL;   break;
		default: break;
		}
		break;
	
	default:
		break;
	}

	return ret;
}

umode_t _alveo_is_visible(struct pf_dev_struct *pf_dev, int sid,
		enum ami_sensor_type type, enum ami_sensor_attribute attr)
{
	struct sdr_record *rec = NULL;

	if (!pf_dev)
		return 0;

	rec = find_sdr_record(pf_dev->sensor_repos, pf_dev->num_sensor_repos,
		(enum gcq_sdr_repo_type)type, sid);

	/* Sensor doesn't exist. */
	if (!rec)
		return 0;

	/* Check if attribute exists - this is only needed for the thresholds. */
	switch (attr) {
	case SENSOR_ATTR_WARN:
	case SENSOR_ATTR_WARN_A:
		if (rec->threshold_support & THRESHOLD_UPPER_WARNING_MASK)
			return READ_ONLY;
		break;

	case SENSOR_ATTR_CRIT:
	case SENSOR_ATTR_CRIT_A:
		if (rec->threshold_support & THRESHOLD_UPPER_CRITICAL_MASK)
			return READ_ONLY;
		break;

	case SENSOR_ATTR_FATAL:
	case SENSOR_ATTR_FATAL_A:
		if (rec->threshold_support & THRESHOLD_UPPER_FATAL_MASK)
			return READ_ONLY;
		break;

	default:
		return READ_ONLY;
	}

	return 0;
}

umode_t alveo_is_visible(const void *data, enum hwmon_sensor_types type,
		u32 attr, int channel)
{
	if (!data)
		return 0;
	
	/* Chip config is special */
	if (type == hwmon_chip) {
		switch (attr) {
		case hwmon_chip_update_interval:
			return READ_WRITE;
		
		default:
			return 0;
		}
	}

	return _alveo_is_visible((struct pf_dev_struct*)data,
		channel, to_ami_sensor_type(type), to_ami_attribute(type, attr));
}

/**
 * get_sensor_value() - Get the value of a sensor attribute.
 * @pf_dev: The PCI device data.
 * @type: The requested sensor type.
 * @attr: The requested sensor attribute.
 * @sid: The requested sensor ID.
 * @value: Void pointer to the output variable.
 * 
 * This function DOES NOT fetch any new data over the PCI bus.
 * It simply parses the data that is already present within the pf_dev
 * struct and returns the requested attribute.
 *
 * It is the caller's responsibility to ensure that the output variable
 * is of the correct type for the requested attribute. For numeric values
 * this must be a signed long. For string/ascii values it must be
 * an unsigned long which will be filled with the address of the respective
 * value buffer. This can then be used like any other C string.
 * 
 * Return: 0 on success or negative error code.
 */
int get_sensor_value(struct pf_dev_struct *pf_dev, enum ami_sensor_type type,
	enum ami_sensor_attribute attr, int sid, void *value)
{
	int ret = 0;
	struct sdr_record *rec = NULL;

	if (!pf_dev || !value)
		return -EINVAL;

	rec = find_sdr_record(
		pf_dev->sensor_repos,
		pf_dev->num_sensor_repos,
		(enum gcq_sdr_repo_type)type,
		sid
	);

	/* Find sensor attribute. */
	if (rec) {
		switch (attr) {
		case SENSOR_ATTR_INSTANT:
			*((long*)(value)) = make_val(
				rec->value_type, rec->value_len, rec->value);
			break;
		
		case SENSOR_ATTR_AVERAGE:
			*((long*)(value)) = make_val(
				rec->value_type, rec->value_len, rec->avg);
			break;
		
		case SENSOR_ATTR_MAX:
			*((long*)(value)) = make_val(
				rec->value_type, rec->value_len, rec->max);
			break;
		
		case SENSOR_ATTR_MIN:
			*((long*)(value)) = make_val(
				rec->value_type, rec->value_len, rec->min);
			break;
		
		case SENSOR_ATTR_STATUS:
			*((long*)(value)) = (long)rec->sensor_status;
			break;
		
		case SENSOR_ATTR_LABEL:
			*((unsigned long*)(value)) = (unsigned long)&rec->name[0];
			break;

		case SENSOR_ATTR_UNIT_MOD:
			*((long*)(value)) = (long)rec->unit_mod;
			break;
		
		/*
		 * For convenience, when an alarm is requested we just return the limit itself.
		 * This avoids another switch statement inside `read_sensor_val`.
		 */
		case SENSOR_ATTR_WARN:
		case SENSOR_ATTR_WARN_A:
			if (rec->threshold_support & THRESHOLD_UPPER_WARNING_MASK) {
				*((long*)(value)) = make_val(
					rec->value_type, rec->value_len, rec->upper_warn_limit
				);
			} else {
				ret = -EINVAL;
			}
			break;
		
		case SENSOR_ATTR_CRIT:
		case SENSOR_ATTR_CRIT_A:
			if (rec->threshold_support & THRESHOLD_UPPER_CRITICAL_MASK) {
				*((long*)(value)) = make_val(
					rec->value_type, rec->value_len, rec->upper_crit_limit
				);
			} else {
				ret = -EINVAL;
			}
			break;
		
		case SENSOR_ATTR_FATAL:
		case SENSOR_ATTR_FATAL_A:
			if (rec->threshold_support & THRESHOLD_UPPER_FATAL_MASK) {
				*((long*)(value)) = make_val(
					rec->value_type, rec->value_len, rec->upper_fatal_limit
				);
			} else {
				ret = -EINVAL;
			}
			break;

		default:
			ret = -EINVAL;
			break;
		}
	}

	return ret;
}

int alveo_write(struct device *dev, enum hwmon_sensor_types type,
			u32 attr, int channel, long val)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev)
		return -EINVAL;
	
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (!pf_dev)
		return -ENODEV;

	if (type == hwmon_chip) {
		switch (attr) {
		case hwmon_chip_update_interval:
			if (val < 0)
				pf_dev->sensor_refresh = 0;
			else
				pf_dev->sensor_refresh = (uint16_t)val;

			break;
		
		default:
			ret = -EINVAL;
			break;
		}
	}

	put_pf_dev_entry(pf_dev);
	return ret;
}

/**
 * convert_milli_units() - Convert the received value into milli units based on the unit mod.
 * @unit_mod: The unit modifier.
 * @val: The raw received value.
 * @mapped_val: The mapped value based on the unit mod.
 *
 * Return: 0 on success or negative error code.
 */
static int convert_milli_units(enum ami_sensor_unit_mod unit_mod, long val, long *mapped_val)
{
	int ret = 0;
	if (!mapped_val)
		return -EINVAL;

	switch(unit_mod)
	{
	case SENSOR_UNIT_MOD_MEGA:
                *mapped_val = MEGA_TO_MILLI_UNIT(val);
                break;
	case SENSOR_UNIT_MOD_KILO:
                *mapped_val = KILO_TO_MILLI_UNIT(val);
                break;
	case SENSOR_UNIT_MOD_NONE:
		*mapped_val = NONE_TO_MILLI_UNIT(val);
		break;
	case SENSOR_UNIT_MOD_MILLI:
		*mapped_val = val;
		break;
	case SENSOR_UNIT_MOD_MICRO:
                *mapped_val = MICRO_TO_MILLI_UNIT(val);
                break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * convert_micro_units() - Convert the received value into micro units based on the unit mod.
 * @unit_mod: The unit modifier.
 * @val: The raw received value.
 * @mapped_val: The mapped value based on the unit mod.
 *
 * Return: 0 on success or negative error code.
 */
static int convert_micro_units(enum ami_sensor_unit_mod unit_mod, long val, long *mapped_val)
{
	int ret = 0;
	if (!mapped_val)
		return -EINVAL;

	switch(unit_mod)
	{
	case SENSOR_UNIT_MOD_MEGA:
                *mapped_val = MEGA_TO_MICRO_UNIT(val);
                break;
	case SENSOR_UNIT_MOD_KILO:
                *mapped_val = KILO_TO_MICRO_UNIT(val);
                break;
	case SENSOR_UNIT_MOD_NONE:
                *mapped_val = NONE_TO_MICRO_UNIT(val);
                break;
	case SENSOR_UNIT_MOD_MILLI:
	        *mapped_val = MILLI_TO_MICRO_UNIT(val);
		break;
	case SENSOR_UNIT_MOD_MICRO:
		*mapped_val = val;
                break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/**
 * alveo_read() - Hwmon read callback.
 * @dev: The character device data.
 * @type: The requested sensor type.
 * @attr: The requested sensor attribute.
 * @channel: The requested sensor ID.
 * @val: Void pointer to the output variable.
 *
 * Return: 0 on success or negative error code.
 */
int alveo_read(struct device *dev, enum hwmon_sensor_types type,
	u32 attr, int channel, long *val)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !val)
		return -EINVAL;
	
	/*
	 * If this callback is triggered, we know that the device supports
	 * the sensor as this is already verified by the `alveo_is_visible`
	 * function.
	 */
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		ret = read_sensor_val(
			pf_dev,
			type,
			attr,
			channel,
			val,
			NULL,
			NULL
		);
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}

/* Non-standard attributes - every sensor has an additional label and status */

/* sysfs callback - same as `alveo_is_visible` but for nonstandard attributes. */
static umode_t is_visible_extra(struct kobject *kobj, struct attribute *attr, int n)
{
	struct device *dev = NULL; /* This is the hwmon device */
	struct pf_dev_struct *pf_dev = NULL;
	struct device_attribute *da = NULL;
	struct sensor_device_attribute_2 *sensor_da = NULL;
	enum ami_sensor_type sensor_type = SENSOR_TYPE_INVALID;

	if(!kobj || !attr)
		return 0;

	dev = container_of(kobj, struct device, kobj);
	pf_dev = dev_get_drvdata(dev);
	da = container_of(attr, struct device_attribute, attr);
	sensor_da = to_sensor_dev_attr_2(da);
	sensor_type = (enum ami_sensor_type)sensor_da->nr;

	/*
	 * Setting attribute to invalid; assume that all extra attributes
	 * are supported by all sensors.
	 */
	return _alveo_is_visible(pf_dev, sensor_da->index, sensor_type,
		SENSOR_ATTR_INVALID);
}

/*
 * For some reason, hwmon does not support average temperature...
 * Have to add it as an extra attribute.
 */
static ssize_t temp_average_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;
	struct sensor_device_attribute_2 *sensor_da = NULL;
	long value = 0;
	long mapped_value = 0;
	enum ami_sensor_unit_mod unit_mod = SENSOR_UNIT_MOD_NONE;

	if (!dev || !da || !buf)
		return 0;

	pf_dev = dev_get_drvdata(dev);
	sensor_da = to_sensor_dev_attr_2(da);

	ret = get_sensor_value(pf_dev, (enum ami_sensor_type)sensor_da->nr,
			SENSOR_ATTR_UNIT_MOD, sensor_da->index, &unit_mod);

	if (!ret) {
		ret = get_sensor_value(pf_dev, (enum ami_sensor_type)sensor_da->nr,
			SENSOR_ATTR_AVERAGE, sensor_da->index, &value);
		
		/* hwmon expects temp in millidegree Celsius (see `read_sensor_val` function) */
		if (!ret && !convert_milli_units(unit_mod, value, &mapped_value))
			return sprintf(buf, "%ld\n", mapped_value);
	}

	return 0;
}
static SENSOR_DEVICE_ATTR_2(temp1_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  0);
static SENSOR_DEVICE_ATTR_2(temp2_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  1);
static SENSOR_DEVICE_ATTR_2(temp3_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  2);
static SENSOR_DEVICE_ATTR_2(temp4_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  3);
static SENSOR_DEVICE_ATTR_2(temp5_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  4);
static SENSOR_DEVICE_ATTR_2(temp6_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  5);
static SENSOR_DEVICE_ATTR_2(temp7_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  6);
static SENSOR_DEVICE_ATTR_2(temp8_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  7);
static SENSOR_DEVICE_ATTR_2(temp9_average,  0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  8);
static SENSOR_DEVICE_ATTR_2(temp10_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP,  9);
/* Extra attributes in case a device needs them */
static SENSOR_DEVICE_ATTR_2(temp11_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 10);
static SENSOR_DEVICE_ATTR_2(temp12_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 11);
static SENSOR_DEVICE_ATTR_2(temp13_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 12);
static SENSOR_DEVICE_ATTR_2(temp14_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 13);
static SENSOR_DEVICE_ATTR_2(temp15_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 14);
static SENSOR_DEVICE_ATTR_2(temp16_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 15);
static SENSOR_DEVICE_ATTR_2(temp17_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 16);
static SENSOR_DEVICE_ATTR_2(temp18_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 17);
static SENSOR_DEVICE_ATTR_2(temp19_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 18);
static SENSOR_DEVICE_ATTR_2(temp20_average, 0, temp_average_show, NULL, SENSOR_TYPE_TEMP, 19);

static struct attribute *temp_avg_attributes[] = {
	&sensor_dev_attr_temp1_average.dev_attr.attr,
	&sensor_dev_attr_temp2_average.dev_attr.attr,
	&sensor_dev_attr_temp3_average.dev_attr.attr,
	&sensor_dev_attr_temp4_average.dev_attr.attr,
	&sensor_dev_attr_temp5_average.dev_attr.attr,
	&sensor_dev_attr_temp6_average.dev_attr.attr,
	&sensor_dev_attr_temp7_average.dev_attr.attr,
	&sensor_dev_attr_temp8_average.dev_attr.attr,
	&sensor_dev_attr_temp9_average.dev_attr.attr,
	&sensor_dev_attr_temp10_average.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_temp11_average.dev_attr.attr,
	&sensor_dev_attr_temp12_average.dev_attr.attr,
	&sensor_dev_attr_temp13_average.dev_attr.attr,
	&sensor_dev_attr_temp14_average.dev_attr.attr,
	&sensor_dev_attr_temp15_average.dev_attr.attr,
	&sensor_dev_attr_temp16_average.dev_attr.attr,
	&sensor_dev_attr_temp17_average.dev_attr.attr,
	&sensor_dev_attr_temp18_average.dev_attr.attr,
	&sensor_dev_attr_temp19_average.dev_attr.attr,
	&sensor_dev_attr_temp20_average.dev_attr.attr,

	NULL
};

static struct attribute_group temp_avg_attr_group = {
	.attrs = temp_avg_attributes,
	.is_visible = is_visible_extra,
};

static ssize_t sensor_status_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	struct pf_dev_struct *pf_dev = NULL;
	struct sensor_device_attribute_2 *sensor_da = NULL;
	long status = 0;

	if (!dev || !da || !buf)
		return 0;

	pf_dev = dev_get_drvdata(dev);
	sensor_da = to_sensor_dev_attr_2(da);

	get_sensor_value(pf_dev, (enum ami_sensor_type)sensor_da->nr,
			SENSOR_ATTR_STATUS, sensor_da->index, &status);

	return sprintf(buf, "%s\n", convert_sensor_status_name_map(status));
}
static SENSOR_DEVICE_ATTR_2(temp1_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     0);
static SENSOR_DEVICE_ATTR_2(temp2_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     1);
static SENSOR_DEVICE_ATTR_2(temp3_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     2);
static SENSOR_DEVICE_ATTR_2(temp4_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     3);
static SENSOR_DEVICE_ATTR_2(temp5_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     4);
static SENSOR_DEVICE_ATTR_2(temp6_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     5);
static SENSOR_DEVICE_ATTR_2(temp7_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     6);
static SENSOR_DEVICE_ATTR_2(temp8_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     7);
static SENSOR_DEVICE_ATTR_2(temp9_status,  0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     8);
static SENSOR_DEVICE_ATTR_2(temp10_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,     9);
static SENSOR_DEVICE_ATTR_2(in0_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  0);
static SENSOR_DEVICE_ATTR_2(in1_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  1);
static SENSOR_DEVICE_ATTR_2(in2_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  2);
static SENSOR_DEVICE_ATTR_2(in3_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  3);
static SENSOR_DEVICE_ATTR_2(in4_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  4);
static SENSOR_DEVICE_ATTR_2(in5_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  5);
static SENSOR_DEVICE_ATTR_2(in6_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  6);
static SENSOR_DEVICE_ATTR_2(in7_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  7);
static SENSOR_DEVICE_ATTR_2(in8_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  8);
static SENSOR_DEVICE_ATTR_2(in9_status,    0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE,  9);
static SENSOR_DEVICE_ATTR_2(in10_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 10);
static SENSOR_DEVICE_ATTR_2(curr1_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  0);
static SENSOR_DEVICE_ATTR_2(curr2_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  1);
static SENSOR_DEVICE_ATTR_2(curr3_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  2);
static SENSOR_DEVICE_ATTR_2(curr4_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  3);
static SENSOR_DEVICE_ATTR_2(curr5_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  4);
static SENSOR_DEVICE_ATTR_2(curr6_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  5);
static SENSOR_DEVICE_ATTR_2(curr7_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  6);
static SENSOR_DEVICE_ATTR_2(curr8_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  7);
static SENSOR_DEVICE_ATTR_2(curr9_status,  0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  8);
static SENSOR_DEVICE_ATTR_2(curr10_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT,  9);
static SENSOR_DEVICE_ATTR_2(curr11_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 10);
static SENSOR_DEVICE_ATTR_2(power1_status, 0, sensor_status_show, NULL, SENSOR_TYPE_POWER,    0);
/* Extra attributes in case a device needs them */
static SENSOR_DEVICE_ATTR_2(temp11_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    10);
static SENSOR_DEVICE_ATTR_2(temp12_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    11);
static SENSOR_DEVICE_ATTR_2(temp13_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    12);
static SENSOR_DEVICE_ATTR_2(temp14_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    13);
static SENSOR_DEVICE_ATTR_2(temp15_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    14);
static SENSOR_DEVICE_ATTR_2(temp16_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    15);
static SENSOR_DEVICE_ATTR_2(temp17_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    16);
static SENSOR_DEVICE_ATTR_2(temp18_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    17);
static SENSOR_DEVICE_ATTR_2(temp19_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    18);
static SENSOR_DEVICE_ATTR_2(temp20_status, 0, sensor_status_show, NULL, SENSOR_TYPE_TEMP,    19);
static SENSOR_DEVICE_ATTR_2(in11_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 11);
static SENSOR_DEVICE_ATTR_2(in12_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 12);
static SENSOR_DEVICE_ATTR_2(in13_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 13);
static SENSOR_DEVICE_ATTR_2(in14_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 14);
static SENSOR_DEVICE_ATTR_2(in15_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 15);
static SENSOR_DEVICE_ATTR_2(in16_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 16);
static SENSOR_DEVICE_ATTR_2(in17_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 17);
static SENSOR_DEVICE_ATTR_2(in18_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 18);
static SENSOR_DEVICE_ATTR_2(in19_status,   0, sensor_status_show, NULL, SENSOR_TYPE_VOLTAGE, 19);
static SENSOR_DEVICE_ATTR_2(curr12_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 11);
static SENSOR_DEVICE_ATTR_2(curr13_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 12);
static SENSOR_DEVICE_ATTR_2(curr14_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 13);
static SENSOR_DEVICE_ATTR_2(curr15_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 14);
static SENSOR_DEVICE_ATTR_2(curr16_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 15);
static SENSOR_DEVICE_ATTR_2(curr17_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 16);
static SENSOR_DEVICE_ATTR_2(curr18_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 17);
static SENSOR_DEVICE_ATTR_2(curr19_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 18);
static SENSOR_DEVICE_ATTR_2(curr20_status, 0, sensor_status_show, NULL, SENSOR_TYPE_CURRENT, 19);
static SENSOR_DEVICE_ATTR_2(power2_status, 0, sensor_status_show, NULL, SENSOR_TYPE_POWER,    1);
static SENSOR_DEVICE_ATTR_2(power3_status, 0, sensor_status_show, NULL, SENSOR_TYPE_POWER,    2);
static SENSOR_DEVICE_ATTR_2(power4_status, 0, sensor_status_show, NULL, SENSOR_TYPE_POWER,    3);
static SENSOR_DEVICE_ATTR_2(power5_status, 0, sensor_status_show, NULL, SENSOR_TYPE_POWER,    4);


static struct attribute *status_attributes[] = {
	&sensor_dev_attr_temp1_status.dev_attr.attr,
	&sensor_dev_attr_temp2_status.dev_attr.attr,
	&sensor_dev_attr_temp3_status.dev_attr.attr,
	&sensor_dev_attr_temp4_status.dev_attr.attr,
	&sensor_dev_attr_temp5_status.dev_attr.attr,
	&sensor_dev_attr_temp6_status.dev_attr.attr,
	&sensor_dev_attr_temp7_status.dev_attr.attr,
	&sensor_dev_attr_temp8_status.dev_attr.attr,
	&sensor_dev_attr_temp9_status.dev_attr.attr,
	&sensor_dev_attr_temp10_status.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_temp11_status.dev_attr.attr,
	&sensor_dev_attr_temp12_status.dev_attr.attr,
	&sensor_dev_attr_temp13_status.dev_attr.attr,
	&sensor_dev_attr_temp14_status.dev_attr.attr,
	&sensor_dev_attr_temp15_status.dev_attr.attr,
	&sensor_dev_attr_temp16_status.dev_attr.attr,
	&sensor_dev_attr_temp17_status.dev_attr.attr,
	&sensor_dev_attr_temp18_status.dev_attr.attr,
	&sensor_dev_attr_temp19_status.dev_attr.attr,
	&sensor_dev_attr_temp20_status.dev_attr.attr,

	&sensor_dev_attr_in0_status.dev_attr.attr,
	&sensor_dev_attr_in1_status.dev_attr.attr,
	&sensor_dev_attr_in2_status.dev_attr.attr,
	&sensor_dev_attr_in3_status.dev_attr.attr,
	&sensor_dev_attr_in4_status.dev_attr.attr,
	&sensor_dev_attr_in5_status.dev_attr.attr,
	&sensor_dev_attr_in6_status.dev_attr.attr,
	&sensor_dev_attr_in7_status.dev_attr.attr,
	&sensor_dev_attr_in8_status.dev_attr.attr,
	&sensor_dev_attr_in9_status.dev_attr.attr,
	&sensor_dev_attr_in10_status.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_in11_status.dev_attr.attr,
	&sensor_dev_attr_in12_status.dev_attr.attr,
	&sensor_dev_attr_in13_status.dev_attr.attr,
	&sensor_dev_attr_in14_status.dev_attr.attr,
	&sensor_dev_attr_in15_status.dev_attr.attr,
	&sensor_dev_attr_in16_status.dev_attr.attr,
	&sensor_dev_attr_in17_status.dev_attr.attr,
	&sensor_dev_attr_in18_status.dev_attr.attr,
	&sensor_dev_attr_in19_status.dev_attr.attr,

	&sensor_dev_attr_curr1_status.dev_attr.attr,
	&sensor_dev_attr_curr2_status.dev_attr.attr,
	&sensor_dev_attr_curr3_status.dev_attr.attr,
	&sensor_dev_attr_curr4_status.dev_attr.attr,
	&sensor_dev_attr_curr5_status.dev_attr.attr,
	&sensor_dev_attr_curr6_status.dev_attr.attr,
	&sensor_dev_attr_curr7_status.dev_attr.attr,
	&sensor_dev_attr_curr8_status.dev_attr.attr,
	&sensor_dev_attr_curr9_status.dev_attr.attr,
	&sensor_dev_attr_curr10_status.dev_attr.attr,
	&sensor_dev_attr_curr11_status.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_curr11_status.dev_attr.attr,
	&sensor_dev_attr_curr12_status.dev_attr.attr,
	&sensor_dev_attr_curr13_status.dev_attr.attr,
	&sensor_dev_attr_curr14_status.dev_attr.attr,
	&sensor_dev_attr_curr15_status.dev_attr.attr,
	&sensor_dev_attr_curr16_status.dev_attr.attr,
	&sensor_dev_attr_curr17_status.dev_attr.attr,
	&sensor_dev_attr_curr18_status.dev_attr.attr,
	&sensor_dev_attr_curr19_status.dev_attr.attr,
	&sensor_dev_attr_curr20_status.dev_attr.attr,

	&sensor_dev_attr_power1_status.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_power2_status.dev_attr.attr,
	&sensor_dev_attr_power3_status.dev_attr.attr,
	&sensor_dev_attr_power4_status.dev_attr.attr,
	&sensor_dev_attr_power5_status.dev_attr.attr,

	NULL
};

static struct attribute_group status_attr_group = {
	.attrs = status_attributes,
	.is_visible = is_visible_extra,
};

static ssize_t sensor_label_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	struct pf_dev_struct *pf_dev = NULL;
	struct sensor_device_attribute_2 *sensor_da = NULL;
	unsigned long label_addr = 0;

	if (!dev || !da || !buf)
		return 0;

	pf_dev = dev_get_drvdata(dev);
	sensor_da = to_sensor_dev_attr_2(da);

	get_sensor_value(pf_dev,(enum ami_sensor_type)sensor_da->nr,
			SENSOR_ATTR_LABEL, sensor_da->index, &label_addr);
	
	if (label_addr)
		return sprintf(buf, "%s\n", (char*)label_addr);

	return 0;
}
static SENSOR_DEVICE_ATTR_2(temp1_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     0);
static SENSOR_DEVICE_ATTR_2(temp2_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     1);
static SENSOR_DEVICE_ATTR_2(temp3_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     2);
static SENSOR_DEVICE_ATTR_2(temp4_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     3);
static SENSOR_DEVICE_ATTR_2(temp5_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     4);
static SENSOR_DEVICE_ATTR_2(temp6_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     5);
static SENSOR_DEVICE_ATTR_2(temp7_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     6);
static SENSOR_DEVICE_ATTR_2(temp8_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     7);
static SENSOR_DEVICE_ATTR_2(temp9_label,  0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     8);
static SENSOR_DEVICE_ATTR_2(temp10_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,     9);
static SENSOR_DEVICE_ATTR_2(in0_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  0);
static SENSOR_DEVICE_ATTR_2(in1_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  1);
static SENSOR_DEVICE_ATTR_2(in2_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  2);
static SENSOR_DEVICE_ATTR_2(in3_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  3);
static SENSOR_DEVICE_ATTR_2(in4_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  4);
static SENSOR_DEVICE_ATTR_2(in5_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  5);
static SENSOR_DEVICE_ATTR_2(in6_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  6);
static SENSOR_DEVICE_ATTR_2(in7_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  7);
static SENSOR_DEVICE_ATTR_2(in8_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  8);
static SENSOR_DEVICE_ATTR_2(in9_label,    0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE,  9);
static SENSOR_DEVICE_ATTR_2(in10_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 10);
static SENSOR_DEVICE_ATTR_2(curr1_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  0);
static SENSOR_DEVICE_ATTR_2(curr2_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  1);
static SENSOR_DEVICE_ATTR_2(curr3_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  2);
static SENSOR_DEVICE_ATTR_2(curr4_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  3);
static SENSOR_DEVICE_ATTR_2(curr5_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  4);
static SENSOR_DEVICE_ATTR_2(curr6_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  5);
static SENSOR_DEVICE_ATTR_2(curr7_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  6);
static SENSOR_DEVICE_ATTR_2(curr8_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  7);
static SENSOR_DEVICE_ATTR_2(curr9_label,  0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  8);
static SENSOR_DEVICE_ATTR_2(curr10_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT,  9);
static SENSOR_DEVICE_ATTR_2(curr11_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 10);
static SENSOR_DEVICE_ATTR_2(power1_label, 0, sensor_label_show, NULL, SENSOR_TYPE_POWER,    0);
/* Extra attributes in case a device needs them */
static SENSOR_DEVICE_ATTR_2(temp11_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    10);
static SENSOR_DEVICE_ATTR_2(temp12_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    11);
static SENSOR_DEVICE_ATTR_2(temp13_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    12);
static SENSOR_DEVICE_ATTR_2(temp14_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    13);
static SENSOR_DEVICE_ATTR_2(temp15_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    14);
static SENSOR_DEVICE_ATTR_2(temp16_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    15);
static SENSOR_DEVICE_ATTR_2(temp17_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    16);
static SENSOR_DEVICE_ATTR_2(temp18_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    17);
static SENSOR_DEVICE_ATTR_2(temp19_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    18);
static SENSOR_DEVICE_ATTR_2(temp20_label, 0, sensor_label_show, NULL, SENSOR_TYPE_TEMP,    19);
static SENSOR_DEVICE_ATTR_2(in11_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 11);
static SENSOR_DEVICE_ATTR_2(in12_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 12);
static SENSOR_DEVICE_ATTR_2(in13_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 13);
static SENSOR_DEVICE_ATTR_2(in14_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 14);
static SENSOR_DEVICE_ATTR_2(in15_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 15);
static SENSOR_DEVICE_ATTR_2(in16_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 16);
static SENSOR_DEVICE_ATTR_2(in17_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 17);
static SENSOR_DEVICE_ATTR_2(in18_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 18);
static SENSOR_DEVICE_ATTR_2(in19_label,   0, sensor_label_show, NULL, SENSOR_TYPE_VOLTAGE, 19);
static SENSOR_DEVICE_ATTR_2(curr12_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 11);
static SENSOR_DEVICE_ATTR_2(curr13_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 12);
static SENSOR_DEVICE_ATTR_2(curr14_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 13);
static SENSOR_DEVICE_ATTR_2(curr15_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 14);
static SENSOR_DEVICE_ATTR_2(curr16_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 15);
static SENSOR_DEVICE_ATTR_2(curr17_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 16);
static SENSOR_DEVICE_ATTR_2(curr18_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 17);
static SENSOR_DEVICE_ATTR_2(curr19_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 18);
static SENSOR_DEVICE_ATTR_2(curr20_label, 0, sensor_label_show, NULL, SENSOR_TYPE_CURRENT, 19);
static SENSOR_DEVICE_ATTR_2(power2_label, 0, sensor_label_show, NULL, SENSOR_TYPE_POWER,    1);
static SENSOR_DEVICE_ATTR_2(power3_label, 0, sensor_label_show, NULL, SENSOR_TYPE_POWER,    2);
static SENSOR_DEVICE_ATTR_2(power4_label, 0, sensor_label_show, NULL, SENSOR_TYPE_POWER,    3);
static SENSOR_DEVICE_ATTR_2(power5_label, 0, sensor_label_show, NULL, SENSOR_TYPE_POWER,    4);

static struct attribute *label_attributes[] = {
	&sensor_dev_attr_temp1_label.dev_attr.attr,
	&sensor_dev_attr_temp2_label.dev_attr.attr,
	&sensor_dev_attr_temp3_label.dev_attr.attr,
	&sensor_dev_attr_temp4_label.dev_attr.attr,
	&sensor_dev_attr_temp5_label.dev_attr.attr,
	&sensor_dev_attr_temp6_label.dev_attr.attr,
	&sensor_dev_attr_temp7_label.dev_attr.attr,
	&sensor_dev_attr_temp8_label.dev_attr.attr,
	&sensor_dev_attr_temp9_label.dev_attr.attr,
	&sensor_dev_attr_temp10_label.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_temp11_label.dev_attr.attr,
	&sensor_dev_attr_temp12_label.dev_attr.attr,
	&sensor_dev_attr_temp13_label.dev_attr.attr,
	&sensor_dev_attr_temp14_label.dev_attr.attr,
	&sensor_dev_attr_temp15_label.dev_attr.attr,
	&sensor_dev_attr_temp16_label.dev_attr.attr,
	&sensor_dev_attr_temp17_label.dev_attr.attr,
	&sensor_dev_attr_temp18_label.dev_attr.attr,
	&sensor_dev_attr_temp19_label.dev_attr.attr,
	&sensor_dev_attr_temp20_label.dev_attr.attr,

	&sensor_dev_attr_in0_label.dev_attr.attr,
	&sensor_dev_attr_in1_label.dev_attr.attr,
	&sensor_dev_attr_in2_label.dev_attr.attr,
	&sensor_dev_attr_in3_label.dev_attr.attr,
	&sensor_dev_attr_in4_label.dev_attr.attr,
	&sensor_dev_attr_in5_label.dev_attr.attr,
	&sensor_dev_attr_in6_label.dev_attr.attr,
	&sensor_dev_attr_in7_label.dev_attr.attr,
	&sensor_dev_attr_in8_label.dev_attr.attr,
	&sensor_dev_attr_in9_label.dev_attr.attr,
	&sensor_dev_attr_in10_label.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_in11_label.dev_attr.attr,
	&sensor_dev_attr_in12_label.dev_attr.attr,
	&sensor_dev_attr_in13_label.dev_attr.attr,
	&sensor_dev_attr_in14_label.dev_attr.attr,
	&sensor_dev_attr_in15_label.dev_attr.attr,
	&sensor_dev_attr_in16_label.dev_attr.attr,
	&sensor_dev_attr_in17_label.dev_attr.attr,
	&sensor_dev_attr_in18_label.dev_attr.attr,
	&sensor_dev_attr_in19_label.dev_attr.attr,

	&sensor_dev_attr_curr1_label.dev_attr.attr,
	&sensor_dev_attr_curr2_label.dev_attr.attr,
	&sensor_dev_attr_curr3_label.dev_attr.attr,
	&sensor_dev_attr_curr4_label.dev_attr.attr,
	&sensor_dev_attr_curr5_label.dev_attr.attr,
	&sensor_dev_attr_curr6_label.dev_attr.attr,
	&sensor_dev_attr_curr7_label.dev_attr.attr,
	&sensor_dev_attr_curr8_label.dev_attr.attr,
	&sensor_dev_attr_curr9_label.dev_attr.attr,
	&sensor_dev_attr_curr10_label.dev_attr.attr,
	&sensor_dev_attr_curr11_label.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_curr12_label.dev_attr.attr,
	&sensor_dev_attr_curr13_label.dev_attr.attr,
	&sensor_dev_attr_curr14_label.dev_attr.attr,
	&sensor_dev_attr_curr15_label.dev_attr.attr,
	&sensor_dev_attr_curr16_label.dev_attr.attr,
	&sensor_dev_attr_curr17_label.dev_attr.attr,
	&sensor_dev_attr_curr18_label.dev_attr.attr,
	&sensor_dev_attr_curr19_label.dev_attr.attr,
	&sensor_dev_attr_curr20_label.dev_attr.attr,

	&sensor_dev_attr_power1_label.dev_attr.attr,
	/* Extra attributes */
	&sensor_dev_attr_power2_label.dev_attr.attr,
	&sensor_dev_attr_power3_label.dev_attr.attr,
	&sensor_dev_attr_power4_label.dev_attr.attr,
	&sensor_dev_attr_power5_label.dev_attr.attr,

	NULL
};

static struct attribute_group label_attr_group = {
	.attrs = label_attributes,
	.is_visible = is_visible_extra,
};

const static struct attribute_group *extra_groups[] = {
	&label_attr_group,
	&status_attr_group,
	&temp_avg_attr_group,
	NULL
};

/*
 * Read a sensor value.
 */
int read_sensor_val(struct pf_dev_struct *pf_dev, enum hwmon_sensor_types type,
	u32 attr, int channel, long *val, char *status, bool *fresh)
{
	int ret = 0;
	long value = 0;
	long mapped_value = 0;
	enum ami_sensor_attribute ami_attr = SENSOR_ATTR_INVALID;
	enum ami_sensor_unit_mod unit_mod = SENSOR_UNIT_MOD_NONE;

	if(!pf_dev || !val)
		return -EINVAL;

	/* Handle chip config */
	if (type == hwmon_chip) {
		switch (attr) {
		case hwmon_chip_update_interval:
			*val = pf_dev->sensor_refresh;
			return 0;
		
		default:
			return -EINVAL;
		}
	}

	/*
	 * Update sensor readings.
	 */
	switch (type) {
	case hwmon_temp:
		ret = read_thermal_sensors(pf_dev, fresh);
		break;

	case hwmon_curr:
		ret = read_current_sensors(pf_dev, fresh);
		break;

	case hwmon_in:
		ret = read_voltage_sensors(pf_dev, fresh);
		break;

	case hwmon_power:
		ret = read_power_sensors(pf_dev, fresh);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (ret)
		return ret;

	ami_attr = to_ami_attribute(type, attr);

	switch (ami_attr) {
	/* Handle alarms */
	case SENSOR_ATTR_WARN_A:
	case SENSOR_ATTR_CRIT_A:
	case SENSOR_ATTR_FATAL_A:
	{
		long limit = 0;
		
		/* Fetch threshold */
		ret = get_sensor_value(pf_dev, to_ami_sensor_type(type),
					ami_attr, channel, &limit);
		if (ret)
			return ret;

		/* Fetch current sensor value */
		ret = get_sensor_value(pf_dev, to_ami_sensor_type(type),
					SENSOR_ATTR_INSTANT, channel, &value);
		if (ret)
			return ret;
		
		*val = (value >= limit);
	}
	break;

	/* Handle all other attributes */
	default:
	{
		/* Get the unit mod attribute value. */
		ret = get_sensor_value(pf_dev, to_ami_sensor_type(type),
					SENSOR_ATTR_UNIT_MOD, channel, &unit_mod);
		if (ret)
			return ret;
		
		/* Get the sensor attribute value. */
		ret = get_sensor_value(pf_dev, to_ami_sensor_type(type),
					ami_attr, channel, &value);
		if (ret)
			return ret;
		
		/*
		 * Check limit.
		 * TODO: We probably want to move this code somewhere else.
		 */
		if (ami_attr == SENSOR_ATTR_INSTANT) {
			long limit = 0;
			int limit_err = get_sensor_value(pf_dev, to_ami_sensor_type(type),
				SENSOR_ATTR_CRIT, channel, &limit);

			if (!limit_err && (value >= limit)) {
				DEV_CRIT_WARN(pf_dev->pci,
					"Sensor reading over critical threshold - killing all applications"
				);
				kill_pf_dev_apps(pf_dev, SIGBUS);
			}
		}

		/*
		* hwmon expects units in (https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface):
		* Voltage Unit: millivolt
		* Temperature Unit: millidegree Celsius
		* Current Unit: milliampere
		* Power Unit: microWatt
		*/
		switch (type) {
		case hwmon_temp:
		case hwmon_curr:
		case hwmon_in:
			ret = convert_milli_units(unit_mod, value, &mapped_value);
			if(!ret)
				*val = mapped_value;
			break;

		case hwmon_power:
			ret = convert_micro_units(unit_mod, value, &mapped_value);
			if(!ret)
				*val = mapped_value;
			break;

		default:
			ret = -EINVAL;
			break;
		}

		/* Check if the status was requested */
		if (status) {
			long s = 0;
			get_sensor_value(pf_dev, to_ami_sensor_type(type),
				SENSOR_ATTR_STATUS, channel, &s);
			sprintf(status, "%s\n", convert_sensor_status_name_map(s));
		}
	}
	break;
	}

	return ret;
}

/*
 * Initialize hwmon.
 */
int register_hwmon(struct device *dev, struct pf_dev_struct *pf_dev)
{
	int i = 0;
	int ret = 0;
	struct device *hwmon_dev = NULL;
	
	if (!dev || !pf_dev)
		return -EINVAL;
	
	/* Check if we have enough sensor channels for this device. */
	for (i = 0; i < pf_dev->num_sensor_repos; i++) {
		int max_sensors = -1;

		switch (pf_dev->sensor_repos[i].repo_type) {
		case SDR_TYPE_TEMP:
			max_sensors = ALVEO_NUM_TEMP_SENSORS;
			break;
		
		case SDR_TYPE_VOLTAGE:
			max_sensors = ALVEO_NUM_VOLTAGE_SENSORS;
			break;
		
		case SDR_TYPE_CURRENT:
			max_sensors = ALVEO_NUM_CURRENT_SENSORS;
			break;
		
		case SDR_TYPE_POWER:
			max_sensors = ALVEO_NUM_POWER_SENSORS;
			break;
		
		default:
			break;
		}

		if ((max_sensors != -1) && (pf_dev->sensor_repos[i].num_records > max_sensors))
			DEV_WARN(
				pf_dev->pci,
				"Not enough %s sensors for this device. Some data may be missing.",
				sdr_repo_type_to_str(pf_dev->sensor_repos[i].repo_type)
			);
	}

	hwmon_dev = devm_hwmon_device_register_with_info(
		dev, "Alveo", pf_dev, &alveo_hwmon_info, extra_groups);

	if(IS_ERR(hwmon_dev))
		return PTR_ERR(hwmon_dev);

	pf_dev->hwmon_dev = hwmon_dev;

	/* Extract hwmon ID for later reference. */
	ret = sscanf(hwmon_dev->kobj.name, "hwmon%d", &pf_dev->hwmon_id);

	if (ret == 1)
		ret = 0;

	return ret;
}

/*
 * Unregister hwmon device.
 */
void remove_hwmon(struct device *dev)
{
	if (!dev)
		return;

	devm_hwmon_device_unregister(dev);
}
