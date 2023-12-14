/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Clock Control (ACC) debug implementation
 *
 * @file acc_proxy_driver_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "acc_proxy_driver_debug.h"
#include "acc_proxy_driver.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define ACC_DBG_NAME        "ACC_DBG"
#define ACC_MAX_SENSOR_ID   ( 255 )
#define ACC_MAX_SENSOR_TYPE ( 4 )

/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxAccTop = NULL;
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
 * @brief   Debug function to enable clock shutdown
 *
 * @return  N/A
 * */
static void vEnableShutdown( void );

/**
 * @brief   Debug function to disable clock shutdown
 *
 * @return  N/A
 */
static void vDisableShutdown( void );

/**
 * @brief   Debug function to simulate a warning exceeded limit
 *
 * @return  N/A
 */
static void vWarningLimitExceeded( void );

/**
 * @brief   Debug function to simulate a critical exceeded limit
 *
 * @return  N/A
 */
static void vCriticalLimitExceeded( void );

/**
 * @brief   Debug function to simulate a fatal exceeded limit
 *
 * @return  N/A
 */
static void vFatalLimitExceeded( void );

/**
 * @brief   Debug function to call ACC clock reset function
 *
 * @return  N/A
 */
static void vResetLimit( void );

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


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the ACC debug access
 */
void vACC_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxAccTop = pxDAL_NewDirectory( "acc" );
        }
        else
        {
            pxAccTop = pxDAL_NewSubDirectory( "acc", pxParentHandle );
        }

        if( NULL != pxAccTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxAccTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxAccTop, vClearStats );
            pxDAL_NewDebugFunction( "bind_callbacks", pxAccTop, vBindCallbacks );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxAccTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxAccTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "enable_shutdown", pxSetDir, vEnableShutdown );
                pxDAL_NewDebugFunction( "disable_shutdown", pxSetDir, vDisableShutdown );
                pxDAL_NewDebugFunction( "warning_exceeded", pxSetDir, vWarningLimitExceeded );
                pxDAL_NewDebugFunction( "critical_exceeded", pxSetDir, vCriticalLimitExceeded );
                pxDAL_NewDebugFunction( "fatal_exceeded", pxSetDir, vFatalLimitExceeded );
                pxDAL_NewDebugFunction( "reset_limit", pxSetDir, vResetLimit );
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
    if( OK != iACC_PrintStatistics() )
    {
        PLL_DAL( ACC_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iACC_ClearStatistics() )
    {
        PLL_DAL( ACC_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to enable clock shutdown
 */
static void vEnableShutdown( void )
{
    if( OK != iACC_EnableShutdown() )
    {
        PLL_DAL( ACC_DBG_NAME, "Error calling EnableShutdown\r\n" );
    }
}

/**
 * @brief   Debug function to disable clock shutdown
 */
static void vDisableShutdown( void )
{
    if( OK != iACC_DisableShutdown() )
    {
        PLL_DAL( ACC_DBG_NAME, "Error calling DisableShutdown\r\n" );
    }
}

/**
 * @brief   Debug function to bind a callback to this module
 */
static void vBindCallbacks( void )
{
    if( OK != iACC_BindCallback( &iTestCallback ) )
    {
        PLL_DAL( ACC_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( ACC_DBG_NAME, "Callback bound\r\n" );
    }
}

/**
 * @brief   Debug function to call ACC Warning Limit Exceeded
 */
static void vWarningLimitExceeded( void )
{
    int iSensorId   = 0;
    int iSensorType = 0;

    if( OK != iDAL_GetIntInRange( "Enter Sensor Id (0 - 255):", &iSensorId, 0, ACC_MAX_SENSOR_ID ) )
    {
        PLL_DAL( ACC_DBG_NAME, "Error retrieving Sensor Id\r\n" );
    }
    else
    {
        if( OK != iDAL_GetIntInRange( "Enter Sensor Type (0-Temp, 1-Voltage, 2-Current, 4-Power):", &iSensorType, 0, ACC_MAX_SENSOR_TYPE ) )
        {
            PLL_DAL( ACC_DBG_NAME, "Error retrieving Sensor Type\r\n" );
        }
        else
        {
            if( OK != iACC_WarningLimitExceeded( ( uint8_t )iSensorId, ( uint32_t )iSensorType ) )
            {
                PLL_DAL( ACC_DBG_NAME, "Error calling iACC_WarningLimitExceeded\r\n" );
            }
            else
            {
                PLL_DAL( ACC_DBG_NAME, "iACC_WarningLimitExceeded called successfully \r\n" );
            }
        }
    }
}

/**
 * @brief   Debug function to call ACC Critical Limit Exceeded
 */
static void vCriticalLimitExceeded( void )
{
    int iSensorId   = 0;
    int iSensorType = 0;

    if( OK != iDAL_GetIntInRange( "Enter Sensor Id (0 - 255):", &iSensorId, 0, ACC_MAX_SENSOR_ID ) )
    {
        PLL_DAL( ACC_DBG_NAME, "Error retrieving Sensor Id\r\n" );
    }
    else
    {
        if( OK != iDAL_GetIntInRange( "Enter Sensor Type (0-Temp, 1-Voltage, 2-Current, 4-Power):", &iSensorType, 0, ACC_MAX_SENSOR_TYPE ) )
        {
            PLL_DAL( ACC_DBG_NAME, "Error retrieving Sensor Type\r\n" );
        }
        else
        {

            if( OK != iACC_CriticalLimitExceeded( ( uint8_t )iSensorId, ( uint32_t )iSensorType ) )
            {
                PLL_DAL( ACC_DBG_NAME, "Error calling iACC_CriticalLimitExceeded\r\n" );
            }
            else
            {
                PLL_DAL( ACC_DBG_NAME, "iACC_CriticalLimitExceeded called successfully \r\n" );
            }
        }
    }
}

/**
 * @brief   Debug function to call ACC Fatal Limit Exceeded
 */
static void vFatalLimitExceeded( void )
{
    int iSensorId   = 0;
    int iSensorType = 0;

    if( OK != iDAL_GetIntInRange( "Enter Sensor Id (0 - 255):", &iSensorId, 0, ACC_MAX_SENSOR_ID ) )
    {
        PLL_DAL( ACC_DBG_NAME, "Error retrieving Sensor Id\r\n" );
    }
    else
    {
        if( OK != iDAL_GetIntInRange( "Enter Sensor Type (0-Temp, 1-Voltage, 2-Current, 4-Power):", &iSensorType, 0, ACC_MAX_SENSOR_TYPE ) )
        {
            PLL_DAL( ACC_DBG_NAME, "Error retrieving Sensor Type\r\n" );
        }
        else
        {

            if( OK != iACC_FatalLimitExceeded( ( uint8_t )iSensorId, ( uint32_t )iSensorType ) )
            {
                PLL_DAL( ACC_DBG_NAME, "Error calling iACC_FatalLimitExceeded\r\n" );
            }
            else
            {
                PLL_DAL( ACC_DBG_NAME, "iACC_FatalLimitExceeded called successfully \r\n" );
            }
        }
    }
}

/**
 * @brief   Debug function to call ACC clock reset function
 */
static void vResetLimit( void )
{
    if( OK != iACC_ResetLimit( ) )
    {
        PLL_DAL( ACC_DBG_NAME, "Error calling iACC_ResetLimit\r\n" );
    }
    else
    {
        PLL_DAL( ACC_DBG_NAME, "iACC_ResetLimit called successfully \r\n" );
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
        PLL_DAL( ACC_DBG_NAME, "ACC Signal raised: Module[%d], Type[%d], Instance[%d]\r\n",
                 pxSignal->ucModule, pxSignal->ucEventType, pxSignal->ucInstance );
        iStatus = OK;
    }

    return iStatus;
}

