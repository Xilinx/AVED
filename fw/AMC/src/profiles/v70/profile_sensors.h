/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the sensors profile for the V70
 *
 * @file profile_sensors.h
 *
 */

#ifndef _PROFILES_SENSORS_H_
#define _PROFILES_SENSORS_H_

#include "asc_proxy_driver.h"
#include "ina3221.h"
#include "isl68221.h"
#include "sys_mon.h"

#define PROFILE_SENSORS_NUM_SENSORS ( 5 )

/**
 * @brief   Wrapper for the iSYS_MON_ReadTemperature function, to keep it the standard driver API format
 *
 * @param   unused1     Unused parameter (normally i2c bus)
 * @param   unused2     Unused parameter (normally i2c address)
 * @param   unused3     Unused parameter (normally i2c channel)
 * @param   pfValue     Pointer to latest sensor value
 *
 * @return  The return value of iSYS_MON_ReadTemperature
 *
 * @note    No sanity checks, etc, are done - this function is solely a wrapper API
 */
static inline int iSYS_MON_WrappedReadTemperature( uint8_t unused1, uint8_t unused2, uint8_t unused3, float *pfValue )
{
    return iSYS_MON_ReadTemperature( pfValue );
}

/**
 * @brief   Function pointer called in profile to enable sensors
 *
 * @return  TRUE to indictate sensor is enabled
 */
static inline int iSensorIsEnabled( void )
{
    return TRUE;
}

ASC_PROXY_DRIVER_SENSOR_DATA PROFILE_SENSORS_SENSOR_DATA [ PROFILE_SENSORS_NUM_SENSORS ] = {
    { "12v_pex", 5, ASC_PROXY_DRIVER_SENSOR_BITFIELD_VOLTAGE | ASC_PROXY_DRIVER_SENSOR_BITFIELD_CURRENT | ASC_PROXY_DRIVER_SENSOR_BITFIELD_POWER, TRUE, 0x40, { ASC_SENSOR_I2C_BUS_INVALID, 0, 0, 0 }, iSensorIsEnabled, { NULL, iINA3221_ReadVoltage, iINA3221_ReadCurrent, iINA3221_ReadPower }, {
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_MILLI },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_MILLI },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE } }
    },
    { "3v3_pex", 10, ASC_PROXY_DRIVER_SENSOR_BITFIELD_VOLTAGE | ASC_PROXY_DRIVER_SENSOR_BITFIELD_CURRENT | ASC_PROXY_DRIVER_SENSOR_BITFIELD_POWER, TRUE, 0x40, { ASC_SENSOR_I2C_BUS_INVALID, 1, 1, 1 }, iSensorIsEnabled, { NULL, iINA3221_ReadVoltage, iINA3221_ReadCurrent, iINA3221_ReadPower }, {
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_MILLI },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_MILLI },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE } }
    },
    { "3V3AUX", 15, ASC_PROXY_DRIVER_SENSOR_BITFIELD_VOLTAGE, FALSE, 0x40, { ASC_SENSOR_I2C_BUS_INVALID, 2, ASC_SENSOR_I2C_BUS_INVALID, ASC_SENSOR_I2C_BUS_INVALID }, iSensorIsEnabled, { NULL, iINA3221_ReadVoltage, NULL, NULL }, {
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_MILLI },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_MILLI },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE } }
    },
    { "vccint", 35, ASC_PROXY_DRIVER_SENSOR_BITFIELD_TEMPERATURE | ASC_PROXY_DRIVER_SENSOR_BITFIELD_VOLTAGE | ASC_PROXY_DRIVER_SENSOR_BITFIELD_CURRENT, FALSE, 0x60, { 0, 0, 0, ASC_SENSOR_I2C_BUS_INVALID }, iSensorIsEnabled, { iISL68221_ReadTemperature, iISL68221_ReadVoltage, iISL68221_ReadCurrent, NULL }, {
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_MILLI },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE } }
    },
    { "FPGA_Temp", 40, ASC_PROXY_DRIVER_SENSOR_BITFIELD_TEMPERATURE, FALSE, 0, { 0, ASC_SENSOR_I2C_BUS_INVALID, ASC_SENSOR_I2C_BUS_INVALID, ASC_SENSOR_I2C_BUS_INVALID }, iSensorIsEnabled, { iSYS_MON_WrappedReadTemperature, NULL, NULL, NULL }, {
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE },
        { 0, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, ASC_SENSOR_INVALID_VAL, 0, 0, ASC_PROXY_DRIVER_SENSOR_STATUS_NOT_PRESENT, ASC_PROXY_DRIVER_SENSOR_UNIT_MOD_NONE } }
    }
};

#endif
