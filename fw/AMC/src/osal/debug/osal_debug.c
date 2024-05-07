/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the OSAL (Operating system abstraction layer) debug implementation.
 *
 * @file osal_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"
#include "osal.h"

#include "dal.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define OSAL_DBG_NAME       "OSAL_DBG"
#define MS_IN_SECS          ( 1000 )
#define SECS_IN_MIN         ( 60 )
#define MINS_IN_HOUR        ( 60 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;
static DAL_HDL pxOsalTop = NULL;


/******************************************************************************/
/* Private function declarations                                              */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats.
 */
static void vPrintStats( void );

/**
 * @brief   Debug function to clear this module's stats.
 */
static void vClearStats( void );

/**
 * @brief   Debug function to print user specified stats. 
 */
static void vPrintStatsCustom( void );

/**
 * @brief   Debug function of Osal Uptime function in ms. 
 */
static void vGetUptimeMs( void );
/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the OSAL debug access.
 */
void vOSAL_DebugInit( void )
{
    if( FALSE == iIsInitialised )
    {
        pxOsalTop = pxDAL_NewDirectory( "osal" );
        if( NULL != pxOsalTop )
        {
            pxDAL_NewDebugFunction( "print_all_stats",    pxOsalTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_all_stats",    pxOsalTop, vClearStats );
            pxDAL_NewDebugFunction( "get_uptime",         pxOsalTop, vGetUptimeMs );
            /* Allows user to select stat type/verbosity */
            pxDAL_NewDebugFunction( "print_stats_custom", pxOsalTop, vPrintStatsCustom );
        }

        iIsInitialised = TRUE;
    }
}

/**
 * @brief   Debug function to print this module's stats.
 */
static void vPrintStats( void )
{
    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_FULL, OSAL_STATS_TYPE_ALL );
}

/**
 * @brief   Debug function to clear this module's stats.
 */
static void vClearStats( void )
{
    vOSAL_ClearAllStats();
}

/**
 * @brief   Debug function to print user specified stats. 
 */
static void vPrintStatsCustom( void )
{
    int iStatVerbosity = 0;
    int iStatType = 0;
    
    vPLL_Printf( "\r\n     0:OS \
                  \r\n     1:Task \
                  \r\n     2:Mutex \
                  \r\n     3:Semaphore \
                  \r\n     4:Mailbox \
                  \r\n     5:Event \
                  \r\n     6:Timer \
                  \r\n     7:Memory" );

    if( OK != iDAL_GetIntInRange( "\r\nEnter stat type: ", &iStatType, -1, MAX_OSAL_STATS_TYPE_ALL ) )
    {
        PLL_DAL( OSAL_DBG_NAME, "Error retrieving stat type\r\n" );
    }

    if( ( OSAL_STATS_TYPE_OS     != iStatType ) &&
        ( OSAL_STATS_TYPE_MEMORY != iStatType ) )
    {
        vPLL_Printf( "\r\n     0:Counts only \
                       \r\n     1:Active only \
                       \r\n     2:Full" );

        if( OK != iDAL_GetIntInRange( "\r\nEnter verbosity level: ", &iStatVerbosity, -1, MAX_OSAL_STATS_VERBOSITY ) )
        {
            PLL_DAL( OSAL_DBG_NAME, "Error retrieving verbosity level\r\n" );
        }
    }
   
    vOSAL_PrintAllStats( ( OSAL_STATS_VERBOSITY )iStatVerbosity, ( OSAL_STATS_TYPE )iStatType );
}

/**
 * @brief   Debug function of Osal Uptime function in ms. 
 */
static void vGetUptimeMs( void )
{
    uint32_t ulMSecs = 0;
    uint32_t ulHrs = 0;
    uint32_t ulMins = 0;
    uint32_t ulSecs = 0;

    ulMSecs = ulOSAL_GetUptimeMs();

    ulSecs = ulMSecs / MS_IN_SECS;
    ulMSecs %= MS_IN_SECS;

    ulMins = ulSecs / SECS_IN_MIN;
    ulSecs %= SECS_IN_MIN;

    ulHrs = ulMins / MINS_IN_HOUR;
    ulMins %= MINS_IN_HOUR;

    PLL_DAL( OSAL_DBG_NAME, "\tUptime: %02d:%02d:%02d:%03d\r\n", ulHrs, ulMins, ulSecs, ulMSecs );
}
