/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF UART debug access
 *
 * @file fw_if_uart_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "fw_if_uart.h"
#include "fw_if_uart_debug.h"

#include "dal.h"
#include "profile_fal.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FW_IF_UART_DBG_NAME                   "FW_IF_UART_DBG"

#define FW_IF_UART_DBG_MAX_BYTE_VALUE         ( 0xFF )
#define FW_IF_UART_DBG_MAX_READ_WRITE_SIZE    ( 256 )
#define FW_IF_UART_DBG_ADDRESS_OFFSET_MAX     ( 0x7F )
#define FW_IF_UART_TIMEOUT_MAX_MS             ( 10000 )
#define FW_IF_UART_NEW_LINE_LENGTH            ( 2 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iInitialised = FALSE;
static DAL_HDL pxFwIfUartTop = NULL;


/******************************************************************************/
/* Private function implementations                                           */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats.
*/
static void vUartPrintStats( void );

/**
 * @brief   Debug function to clear this module's stats.
*/
static void vUartClearStats( void );

/**
 * @brief   Debug function to open an Uart instance.
 */
static void vUartOpenInstance( void );

/**
 * @brief   Debug function to close an Uart instance.
 */
static void vUartCloseInstance( void );

/**
 * @brief   Debug function to write.
 */
static void vUartWrite( void );

/**
 * @brief   Debug function to read.
 */
static void vUartRead( void );

/**
 * @brief   Debug function to ioctrl.
 */
static void vUartIoCtrl( void );

/**
 * @brief   Debug function to bind a callback.
 */
static void vUartBindCallback( void );

/***** Debug print functions *****/

/**
 * @brief   Debug error print function.
 */
static void vUartUserInputError( char *string );

/***** Helper functions *****/

/**
 * @brief   Debug bind function.
 */
static uint32_t ulUartTestCallback( uint16_t usEventId, uint8_t *pucData, uint32_t ulSize );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief    Initialise the FW IF UART debug access
 */
void vFW_IF_UART_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iInitialised )
    {   
        if( NULL == pxParentHandle )
        {
            pxFwIfUartTop = pxDAL_NewDirectory( "fw_if_uart" );
        }
        else
        {
            pxFwIfUartTop = pxDAL_NewSubDirectory( "fw_if_uart", pxParentHandle );
        }

        if( NULL != pxFwIfUartTop )
        {
            pxDAL_NewDebugFunction( "print_all_stats", pxFwIfUartTop, vUartPrintStats );
            pxDAL_NewDebugFunction( "clear_all_stats", pxFwIfUartTop, vUartClearStats );
            pxDAL_NewDebugFunction( "open", pxFwIfUartTop, vUartOpenInstance );
            pxDAL_NewDebugFunction( "close", pxFwIfUartTop, vUartCloseInstance );
            pxDAL_NewDebugFunction( "write", pxFwIfUartTop, vUartWrite );
            pxDAL_NewDebugFunction( "read", pxFwIfUartTop, vUartRead );
            pxDAL_NewDebugFunction( "io_ctrl", pxFwIfUartTop, vUartIoCtrl );
            pxDAL_NewDebugFunction( "bind_callback", pxFwIfUartTop, vUartBindCallback );
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
static void vUartPrintStats( void )
{
    if( FW_IF_ERRORS_NONE != iFW_IF_UART_PrintStatistics() )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats.
 */
static void vUartClearStats( void )
{
    if( FW_IF_ERRORS_NONE != iFW_IF_UART_ClearStatistics() )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to open a UART instance
*/
static void vUartOpenInstance( void )
{
    if( ( NULL == pxUartIf->open ) || ( OK != pxUartIf->open( pxUartIf ) ) )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error opening Uart instance\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Success opening Uart instance\r\n" );
    }

}

/**
 * @brief   Debug function to close a UART instance
*/
static void vUartCloseInstance( void )
{
    if( ( NULL == pxUartIf->close ) || ( OK != pxUartIf->close( pxUartIf ) ) )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error closing Uart instance\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Success closing Uart instance\r\n" );
    }

}

/**
 * @brief   Debug function to write
*/
static void vUartWrite( void )
{
    int i = 0;
    int iAddrOffset = 0;
    int iLength = 0;
    int iTimeMs = 0;
    uint32_t ulTimeoutMs = 0;
    uint32_t ulData = 0;
    uint8_t pucBufNewLine[ FW_IF_UART_NEW_LINE_LENGTH ] = "\r\n";
    uint8_t pucData[ FW_IF_UART_DBG_MAX_READ_WRITE_SIZE ] = { 0 };

    /* Address offset */
    if( OK != iDAL_GetIntInRange( "Enter address offset: ", &iAddrOffset, 0x00, FW_IF_UART_DBG_ADDRESS_OFFSET_MAX ) )
    {
        vUartUserInputError( "Address offset" );
    }

    /* Bytes in data buffer */
    if( OK != iDAL_GetIntInRange( "Enter data buffer bytes: ", &iLength, 0, FW_IF_UART_DBG_MAX_READ_WRITE_SIZE ) )
    {
        vUartUserInputError( "Data buffer bytes" );
    }

    /* Data buffer */
    for( i = 0; i < iLength; i++ )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "==> Data byte %d", i );
        if ( OK != iDAL_GetHexInRange( "Enter data buffer: ", &ulData, 0, FW_IF_UART_DBG_MAX_BYTE_VALUE ) )
        {
            vUartUserInputError( "Data buffer" );
        }
        else
        {
            pucData[ i ] = ( uint8_t )ulData;
        }
    }

    /* Timeout to complete write */
    if( OK != iDAL_GetIntInRange( "Enter timeout: ", &iTimeMs, -1, FW_IF_UART_TIMEOUT_MAX_MS) )
    {
        vUartUserInputError( "Timeout" );
    }

    ulTimeoutMs = ( uint32_t )iTimeMs;

    if( ( NULL == pxUartIf->write ) || ( OK != pxUartIf->write( pxUartIf, iAddrOffset, pucData, iLength, ulTimeoutMs ) ) )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error writing\r\n" );
    }
    else
    {
        /* Add new line */
        pxUartIf->write( pxUartIf, iAddrOffset, pucBufNewLine, FW_IF_UART_NEW_LINE_LENGTH, FW_IF_TIMEOUT_WAIT_FOREVER );
        PLL_DAL( FW_IF_UART_DBG_NAME, "Success writing\r\n" );
    }

}

/**
 * @brief   Debug function to read
*/
static void vUartRead( void )
{
    int      i           = 0;
    int      iAddrOffset = 0;
    uint32_t ulLength    = 0;
    int      iTimeoutMs  = 0;
    uint8_t  pucData[ FW_IF_UART_DBG_MAX_READ_WRITE_SIZE ] = { 0 };

    /* Address offset */
    if( OK != iDAL_GetIntInRange( "Enter address offset: ", &iAddrOffset, 0x00, FW_IF_UART_DBG_ADDRESS_OFFSET_MAX ) )
    {
        vUartUserInputError( "Address offset" );
    }

    /* Timeout to complete read */
    if( ( OK != iDAL_GetIntInRange( "Enter timeout: ", &iTimeoutMs, -1, FW_IF_UART_TIMEOUT_MAX_MS ) ) )
    {
        vUartUserInputError( "Timeout" );
    }

    if( ( NULL == pxUartIf->read ) || ( OK != pxUartIf->read( pxUartIf, iAddrOffset, pucData, &ulLength, iTimeoutMs ) ) )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error reading\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Success reading\r\n" );
        PLL_DAL( FW_IF_UART_DBG_NAME, "=== Data ===\r\n" );

        /* Data buffer */
        for ( i = 0; i < ulLength; i++ )
        {
            PLL_DAL( FW_IF_UART_DBG_NAME, "Byte %d: 0x%02x\r\n", i, pucData[i] );
        }
    }

}

/**
 * @brief   Debug function to ioctrl
*/
static void vUartIoCtrl( void )
{
    uint32_t ulOption = 0;
    uint32_t ulValue  = 0;
    void     *pvValue = NULL;

    /* IO ctrl option */
    if( OK != iDAL_GetHex( "Enter IO ctrl option: ", &ulOption ) )
    {
        vUartUserInputError( "IO ctrl option" );
    }

    /* Get/set value */
    if( OK != iDAL_GetHex( "Enter value to set/get: ", &ulValue ) )
    {
        vUartUserInputError( "Value to set/get" );
    }
    else
    {
        pvValue = &ulValue;
    }

    if( ( NULL == pxUartIf->ioctrl ) || ( OK != pxUartIf->ioctrl( pxUartIf, ulOption, pvValue ) ) )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error controlling IO\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Success controlling IO\r\n" );
    }
}

/**
 * @brief Debug function to bind a callback
*/
static void vUartBindCallback( void )
{
    if( ( NULL == pxUartIf->bindCallback ) || ( OK != pxUartIf->bindCallback( pxUartIf, &ulUartTestCallback ) ) )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Success binding callback\r\n" );
    }
}

/***** Debug print functions *****/

/**
 * @brief   Debug error print function.
 */
static void vUartUserInputError( char *pcString )
{
    if( NULL == pcString )
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error: null data entry\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_UART_DBG_NAME, "Error retrieving %s\r\n", pcString );
    }
}

/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 */
static uint32_t ulUartTestCallback( uint16_t usEventId, uint8_t *pucData, uint32_t ulSize )
{
    uint32_t ulStatus = OK;

    PLL_DAL( FW_IF_UART_DBG_NAME, "Uart Signal raised: ID[%d], Data buffer pointer[%p], Bytes[%d]\r\n", usEventId, pucData, ulSize );

    return ulStatus;
}
