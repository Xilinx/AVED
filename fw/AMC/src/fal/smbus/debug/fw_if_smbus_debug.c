/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the SMBUS debug implementation.
 *
 * @file fw_if_smbus_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "fw_if_smbus.h"
#include "fw_if_smbus_debug.h"

#include "dal.h"
#include "profile_fal.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FW_IF_SMBUS_DBG_NAME    "FW_IF_SMBUS_DBG"

#define FW_IF_SMBUS_DBG_MAX_BYTE_VALUE         ( 0xFF )
#define FW_IF_SMBUS_DBG_MAX_READ_WRITE_SIZE    ( 256 )
#define FW_IF_SMBUS_DBG_ADDRESS_OFFSET_MAX     ( 0x7F )

/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iInitialised       = FALSE;
static DAL_HDL pxFwIfSMBusTop = NULL;

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

/**
 * @brief   Debug function to open an SMBus instance.
 */
static void vOpenSmbusInstance( void );

/**
 * @brief   Debug function to close an SMBus instance.
 */
static void vCloseSmbusInstance( void );

/**
 * @brief   Debug function to write.
 */
static void vWrite( void );

/**
 * @brief   Debug function to read.
 */
static void vRead( void );

/**
 * @brief   Debug function to ioctrl.
 */
static void vIoCtrl( void );

/**
 * @brief   Debug function to bind a callback.
 */
static void vBindCallback( void );

/***** Debug print functions *****/

/**
 * @brief   Debug error print function.
 */
static void vUserInputError( char *string );

/***** Helper functions *****/

/**
 * @brief   Debug bind function.
 */
static uint32_t iTestCallback( uint16_t usEventId, uint8_t *pucData, uint32_t ulSize );

/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief    Initialise the FW IF SMBUS debug access
 */
void vFW_IF_SMBUS_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxFwIfSMBusTop = pxDAL_NewDirectory( "fw_if_smbus" );
        }
        else
        {
            pxFwIfSMBusTop = pxDAL_NewSubDirectory( "fw_if_smbus", pxParentHandle );
        }

        if( NULL != pxFwIfSMBusTop )
        {
            pxDAL_NewDebugFunction( "print_all_stats", pxFwIfSMBusTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_all_stats", pxFwIfSMBusTop, vClearStats );
            pxDAL_NewDebugFunction( "open", pxFwIfSMBusTop, vOpenSmbusInstance );
            pxDAL_NewDebugFunction( "close", pxFwIfSMBusTop, vCloseSmbusInstance );
            pxDAL_NewDebugFunction( "write", pxFwIfSMBusTop, vWrite );
            pxDAL_NewDebugFunction( "read", pxFwIfSMBusTop, vRead );
            pxDAL_NewDebugFunction( "io_ctrl", pxFwIfSMBusTop, vIoCtrl );
            pxDAL_NewDebugFunction( "bind_callback", pxFwIfSMBusTop, vBindCallback );
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
    if( OK != iFW_IF_SMBUS_PrintStatistics() )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats.
 */
static void vClearStats( void )
{
    if( OK != iFW_IF_SMBUS_ClearStatistics() )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to open an SMBus instance.
 */
static void vOpenSmbusInstance( void )
{
    if( ( NULL != pxSMBusIf->open ) &&
        ( OK != pxSMBusIf->open( pxSMBusIf ) ) )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error opening SMBus instance\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Success opening SMBus instance\r\n" );
    }
}

/**
 * @brief   Debug function to close an SMBus instance.
 */
static void vCloseSmbusInstance( void )
{
    if( ( NULL != pxSMBusIf->close ) &&
        ( OK != pxSMBusIf->close( pxSMBusIf ) ) )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error closing SMBus instance\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Success closing SMBus instance\r\n" );
    }
}

/**
 * @brief   Debug function to write.
 */
static void vWrite( void )
{
    int      i            = 0;
    uint32_t ulAddrOffset = 0;
    uint32_t ulLength     = 0;
    uint32_t ulTimeoutMs  = 0;
    uint32_t ulData       = 0;
    uint8_t  ucData[FW_IF_SMBUS_DBG_MAX_READ_WRITE_SIZE] =
    {
        0
    };

    /* Address offset */
    if( OK != iDAL_GetHexInRange( "Enter address offset: ", &ulAddrOffset, 0x00, FW_IF_SMBUS_DBG_ADDRESS_OFFSET_MAX ) )
    {
        char *pcOffset = "Address offset";
        vUserInputError( pcOffset );
    }

    /* Bytes in data buffer */
    if( OK != iDAL_GetHexInRange( "Enter data buffer bytes: ", &ulLength, 0, FW_IF_SMBUS_DBG_MAX_READ_WRITE_SIZE ) )
    {
        char *pcBufferBytes = "Data buffer bytes";
        vUserInputError( pcBufferBytes );
    }

    /* Data buffer */
    for ( i = 0; i < ulLength; i++ )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "==> Data byte %d", i );
        if ( OK != iDAL_GetHexInRange( "Enter data buffer: ", &ulData, 0, FW_IF_SMBUS_DBG_MAX_BYTE_VALUE ) )
        {
            char *pcDataBuffer = "Data buffer";
            vUserInputError( pcDataBuffer );
        }
        else
        {
            ucData[i] = ( uint8_t )ulData;
        }
    }

    /* Timeout to complete write */
    if( OK != iDAL_GetHex( "Enter timeout: ", &ulTimeoutMs) )
    {
        char *pcTimeout = "Timeout";
        vUserInputError( pcTimeout );
    }

    if( ( NULL != pxSMBusIf->write ) &&
        ( OK != pxSMBusIf->write( pxSMBusIf, ulAddrOffset, ucData, ulLength, ulTimeoutMs ) ) )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error writing\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Success writing\r\n" );
    }
}

/**
 * @brief   Debug function to read.
 */
static void vRead( void )
{
    int      i            = 0;
    uint32_t ulAddrOffset = 0;
    uint32_t ulLength     = 0;
    uint32_t ulTimeoutMs  = 0;
    uint8_t  ucData[FW_IF_SMBUS_DBG_MAX_READ_WRITE_SIZE] =
    {
        0
    };

    /* Address offset */
    if( OK != iDAL_GetHexInRange( "Enter address offset: ", &ulAddrOffset, 0x00, FW_IF_SMBUS_DBG_ADDRESS_OFFSET_MAX ) )
    {
            char *pcOffset= "Address offset";
            vUserInputError( pcOffset );
    }

    /* Timeout to complete read */
    if( OK != iDAL_GetHex( "Enter timeout: ", &ulTimeoutMs ) )
    {
        char *pcTimeout = "Timeout";
        vUserInputError( pcTimeout );
    }

    if( ( NULL != pxSMBusIf->read ) &&
        ( OK != pxSMBusIf->read( pxSMBusIf, ulAddrOffset, ucData, &ulLength, ulTimeoutMs ) ) )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error reading\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Success reading\r\n" );
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "=== Data ===\r\n" );

        /* Data buffer */
        for ( i = 0; i < ulLength; i++ )
        {
            PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Byte %d: 0x%02x\r\n", i, ucData[i] );
        }
    }
}

/**
 * @brief   Debug function to ioctrl.
 */
static void vIoCtrl( void )
{
    uint32_t ulOption = 0;
    uint32_t ulValue  = 0;
    void     *pvValue = NULL;

    /* IO ctrl option */
    if( OK != iDAL_GetHex( "Enter IO ctrl option: ", &ulOption ) )
    {
        char *pcIoCtrl = "IO ctrl option";
        vUserInputError( pcIoCtrl );
    }

    /* Pointer */
    if( OK != iDAL_GetHex( "Enter pointer to value to set/get: ", &ulValue ) )
    {
        char *pcPointer = "Pointer to value to set/get";
        vUserInputError( pcPointer );
    }
    else
    {
        pvValue = &ulValue;
    }

    /* pxSMBusIf->ioctrl function is TODO */
    if( ( NULL != pxSMBusIf->ioctrl ) &&
        ( OK != pxSMBusIf->ioctrl( pxSMBusIf, ulOption, &pvValue ) ) )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error controlling IO\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Success controlling IO\r\n" );
    }
}

/**
 * @brief   Debug function to bind a callback.
 */
static void vBindCallback( void )
{
    /* pxSMBusIf->bindCallback function is TODO */
    if( ( NULL != pxSMBusIf->bindCallback ) &&
        ( OK != pxSMBusIf->bindCallback( pxSMBusIf, &iTestCallback ) ) )
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Success binding callback\r\n" );
    }
}

/***** Debug print functions *****/

/**
 * @brief   Debug error print function.
 */
static void vUserInputError( char *string )
{
    PLL_DAL( FW_IF_SMBUS_DBG_NAME, "Error retrieving %s\r\n", string );
}

/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 */
static uint32_t iTestCallback( uint16_t usEventId, uint8_t *pucData, uint32_t ulSize )
{
    uint32_t ulStatus = OK;

    PLL_DAL( FW_IF_SMBUS_DBG_NAME, "SMBus Signal raised: ID[%d], Data buffer pointer[%p], Bytes[%d]\r\n",
        usEventId, pucData, ulSize );

    return ulStatus;
}
