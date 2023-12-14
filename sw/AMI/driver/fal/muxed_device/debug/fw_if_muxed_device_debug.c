/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF Muxed Device debug implementation.
 *
 * @file fw_if_muxed_device_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "fw_if_muxed_device_debug.h"
#include "fw_if_muxed_device.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FW_IF_MUXED_DEVICE_DBG_NAME     "FW_IF_MUXED_DEVICE_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised               = FALSE;
static DAL_HDL pxFwIfMuxedDeviceTop     = NULL;


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
 * @brief   Initialise the FW IF MUXED_DEVICE debug access
 */
void vFW_IF_MUXED_DEVICE_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxFwIfMuxedDeviceTop = pxDAL_NewDirectory( "fw_if_muxed_device" );
        }
        else
        {
            pxFwIfMuxedDeviceTop = pxDAL_NewSubDirectory( "fw_if_muxed_device", pxParentHandle );
        }

        if( NULL != pxFwIfMuxedDeviceTop )
        {
            pxDAL_NewDebugFunction( "print_all_stats", pxFwIfMuxedDeviceTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_all_stats", pxFwIfMuxedDeviceTop, vClearStats );
        }

        iIsInitialised = TRUE;
    }
}

/******************************************************************************/
/* Private function implementations                                           */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void )
{
    if( FW_IF_ERRORS_NONE != FW_IF_MUXED_DEVICE_PrintStatistics() )
    {
        PLL_DAL( FW_IF_MUXED_DEVICE_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( FW_IF_ERRORS_NONE != FW_IF_MUXED_DEVICE_ClearStatistics() )
    {
        PLL_DAL( FW_IF_MUXED_DEVICE_DBG_NAME, "Error clearing statistics\r\n" );
    }
}
