/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the GCQ debug implementation.
 *
 * @file fw_if_gcq_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "fw_if_gcq.h"
#include "fw_if_gcq_debug.h"

#include "dal.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FW_IF_GCQ_DBG_NAME        "FW_IF_GCQ_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;
static DAL_HDL pxFwIfGcqTop = NULL;


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


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the GCQ debug access.
 */
void vFW_IF_GCQ_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {   
        if( NULL == pxParentHandle )
        {
            pxFwIfGcqTop = pxDAL_NewDirectory( "fw_if_gcq" );
        }
        else
        {
            pxFwIfGcqTop = pxDAL_NewSubDirectory( "fw_if_gcq", pxParentHandle );
        }

        if( NULL != pxFwIfGcqTop )
        {
            pxDAL_NewDebugFunction( "print_all_stats", pxFwIfGcqTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_all_stats", pxFwIfGcqTop, vClearStats );
        }

        iIsInitialised = TRUE;
    }
}


/******************************************************************************/
/* Private function implementations                                           */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats.
 */
static void vPrintStats( void )
{
    if( OK != iFW_IF_GCQ_PrintStatistics() )
    {
        PLL_DAL( FW_IF_GCQ_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats.
 */
static void vClearStats( void )
{
    if( OK != iFW_IF_GCQ_ClearStatistics() )
    {
        PLL_DAL( FW_IF_GCQ_DBG_NAME, "Error printing statistics\r\n" );
    }
}
