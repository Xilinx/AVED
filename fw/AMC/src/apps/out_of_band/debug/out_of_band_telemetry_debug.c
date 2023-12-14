/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the in band telemetry debug implementation
 *
 * @file out_of_band_telemetry_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "out_of_band_telemetry_debug.h"
#include "out_of_band_telemetry.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define AMC_OUT_OF_BAND_DBG_NAME        "AMC_OUT_OF_BAND_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxOutOfBandTop = NULL;


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
void vOUT_OF_BAND_TELEMETRY_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxOutOfBandTop = pxDAL_NewDirectory( "out_of_band_telemetry" );
        }
        else
        {
            pxOutOfBandTop = pxDAL_NewSubDirectory( "out_of_band_telemetry", pxParentHandle );
        }

        if( NULL != pxOutOfBandTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxOutOfBandTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxOutOfBandTop, vClearStats );
        }

        iIsInitialised = TRUE;
    }
}

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void )
{
    if( OK != iOUT_OF_BAND_TELEMETRY_PrintStatistics() )
    {
        PLL_DAL( AMC_OUT_OF_BAND_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iOUT_OF_BAND_TELEMETRY_ClearStatistics() )
    {
        PLL_DAL( AMC_OUT_OF_BAND_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

