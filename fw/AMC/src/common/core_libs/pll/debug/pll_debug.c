/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Printing and Logging Library (PLL) debug implementation
 *
 * @file pll_debug.c
 *
 */

#include "standard.h"
#include "util.h"

#include "dal.h"

#include "pll_debug.h"
#include "pll.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define PLL_BDG_NAME "PLL_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxPllTop = NULL;
static DAL_HDL pxGetDir = NULL;
static DAL_HDL pxSetDir = NULL;


/******************************************************************************/
/* Private function declarations                                              */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void );

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void );

/**
 * @brief   Function to get the output verbosity level (DAL)
 */
static void vPLL_GetOutputLevel( void );

/**
 * @brief   Function to get the logging verbosity level (dmesg)
 */
static void vPLL_GetLoggingLevel( void );

/**
 * @brief   Function to set the output verbosity level (DAL)
 */
static void vPLL_SetOutputLevel( void );

/**
 * @brief   Function to set the logging verbosity level (dmesg)
 */
static void vPLL_SetLoggingLevel( void );

/**
 * @brief   Function to test the PPL print macros 
 */
static void vTestPrint( void );

/**
 * @brief   Dumps log from shared memory 
 */
static void vPLL_DumpLog( void );

/**
 * @brief   Dumps FSBL log
 */
static void vPLL_DumpFsblLog( void );

/**
 * @brief   Clears shared memory log 
 */
static void vPLL_ClearLog( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the PLL debug access
 */
void vPLL_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxPllTop = pxDAL_NewDirectory( "pll" );
        }
        else
        {
            pxPllTop = pxDAL_NewSubDirectory( "pll", pxParentHandle );
        }

        if( NULL != pxPllTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxPllTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxPllTop, vClearStats );

            pxDAL_NewDebugFunction( "test_print", pxPllTop, vTestPrint );

            pxSetDir = pxDAL_NewSubDirectory( "sets", pxPllTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxPllTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_output_level", pxSetDir, vPLL_SetOutputLevel );
                pxDAL_NewDebugFunction( "set_logging_level", pxSetDir, vPLL_SetLoggingLevel );
                pxDAL_NewDebugFunction( "clear_log", pxSetDir, vPLL_ClearLog );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_output_level", pxGetDir, vPLL_GetOutputLevel );
                pxDAL_NewDebugFunction( "get_logging_level", pxGetDir, vPLL_GetLoggingLevel );
                pxDAL_NewDebugFunction( "dump_log", pxGetDir, vPLL_DumpLog );
                pxDAL_NewDebugFunction( "dump_fsbl_log", pxGetDir, vPLL_DumpFsblLog );
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
    if( OK != iPLL_PrintStatistics() )
    {
        PLL_DAL( PLL_BDG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iPLL_ClearStatistics() )
    {
        PLL_DAL( PLL_BDG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to get current PLL verbosity level 
 */
static void vPLL_GetOutputLevel( void )
{
    PLL_OUTPUT_LEVEL *pxVerbosity = pvOSAL_MemAlloc( sizeof( PLL_OUTPUT_LEVEL ) );
    
    if( NULL != pxVerbosity )
    {
        if( OK != iPLL_GetOutputLevel( pxVerbosity ) )
        {
            PLL_DAL( PLL_BDG_NAME, "Error getting output verbosity level\r\n" );
        }
        else
        {
            PLL_DAL( PLL_BDG_NAME, "Current output verbosity level: %d\r\n", *pxVerbosity );
        }

        vOSAL_MemFree( ( void** ) &pxVerbosity );
    }
}

/**
 * @brief   Debug function to get current PLL verbosity level 
 */
static void vPLL_GetLoggingLevel( void )
{
    PLL_OUTPUT_LEVEL *pxVerbosity = pvOSAL_MemAlloc( sizeof( PLL_OUTPUT_LEVEL ) );
    
    if( NULL != pxVerbosity )
    {
        if( OK != iPLL_GetLoggingLevel( pxVerbosity ) )
        {
            PLL_DAL( PLL_BDG_NAME, "Error getting logging verbosity level\r\n" );
        }
        else
        {
            PLL_DAL( PLL_BDG_NAME, "Current logging verbosity level: %d\r\n", *pxVerbosity );
        }

        vOSAL_MemFree( ( void** ) &pxVerbosity );
    }
}

/**
 * @brief   Debug function to set PLL verbosity level 
 */
static void vPLL_SetOutputLevel( void )
{
    int iVerbosity = 0;
    
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_LOGGING    ", PLL_OUTPUT_LEVEL_LOGGING );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_INFO       ", PLL_OUTPUT_LEVEL_INFO );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_ERROR      ", PLL_OUTPUT_LEVEL_ERROR );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_WARNING    ", PLL_OUTPUT_LEVEL_WARNING );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_DEBUG \r\n ", PLL_OUTPUT_LEVEL_DEBUG );

    if( OK != iDAL_GetIntInRange( "\r\nEnter output verbosity level: ", &iVerbosity, 0, MAX_PLL_OUTPUT_LEVEL ) )
    {
        PLL_DAL( PLL_BDG_NAME, "Error retrieving output verbosity level\r\n" );
    }

    if( OK != iPLL_SetOutputLevel( ( PLL_OUTPUT_LEVEL )iVerbosity ) )
    {
        PLL_DAL( PLL_BDG_NAME, "Error setting output verbosity level\r\n" );
    }
    else
    {
        PLL_DAL( PLL_BDG_NAME, "Successfully set output verbosity level\r\n" );
    }
}

/**
 * @brief   Debug function to set PLL verbosity level 
 */
static void vPLL_SetLoggingLevel( void )
{
    int iVerbosity = 0;
    
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_LOGGING    ", PLL_OUTPUT_LEVEL_LOGGING );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_INFO       ", PLL_OUTPUT_LEVEL_INFO );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_ERROR      ", PLL_OUTPUT_LEVEL_ERROR );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_WARNING    ", PLL_OUTPUT_LEVEL_WARNING );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_DEBUG \r\n ", PLL_OUTPUT_LEVEL_DEBUG );

    if( OK != iDAL_GetIntInRange( "\r\nEnter logging verbosity level: ", &iVerbosity, 0, MAX_PLL_OUTPUT_LEVEL ) )
    {
        PLL_DAL( PLL_BDG_NAME, "Error retrieving logging verbosity level\r\n" );
    }

    if( OK != iPLL_SetLoggingLevel( ( PLL_OUTPUT_LEVEL )iVerbosity ) )
    {
        PLL_DAL( PLL_BDG_NAME, "Error setting logging verbosity level\r\n" );
    }
    else
    {
        PLL_DAL( PLL_BDG_NAME, "Successfully set logging verbosity level\r\n" );
    }
}

/**
 * @brief   Debug function to test PLL macros
 */
static void vTestPrint( void )
{
    vPLL_Printf( "vPLL_Output Print\r\n" );
    vPLL_Printf( "Will now test PLL macros in the order LOG, INFO, ERROR, WARNING, DEBUG\r\n\r\n" );

    PLL_LOG( PLL_BDG_NAME, "LOG Print\r\n" );
    PLL_INF( PLL_BDG_NAME, "INFO Print\r\n" );
    PLL_ERR( PLL_BDG_NAME, "ERROR Print\r\n" );
    PLL_WRN( PLL_BDG_NAME, "WARNING Print\r\n" );
    PLL_DBG( PLL_BDG_NAME, "DEBUG Print\r\n" );
}

/**
 * @brief   Dumps log from shared memory
 */
static void vPLL_DumpLog( void )
{
    if( OK != iPLL_DumpLog() )
    {
        PLL_DAL( PLL_BDG_NAME, "Error dumping log\r\n" );
    }
}

/**
 * @brief   Clears shared memory log
 */
static void vPLL_ClearLog( void )
{
    if( OK != iPLL_ClearLog() )
    {
        PLL_DAL( PLL_BDG_NAME, "Error clearing log\r\n" );
    }
}

/**
 * @brief   Dumps log from shared memory
 */
static void vPLL_DumpFsblLog( void )
{
    if( OK != iPLL_DumpFsblLog() )
    {
        PLL_DAL( PLL_BDG_NAME, "Error dumping fsbl log\r\n" );
    }
}
