/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Event Library (EVL) debug implementation
 *
 * @file evl_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "evl_debug.h"
#include "evl.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define EVL_DBG_NAME      "EVL_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxEvlTop = NULL;
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
 * @brief   Debug function to retrieve the EVL log
 *
 * @return  N/A
 */
static void vGetLog( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the EVL debug access
 */
void vEVL_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxEvlTop = pxDAL_NewDirectory( "evl" );
        }
        else
        {
            pxEvlTop = pxDAL_NewSubDirectory( "evl", pxParentHandle );
        }

        if( NULL != pxEvlTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxEvlTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxEvlTop, vClearStats );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxEvlTop );

            if( NULL!= pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_log", pxGetDir, vGetLog );
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
    if( OK != iEVL_PrintStatistics() )
    {
        PLL_DAL( EVL_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iEVL_ClearStatistics() )
    {
        PLL_DAL( EVL_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to retrieve the EVL log
 */
static void vGetLog( void )
{
    if( OK != iEVL_PrintLog() )
    {
        PLL_DAL( EVL_DBG_NAME, "Error retrieving EVL Log\r\n" );
    }
}

