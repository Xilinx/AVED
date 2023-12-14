/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the EMMC driver debug implementation
 *
 * @file emmc_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "emmc_debug.h"
#include "emmc.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define EMMC_DBG_NAME               "EMMC_DBG"
#define EMMC_MAX_BLOCKS_MALLOCED    ( 32 )
#define EMMC_BLOCK_SIZE             ( 0x200 )
#define EMMC_BLOCKS_PER_MB          ( ( 1024 * 1024 ) / EMMC_BLOCK_SIZE )
#define HAL_EMMC_MAX_BLOCKS         ( 0x7690000 )
#define EMMC_BLOCK_BITSHIFT         ( 9 )
#define EMMC_LAST_BLOCK             ( 0x7690000 - 1 )
#define EMMC_BLOCKS_IN_1GB          ( 1024 * EMMC_BLOCKS_PER_MB )
#define EMMC_BLOCKS_IN_32MB         ( 32 * EMMC_BLOCKS_PER_MB )
#define EMMC_BLOCKS_IN_64MB         ( 64 * EMMC_BLOCKS_PER_MB )
#define EMMC_BLOCKS_IN_128MB        ( 128 * EMMC_BLOCKS_PER_MB )
#define EMMC_BLOCKS_IN_256MB        ( 256 * EMMC_BLOCKS_PER_MB )
#define EMMC_BLOCKS_IN_512MB        ( 512 * EMMC_BLOCKS_PER_MB )
#define EMMC_NON_ZERO_VALUE         ( 0xA5 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int      iIsInitialised = FALSE;
static DAL_HDL  pxEMMCTop = NULL;
static DAL_HDL  pxGetDir = NULL;
static DAL_HDL  pxSetDir = NULL;
static DAL_HDL  pxTestDir = NULL;
static int      iReadResult = 0;
static int      iReadTestComplete = 0;
static uint32_t ulFailedBlock = 0;

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
 * @brief   Debug function to read from EMMC
 *
 * @return  N/A
 */
static void vEmmcRead( void );

/**
 * @brief   Debug function to Read and check EMMC
 *
 * @return  N/A
 */
static void vEmmcGetReadTestResults( void );

/**
 * @brief   Debug function to read selected addresses from EMMC
 *
 * @return  N/A
 */
static void vEmmcReadTest( void );

/**
 * @brief   Debug function to write to EMMC
 *
 * @return  N/A
 */
static void vEmmcWrite( void );

/**
 * @brief   Erase a range of the EMMC
 *
 * @return  N/A
 */
static void vEmmcErase( void );

/**
 * @brief   Debug function to erase all of the EMMC
 *          using fixed number of blocks
 *
 * @return  N/A
 */
static void vEmmcEraseAll( void );

/**
 * @brief   Debug function to erase all of the EMMC
 *
 * @return  N/A
 */
static void vEmmcEraseBlocks( void );

/**
 * @brief   Debug function to print EMMC instance details
 *
 * @return  N/A
 */
static void vPrintDetails( void );

/**
 * @brief   Debug function to fill variable Gbyte chunks of EMMC
 *
 * @return  N/A
 */
static void vEmmcFillTest( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the EMMC debug access
 */
void vEMMC_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxEMMCTop = pxDAL_NewDirectory( "EMMC" );
        }
        else
        {
            pxEMMCTop = pxDAL_NewSubDirectory( "EMMC", pxParentHandle );
        }

        if( NULL != pxEMMCTop )
        {
            pxDAL_NewDebugFunction( "print_stats",              pxEMMCTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",              pxEMMCTop, vClearStats );
            pxDAL_NewDebugFunction( "print_instance_details",   pxEMMCTop, vPrintDetails );

            pxSetDir  = pxDAL_NewSubDirectory( "sets", pxEMMCTop );
            pxGetDir  = pxDAL_NewSubDirectory( "gets", pxEMMCTop );
            pxTestDir = pxDAL_NewSubDirectory( "tests", pxEMMCTop );

            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "read",         pxGetDir, vEmmcRead );
            }
            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "write",        pxSetDir, vEmmcWrite );
                pxDAL_NewDebugFunction( "erase",        pxSetDir, vEmmcErase );
                pxDAL_NewDebugFunction( "erase_all",    pxSetDir, vEmmcEraseAll );
                pxDAL_NewDebugFunction( "erase_blocks", pxSetDir, vEmmcEraseBlocks );
            }
            if( NULL != pxTestDir )
            {
                pxDAL_NewDebugFunction( "read_test",    pxTestDir, vEmmcReadTest );
                pxDAL_NewDebugFunction( "get_results",  pxTestDir, vEmmcGetReadTestResults );

                pxDAL_NewDebugFunction( "fill_test",    pxTestDir, vEmmcFillTest );
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
    if( OK != iEMMC_PrintStatistics( ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iEMMC_ClearStatistics( ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to read from the EMMC
 */
static void vEmmcRead( void )
{
    uint32_t    ulByteCount = 0;
    uint64_t    ullAddress64 = 0;
    uint32_t    ulAddress = 0;
    int         iAddressShift = 0;
    uint64_t    ullPrintAddress = 0;
    uint32_t    ulBlockCount = 0;
    uint8_t     *pucReadBuffer = NULL;

    if( OK != iDAL_GetHexInRange( "Enter Address:", &ulAddress, 0, UTIL_MAX_UINT32 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Address Shift:", &iAddressShift, 0, 4 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Address\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "Enter Number of Blocks:", &ulBlockCount, 1, 32 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrievingBlock Count\r\n" );
    }
    else
    {
        pucReadBuffer = ( uint8_t* )pvOSAL_MemAlloc( EMMC_BLOCK_SIZE * ulBlockCount * sizeof( uint8_t ) );

        if( NULL != pucReadBuffer )
        {
            pvOSAL_MemSet( pucReadBuffer, 0xA5, ( EMMC_BLOCK_SIZE * ulBlockCount * sizeof( uint8_t ) ) );
            ullAddress64 = ulAddress;
            ullAddress64 = ullAddress64 << iAddressShift;
            ullPrintAddress = ullAddress64;
            PLL_DAL( EMMC_DBG_NAME, "ullAddress64 0x%llx ulBlockCount 0x%x\r\n",
                                     ullAddress64, ulBlockCount );
            if( OK != iEMMC_Read( ullAddress64, ulBlockCount, pucReadBuffer ) )
            {
                PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_Read\r\n" );
            }
            else
            {
                for( ulByteCount = 0; ulByteCount < ( EMMC_BLOCK_SIZE * ulBlockCount ); ulByteCount++ )
                {
                    if( 0 == ulByteCount % 16 )
                    {
                        vPLL_Printf( "\r\n0x%08llx ", ullPrintAddress );
                        ullPrintAddress = ullPrintAddress + 16;
                    }
                    vPLL_Printf( "0x%02x ", pucReadBuffer[ ulByteCount ] );
                }
            }
            vOSAL_MemFree( ( void** )&pucReadBuffer );
        }
        else
        {
            PLL_DAL( EMMC_DBG_NAME, "Error malloc failed\r\n" );
        }
    }
}

/**
 * @brief   Debug function to write to the EMMC
 */
static void vEmmcWrite( void )
{
    uint32_t    ulByteCount = 0;
    uint64_t    ullAddress64 = 0;
    uint32_t    ulAddress = 0;
    int         iAddressShift = 0;
    uint32_t    ulBlockCount = 0;
    int         iInitialByte = 0;
    uint8_t     *pucData = NULL;

    if( OK != iDAL_GetHexInRange( "Enter Address:", &ulAddress, 0, UTIL_MAX_UINT32 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Address Shift:", &iAddressShift, 0, 4 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Address\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "Enter Number of Blocks:", &ulBlockCount, 1, 32 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrievingBlock Count\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Initial Byte:", &iInitialByte, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Initial Byte\r\n" );
    }
    else
    {
        pucData = ( uint8_t* )pvOSAL_MemAlloc( EMMC_BLOCK_SIZE * ulBlockCount * sizeof( uint8_t ) );

        if( NULL != pucData )
        {
            /* Create incrementing data */
            for ( ulByteCount = 0; ulByteCount < ( EMMC_BLOCK_SIZE * ulBlockCount ); ulByteCount++ )
            {
                    pucData[ ulByteCount ] = iInitialByte++;
            }

            ullAddress64 = ulAddress;
            ullAddress64 = ullAddress64 << iAddressShift;

            if( OK != iEMMC_Write( ullAddress64, ulBlockCount, pucData ) )
            {
                PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_Write\r\n" );
            }

            vOSAL_MemFree( ( void** )&pucData );
        }
        else
        {
            PLL_DAL( EMMC_DBG_NAME, "Error malloc failed\r\n" );
        }
    }
}

/**
 * @brief   Debug function to erase part of the EMMC
 */
static void vEmmcErase( void )
{
    uint32_t    ulStartAddress = 0;
    uint32_t    ulEndAddress = 0;
    int         iStartAddressShift = 0;
    int         iEndAddressShift = 0;

    if( OK != iDAL_GetHexInRange( "Enter Start Address:", &ulStartAddress, 0, UTIL_MAX_UINT32 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Start Address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Address Multiplier:", &iStartAddressShift, 0, 4 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Address\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "Enter End Address:", &ulEndAddress, 0, UTIL_MAX_UINT32 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving End Address\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Enter Address Multiplier:", &iEndAddressShift, 0, 4 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Address\r\n" );
    }
    else
    {
        uint64_t    ullStart = 0;
        uint64_t    ullEnd = 0;

        ullStart = ulStartAddress;
        ullStart = ullStart << iStartAddressShift;
        ullEnd = ulEndAddress;
        ullEnd = ullEnd << iEndAddressShift;

        if( OK != iEMMC_Erase( ullStart, ullEnd ) )
        {
            PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_Erase\r\n" );
        }
    }
}

/**
 * @brief   Debug function to erase all of the EMMC
 */
static void vEmmcEraseAll( void )
{
    if( OK != iEMMC_EraseAll() )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_EraseAll\r\n" );
    }
}

/**
 * @brief   Debug function to fill variable Gbyte chunks of EMMC
 */
static void vEmmcFillTest( void )
{
    uint32_t    ulBlock = 0;
    uint64_t    ullAddress = 0;
    uint8_t     *pucData = NULL;
    int         iSizeGB = 0;
    uint32_t    ulElapsedTimeMs = 0;

    if( OK != iDAL_GetIntInRange( "Fill how many GB ( 1 - 64 )", &iSizeGB, 1, 64 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Check\r\n" );
    }
    else
    {
        /* Can only malloc 32 blocks at a time */
        pucData = ( uint8_t* )pvOSAL_MemAlloc( EMMC_BLOCK_SIZE * EMMC_MAX_BLOCKS_MALLOCED * sizeof( uint8_t ) );

        if( NULL != pucData )
        {
            uint32_t ulStartMs = 0;

            pvOSAL_MemSet( pucData, EMMC_NON_ZERO_VALUE, ( EMMC_BLOCK_SIZE * EMMC_MAX_BLOCKS_MALLOCED * sizeof( uint8_t ) ) );

            ulStartMs = ulOSAL_GetUptimeMs();

            for( ulBlock = 0; ulBlock < HAL_EMMC_MAX_BLOCKS; ulBlock = ulBlock + EMMC_MAX_BLOCKS_MALLOCED )
            {
                if( ( EMMC_BLOCKS_IN_1GB * iSizeGB ) == ulBlock )
                {
                    break;
                }

                ullAddress = ulBlock;
                ullAddress = ullAddress << EMMC_BLOCK_BITSHIFT;

                if( OK != iEMMC_Write( ullAddress, EMMC_MAX_BLOCKS_MALLOCED, pucData ) )
                {
                    PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_Write at block 0x%x\r\n", ulBlock );
                }

                if( ( 0 == ulBlock % EMMC_BLOCKS_IN_1GB ) ||
                    ( EMMC_BLOCKS_IN_32MB   == ulBlock )  ||
                    ( EMMC_BLOCKS_IN_64MB   == ulBlock )  ||
                    ( EMMC_BLOCKS_IN_128MB  == ulBlock )  ||
                    ( EMMC_BLOCKS_IN_256MB  == ulBlock )  ||
                    ( EMMC_BLOCKS_IN_512MB  == ulBlock ) )
                {
                    if( 0 != ulBlock )
                    {
                        switch( ulBlock )
                        {
                            case EMMC_BLOCKS_IN_32MB:
                            case EMMC_BLOCKS_IN_64MB:
                            case EMMC_BLOCKS_IN_128MB:
                            case EMMC_BLOCKS_IN_256MB:
                            case EMMC_BLOCKS_IN_512MB:
                                ulElapsedTimeMs = UTIL_ELAPSED_TIME_MS( ulStartMs )
                                vPLL_Printf( "%dMB %ds\r\n",
                                             ( ( ( 32 *ulBlock ) / EMMC_BLOCKS_IN_32MB ) ), ( ulElapsedTimeMs / 1000 ) );
                                break;

                            default:
                                ulElapsedTimeMs = UTIL_ELAPSED_TIME_MS( ulStartMs )
                                vPLL_Printf( "%dGB %ds\r\n",
                                             ( ( ulBlock / EMMC_BLOCKS_IN_1GB ) ), ( ulElapsedTimeMs / 1000 ) );
                                break;
                        }
                    }
                }
            }

            ulElapsedTimeMs = UTIL_ELAPSED_TIME_MS( ulStartMs )
            PLL_DAL( EMMC_DBG_NAME, "Write time for %dGB was %ds\r\n", iSizeGB, ulElapsedTimeMs / 1000 );

            vOSAL_MemFree( ( void** )&pucData );
        }
        else
        {
            PLL_DAL( EMMC_DBG_NAME, "Error malloc failed\r\n" );
        }
    }
}

/**
 * @brief   Debug function to Read and check EMMC
 */
static void vEmmcGetReadTestResults( void )
{
    if( 1 == iReadTestComplete )
    {
        PLL_DAL( EMMC_DBG_NAME, "Test complete\r\n" );
        if( 1 == iReadResult )
        {
            PLL_DAL( EMMC_DBG_NAME, "Test failed at block 0x%lx\r\n", ulFailedBlock );
        }
        else
        {
            PLL_DAL( EMMC_DBG_NAME, "Test passed\r\n" );
        }
    }
    else
    {
        PLL_DAL( EMMC_DBG_NAME, "Test not complete yet\r\n" );
    }
}

/**
 * @brief   Debug function to Read and check EMMC
 */
static void vEmmcReadTest( void )
{
    uint32_t    ulBlock = 0;
    uint8_t     *pucReadBuffer = NULL;
    uint64_t    ullAddress64 = 0;
    int         iSizeGB = 0;
    int         iCheck = 0;

    iReadResult = 0;
    iReadTestComplete = 0;
    ulFailedBlock = 0;

    if( OK != iDAL_GetIntInRange( "Read how many GB ( 1 - 64 )", &iSizeGB, 1, 64 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Check\r\n" );
    }
    else if( OK != iDAL_GetIntInRange( "Check for zero - 0, Check for non-zero - 1", &iCheck, 0, 1 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Check\r\n" );
    }
    else
    {
        pucReadBuffer = ( uint8_t* )pvOSAL_MemAlloc( EMMC_BLOCK_SIZE * sizeof( uint8_t ) );

        if( NULL != pucReadBuffer )
        {
            for( ulBlock = 0; ulBlock < HAL_EMMC_MAX_BLOCKS; ulBlock++ )
            {
                if( ( EMMC_BLOCKS_IN_1GB * iSizeGB ) == ulBlock )
                {
                    break;
                }

                ullAddress64 = ulBlock;
                ullAddress64 = ullAddress64 << EMMC_BLOCK_BITSHIFT;

                if( 0 == iCheck)
                {
                    /* Set the read buffer to non-zero before each read */
                    pvOSAL_MemSet( pucReadBuffer, 0xFF, ( EMMC_BLOCK_SIZE * sizeof( uint8_t ) ) );
                }
                else
                {
                    /* Set the read buffer to 0 before each read */
                    pvOSAL_MemSet( pucReadBuffer, 0, ( EMMC_BLOCK_SIZE * sizeof( uint8_t ) ) );
                }

                if( OK != iEMMC_Read( ullAddress64, 1, pucReadBuffer ) )
                {
                    PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_Read\r\n" );
                }
                else
                {
                    if( 0 == iCheck)
                    {
                        if( 0 != pucReadBuffer[ 0 ] )
                        {
                            PLL_DAL( EMMC_DBG_NAME, "Block 0x%x First byte 0x%x \r\n", ulBlock, pucReadBuffer[ 0 ] );
                            iReadResult = 1;
                            ulFailedBlock = ulBlock;
                        }
                    }
                    else
                    {
                        if( 0 == pucReadBuffer[ 0 ] )
                        {
                            PLL_DAL( EMMC_DBG_NAME, "Block 0x%x First byte 0x%x \r\n", ulBlock, pucReadBuffer[ 0 ] );
                            iReadResult = 1;
                            ulFailedBlock = ulBlock;
                        }
                    }
                }
            }
            vOSAL_MemFree( ( void** )&pucReadBuffer );
        }
        else
        {
            PLL_DAL( EMMC_DBG_NAME, "Error malloc failed\r\n" );
        }
    }
    iReadTestComplete = 1;
}

/**
 * @brief   Debug function to erase all of the EMMC
 */
static void vEmmcEraseBlocks( void )
{
    uint64_t ullStart = 0;
    uint64_t ullEnd = 0;
    uint32_t ulBlockIncrement = 0;
    uint32_t ulBlock = 0;

    ullEnd = EMMC_LAST_BLOCK;
    ullEnd = ullEnd << EMMC_BLOCK_BITSHIFT;

    if( OK != iDAL_GetHexInRange( "Erase how many blocks at a time ( 0 - All blocks )", &ulBlockIncrement, 0, 0x8000000 ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error retrieving Check\r\n" );
    }
    else
    {
        if( 0 == ulBlockIncrement )
        {
            if( OK != iEMMC_Erase( ullStart, ullEnd ) )
            {
                PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_Erase\r\n" );
            }
        }
        else
        {
            for( ulBlock = 0; ulBlock < HAL_EMMC_MAX_BLOCKS; ulBlock = ulBlock + ulBlockIncrement )
            {
                ullStart = ulBlock;
                ullStart = ullStart << EMMC_BLOCK_BITSHIFT;
                ullEnd = ulBlock + ulBlockIncrement - 1;
                if( HAL_EMMC_MAX_BLOCKS < ullEnd )
                {
                    ullEnd = HAL_EMMC_MAX_BLOCKS;
                }
                ullEnd = ullEnd << EMMC_BLOCK_BITSHIFT;

                if( OK != iEMMC_Erase( ullStart, ullEnd ) )
                {
                    PLL_DAL( EMMC_DBG_NAME, "Error iEMMC_Erase\r\n" );
                }
            }
        }
    }
}

/**
 * @brief   Debug function to print EMMC instance details
 *
 * @return  N/A
 */
static void vPrintDetails( void )
{
    if( OK != iEMMC_PrintInstanceDetails( ) )
    {
        PLL_DAL( EMMC_DBG_NAME, "Error printing statistics\r\n" );
    }
}
