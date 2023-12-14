/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the ASDM debug implementation
 *
 * @file asdm_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "asdm_debug.h"
#include "asdm.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define AMC_ASDM_DBG_NAME        "AMC_IN_BAND_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxInBandTop = NULL;


/******************************************************************************/
/* Private function declarations                                              */
/******************************************************************************/

/**
 * @brief   Debug function to print the ASDM repos
 *
 * @return  N/A
 */
static void vASDM_DumpRepo( void );

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
 * @brief   Initialise the ASDM application debug access
 */
void vASDM_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxInBandTop = pxDAL_NewDirectory( "asdm" );
        }
        else
        {
            pxInBandTop = pxDAL_NewSubDirectory( "asdm", pxParentHandle );
        }

        if( NULL != pxInBandTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxInBandTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxInBandTop, vClearStats );
            pxDAL_NewDebugFunction( "dump_repo", pxInBandTop, vASDM_DumpRepo );
        }

        iIsInitialised = TRUE;
    }
}

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void )
{
    if( OK != iASDM_PrintStatistics() )
    {
        PLL_DAL( AMC_ASDM_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iASDM_ClearStatistics() )
    {
        PLL_DAL( AMC_ASDM_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to print out the internal ASDM repo data
 */
static void vASDM_DumpRepo( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this repo index instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMC_ASDM_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        if( OK != iASDM_PrintAsdmRepoData( iInstance ) )
        {
             PLL_DAL( AMC_ASDM_DBG_NAME, "Failed to parse repo at index [%d]\r\n", iInstance );
        }
    }
}
