/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the i2c driver debug implementation
 *
 * @file i2c_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "i2c_debug.h"
#include "i2c.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define I2C_DBG_NAME               "I2C_DBG"

#define I2C_BUFFER_DUMP_WIDTH      ( 16 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxI2cTop = NULL;
static DAL_HDL pxGetDir = NULL;
static DAL_HDL pxSetDir = NULL;


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
 * @brief   Debug function to print this module's logs
 *
 * @return  N/A
 */
static void vPrintLog( void );

/**
 * @brief   Debug function to reads data from the I2C device into a specified buffer
 *
 * @return  N/A
 */
static void vSetI2cSend( void );

/**
 * @brief   Debug function to write data from the I2C device into a specified buffer.
 *
 * @return  N/A
 */
static void vGetI2cRecv ( void );

/**
 * @brief   This function sends data from the I2C device and waits for a response coming back.
 *
 * @return  N/A
 */
static void vGetI2cSendRecv (void );

/**
 * @brief   Debug function to test iI2C_ReInit
 *
 * @return  N/A
 */
static void vI2cReInit( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the i2c debug access
 */
void vI2C_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxI2cTop = pxDAL_NewDirectory( "I2c" );
        }
        else
        {
            pxI2cTop = pxDAL_NewSubDirectory( "I2c", pxParentHandle );
        }

        if( NULL != pxI2cTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxI2cTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxI2cTop, vClearStats );
            pxDAL_NewDebugFunction( "print_log",      pxI2cTop, vPrintLog );
            pxDAL_NewDebugFunction( "i2c_reinit",     pxI2cTop, vI2cReInit );

            pxSetDir = pxDAL_NewSubDirectory( "sets", pxI2cTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxI2cTop );

            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_i2c_receive",      pxGetDir, vGetI2cRecv );
                pxDAL_NewDebugFunction( "get_i2c_send_receive", pxGetDir, vGetI2cSendRecv );
            }
            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_i2c_send", pxSetDir, vSetI2cSend );
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
    if( OK != iI2C_PrintStatistics() )
    {
        PLL_DAL( I2C_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iI2C_ClearStatistics() )
    {
        PLL_DAL( I2C_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to print this module's log
 */
static void vPrintLog( void )
{
    int iDeviceId = 0;

    if( OK != iDAL_GetIntInRange( "Enter Device ID:", &iDeviceId, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Device ID\r\n" );
    }
    else
    {
        if( OK != iI2C_PrintLog( iDeviceId ) )
        {
            PLL_DAL( I2C_DBG_NAME, "Error printing log\r\n" );
        }
    }
}

/**
 * @brief   Debug function to test iI2C_Send
 */
static void vSetI2cSend(void)
{
    int iDeviceId = 0;
    int iLength = 0;
    uint32_t ulAddr = 0;
    uint8_t *pucDataBuff = NULL;

    if( OK != iDAL_GetIntInRange( "Enter Device ID:", &iDeviceId, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Device ID\r\n" );
    }
    else if( OK != iDAL_GetHex( "Enter Slave Address:", &ulAddr ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Slave Address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Data Length:", &iLength, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Data Length\r\n" );
    }
    else
    {
        pucDataBuff = ( uint8_t* )pvOSAL_MemAlloc( iLength * sizeof( uint8_t ) );

        if( NULL != pucDataBuff )
        {
            int i = 0;

            for( i = 0; i < iLength; i++ )
            {
                uint32_t ulTemp = 0;

                if ( OK != iDAL_GetHexInRange( "Enter byte to send (hex):", &ulTemp, 0, UTIL_MAX_UINT8 ) ||
                   ( ulTemp > 0xFF ) )
                {
                    PLL_DAL( I2C_DBG_NAME,"Invalid input\r\n");
                }

                pucDataBuff[ i ] = ( uint8_t )ulTemp;
            }

            if( OK != iI2C_Send( ( uint8_t )iDeviceId, ( uint8_t )ulAddr, pucDataBuff, ( uint32_t )iLength ) )
            {
                PLL_DAL( I2C_DBG_NAME, "Error sending data to Device ID: %d, Slave Address: 0x%08X\r\n", iDeviceId, ulAddr );
            }
            else
            {
                PLL_DAL( I2C_DBG_NAME, "Successfully sent data to Device ID: %d, Slave Address: 0x%08X\r\n", iDeviceId, ulAddr );
            }

            vOSAL_MemFree( ( void** )&pucDataBuff );
        }
        else
        {
            PLL_DAL( I2C_DBG_NAME, "Malloc for send buffer failed\r\n" );
        }
    }
}

/**
 * @brief   Debug function to test iI2C_Recv
 */
static void vGetI2cRecv(void)
{
    int iDeviceId = 0;
    int iLength = 0;
    uint32_t ulAddr = 0;
    uint8_t *pucDataBuff = NULL;

    if( OK != iDAL_GetIntInRange( "Enter Device ID:", &iDeviceId, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Device ID\r\n" );
    }
    else if( OK != iDAL_GetHex( "Enter Slave Address:", &ulAddr ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Slave Address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Data Length:", &iLength, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Data Length\r\n" );
    }
    else
    {
        pucDataBuff = ( uint8_t* )pvOSAL_MemAlloc( iLength * sizeof( uint8_t ) );

        if( NULL != pucDataBuff )
        {
            if( OK != iI2C_Recv( ( uint8_t )iDeviceId, ( uint8_t )ulAddr, pucDataBuff, ( uint32_t )iLength ) )
            {
                PLL_DAL( I2C_DBG_NAME, "Error receiving data from Device ID: %d, Slave Address: 0x%08X\r\n", iDeviceId, ulAddr );
            }
            else
            {
                int i = 0;

                PLL_DAL( I2C_DBG_NAME, "Successfully received data from Device ID: %d, Slave Address: 0x%08X\r\n", iDeviceId, ulAddr );
                PLL_DAL( I2C_DBG_NAME, "Buffer: \r\n" );
                for( i = 0; i < iLength; i++ )
                {
                    if( 0 == ( i % I2C_BUFFER_DUMP_WIDTH ) )
                    {
                        if( 0 != i )
                        {
                            vPLL_Printf( "\r\n%d", i );
                        }
                        PLL_DAL( I2C_DBG_NAME, "\t[ 0x%04X ]. . . . . :", i );
                    }
                    vPLL_Printf( " %02X", pucDataBuff[ i ] );
                }
                vPLL_Printf( "\r\n" );
            }

            vOSAL_MemFree( ( void** )&pucDataBuff );
        }
        else
        {
            PLL_DAL( I2C_DBG_NAME, "Malloc for receive buffer failed\r\n" );
        }
    }
}

/**
 * @brief   Debug function to test iI2C_SendRecv
 */
static void vGetI2cSendRecv(void)
{
    int iDeviceId = 0;
    int iReadLength = 0;
    int iWriteLength = 0;
    uint32_t ulWriteAddr = 0;
    uint8_t *pucWriteDataBuff = NULL;
    uint8_t *pucReadDataBuff = NULL;

    if( OK != iDAL_GetIntInRange( "Enter Device ID:", &iDeviceId, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Device ID\r\n" );
    }
    else if( OK != iDAL_GetHex( "Enter Slave Write Address:", &ulWriteAddr ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Slave Write Address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Write Data Length:", &iWriteLength, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Write Data Length\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Read Data Length:", &iReadLength, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Read Data Length\r\n" );
    }
    else
    {
        int iStatus = OK;

        if( OK == iStatus)
        {
            pucWriteDataBuff = ( uint8_t* )pvOSAL_MemAlloc( iWriteLength * sizeof( uint8_t ) );
            if( NULL == pucWriteDataBuff )
            {
                iStatus = ERROR;
            }
        }

        if( OK == iStatus)
        {
            pucReadDataBuff = ( uint8_t* )pvOSAL_MemAlloc( iReadLength * sizeof( uint8_t ) );
            if( NULL == pucReadDataBuff )
            {
                iStatus = ERROR;
            }
        }

        if( OK == iStatus )
        {
            int i = 0;

            for( i = 0; i < iWriteLength; i++ )
            {
                uint32_t ulTemp = 0;
                if( OK != iDAL_GetHexInRange( "Enter byte to send (hex):", &ulTemp, 0, UTIL_MAX_UINT8 ) ||
                  ( ulTemp > 0xFF ) )
                {
                    PLL_DAL( I2C_DBG_NAME, "Invalid input\r\n" );
                }

                pucWriteDataBuff[ i ] = ( uint8_t )ulTemp;
            }

            if( OK != iI2C_SendRecv( ( uint8_t )iDeviceId,
                                     ( uint8_t )ulWriteAddr,
                                     pucWriteDataBuff,
                                     ( uint32_t )iWriteLength,
                                     pucReadDataBuff,
                                     ( uint32_t )iReadLength ) )
            {
                PLL_DAL( I2C_DBG_NAME, "Error in SendRecv with Device ID: %d, Slave Write Address: 0x%08X\r\n", iDeviceId, ulWriteAddr );
            }
            else
            {
                int i = 0;

                PLL_DAL( I2C_DBG_NAME, "Successfully completed SendRecv with Device ID: %d, Slave Write Address: 0x%08X\r\n", iDeviceId, ulWriteAddr );
                PLL_DAL( I2C_DBG_NAME, "Received Buffer: \r\n" );
                for( i = 0; i < iReadLength; i++ )
                {
                    if( 0 == ( i % I2C_BUFFER_DUMP_WIDTH ) )
                    {
                        if( 0 != i )
                        {
                            vPLL_Printf( "\r\n%d", i );
                        }
                        PLL_DAL( I2C_DBG_NAME, "\t[ 0x%04X ]. . . . . :", i );
                    }
                    vPLL_Printf( " %02X", pucReadDataBuff[ i ] );
                }
                vPLL_Printf( "\r\n" );
            }
        }
        else
        {
            PLL_DAL( I2C_DBG_NAME, "Malloc for send/receive buffer failed\r\n" );
        }
        if( pucWriteDataBuff )
        {
            vOSAL_MemFree( ( void** )&pucWriteDataBuff );
        }

        if( pucReadDataBuff )
        {
            vOSAL_MemFree( ( void** )&pucReadDataBuff );
        }
    }
}

/**
 * @brief   Debug function to test iI2C_ReInit
 */
static void vI2cReInit( void )
{
    int iDeviceId = 0;

    if( OK != iDAL_GetIntInRange( "Enter Device ID:", &iDeviceId, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( I2C_DBG_NAME, "Error retrieving Device ID\r\n" );
    }
    else
    {
        if( OK == iI2C_ReInit( iDeviceId ) )
        {
            PLL_DAL( I2C_DBG_NAME, "iI2C_ReInit successful\r\n" );
        }
        else
        {
            PLL_DAL( I2C_DBG_NAME, "iI2C_ReInit failed\r\n" );
        }
    }
}
