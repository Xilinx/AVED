/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the OSPI debug implementation.
 *
 * @file fw_if_ospi_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "fw_if_ospi.h"
#include "fw_if_ospi_debug.h"

#include "dal.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FW_IF_OSPI_DBG_NAME       "FW_IF_OSPI_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;
static DAL_HDL pxFwIfOspiTop = NULL;


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
 * @brief   Initialise the OSPI debug access.
 */
void vFW_IF_OSPI_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {   
        if( NULL == pxParentHandle )
        {
            pxFwIfOspiTop = pxDAL_NewDirectory( "fw_if_ospi" );
        }
        else
        {
            pxFwIfOspiTop = pxDAL_NewSubDirectory( "fw_if_ospi", pxParentHandle );
        }

        if( NULL != pxFwIfOspiTop )
        {
            pxDAL_NewDebugFunction( "print_all_stats", pxFwIfOspiTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_all_stats", pxFwIfOspiTop, vClearStats );
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
    if( OK != iFW_IF_OSPI_PrintStatistics() )
    {
        PLL_DAL( FW_IF_OSPI_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats.
 */
static void vClearStats( void )
{
    if( OK != iFW_IF_OSPI_ClearStatistics() )
    {
        PLL_DAL( FW_IF_OSPI_DBG_NAME, "Error printing statistics\r\n" );
    }
}
