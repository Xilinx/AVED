/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Debug Access Library (DAL) debug implementation
 *
 * @file dal_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "dal_debug.h"
#include "dal.h"

/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define DAL_DBG_NAME        "DAL_DBG"
#define DAL_TEST_MIN_INT    ( 15 )
#define DAL_TEST_MAX_INT    ( 25 )
#define DAL_TEST_MIN_FLOAT  ( 15.01 )
#define DAL_TEST_MAX_FLOAT  ( 15.99 )
#define DAL_TEST_MIN_HEX    ( 0xAA )
#define DAL_TEST_MAX_HEX    ( 0xBB )
#define DAL_TEST_STRING_LEN ( 10 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised  = FALSE;

static DAL_HDL pxDalTop    = NULL;
static DAL_HDL pxDalGetDir = NULL;

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
 * @brief   Debug function to check iDAL_GetInt
 * 
 * @return N/A
 */
static void vGetInt( void );

/**
 * @brief   Debug function to check iDAL_GetIntInRange
 * 
 * @return N/A
 */
static void vGetIntInRange( void );

/**
 * @brief   Debug function to check iDAL_GetFloat
 * 
 * @return N/A
 */
static void vGetFloat( void );

/**
 * @brief   Debug function to check iDAL_GetFloatInRange
 * 
 * @return N/A
 */
static void vGetFloatInRange( void );

/**
 * @brief   Debug function to check iDAL_GetHex
 * 
 * @return N/A
 */
static void vGetHex( void );

/**
 * @brief   Debug function to check iDAL_GetHexInRange
 * 
 * @return N/A
 */
static void vGetHexInRange( void );


/**
 * @brief   Debug function to check iDAL_GetString
 * 
 * @return N/A
 */
static void vGetString( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the DAL debug access
 */
void vDAL_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxDalTop = pxDAL_NewDirectory( "dal" );
        }
        else
        {
            pxDalTop = pxDAL_NewSubDirectory( "dal", pxParentHandle );
        }

        if( NULL != pxDalTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxDalTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxDalTop, vClearStats );
            pxDalGetDir = pxDAL_NewSubDirectory( "gets", pxDalTop );

            if( NULL != pxDalGetDir)
            {
                pxDAL_NewDebugFunction( "iDAL_GetInt", pxDalGetDir, vGetInt );
                pxDAL_NewDebugFunction( "iDAL_GetIntInRange", pxDalGetDir, vGetIntInRange );
                pxDAL_NewDebugFunction( "iDAL_GetFloat", pxDalGetDir, vGetFloat );
                pxDAL_NewDebugFunction( "iDAL_GetFloatInRange", pxDalGetDir, vGetFloatInRange );
                pxDAL_NewDebugFunction( "iDAL_GetHex", pxDalGetDir, vGetHex );
                pxDAL_NewDebugFunction( "iDAL_GetHexInRange", pxDalGetDir, vGetHexInRange );
                pxDAL_NewDebugFunction( "iDAL_GetString", pxDalGetDir, vGetString );
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
    if( OK != iDAL_PrintStatistics() )
    {
        PLL_DAL( DAL_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iDAL_ClearStatistics() )
    {
        PLL_DAL( DAL_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to check iDAL_GetInt
 */
static void vGetInt( void )
{
    int iTestValue = 0;

    if( OK == iDAL_GetInt( "Please enter an integer: ", &iTestValue ) )
    {
        vPLL_Printf( "Input integer is %d\r\n", iTestValue);
    }
    else
    {
        PLL_DAL( DAL_DBG_NAME, "Input integer is not valid\r\n");
    }
}

/**
 * @brief   Debug function to check iDAL_GetIntInRange
 */
static void vGetIntInRange( void )
{
    int iTestValue = 0;

    if( OK == iDAL_GetIntInRange( "Please enter an integer: ", &iTestValue, DAL_TEST_MIN_INT, DAL_TEST_MAX_INT ) )
    {
        vPLL_Printf( "Input integer is %d\r\n", iTestValue);
    }
    else
    {
        PLL_DAL( DAL_DBG_NAME, "Input integeR is not valid\r\n");
    }
}

/**
 * @brief   Debug function to check iDAL_GetFloat
 */
static void vGetFloat( void )
{
    float fTestValue = 0;

    if( OK == iDAL_GetFloat( "Please enter a float: ", &fTestValue ) )
    {
        vPLL_Printf( "Input float is %f\r\n", fTestValue);
    }
    else
    {
        PLL_DAL( DAL_DBG_NAME, "Input float is not valid\r\n");
    }

}

/**
 * @brief   Debug function to check iDAL_GetFloatInRange
 */
static void vGetFloatInRange( void )
{
    float fTestValue = 0;

    if( OK == iDAL_GetFloatInRange( "Please enter a float: ", &fTestValue, DAL_TEST_MIN_FLOAT, DAL_TEST_MAX_FLOAT ) )
    {
        vPLL_Printf( "Input float is %f\r\n", fTestValue);
    }
    else
    {
        PLL_DAL( DAL_DBG_NAME, "Input float is not valid\r\n");
    }
}

/**
 * @brief   Debug function to check iDAL_GetHex
 */
static void vGetHex( void )
{
    uint32_t ulTestValue = 0;

    if( OK == iDAL_GetHex( "Please enter a hex value: ", &ulTestValue ) )
    {
        vPLL_Printf( "Input hex is %X\r\n", ulTestValue);
    }
    else
    {
        PLL_DAL( DAL_DBG_NAME, "Input hex is not valid\r\n");
    }
}

/**
 * @brief   Debug function to check iDAL_GetHexInRange
 */
static void vGetHexInRange( void )
{
    uint32_t ulTestValue = 0;

    if( OK == iDAL_GetHexInRange( "Please enter a hex value: ", &ulTestValue, DAL_TEST_MIN_HEX, DAL_TEST_MAX_HEX ) )
    {
        vPLL_Printf( "Input hex is %X\r\n", ulTestValue);
    }
    else
    {
        PLL_DAL( DAL_DBG_NAME, "Input hex is not valid\r\n");
    }
}


/**
 * @brief   Debug function to check iDAL_GetString
 */
static void vGetString( void )
{
    char cTest[DAL_TEST_STRING_LEN] = { 0 };

    if( OK == iDAL_GetString ( "Please enter a string: ", ( char* )&cTest, DAL_TEST_STRING_LEN ) )
    {
        vPLL_Printf( "Input string is %s\r\n", cTest);
    }
    else
    {
        PLL_DAL( DAL_DBG_NAME, "Input string is not valid\r\n");
    }
}
