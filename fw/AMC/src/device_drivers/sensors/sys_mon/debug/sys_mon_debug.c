/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the System Monitoring (SYS_MON) debug implementation
 *
 * @file sys_mon_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "sys_mon_debug.h"
#include "sys_mon.h"

/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define SYS_MON_DBG_NAME        "SYS_MON_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxSysMonTop = NULL;
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
 * @brief   Debug function to retrieve the temperature
 *
 * @return  N/A
 */
static void vGetTemperature( void );

/**
 * @brief   Debug function to retrieve the voltage
 *
 * @return  N/A
 */
static void vGetVoltage( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the SYS_MON debug access
 */
void vSYS_MON_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxSysMonTop = pxDAL_NewDirectory( "sys_mon" );
        }
        else
        {
            pxSysMonTop = pxDAL_NewSubDirectory( "sys_mon", pxParentHandle );
        }

        if( NULL != pxSysMonTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxSysMonTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxSysMonTop, vClearStats );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxSysMonTop );

            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_temperature", pxGetDir, vGetTemperature );
                pxDAL_NewDebugFunction( "get_voltage",     pxGetDir, vGetVoltage );
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
    if( OK != iSYS_MON_PrintStatistics() )
    {
        PLL_DAL( SYS_MON_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iSYS_MON_ClearStatistics() )
    {
        PLL_DAL( SYS_MON_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to retrieve the temperature
 */
static void vGetTemperature( void )
{
    float fTemperatureC = 0.0;

    if( OK != iSYS_MON_ReadTemperature( &fTemperatureC ) )
    {
        PLL_DAL( SYS_MON_DBG_NAME, "Error retrieving SYS_MON Temperature\r\n" );
    }
    else
    {
        PLL_DAL( SYS_MON_DBG_NAME, "Temperature (C): %f\r\n", fTemperatureC );
    }
}

/**
 * @brief   Debug function to retrieve the voltage
 */
static void vGetVoltage( void )
{
    float fVoltageMv = 0.0;
    int   iVType    = 0;

    if( OK != iDAL_GetIntInRange( "Enter voltage type:", &iVType, 0, MAX_SYS_MON_VOLTAGE ) )
    {
        PLL_DAL( SYS_MON_DBG_NAME, "Error retrieving voltage type\r\n" );
    }
    else if( OK != iSYS_MON_ReadVoltage( ( SYS_MON_VOLTAGES_ENUM )iVType, &fVoltageMv ) )
    {
        PLL_DAL( SYS_MON_DBG_NAME, "Error retrieving SYS_MON Voltage\r\n" );
    }
    else
    {
        PLL_DAL( SYS_MON_DBG_NAME, "Voltage (V): %f\r\n", fVoltageMv );
    }
}

