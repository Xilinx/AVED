/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the ISL68221 debug implementation
 *
 * @file isl68221_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "isl68221_debug.h"
#include "isl68221.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define ISL68221_DBG_NAME     "ISL68221_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxIsl68221Top = NULL;
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
 * @brief   Debug function to retrieve voltage
 *
 * @return  N/A
 */
static void vGetVoltage( void );

/**
 * @brief   Debug function to retrieve current
 *
 * @return  N/A
 */
static void vGetCurrent( void );

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
 * @brief   Initialise the ISL68221 debug access
 */
void vISL68221_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxIsl68221Top = pxDAL_NewDirectory( "isl68221" );
        }
        else
        {
            pxIsl68221Top = pxDAL_NewSubDirectory( "isl68221", pxParentHandle );
        }

        if( NULL != pxIsl68221Top )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxIsl68221Top, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxIsl68221Top, vClearStats );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxIsl68221Top );

            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_voltage",     pxGetDir, vGetVoltage );
                pxDAL_NewDebugFunction( "get_current",     pxGetDir, vGetCurrent );
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
    if( OK != iISL68221_PrintStatistics() )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iISL68221_ClearStatistics() )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to retrieve voltage
 */
static void vGetVoltage( void )
{
    float    fVoltageMv = 0.0;
    int      iBusNum    = 0;
    uint32_t ulI2cAddr  = 0;
    int      iPageNum   = 0;

    if( OK != iDAL_GetIntInRange( "Bus number:", &iBusNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving bus number\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "I2C address:", &ulI2cAddr, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving i2c address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Page number:", &iPageNum, 0, MAX_ISL68221_SENSOR_PAGE ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving page number\r\n" );
    }
    else if( OK != iISL68221_ReadVoltage( ( uint8_t )iBusNum,
                                          ( uint8_t )ulI2cAddr,
                                          ( uint8_t )iPageNum,
                                          &fVoltageMv ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving ISL68221 voltage %d\r\n", iPageNum );
    }
    else
    {
        PLL_DAL( ISL68221_DBG_NAME, "Voltage %d (mV): %f\r\n", iPageNum, fVoltageMv );
    }
}

/**
 * @brief   Debug function to retrieve current
 */
static void vGetCurrent( void )
{
    float    fCurrentA  = 0.0;
    int      iBusNum    = 0;
    uint32_t ulI2cAddr  = 0;
    int      iPageNum   = 0;

    if( OK != iDAL_GetIntInRange( "Bus number:", &iBusNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving bus number\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "I2C address:", &ulI2cAddr, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving i2c address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Page number:", &iPageNum, 0, MAX_ISL68221_SENSOR_PAGE ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving page number\r\n" );
    }
    else if( OK != iISL68221_ReadCurrent( ( uint8_t )iBusNum,
                                          ( uint8_t )ulI2cAddr,
                                          ( uint8_t )iPageNum,
                                          &fCurrentA ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving ISL68221 current %d\r\n", iPageNum );
    }
    else
    {
        PLL_DAL( ISL68221_DBG_NAME, "Current %d (A): %f\r\n", iPageNum, fCurrentA );
    }
}

/**
 * @brief   Debug function to retrieve temperature
 */
static void vGetTemperature( void )
{
    float    fTemperatureC = 0.0;
    int      iBusNum       = 0;
    uint32_t ulI2cAddr     = 0;
    int      iPageNum      = 0;

    if( OK != iDAL_GetIntInRange( "Bus number:", &iBusNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving bus number\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "I2C address:", &ulI2cAddr, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving i2c address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Page number:", &iPageNum, 0, MAX_ISL68221_SENSOR_PAGE ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving page number\r\n" );
    }
    else if( OK != iISL68221_ReadTemperature( ( uint8_t )iBusNum,
                                              ( uint8_t )ulI2cAddr,
                                              ( uint8_t )iPageNum,
                                              &fTemperatureC ) )
    {
        PLL_DAL( ISL68221_DBG_NAME, "Error retrieving ISL68221 temperature %d\r\n", iPageNum );
    }
    else
    {
        PLL_DAL( ISL68221_DBG_NAME, "Temperature %d (C): %f\r\n", iPageNum, fTemperatureC );
    }
}

