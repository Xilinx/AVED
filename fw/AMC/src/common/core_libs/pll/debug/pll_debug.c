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
 * @brief   Fucntion to get verbosity level
 */
static void vPLL_GetLevel( void );

/**
 * @brief   Fucntion to set verbosity level
 */
static void vPLL_SetLevel( void );

/**
 * @brief   Function to test the PPL print macros 
 */
static void vTestPrint( void );

/**
 * @brief   Dumps log from shared memory 
 */
static void vPLL_DumpLog( void );

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
                pxDAL_NewDebugFunction( "set_level", pxSetDir, vPLL_SetLevel );
                pxDAL_NewDebugFunction( "clear_log", pxSetDir, vPLL_ClearLog );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_level", pxGetDir, vPLL_GetLevel );
                pxDAL_NewDebugFunction( "dump_log", pxGetDir, vPLL_DumpLog );
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
static void vPLL_GetLevel( void )
{
    PLL_OUTPUT_LEVEL *pxVerbosity = pvOSAL_MemAlloc( sizeof( PLL_OUTPUT_LEVEL ) );
    
    if( NULL != pxVerbosity )
    {
        if( OK != iPLL_GetLevel( pxVerbosity ) )
        {
            PLL_DAL( PLL_BDG_NAME, "Error getting verbosity level\r\n" );
        }
        else
        {
            PLL_DAL( PLL_BDG_NAME, "Current verbosity level: %d\r\n", *pxVerbosity );
        }

        vOSAL_MemFree( ( void** ) &pxVerbosity );
    }
}

/**
 * @brief   Debug function to set PLL verbosity level 
 */
static void vPLL_SetLevel( void )
{
    int iVerbosity = 0;
    
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_INFO       ", PLL_OUTPUT_LEVEL_INFO );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_ERROR      ", PLL_OUTPUT_LEVEL_ERROR );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_WARNING    ", PLL_OUTPUT_LEVEL_WARNING );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_LOGGING    ", PLL_OUTPUT_LEVEL_LOGGING );
    vPLL_Printf( "\r\n     %d: PLL_OUTPUT_LEVEL_DEBUG \r\n ", PLL_OUTPUT_LEVEL_DEBUG );

    if( OK != iDAL_GetIntInRange( "\r\nEnter verbosity level: ", &iVerbosity, 0, MAX_PLL_OUTPUT_LEVEL ) )
    {
        PLL_DAL( PLL_BDG_NAME, "Error retrieving verbosity level\r\n" );
    }

    if( OK != iPLL_SetLevel( ( PLL_OUTPUT_LEVEL )iVerbosity ) )
    {
        PLL_DAL( PLL_BDG_NAME, "Error setting verbosity level\r\n" );
    }
    else
    {
        PLL_DAL( PLL_BDG_NAME, "Successfuly set verbosity level\r\n" );
    }
}

/**
 * @brief   Debug function to test PLL macros
 */
static void vTestPrint( void )
{
    vPLL_Printf( "vPLL_Output Print\r\n" );

    PLL_INF( PLL_BDG_NAME, "Info Print\r\n" );
    PLL_ERR( PLL_BDG_NAME, "Error Print\r\n" );
    PLL_WRN( PLL_BDG_NAME, "Warning Print\r\n" );
    PLL_LOG( PLL_BDG_NAME, "Log Print\r\n" );
    PLL_DBG( PLL_BDG_NAME, "Debug Print\r\n" );
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
