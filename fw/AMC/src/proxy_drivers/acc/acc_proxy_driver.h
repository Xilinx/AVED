/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the API for the Alveo Clock Control ( ACC ) proxy driver
 *
 * @file acc_proxy_driver.h
 *
 */

#ifndef _ACC_PROXY_DRIVER_H_
#define _ACC_PROXY_DRIVER_H_

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include "standard.h"
#include "evl.h"

#define ACC_MAX_SHUTDOWN_SENSORS_MONITORED  ( 10 )


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    ACC_PROXY_DRIVER_EVENTS
 * @brief   Events raised by this proxy driver
 */
typedef enum ACC_PROXY_DRIVER_EVENTS
{
    ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ENABLED  = 0,
    ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_DISABLED,
    ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ACTIVATED,
    ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_CLOCK_REENABLED,

    MAX_ACC_PROXY_DRIVER_EVENTS

} ACC_PROXY_DRIVER_EVENTS;

/**
 * @enum    ACC_PROXY_DRIVER_SENSOR_TYPE
 * @brief   Type of sensor
 */
typedef enum ACC_PROXY_DRIVER_SENSOR_TYPE
{
    ACC_PROXY_DRIVER_SENSOR_TYPE_TEMPERATURE = 0,
    ACC_PROXY_DRIVER_SENSOR_TYPE_VOLTAGE,
    ACC_PROXY_DRIVER_SENSOR_TYPE_CURRENT,
    ACC_PROXY_DRIVER_SENSOR_TYPE_POWER,

    MAX_ACC_PROXY_DRIVER_SENSOR_TYPE

} ACC_PROXY_DRIVER_SENSOR_TYPE;


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct  ACC_PROXY_DRIVER_SHUTDOWN_SENSOR
 * @brief   Sensor ID and type monitored for shutdown
 */
typedef struct ACC_PROXY_DRIVER_SHUTDOWN_SENSOR
{
    uint8_t                         ucShutdownId;
    ACC_PROXY_DRIVER_SENSOR_TYPE    xShutdownType;

} ACC_PROXY_DRIVER_SHUTDOWN_SENSOR;

/**
 * @struct  ACC_PROXY_DRIVER_SHUTDOWN_DATA
 * @brief   Number of sensors and their data
 */
typedef struct ACC_PROXY_DRIVER_SHUTDOWN_SENSOR_DATA
{
    uint8_t                             ucNumSensors;
    ACC_PROXY_DRIVER_SHUTDOWN_SENSOR    xSensor[ ACC_MAX_SHUTDOWN_SENSORS_MONITORED ];

} ACC_PROXY_DRIVER_SHUTDOWN_SENSOR_DATA;


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/* Init functions *************************************************************/

/**
 * @brief   Main initialisation point for the ACC Proxy Driver
 *
 * @param   ucProxyId       Unique ID for this Proxy driver
 * @param   ulTaskPrio      Priority of the Proxy driver task ( if RR disabled )
 * @param   ulTaskStack     Stack size of the Proxy driver task
 * @param   pxShutdownData  Sensor IDs required to monitor for shutdown
 * @param   User            Clock Control Base Address
 *
 * @return  OK              Proxy driver initialised correctly
 *          ERROR           Proxy driver not initialised, or was already initialised
 *
 * @note    Proxy drivers can have 0 or more firmware interfaces
 */
int iACC_Initialise( uint8_t ucProxyId, uint32_t ulTaskPrio, uint32_t ulTaskStack,
                        ACC_PROXY_DRIVER_SHUTDOWN_SENSOR_DATA *pxShutdownData,
                        void *pvUCCBaseAddress );

/**
 * @brief   Bind into this proxy driver
 *
 * @param   pxCallback  Callback to bind into the proxy driver
 *
 * @return  OK          Callback successfully bound
 *          ERROR       Callback not bound
 *
 */
int iACC_BindCallback( EVL_CALLBACK *pxCallback );

/* Set functions **************************************************************/

/**
 * @brief   Print all the stats gathered in this driver
 *
 * @return  OK              Stats retrieved from proxy driver successfully
 *          ERROR           Stats not retrieved successfully
 *
 */
int iACC_PrintStatistics( void );

/**
 * @brief   Clear all the stats in this driver
 *
 * @return  OK              Stats cleared successfully
 *          ERROR           Stats not cleared successfully
 *
 */
int iACC_ClearStatistics( void );

/**
 * @brief   Enable Clock Shutdown for threshold crossing
 *
 * @return  OK              Shutdown enabled successfully
 *          ERROR           Shutdown enable unsuccessful
 *
 */
int iACC_EnableShutdown( void );

/**
 * @brief   Disable Clock Shutdown for threshold crossing
 *
 * @return  OK              Shutdown disabled successfully
 *          ERROR           Shutdown disable unsuccessful
 *
 */
int iACC_DisableShutdown( void );

/**
 * @brief   Report to ACC that a sensor has exceeded fatal limit
 *
 * @param   ucSensorId      The ID to identify the sensor
 * @param   ucSensorType    The type of sensor, temp, current, voltage or power
 *
 * @return  OK              Fatal Limit reported successfully
 *          ERROR           Fatal Limit report was unsuccessful
 */
int iACC_FatalLimitExceeded( uint8_t ucSensorId, uint8_t ucSensorType  );

/**
 * @brief   Report to ACC that a sensor has exceeded critical limit
 *
 * @param   ucSensorId      The ID to identify the sensor
 * @param   ucSensorType    The type of sensor, temp, current, voltage or power
 *
 * @return  OK              Critical Limit reported successfully
 *          ERROR           Critical Limit report was unsuccessful
 */
int iACC_CriticalLimitExceeded( uint8_t ucSensorId, uint8_t ucSensorType  );

/**
 * @brief   Report to ACC that a sensor has exceeded warning limit
 *
 * @param   ucSensorId      The ID to identify the sensor
 * @param   ucSensorType    The type of sensor, temp, current, voltage or power
 *
 * @return  OK              Warning Limit reported successfully
 *          ERROR           Warning Limit report was unsuccessful
 */
int iACC_WarningLimitExceeded( uint8_t ucSensorId, uint8_t ucSensorType  );

/**
 * @brief   Re-enable the clock if previously shutdown
 *
 * @return  OK              Reset Limit successful
 *          ERROR           Reset Limit was unsuccessful
 */
int iACC_ResetLimit( void  );

/**
 * @brief   Gets the current state of the proxy driver
 *
 * @param   pxState         Pointer to the state
 *
 * @return  OK              If successful
 *          ERROR           If not successful
 */
int iACC_GetState( MODULE_STATE *pxState );

#endif
