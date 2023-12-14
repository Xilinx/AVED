/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Remote Management Interface (RMI) handler debug
 * implementation
 *
 * @file rmi_handler_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "rmi_handler_debug.h"
#include "rmi_handler.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define RMI_HANDLER_DBG_NAME        "RMI_HANDLER_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxRmiHandlerTop = NULL;
static DAL_HDL pxSetDir        = NULL;
static DAL_HDL pxGetDir        = NULL;


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
 * @brief   Debug function to set the sensor response after the request completed
 *
 * @return  N/A
 */
static void vSetSensorResponse( void );

/**
 * @brief   Debug function to get the RMI handler sensor request
 *
 * @return  N/A
 */
static void vGetSensorRequest( void );


/******************************************************************************/
/***** Helper functions *****/
/******************************************************************************/

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
/* Local variables                                                            */
/******************************************************************************/

const char *pcSensorRequestString[ MAX_RMI_HANDLER_REQUEST_TYPE ] =
{
    "Get single SDR", "Get all SDRs"
};

const char *pcSensorResponseString[ MAX_RMI_HANDLER_REPO_TYPE ] =
{
    "Temperature", "Voltage", "Current", "Power"
};

const char *pcResultsString[ MAX_RMI_HANDLER_RESULT ] =
{
    "SUCCESS", "FAILURE"
};


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the RMI_HANDLER debug access
 */
void vRMI_HANDLER_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxRmiHandlerTop = pxDAL_NewDirectory( "rmi_handler" );
        }
        else
        {
            pxRmiHandlerTop = pxDAL_NewSubDirectory( "rmi_handler", pxParentHandle );
        }

        if( NULL != pxRmiHandlerTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxRmiHandlerTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxRmiHandlerTop, vClearStats );
            pxDAL_NewDebugFunction( "bind_callbacks", pxRmiHandlerTop, vBindCallbacks );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxRmiHandlerTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxRmiHandlerTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_sensor_response", pxSetDir, vSetSensorResponse );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_sensor_request", pxGetDir, vGetSensorRequest );
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
    if( OK != iRMI_HANDLER_PrintStatistics() )
    {
        PLL_DAL( RMI_HANDLER_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iRMI_HANDLER_ClearStatistics() )
    {
        PLL_DAL( RMI_HANDLER_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to bind a callback to this module
 */
static void vBindCallbacks( void )
{
    if( OK != iRMI_HANDLER_BindCallback( &iTestCallback ) )
    {
        PLL_DAL( RMI_HANDLER_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( RMI_HANDLER_DBG_NAME, "Callback bound\r\n" );
    }
}

/**
 * @brief   Debug function to set the sensor response after the request completed
 */
static void vSetSensorResponse( void )
{
        EVL_SIGNAL xEv          = { 0 };
        RMI_HANDLER_RESULT xRes = { 0 };
        int iResult             = 0;
        uint16_t usRespSize     = 0;
        int i                   = 0;

        for ( i = 0; i < MAX_RMI_HANDLER_RESULT; i++ )
        {
            PLL_DAL( RMI_HANDLER_DBG_NAME, "\t- %d - %s\r\n", i, pcResultsString[ i ] );
        }

        if( OK != iDAL_GetIntInRange( "Select result:", &iResult, 0, MAX_RMI_HANDLER_RESULT - 1 ) )
        {
            PLL_DAL( RMI_HANDLER_DBG_NAME, "Invalid result\r\n" );
        }
        else
        {
            xRes = ( RMI_HANDLER_RESULT )iResult;
            /* check iresult against the max */
            if( OK != iRMI_HANDLER_SetSensorResponse( &xEv, xRes, usRespSize ) )
            {
                PLL_DAL( RMI_HANDLER_DBG_NAME, "Error setting result %s\r\n",
                            pcResultsString[ iResult ] );
            }
            else
            {
                PLL_DAL( RMI_HANDLER_DBG_NAME, "Set result %s \r\n",
                            pcResultsString[ iResult ] );
            }
        }
}

/**
 * @brief   Debug function to get the RMI handler sensor request
 */
static void vGetSensorRequest( void )
{

        EVL_SIGNAL xEv           = { 0 };
        uintptr_t  piAddress     = 0xFFFFFFFF;
        uint16_t   usPayloadSize = 0;
        /* DUMMY STRUCTURE MIMICKING THE RESPONSE BACK FROM OoB */
        RMI_HANDLER_SENSOR_REQUEST xReq = { RMI_HANDLER_REQUEST_TYPE_GET_SDR, /* request type */
                                            RMI_HANDLER_REPO_TYPE_TEMPERATURE, /* repo type */
                                            usPayloadSize, /* payload Size*/
                                            piAddress /* Payload Address */ };

        if( OK != iRMI_HANDLER_GetSensorRequest( &xEv, &xReq ) )
        {
            PLL_DAL( RMI_HANDLER_DBG_NAME, "Error getting sensor request\r\n" );
        }
        else
        {
            PLL_DAL( RMI_HANDLER_DBG_NAME, "\tSensor req. . . . %s\r\n", pcSensorRequestString[ xReq.xReq ] );
            PLL_DAL( RMI_HANDLER_DBG_NAME, "\tSensor repo . . . %s\r\n", pcSensorResponseString[ xReq.xType ] );
            /* TODO NOT Working currently */
            PLL_DAL( RMI_HANDLER_DBG_NAME, "\tLength. . . . . . 0x%08X\r\n", xReq.usPayloadSize );
            PLL_DAL( RMI_HANDLER_DBG_NAME, "\tAddress . . . . . 0x%08X\r\n", xReq.xPayloadAddress );
        }
}


/******************************************************************************/
/***** Helper functions *****/
/******************************************************************************/

/**
 * @brief   EVL Callback for binding test prints
 */
static int iTestCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( NULL != pxSignal )
    {
        PLL_DAL( RMI_HANDLER_DBG_NAME, "RMI_HANDLER Signal raised: Module[%d], Type[%d], Instance[%d]\r\n",
                 pxSignal->ucModule, pxSignal->ucEventType, pxSignal->ucInstance );
        iStatus = OK;
    }

    return iStatus;
}
