/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the CAT34TS02 debug implementation
 *
 * @file cat34ts02_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "cat34ts02_debug.h"
#include "cat34ts02.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define CAT34TS02_DBG_NAME      "CAT34TS02_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxCat34ts02Top = NULL;
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
 * @brief   Debug function to retrieve temperature
 *
 * @return  N/A
 */
static void vGetTemperature( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the CAT34TS02 debug access
 */
void vCAT34TS02_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxCat34ts02Top = pxDAL_NewDirectory( "cat34ts02" );
        }
        else
        {
            pxCat34ts02Top = pxDAL_NewSubDirectory( "cat34ts02", pxParentHandle );
        }
        
        if( NULL != pxCat34ts02Top )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxCat34ts02Top, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxCat34ts02Top, vClearStats );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxCat34ts02Top );

            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_temperature", pxGetDir, vGetTemperature );
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
    if( OK != iCAT34TS02_PrintStatistics() )
    {
        PLL_DAL( CAT34TS02_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iCAT34TS02_ClearStatistics() )
    {
        PLL_DAL( CAT34TS02_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to retrieve temperature
 */
static void vGetTemperature( void )
{
    int       iI2cNum        = 0;
    uint32_t  ulI2cAddr      = 0;
    int       iChanNum       = 0;
    float     fTemperatureC  = 0;

    if( OK != iDAL_GetIntInRange( "I2c number:", &iI2cNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( CAT34TS02_DBG_NAME, "Error retrieving i2c number\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "I2C address:", &ulI2cAddr, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( CAT34TS02_DBG_NAME, "Error retrieving i2c address\r\n" );
    }
    else if( OK != iCAT34TS02_ReadTemperature( ( uint8_t )iI2cNum,
                                               ( uint8_t )ulI2cAddr,
                                               ( uint8_t )iChanNum,
                                               &fTemperatureC ) )
    {
        PLL_DAL( CAT34TS02_DBG_NAME, "Error retrieving CAT34TS02 temperature\r\n" );
    }
    else
    {
        PLL_DAL( CAT34TS02_DBG_NAME, "Temperature (C): %f\r\n", fTemperatureC );
    }
}
