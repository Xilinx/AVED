/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED External Device Control (AXC) debug implementation
 *
 * @file axc_proxy_driver_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "axc_proxy_driver_debug.h"
#include "axc_proxy_driver.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define AXC_DBG_NAME                    "AXC_DBG"
#define AXC_DBG_MAX_QSFP_NUM            ( 4 )
#define AXC_DBG_DIMM_ID                 ( 5 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxAxcTop      = NULL;
static DAL_HDL pxSetDir      = NULL;
static DAL_HDL pxGetDir      = NULL;


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
 * @brief   Debug function to bind a callback to this module
 *
 * @return  N/A
 */
static void vBindCallbacks( void );

/**
 * @brief   Debug function to set byte in qsfp memory map
 *
 * @return  N/A
 */
static void vSetByte( void );

/**
 * @brief   Debug function to get byte from qsfp memory map
 *
 * @return  N/A
 */
static void vGetByte( void );

/**
 * @brief   Debug function to get page from qsfp memory map
 *
 * @return  N/A
 */
static void vGetPage( void );

/**
 * @brief   Debug function to get single IO status from qsfp IO expander
 *
 * @return  N/A
 */
static void vGetIoStatus( void );

/**
 * @brief   Debug function to get all IO statuses from qsfp IO expander
 *
 * @return  N/A
 */
static void vGetAllIoStatuses( void );

/**
 * @brief   Debug function to get temperature value from qsfp or DIMM
 *
 * @return  N/A
 */
static void vGetTemperature( void );



/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 *
 * @param   pxSignal     Event raised
 *
 * @return  OK if no errors were raised in the callback
 *          ERROR if an error was raised in the callback
 *
 */
static int iTestCallback( EVL_SIGNAL *pxSignal );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the AXC debug access
 */
void vAXC_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxAxcTop = pxDAL_NewDirectory( "axc" );
        }
        else
        {
            pxAxcTop = pxDAL_NewSubDirectory( "axc", pxParentHandle );
        }

        if( NULL != pxAxcTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxAxcTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxAxcTop, vClearStats );
            pxDAL_NewDebugFunction( "bind_callbacks", pxAxcTop, vBindCallbacks );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxAxcTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxAxcTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_byte", pxSetDir, vSetByte );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_byte", pxGetDir, vGetByte );
                pxDAL_NewDebugFunction( "get_page", pxGetDir, vGetPage );
                pxDAL_NewDebugFunction( "get_single_status", pxGetDir, vGetIoStatus );
                pxDAL_NewDebugFunction( "get_all_statuses", pxGetDir, vGetAllIoStatuses );
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
    if( OK != iAXC_PrintStatistics() )
    {
        PLL_DAL( AXC_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iAXC_ClearStatistics() )
    {
        PLL_DAL( AXC_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to bind a callback to this module
 */
static void vBindCallbacks( void )
{
    if( OK != iAXC_BindCallback( &iTestCallback ) )
    {
        PLL_DAL( AXC_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( AXC_DBG_NAME, "Callback bound\r\n" );
    }
}

/***** Sets *****/

/**
 * @brief   Debug function to set byte in device memory map
 */
static void vSetByte( void )
{
    int iExDevId = 0;
    int iPageNum = 0;
    int iByteOffset = 0;
    int ucByteValue = 0;
    if( OK != iDAL_GetIntInRange( "Enter External Device ID (1-4):", &iExDevId, 0, AXC_DBG_MAX_QSFP_NUM ) )
    {
        PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device ID\r\n" );
    }
    else
    {
        if( OK != iDAL_GetIntInRange( "Enter page num:", &iPageNum, 0, UTIL_MAX_UINT8 ) )
        {
            PLL_DAL( AXC_DBG_NAME, "Error retrieving page num\r\n" );
        }
        else
        {
            if( OK != iDAL_GetIntInRange( "Enter byte offset:", &iByteOffset, 0, UTIL_MAX_UINT8 ) )
            {
                PLL_DAL( AXC_DBG_NAME, "Error retrieving byte offset\r\n" );
            }
            else
            {
                if( OK != iDAL_GetIntInRange( "Enter byte value:", &ucByteValue, 0, UTIL_MAX_UINT8 ) )
                {
                    PLL_DAL( AXC_DBG_NAME, "Error retrieving byte value\r\n" );
                }
                else
                {
                    if( OK != iAXC_SetByte( ( uint8_t )iExDevId, ( uint32_t )iPageNum, ( uint32_t )iByteOffset, ( uint8_t )ucByteValue ) )
                    {
                        PLL_DAL( AXC_DBG_NAME, "Error setting External Device memory byte\r\n" );
                    }
                    else
                    {
                        PLL_DAL( AXC_DBG_NAME, "External Device memory byte (%d) from page (%d) set successfully \r\n", iByteOffset, iPageNum );
                    }
                }
            }
        }
    }
}

/***** Gets *****/

/**
 * @brief   Debug function to get byte from device memory map
 */
static void vGetByte( void )
{
    int iExDevId = 0;
    int iPageNum = 0;
    int iByteOffset = 0;
    uint8_t ucByteValue = 0;
    if( OK != iDAL_GetIntInRange( "Enter External Device ID (1-4):", &iExDevId, 0, AXC_DBG_MAX_QSFP_NUM ) )
    {
        PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device ID\r\n" );
    }
    else
    {
        if( OK != iDAL_GetIntInRange( "Enter page num:", &iPageNum, 0, UTIL_MAX_UINT8 ) )
        {
            PLL_DAL( AXC_DBG_NAME, "Error retrieving page num\r\n" );
        }
        else
        {
            if( OK != iDAL_GetIntInRange( "Enter byte offset:", &iByteOffset, 0, UTIL_MAX_UINT8 ) )
            {
                PLL_DAL( AXC_DBG_NAME, "Error retrieving byte offset\r\n" );
            }
            else
            {
                if( OK != iAXC_GetByte( ( uint8_t )iExDevId, ( uint32_t )iPageNum, ( uint32_t )iByteOffset, &ucByteValue ) )
                {
                    PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device memory byte\r\n" );
                }
                else
                {
                    PLL_DAL( AXC_DBG_NAME, "Retrieved External Device memory byte, value (hex): 0x%02X\r\n", ucByteValue );
                }
            }
        }
    }
}

/**
 * @brief   Debug function to get page from device memory map
 */
static void vGetPage( void )
{
    int iExDevId = 0;
    int iPageNum = 0;
    int i = 0;
    AXC_PROXY_DRIVER_PAGE_DATA xTestPage = { { 0 } };
    if( OK != iDAL_GetIntInRange( "Enter External Device ID (1-4):", &iExDevId, 0, AXC_DBG_MAX_QSFP_NUM ) )
    {
        PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device ID\r\n" );
    }
    else
    {
        if( OK != iDAL_GetIntInRange( "Enter page num:", &iPageNum, 0, UTIL_MAX_UINT8 ) )
        {
            PLL_DAL( AXC_DBG_NAME, "Error retrieving page num\r\n" );
        }
        else
        {
            if( OK != iAXC_GetPage( ( uint8_t )iExDevId, ( uint32_t )iPageNum, &xTestPage ) )
            {
                PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device memory page\r\n" );
            }
            else
            {
                PLL_DAL( AXC_DBG_NAME, "Retrieved External Device memory page, values: \r\n" );

                for( i = 0; i < xTestPage.ulPageDataSize; i++ )
                {
                    vPLL_Printf( "0x%02X ", xTestPage.pucPageData[ i ] );
                }

                vPLL_Printf( "\r\n" );
            }
        }
    }
}

/**
 * @brief   Debug function to get single IO status from External Device IO expander
 */
static void vGetIoStatus( void )
{
    int iExDevId = 0;
    int iIoCtrlId = 0;
    uint8_t ucIoStatusValue = 0;
    if( OK != iDAL_GetIntInRange( "Enter External Device ID (1-4):", &iExDevId, 0, AXC_DBG_MAX_QSFP_NUM ) )
    {
        PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device ID\r\n" );
    }
    else
    {
        if( OK != iDAL_GetIntInRange( "Enter IO control line offset (0-4)\r\nMODSEL:0\r\nRESET:1\r\nLPMODE:2\r\nMODPRS:3\r\nINT:4\r\n",
                                      &iIoCtrlId, 0, MAX_AXC_PROXY_DRIVER_QSFP_IO ) )
        {
            PLL_DAL( AXC_DBG_NAME, "Error retrieving page num\r\n" );
        }
        else
        {
            if( OK != iAXC_GetSingleIoStatus( ( uint8_t )iExDevId, ( AXC_PROXY_DRIVER_QSFP_IO )iIoCtrlId, &ucIoStatusValue ) )
            {
                PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device IO expander status\r\n" );
            }
            else
            {
                PLL_DAL( AXC_DBG_NAME, "Retrieved External Device IO expander status, value: %d\r\n", ucIoStatusValue );
            }
        }
    }
}

/**
 * @brief   Debug function to get all IO statuses from External Device IO expander
 */
static void vGetAllIoStatuses( void )
{
    int iExDevId = 0;
    AXC_PROXY_DRIVER_QSFP_IO_STATUSES xTestStatuses = { 0 };
    if( OK != iDAL_GetIntInRange( "Enter External Device ID (1-4):", &iExDevId, 0, AXC_DBG_MAX_QSFP_NUM ) )
    {
        PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device ID\r\n" );
    }
    else
    {
        if( OK != iAXC_GetAllIoStatuses( ( uint8_t )iExDevId, &xTestStatuses ) )
        {
            PLL_DAL( AXC_DBG_NAME, "Error retrieving External Device IO expander statuses\r\n" );
        }
        else
        {
            PLL_DAL( AXC_DBG_NAME, "Retrieved External Device IO expander statuses\r\n" );
            PLL_DAL( AXC_DBG_NAME, "MODSEL value: (%d)\r\n", xTestStatuses.ucModSel );
            PLL_DAL( AXC_DBG_NAME, "RESET  value: (%d)\r\n", xTestStatuses.ucReset );
            PLL_DAL( AXC_DBG_NAME, "LPMODE value: (%d)\r\n", xTestStatuses.ucLpMode );
            PLL_DAL( AXC_DBG_NAME, "MODPRS value: (%d)\r\n", xTestStatuses.ucModPrs );
            PLL_DAL( AXC_DBG_NAME, "INT    value: (%d)\r\n", xTestStatuses.ucInterrupt );
        }
    }
}

/**
 * @brief   Debug function to get temperature value from External Device
 */
static void vGetTemperature( void )
{
    int iDeviceId = 0;
    int iDevice = 0;
    float fTemperatureByte = 0;

    if( OK != iDAL_GetIntInRange( "Enter Device: QSFP - 1,  DIMM - 2 :", &iDevice, 1, 2 ) )
    {
        PLL_DAL( AXC_DBG_NAME, "Error retrieving device\r\n" );
    }
    else
    {
        if( 1 == iDevice )
        {
            if( OK != iDAL_GetIntInRange( "Enter QSFP ID (1-4):", &iDeviceId, 1, AXC_DBG_MAX_QSFP_NUM ) )
            {
                PLL_DAL( AXC_DBG_NAME, "Error retrieving QSFP ID\r\n" );
            }
            else
            {
                if( OK !=  iAXC_GetTemperature( ( uint8_t )iDeviceId, &fTemperatureByte ) )
                {
                    PLL_DAL( AXC_DBG_NAME, "Error retrieving QSFP temperature\r\n" );
                }
                else
                {
                    PLL_DAL( AXC_DBG_NAME, "Retrieved QSFP temperature value: (%f)\r\n", fTemperatureByte );
                }
            }
        }
        else
        {
            iDeviceId = AXC_DBG_DIMM_ID;
            if( OK !=  iAXC_GetTemperature( ( uint8_t )iDeviceId, &fTemperatureByte ) )
            {
                PLL_DAL( AXC_DBG_NAME, "Error retrieving DIMM temperature\r\n" );
            }
            else
            {
                PLL_DAL( AXC_DBG_NAME, "Retrieved DIMM temperature value: (%f)\r\n", fTemperatureByte );
            }
        }
    }
}


/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 */
static int iTestCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( NULL != pxSignal )
    {
        PLL_DAL( AXC_DBG_NAME, "AXC Signal raised: Module[%d], Type[%d], Instance[%d]\r\n",
                 pxSignal->ucModule, pxSignal->ucEventType, pxSignal->ucInstance );
        iStatus = OK;
    }

    return iStatus;
}

