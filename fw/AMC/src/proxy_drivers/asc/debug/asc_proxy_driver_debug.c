/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Sensor Control (ASC) debug implementation
 *
 * @file asc_proxy_driver_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "asc_proxy_driver_debug.h"
#include "asc_proxy_driver.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define ASC_DBG_NAME        "ASC_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxAscTop = NULL;
static DAL_HDL pxSetDir = NULL;
static DAL_HDL pxGetDir = NULL;


/******************************************************************************/
/* Private function declarations                                              */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats
 *
 * @return  N/A
 */
static void vPrintStats( void );

/**
 * @brief   Debug function to clear this module's stats
 *
 * @return  N/A
 */
static void vClearStats( void );

/**
 * @brief   Debug function to bind a callback to this module
 *
 * @return  N/A
 */
static void vBindCallbacks( void );

/**
 * @brief   Debug function to get all sensors
 *
 * @return  N/A
 */
static void vGetAllSensors( void );

/**
 * @brief   Debug function to get a single sensor by its ID
 *
 * @return  N/A
 */
static void vGetSingleSensorById( void );

/**
 * @brief   Debug function to get a single sensor by its name
 *
 * @return  N/A
 */
static void vGetSingleSensorByName( void );

/**
 * @brief   Debug function to reset all sensors
 *
 * @return  N/A
 */
static void vResetAllSensors( void );

/**
 * @brief   Debug function to reset a single sensor by its ID
 *
 * @return  N/A
 */
static void vResetSingleSensorById( void );

/**
 * @brief   Debug function to reset a single sensor by its name
 *
 * @return  N/A
 */
static void vResetSingleSensorByName( void );

/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 *
 * @param   pxSignal     Event raised
 *
 * @return  OK if no errors were raised in the callback
 *          ERROR if an error was raised in the callback
 *
 */
static int iTestCallback( EVL_SIGNAL *pxSignal );

/**
 * @brief   Display the full data of a single sensor
 *
 * @param   pxSensor    Pointer to sensor data to print
 *
 * @return  N/A
 */
static void vPrintSensorData( ASC_PROXY_DRIVER_SENSOR_DATA *pxSensor );


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static const char *pcTypeStrings[ MAX_ASC_PROXY_DRIVER_SENSOR_TYPE ] =
{
    "Temperature", "Voltage", "Current", "Power"
};

static const char *pcStatusStrings[ MAX_ASC_PROXY_DRIVER_SENSOR_STATUS ] =
{
    "Not present", "Present and valid", "No data", "Not available"
};

static const char *pcModStrings[ MAX_ASC_PROXY_DRIVER_SENSOR_UNIT_MOD ] =
{
    "Mega", "Kilo", "None", "Milli", "Micro"
};


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the ASC debug access
 */
void vASC_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxAscTop = pxDAL_NewDirectory( "asc" );
        }
        else
        {
            pxAscTop = pxDAL_NewSubDirectory( "asc", pxParentHandle );
        }

        if( NULL != pxAscTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxAscTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxAscTop, vClearStats );
            pxDAL_NewDebugFunction( "bind_callbacks", pxAscTop, vBindCallbacks );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxAscTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxAscTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "reset_all_sensors",    pxSetDir, vResetAllSensors );
                pxDAL_NewDebugFunction( "reset_sensor_by_id",   pxSetDir, vResetSingleSensorById );
                pxDAL_NewDebugFunction( "reset_sensor_by_name", pxSetDir, vResetSingleSensorByName );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_all_sensors",    pxGetDir, vGetAllSensors );
                pxDAL_NewDebugFunction( "get_sensor_by_id",   pxGetDir, vGetSingleSensorById );
                pxDAL_NewDebugFunction( "get_sensor_by_name", pxGetDir, vGetSingleSensorByName );
            }
        }

        iIsInitialised = TRUE;
    }
}

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void )
{
    if( OK != iASC_PrintStatistics() )
    {
        PLL_DAL( ASC_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iASC_ClearStatistics() )
    {
        PLL_DAL( ASC_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to bind a callback to this module
 */
static void vBindCallbacks( void )
{
    if( OK != iASC_BindCallback( &iTestCallback ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( ASC_DBG_NAME, "Callback bound\r\n" );
    }
}

/**
 * @brief   Debug function to reset all sensors
 */
static void vResetAllSensors( void )
{
    if( OK != iASC_ResetAllSensorData() )
    {
        PLL_DAL( ASC_DBG_NAME, "Error resetting sensors data\r\n" );
    }
    else
    {
        PLL_DAL( ASC_DBG_NAME, "All sensor data reset\r\n" );
    }
}

/**
 * @brief   Debug function to reset a single sensor by its ID
 */
static void vResetSingleSensorById( void )
{
    int iId = 0;

    if( OK != iDAL_GetIntInRange( "Enter sensor ID:", &iId, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error retrieving sensor ID\r\n" );
    }
    else if( OK != iASC_ResetSingleSensorDataById( ( uint8_t )iId ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error resetting sensor %d data\r\n", iId );
    }
    else
    {
        PLL_DAL( ASC_DBG_NAME, "Sensor %d data reset\r\n", iId );
    }
}

/**
 * @brief   Debug function to reset a single sensor by its name
 */
static void vResetSingleSensorByName( void )
{
    char pcName[ ASC_SENSOR_NAME_MAX ] = { 0 };

    if( OK != iDAL_GetString( "Enter sensor name:", pcName, ASC_SENSOR_NAME_MAX ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error retrieving sensor name\r\n" );
    }
    else if( OK != iASC_ResetSingleSensorDataByName( pcName ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error resetting sensor %s data\r\n", pcName );
    }
    else
    {
        PLL_DAL( ASC_DBG_NAME, "Sensor %s data reset\r\n", pcName );
    }
}

/**
 * @brief   Debug function to get all sensors
 */
static void vGetAllSensors( void )
{
    int iMaxSensors = 0;
    if( OK != iDAL_GetIntInRange( "Enter max number of sensors:", &iMaxSensors, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error retrieving number of sensors\r\n" );
    }
    else
    {
        PLL_DAL( ASC_DBG_NAME, "Allocating memory for %d sensors\r\n", iMaxSensors );
        ASC_PROXY_DRIVER_SENSOR_DATA *pxSensors = pvOSAL_MemAlloc( iMaxSensors * 
                                                                  sizeof( ASC_PROXY_DRIVER_SENSOR_DATA ) );

        if( NULL == pxSensors )
        {
            PLL_DAL( ASC_DBG_NAME, "Unable to allocate data for %d sensors\r\n", iMaxSensors );
        }
        else
        {
            uint8_t uNumSensors = ( uint8_t )iMaxSensors;
            if( OK != iASC_GetAllSensorData( pxSensors, &uNumSensors ) )
            {
                PLL_DAL( ASC_DBG_NAME, "Error retrieving sensor data\r\n" );
            }
            else
            {
                PLL_DAL( ASC_DBG_NAME, "Retrieved %d sensors\r\n", uNumSensors );

                int i = 0;
                for( i = 0; i < uNumSensors; i++ )
                {
                    PLL_DAL( ASC_DBG_NAME, "Sensor %d:\r\n", i );
                    PLL_DAL( ASC_DBG_NAME, "****************************************************\r\n" );
                    vPrintSensorData( &pxSensors[ i ] );
                }
            }
            vOSAL_MemFree( ( void** )&pxSensors );
        }
    }
}

/**
 * @brief   Debug function to get a single sensor by its ID
 */
static void vGetSingleSensorById( void )
{
    int iId = 0;

    if( OK != iDAL_GetIntInRange( "Enter sensor ID:", &iId, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error retrieving sensor ID\r\n" );
    }
    else
    {
        ASC_PROXY_DRIVER_SENSOR_DATA xSensor = { { 0 } };
        
        if( OK != iASC_GetSingleSensorDataById( ( uint8_t )iId, &xSensor ) )
        {
            PLL_DAL( ASC_DBG_NAME, "Error retrieving sensor %d data\r\n", iId );
        }
        else
        {
            vPrintSensorData( &xSensor );
        }
    }
}

/**
 * @brief   Debug function to get a single sensor by its name
 */
static void vGetSingleSensorByName( void )
{
    char pcName[ ASC_SENSOR_NAME_MAX ] = { 0 };

    if( OK != iDAL_GetString( "Enter sensor name:", pcName, ASC_SENSOR_NAME_MAX ) )
    {
        PLL_DAL( ASC_DBG_NAME, "Error retrieving sensor name\r\n" );
    }
    else
    {
        ASC_PROXY_DRIVER_SENSOR_DATA xSensor = { { 0 } };
        
        if( OK != iASC_GetSingleSensorDataByName( pcName, &xSensor ) )
        {
            PLL_DAL( ASC_DBG_NAME, "Error retrieving sensor %s data\r\n", pcName );
        }
        else
        {
            vPrintSensorData( &xSensor );
        }
    }
}

/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 */
static int iTestCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( NULL != pxSignal )
    {
        PLL_DAL( ASC_DBG_NAME, "ASC Signal raised: Module[%d], Type[%d], Instance[%d]\r\n",
                 pxSignal->ucModule, pxSignal->ucEventType, pxSignal->ucInstance );
        iStatus = OK;
    }

    return iStatus;
}

/**
 * @brief   Display the full data of a single sensor
 */
static void vPrintSensorData( ASC_PROXY_DRIVER_SENSOR_DATA *pxSensor )
{
    if( NULL != pxSensor )
    {
        int i = 0;
            
        PLL_DAL( ASC_DBG_NAME, "====================================================\r\n" );
        PLL_DAL( ASC_DBG_NAME, "Sensor . . . . . . . . . . . . . . %s\r\n", pxSensor->pcSensorName );
        PLL_DAL( ASC_DBG_NAME, "- ID . . . . . . . . . . . . . . . %d\r\n", pxSensor->ucSensorId );
        PLL_DAL( ASC_DBG_NAME, "- Type . . . . . . . . . . . . . . 0x%02X\r\n", pxSensor->ucSensorType );
        PLL_DAL( ASC_DBG_NAME, "- Address. . . . . . . . . . . . . 0x%02X\r\n", pxSensor->ucSensorAddress );
        for( i = 0; i < MAX_ASC_PROXY_DRIVER_SENSOR_TYPE; i++ )
        {
            PLL_DAL( ASC_DBG_NAME, "----------------------------------------------------\r\n" );
            PLL_DAL( ASC_DBG_NAME, "Type: %s\r\n", pcTypeStrings[ i ] );
            PLL_DAL( ASC_DBG_NAME, "- Channel number . . . . . . . . . %d\r\n", pxSensor->ucChannelNumber[ i ] );
            PLL_DAL( ASC_DBG_NAME, "- Driver callback. . . . . . . . . %s\r\n",
                   ( NULL != pxSensor->ppxReadSensorFunc[ i ] ) ? ( "Valid" ):( "NULL" ) );
            PLL_DAL( ASC_DBG_NAME, "- reading: value . . . . . . . . . %d\r\n", pxSensor->pxReadings[ i ].ulSensorValue );
            PLL_DAL( ASC_DBG_NAME, "- reading: lower Warning limit . . %d\r\n", pxSensor->pxReadings[ i ].ulLowerWarningLimit );
            PLL_DAL( ASC_DBG_NAME, "- reading: lower critical limit. . %d\r\n", pxSensor->pxReadings[ i ].ulLowerCriticalLimit );
            PLL_DAL( ASC_DBG_NAME, "- reading: lower fatal limit . . . %d\r\n", pxSensor->pxReadings[ i ].ulLowerFatalLimit );
            PLL_DAL( ASC_DBG_NAME, "- reading: upper Warning limit . . %d\r\n", pxSensor->pxReadings[ i ].ulUpperWarningLimit );
            PLL_DAL( ASC_DBG_NAME, "- reading: upper critical limit. . %d\r\n", pxSensor->pxReadings[ i ].ulUpperCriticalLimit );
            PLL_DAL( ASC_DBG_NAME, "- reading: upper fatal limit . . . %d\r\n", pxSensor->pxReadings[ i ].ulUpperFatalLimit );
            PLL_DAL( ASC_DBG_NAME, "- reading: average sensor value. . %d\r\n", pxSensor->pxReadings[ i ].ulAverageSensorValue );
            PLL_DAL( ASC_DBG_NAME, "- reading: max sensor value. . . . %d\r\n", pxSensor->pxReadings[ i ].ulMaxSensorValue );
            PLL_DAL( ASC_DBG_NAME, "- reading: sensor status . . . . . %s\r\n",
                     pcStatusStrings[ pxSensor->pxReadings[ i ].xSensorStatus ] );
            PLL_DAL( ASC_DBG_NAME, "- reading: sensor unit modifier. . %s\r\n",
                     pcModStrings[  pxSensor->pxReadings[ i ].xSensorUnitModifier ] );
        }
    }
    else
    {
        PLL_DAL( ASC_DBG_NAME, "Error - invalid sensor data\r\n" );
    }
}
