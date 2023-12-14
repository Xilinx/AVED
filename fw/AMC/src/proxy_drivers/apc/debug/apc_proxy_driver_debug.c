/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Programming Control (APC) debug implementation
 *
 * @file apc_proxy_driver_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "apc_proxy_driver_debug.h"
#include "apc_proxy_driver.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define APC_DBG_NAME    "APC_DBG"

#define BUFFER_WIDTH    ( 16 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxApcTop = NULL;
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
 * @brief   Debug function to bind a callback to this module
 *
 * @return  N/A
 */
static void vBindCallbacks( void );

/**
 * @brief   Debug function to download an image to flash
 *
 * @return  N/A
 */
static void vSetDownloadImage( void );

/**
 * @brief   Debug function to copy an image in flash
 *
 * @return  N/A
 */
static void vSetCopyImage( void );

/**
 * @brief   Debug function to select the next boot partition
 *
 * @return  N/A
 */
static void vSetNextPartition( void );

/**
 * @brief   Debug function to enable hot reset
 *
 * @return  N/A
 */
static void vSetEnableHotReset( void );

/**
 * @brief   Debug function to retrieve the FPT header
 *
 * @return  N/A
 */
static void vGetFptHeader( void );

/**
 * @brief   Debug function to retrieve an FPT partition
 *
 * @return  N/A
 */
 static void vGetFptPartition( void );

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
 * @brief   Initialise the APC debug access
 */
void vAPC_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxApcTop = pxDAL_NewDirectory( "apc" );
        }
        else
        {
            pxApcTop = pxDAL_NewSubDirectory( "apc", pxParentHandle );
        }

        if( NULL != pxApcTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxApcTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxApcTop, vClearStats );
            pxDAL_NewDebugFunction( "bind_callbacks", pxApcTop, vBindCallbacks );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxApcTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxApcTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_download_image", pxSetDir, vSetDownloadImage );
                pxDAL_NewDebugFunction( "set_copy_image",     pxSetDir, vSetCopyImage );
                pxDAL_NewDebugFunction( "set_next_partition", pxSetDir, vSetNextPartition );
                pxDAL_NewDebugFunction( "enable_hot_reset",   pxSetDir, vSetEnableHotReset );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_fpt_header",    pxGetDir, vGetFptHeader );
                pxDAL_NewDebugFunction( "get_fpt_partition", pxGetDir, vGetFptPartition );
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
    if( OK != iAPC_PrintStatistics() )
    {
        PLL_DAL( APC_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iAPC_ClearStatistics() )
    {
        PLL_DAL( APC_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to bind a callback to this module
 */
static void vBindCallbacks( void )
{
    if( OK != iAPC_BindCallback( &iTestCallback ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( APC_DBG_NAME, "Callback bound\r\n" );
    }
}

/**
 * @brief   Debug function to download an image to flash
 */
static void vSetDownloadImage( void )
{
    int iInstance      = 0;
    int iPartition     = 0;
    int iImageSize     = 0;
    uint32_t ulSrcAddr = 0;
    int iPacketNum = 0; 
    int iPacketSize = 0;
    
    if( OK != iDAL_GetIntInRange( "Enter request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving instance\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter partition num:", &iPartition ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving partition number\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter number of bytes:", &iImageSize ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving image size\r\n" );
    }
    else if( OK != iDAL_GetHex( "Enter source (RAM) address:", &ulSrcAddr ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving source address\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter packet number:", &iPacketNum ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving packet number\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter packet size:", &iPacketSize ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving packet size\r\n" );
    }

    else
    {
        EVL_SIGNAL xSignal = { 0 };
        xSignal.ucInstance = iInstance;

        if( OK != iAPC_DownloadImage( &xSignal, iPartition, ulSrcAddr, ( uint32_t )iImageSize, 
                                      ( uint16_t )iPacketNum, ( uint16_t )iPacketSize ) )
        {
            PLL_DAL( APC_DBG_NAME, "Error writing %d bytes to partition %d\r\n", iImageSize, iPartition );
        }
        else
        {
            PLL_DAL( APC_DBG_NAME, "%d bytes written from 0x%08X to partition %d\r\n",
                     iImageSize, ulSrcAddr, iPartition );
        }
    }
}

/**
 * @brief   Debug function to download an image to flash
 */
static void vSetCopyImage( void )
{
    int iInstance      = 0;
    int iSrcPartition  = 0;
    int iDestPartition = 0;
    int iImageSize     = 0;
    uint32_t ulSrcAddr = 0;

    if( OK != iDAL_GetIntInRange( "Enter request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving instance\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter source partition num:", &iSrcPartition ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving source partition number\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter destination partition num:", &iDestPartition ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving destination partition number\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter maximum number of bytes:", &iImageSize ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving maximum image size\r\n" );
    }
    else if( OK != iDAL_GetHex( "Enter source (RAM) address:", &ulSrcAddr ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving source address\r\n" );
    }
    else
    {
        EVL_SIGNAL xSignal = { 0 };
        xSignal.ucInstance = iInstance;

        if( OK != iAPC_CopyImage( &xSignal, iSrcPartition, iDestPartition, ulSrcAddr, ( uint32_t )iImageSize ) )
        {
            PLL_DAL( APC_DBG_NAME, "Error copying partition %d partition %d\r\n", iSrcPartition, iDestPartition );
        }
        else
        {
            PLL_DAL( APC_DBG_NAME, "Partition %d copied to partition %d\r\n",
                     iSrcPartition, iDestPartition );
        }
    }
}
/**
 * @brief   Debug function to select the next boot partition
 */
static void vSetNextPartition( void )
{
    int iInstance  = 0;
    int iPartition = 0;
    
    if( OK != iDAL_GetIntInRange( "Enter request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving instance\r\n" );
    }
    else if( OK != iDAL_GetInt( "Enter partition num:", &iPartition ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving partition number\r\n" );
    }
    else
    {
        EVL_SIGNAL xSignal = { 0 };
        xSignal.ucInstance = iInstance;
        
        if( OK != iAPC_SetNextPartition( &xSignal, iPartition ) )
        {
            PLL_DAL( APC_DBG_NAME, "Error selecting partition %d\r\n", iPartition );
        }
        else
        {
            PLL_DAL( APC_DBG_NAME, "Selected partition %d\r\n", iPartition );
        }
    }
}

/**
 * @brief   Debug function to enable hot reset
 */
static void vSetEnableHotReset( void )
{
    int iInstance  = 0;
    
    if( OK != iDAL_GetIntInRange( "Enter request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving instance\r\n" );
    }
    else
    {
        EVL_SIGNAL xSignal = { 0 };
        xSignal.ucInstance = iInstance;
        
        if( OK != iAPC_EnableHotReset( &xSignal ) )
        {
            PLL_DAL( APC_DBG_NAME, "Error enabling hot reset\r\n" );
        }
        else
        {
            PLL_DAL( APC_DBG_NAME, "Hot reset enabled\r\n" );
        }
    }
}

/**
 * @brief   Debug function to retrieve the FPT header
 */
static void vGetFptHeader( void )
{
    APC_PROXY_DRIVER_FPT_HEADER xFptHeader = { 0 };

    if( OK != iAPC_GetFptHeader( &xFptHeader ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving FPT header\r\n" );
    }
    else
    {
        PLL_DAL( APC_DBG_NAME, "======================================================================\r\n" );
        PLL_DAL( APC_DBG_NAME, "FPT header:\r\n" );
        PLL_DAL( APC_DBG_NAME, "\tMagic number . . . . . : 0x%08X\r\n", xFptHeader.ulMagicNum );
        PLL_DAL( APC_DBG_NAME, "\tFPT version. . . . . . : 0x%02X\r\n", xFptHeader.ucFptVersion );
        PLL_DAL( APC_DBG_NAME, "\tFPT header size. . . . : 0x%02X\r\n", xFptHeader.ucFptHeaderSize );
        PLL_DAL( APC_DBG_NAME, "\tEntry size . . . . . . : 0x%02X\r\n", xFptHeader.ucEntrySize );
        PLL_DAL( APC_DBG_NAME, "\tNum entries. . . . . . : 0x%02X\r\n", xFptHeader.ucNumEntries );
        PLL_DAL( APC_DBG_NAME, "======================================================================\r\n" );    
    }
}

/**
 * @brief   Debug function to retrieve a FPT partition
 */
static void vGetFptPartition( void )
{
    int iPartition = 0;

    if( OK != iDAL_GetInt( "Enter partition num:", &iPartition ) )
    {
        PLL_DAL( APC_DBG_NAME, "Error retrieving partition number\r\n" );
    }
    else
    {
        APC_PROXY_DRIVER_FPT_PARTITION xFptPartition = { 0 };

        if( OK != iAPC_GetFptPartition( iPartition, &xFptPartition ) )
        {
            PLL_DAL( APC_DBG_NAME, "Error retrieving FPT Partition %d\r\n", iPartition );
        }
        else
        {
            PLL_DAL( APC_DBG_NAME, "======================================================================\r\n" );
            PLL_DAL( APC_DBG_NAME, "FPT partition %d:\r\n", iPartition );
            PLL_DAL( APC_DBG_NAME, "\tPartition type . . . . : 0x%08X\r\n", xFptPartition.ulPartitionType );
            PLL_DAL( APC_DBG_NAME, "\tPartition base address : 0x%08X\r\n", xFptPartition.ulPartitionBaseAddr );
            PLL_DAL( APC_DBG_NAME, "\tPartition size . . . . : 0x%08X\r\n", xFptPartition.ulPartitionSize );
            PLL_DAL( APC_DBG_NAME, "======================================================================\r\n" );    
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
        PLL_DAL( APC_DBG_NAME, "APC Signal raised: Module[%d], Type[%d], Instance[%d]\r\n",
                 pxSignal->ucModule, pxSignal->ucEventType, pxSignal->ucInstance );
        iStatus = OK;
    }

    return iStatus;
}

