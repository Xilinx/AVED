/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Built in Monitoring (BIM) debug implementation
 *
 * @file bim_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "bim_debug.h"
#include "bim.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define BIM_DBG_NAME      "BIM_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxPllTop = NULL;
static DAL_HDL pxGetDir = NULL;
static DAL_HDL pxSetDir = NULL;

/* BIM_STATUS string mapping */
static const char *pcBimStatusStr[ ] = { "HEALTHY", "DEGRADED", "CRITICAL", "FATAL" };


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
 * @brief   Debug function to get the overall health status of the AMC
 *
 * @return  N/A
 */
static void vBIM_GetOverallHealthStatus( void );

/**
 * @brief   Debug function to set the overall health status of the AMC
 *
 * @return  N/A
 */
static void vBIM_SetOverallHealthStatus( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the BIM debug access
 */
void vBIM_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxPllTop = pxDAL_NewDirectory( "bim" );
        }
        else
        {
            pxPllTop = pxDAL_NewSubDirectory( "bim", pxParentHandle );
        }

        if( NULL != pxPllTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxPllTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxPllTop, vClearStats );

            pxSetDir = pxDAL_NewSubDirectory( "sets", pxPllTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxPllTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_overall_status", pxSetDir, vBIM_SetOverallHealthStatus );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_overall_status", pxGetDir, vBIM_GetOverallHealthStatus );
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
    if( OK != iBIM_PrintStatistics() )
    {
        PLL_DAL( BIM_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iBIM_ClearStatistics() )
    {
        PLL_DAL( BIM_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to get the overall health status of the AMC
 */
static void vBIM_GetOverallHealthStatus( void )
{
    BIM_STATUS *pxStatus = pvOSAL_MemAlloc( sizeof( BIM_STATUS ) );
    
    if( NULL != pxStatus )
    {
        if( OK != iBIM_GetOverallHealthStatus( pxStatus ) )
        {
            PLL_DAL( BIM_DBG_NAME, "Error status\r\n" );
        }
        else
        {
            const char *pcStatusStr = "UNKNOWN";
            if( ( 0 <= *pxStatus ) &&
                ( MAX_BIM_STATUS > *pxStatus ) )
            {
                pcStatusStr = pcBimStatusStr[ *pxStatus ];
            }
            
            PLL_DAL( BIM_DBG_NAME, "Current status: %d - %s\r\n", *pxStatus, pcStatusStr );
        }

        vOSAL_MemFree( ( void** ) &pxStatus );
    }
}

/**
 * @brief   Debug function to set the overall health status of the AMC
 */
static void vBIM_SetOverallHealthStatus( void )
{
    int iStatus = 0;
    
    vPLL_Printf( "\r\n     %d: BIM_STATUS_HEALTHY",  BIM_STATUS_HEALTHY );
    vPLL_Printf( "\r\n     %d: BIM_STATUS_DEGRADED", BIM_STATUS_DEGRADED );
    vPLL_Printf( "\r\n     %d: BIM_STATUS_CRITICAL", BIM_STATUS_CRITICAL );
    vPLL_Printf( "\r\n     %d: BIM_STATUS_FATAL",    BIM_STATUS_FATAL );

    if( OK != iDAL_GetIntInRange( "\r\nEnter status : ", &iStatus, 0, MAX_PLL_OUTPUT_LEVEL ) )
    {
        PLL_DAL( BIM_DBG_NAME, "Error retrieving status\r\n" );
    }

    if( OK != iBIM_SetOverallHealthStatus( ( BIM_STATUS )iStatus ) )
    {
        PLL_DAL( BIM_DBG_NAME, "Error setting status\r\n" );
    }
    else
    {
        PLL_DAL( BIM_DBG_NAME, "Successfuly set status\r\n" );
    }
}
