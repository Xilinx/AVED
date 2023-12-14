/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the in band telemetry debug implementation
 *
 * @file in_band_telemetry_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "in_band_telemetry_debug.h"
#include "in_band_telemetry.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define AMC_IN_BAND_DBG_NAME        "AMC_IN_BAND_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxInBandTop = NULL;


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


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the in band telemetry application debug access
 */
void vIN_BAND_TELEMETRY_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxInBandTop = pxDAL_NewDirectory( "in_band_telemetry" );
        }
        else
        {
            pxInBandTop = pxDAL_NewSubDirectory( "in_band_telemetry", pxParentHandle );
        }

        if( NULL != pxInBandTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxInBandTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxInBandTop, vClearStats );
        }

        iIsInitialised = TRUE;
    }
}

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void )
{
    if( OK != iIN_BAND_TELEMETRY_PrintStatistics() )
    {
        PLL_DAL( AMC_IN_BAND_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iIN_BAND_TELEMETRY_ClearStatistics() )
    {
        PLL_DAL( AMC_IN_BAND_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

