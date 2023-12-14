/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the INA3221 debug implementation
 *
 * @file ina3221_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "ina3221_debug.h"
#include "ina3221.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define INA3221_DBG_NAME        "INA3221_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxIna3221Top = NULL;
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
 * @brief   Debug function to retrieve the voltage
 *
 * @return  N/A
 */
static void vGetVoltage( void );

/**
 * @brief   Debug function to retrieve the current
 *
 * @return  N/A
 */
static void vGetCurrent( void );

/**
 * @brief   Debug function to retrieve the power
 *
 * @return  N/A
 */
static void vGetPower( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the INA3221 debug access
 */
void vINA3221_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxIna3221Top = pxDAL_NewDirectory( "ina3221" );
        }
        else
        {
            pxIna3221Top = pxDAL_NewSubDirectory( "ina3221", pxParentHandle );
        }

        if( NULL != pxIna3221Top )
        {
            pxDAL_NewDebugFunction( "print_stats", pxIna3221Top, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxIna3221Top, vClearStats );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxIna3221Top );

            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_voltage", pxGetDir, vGetVoltage );
                pxDAL_NewDebugFunction( "get_current", pxGetDir, vGetCurrent );
                pxDAL_NewDebugFunction( "get_power",   pxGetDir, vGetPower );
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
    if( OK != iINA3221_PrintStatistics() )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iINA3221_ClearStatistics() )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to retrieve the voltage
 */
static void vGetVoltage( void )
{
    float    fVoltageMv = 0.0;
    int      iBusNum    = 0;
    uint32_t ulI2cAddr  = 0;
    int      iChanNum   = 0;

    if( OK != iDAL_GetIntInRange( "Bus number:", &iBusNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving bus number\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "I2C address:", &ulI2cAddr, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving i2c address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Channel number:", &iChanNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving channel number\r\n" );
    }
    else if( OK != iINA3221_ReadVoltage( ( uint8_t )iBusNum,
                                         ( uint8_t )ulI2cAddr,
                                         ( uint8_t )iChanNum,
                                         &fVoltageMv ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving INA3221 voltage\r\n" );
    }
    else
    {
        PLL_DAL( INA3221_DBG_NAME, "Voltage (mV): %f\r\n", fVoltageMv );
    }
}

/**
 * @brief   Debug function to retrieve the current
 */
static void vGetCurrent( void )
{
    float    fCurrentMa = 0.0;
    int      iBusNum    = 0;
    uint32_t ulI2cAddr  = 0;
    int      iChanNum   = 0;

    if( OK != iDAL_GetIntInRange( "Bus number:", &iBusNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving bus number\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "I2C address:", &ulI2cAddr, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving i2c address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Channel number:", &iChanNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving channel number\r\n" );
    }
    else if( OK != iINA3221_ReadCurrent( ( uint8_t )iBusNum,
                                         ( uint8_t )ulI2cAddr,
                                         ( uint8_t )iChanNum,
                                         &fCurrentMa ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving INA3221 current\r\n" );
    }
    else
    {
        PLL_DAL( INA3221_DBG_NAME, "Current (mA): %f\r\n", fCurrentMa );
    }
}

/**
 * @brief   Debug function to retrieve the power
 */
static void vGetPower( void )
{
    float    fPowerMw  = 0.0;
    int      iBusNum   = 0;
    uint32_t ulI2cAddr = 0;
    int      iChanNum  = 0;

    if( OK != iDAL_GetIntInRange( "Bus number:", &iBusNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving bus number\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "I2C address:", &ulI2cAddr, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving i2c address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Channel number:", &iChanNum, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving channel number\r\n" );
    }
    else if( OK != iINA3221_ReadPower( ( uint8_t )iBusNum,
                                       ( uint8_t )ulI2cAddr,
                                       ( uint8_t )iChanNum,
                                       &fPowerMw ) )
    {
        PLL_DAL( INA3221_DBG_NAME, "Error retrieving INA3221 power\r\n" );
    }
    else
    {
        PLL_DAL( INA3221_DBG_NAME, "Power (mW): %f\r\n", fPowerMw );
    }
}
