/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the EMMC debug implementation.
 *
 * @file fw_if_emmc_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "fw_if_emmc.h"
#include "fw_if_emmc_debug.h"

#include "dal.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FW_IF_EMMC_DBG_NAME       "FW_IF_EMMC_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iInitialised = FALSE;
static DAL_HDL pxFwIfEmmcTop = NULL;


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
 * @brief    Initialise the FW IF EMMC debug access
 */
void vFW_IF_EMMC_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iInitialised )
    {   
        if( NULL == pxParentHandle )
        {
            pxFwIfEmmcTop = pxDAL_NewDirectory( "fw_if_emmc" );
        }
        else
        {
            pxFwIfEmmcTop = pxDAL_NewSubDirectory( "fw_if_emmc", pxParentHandle );
        }

        if( NULL != pxFwIfEmmcTop )
        {
            pxDAL_NewDebugFunction( "print_all_stats", pxFwIfEmmcTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_all_stats", pxFwIfEmmcTop, vClearStats );
        }
        
        iInitialised = TRUE;
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
    if( OK != iFW_IF_EMMC_PrintStatistics() )
    {
        PLL_DAL( FW_IF_EMMC_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats.
 */
static void vClearStats( void )
{
    if( OK != iFW_IF_EMMC_ClearStatistics() )
    {
        PLL_DAL( FW_IF_EMMC_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

