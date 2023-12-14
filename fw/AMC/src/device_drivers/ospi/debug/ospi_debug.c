/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the OSPI driver debug implementation
 *
 * @file ospi_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "ospi_debug.h"
#include "ospi.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define OSPI_DBG_NAME       "OSPI_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxOspiTop = NULL;
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
 * @brief   Debug function to read a number of bytes from the flash device.
 *
 * @return  N/A
 */
static void vGetFlashRead( void );

/**
 * @brief   Debug function to get flash opertation progress
 *
 * @return  N/A
 */
static void vGetOperationProgress( void );

/**
 * @brief   Debug function erase flash device
 *
 * @return  N/A
 */
static void vSetFlashErase( void );

/**
 * @brief   Debug function to write to flash device
 *
 * @return  N/A
 */
static void vSetFlashWrite( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the OSPI debug access
 */
void vOSPI_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxOspiTop = pxDAL_NewDirectory( "ospi" );
        }
        else
        {
            pxOspiTop = pxDAL_NewSubDirectory( "ospi", pxParentHandle );
        }

        if( NULL != pxOspiTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxOspiTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxOspiTop, vClearStats );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxOspiTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxOspiTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_flash_erase", pxSetDir, vSetFlashErase );
                pxDAL_NewDebugFunction( "set_flash_write", pxSetDir, vSetFlashWrite );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_progress",   pxGetDir, vGetOperationProgress );
                pxDAL_NewDebugFunction( "get_flash_read", pxGetDir, vGetFlashRead );
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
    if( OK != iOSPI_PrintStatistics() )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iOSPI_ClearStatistics() )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to erase flash
 */
static void vSetFlashErase( void )
{
    int iLength     = 0;
    uint32_t ulAddr = 0;

    if( OK != iDAL_GetHex( "Enter address to erase:", &ulAddr ) )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error retrieving address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter length:", &iLength, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error retrieving length \r\n" );
    }
    else
    {
        if( OK != iOSPI_FlashErase( ulAddr, ( uint32_t )iLength ) )
        {
            PLL_DAL( OSPI_DBG_NAME, "Error erasing address 0x%08X\r\n", ulAddr );
        }
        else
        {
            PLL_DAL( OSPI_DBG_NAME, "Successfuly erased address 0x%08X\r\n", ulAddr );
        }
    }
}

/**
 * @brief   Debug function to write to flash
 */
static void vSetFlashWrite( void )
{
    int iLength = 0;
    uint32_t ulAddr = 0;
    uint8_t *pucWriteBuff = NULL;

    if( OK != iDAL_GetHex( "Enter address to write to:", &ulAddr ) )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error retrieving address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter length:", &iLength, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error retrieving length \r\n" );
    }
    else
    {
        pucWriteBuff = ( uint8_t* )pvOSAL_MemAlloc( iLength * sizeof( uint8_t ) );

        if( NULL != pucWriteBuff )
        {
            int i = 0;

            /* getting 1 byte at a time via stdin */
            for( i = 0; i < iLength; i++ )
            {
                uint32_t ulTemp = 0;

                if( OK != iDAL_GetHex( "Enter byte to send (hex):", &ulTemp ) || 
                  ( ulTemp > 0xFF ) )
                {
                    PLL_DAL( OSPI_DBG_NAME, "Invalid input\r\n" );
                }

                pucWriteBuff[i] = ( uint8_t )ulTemp;
            }

            if( OK != iOSPI_FlashWrite( ulAddr, pucWriteBuff, ( uint32_t )iLength ) )
            {
                PLL_DAL( OSPI_DBG_NAME, "Error writing to address 0x%08X\r\n", ulAddr );
            }
            else
            {
                PLL_DAL( OSPI_DBG_NAME, "Successfuly wrote to address 0x%08X\r\n", ulAddr );
            }

            vOSAL_MemFree( ( void** ) &pucWriteBuff );
        }
        else
        {
            PLL_DAL( OSPI_DBG_NAME, "Malloc for write buffer failed" );
        }
    }
}

/**
 * @brief   Debug function to read from flash
 */
static void vGetFlashRead( void )
{
    int iLength = 0;
    uint32_t ulAddr = 0;  
    uint8_t *pucReadBuff = NULL;

    if( OK != iDAL_GetHex( "Enter address to read:", &ulAddr ) )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error retrieving address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter length:", &iLength, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error retrieving length \r\n" );
    }
    else
    {
        pucReadBuff = ( uint8_t* )pvOSAL_MemAlloc( iLength * sizeof( uint8_t ) );

        if( NULL != pucReadBuff )
        {
            if( OK != iOSPI_FlashRead( ulAddr, pucReadBuff, ( uint32_t* )&iLength ) )
            {
                PLL_DAL( OSPI_DBG_NAME, "Error reading address 0x%08X\r\n", ulAddr );
            }
            else
            {
                int i = 0;
                
                PLL_DAL( OSPI_DBG_NAME, "Successfully read address 0x%08X\r\n", ulAddr );
                PLL_DAL( OSPI_DBG_NAME, "Buffer: \r\n" );
                for(i = 0; i < iLength; i++)
                {
                    vPLL_Printf( "%02X ", pucReadBuff[i] );
                }

                vPLL_Printf( "\r\n" );
            }

            vOSAL_MemFree( ( void** ) &pucReadBuff );
        }
        else
        {
            PLL_DAL( OSPI_DBG_NAME, "Malloc for read buffer failed" );    
        }
    }
}

/**
 * @brief   Debug function to get flash operation progress
 */
static void vGetOperationProgress( void )
{
    uint8_t pucPercentage = 0;

    if( OK != iOSPI_GetOperationProgress( &pucPercentage ) )
    {
        PLL_DAL( OSPI_DBG_NAME, "Error getting operation progress\r\n" );
    }
    else
    {
        PLL_DAL( OSPI_DBG_NAME, "Operation Progress: %d%%\r\n", pucPercentage );
    }
}
