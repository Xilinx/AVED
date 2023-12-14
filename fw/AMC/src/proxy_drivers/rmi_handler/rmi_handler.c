/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the implementation for the RMI Handler
 * proxy driver.
 *
 * @file rmi_handler_proxy_driver.c
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include "util.h"
#include "pll.h"
#include "osal.h"
#include "rmi_handler.h"
#include "rmi.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define UPPER_FIREWALL                          ( 0xBABECAFE )
#define LOWER_FIREWALL                          ( 0xDEADFACE )

#define RMI_HANDLER_TASK_SLEEP_MS               ( 100 )

#define RMI_HANDLER_NAME                        "RMI_HANDLER"

#define RMI_HANDLER_MAX_MSG_SIZE                ( 1024 )
#define RMI_HANDLER_MBOX_SIZE                   ( 10 )

#define RMI_HANDLER_TIMEOUT_MS                  ( 100 )

#define RMI_HANDLER_RXDATA_SIZE                 ( 8 )

/* Stat & Error definitions */
#define RMI_HANDLER_STATS( DO )                             \
    DO( RMI_HANDLER_STATS_INIT_OVERALL_COMPLETE )           \
    DO( RMI_HANDLER_STATS_MUTEX_CREATE )                    \
    DO( RMI_HANDLER_STATS_TAKE_MUTEX )                      \
    DO( RMI_HANDLER_STATS_RELEASE_MUTEX )                   \
    DO( RMI_HANDLER_STATS_CREATE_MBOX )                     \
    DO( RMI_HANDLER_STATS_SENSOR_MBOX_POST )                \
    DO( RMI_HANDLER_STATS_GET_SDR_MBOX_PEND )               \
    DO( RMI_HANDLER_STATS_TASK_TIME_MS )                    \
    DO( RMI_HANDLER_STATS_GET_SENSOR_REQUEST )              \
    DO( RMI_HANDLER_STATS_CREATE_SEMAPHORE )                \
    DO( RMI_HANDLER_STATS_POST_SEMAPHORE )                  \
    DO( RMI_HANDLER_STATS_PEND_SEMAPHORE )                  \
    DO( RMI_HANDLER_STATS_STATUS_RETRIEVAL )                \
    DO( RMI_HANDLER_STATS_RESPONSE_SUCCESS )                \
    DO( RMI_HANDLER_STATS_RESPONSE_FAILURE )                \
    DO( RMI_HANDLER_STATS_HANDLER_SUCCESS )                 \
    DO( RMI_HANDLER_STATS_MAX )

#define RMI_HANDLER_ERRORS( DO )                            \
    DO( RMI_HANDLER_ERRORS_MAILBOX_POST_FAILED )            \
    DO( RMI_HANDLER_ERRORS_INIT_MBOX_CREATE_FAILED )        \
    DO( RMI_HANDLER_ERRORS_UNKNOWN_MAILBOX_MSG )            \
    DO( RMI_HANDLER_ERRORS_INIT_TASK_CREATE_FAILED )        \
    DO( RMI_HANDLER_ERRORS_BIND_CB_FAILED )                 \
    DO( RMI_HANDLER_ERRORS_VALIDATION_FAILED )              \
    DO( RMI_HANDLER_ERRORS_EVENT_SENSOR_REQUEST_FAILED )    \
    DO( RMI_HANDLER_ERRORS_GET_SENSOR_REQUEST )             \
    DO( RMI_HANDLER_ERRORS_CREATE_SEMAPHORE_FAILED )        \
    DO( RMI_HANDLER_ERRORS_POST_SEMAPHORE_FAILED )          \
    DO( RMI_HANDLER_ERRORS_PEND_SEMAPHORE_FAILED )          \
    DO( RMI_HANDLER_ERRORS_TAKE_MUTEX_FAILED )              \
    DO( RMI_HANDLER_ERRORS_RELEASE_MUTEX_FAILED )           \
    DO( RMI_HANDLER_ERRORS_INIT_MUTEX_CREATE_FAILED )       \
    DO( RMI_HANDLER_ERRORS_INIT_EVL_RECORD_FAILED )         \
    DO( RMI_HANDLER_ERRORS_SET_SENSOR_RESPONSE_FAILED )     \
    DO( RMI_HANDLER_ERRORS_HANDLER_FAILURE )                \
    DO( RMI_HANDLER_ERRORS_MAX )

#define PRINT_STAT_COUNTER( x )             PLL_INF( RMI_HANDLER_NAME, "%50s . . . . %d\r\n",          \
                                                     RMI_HANDLER_STATS_STR[ x ],                       \
                                                     pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )            PLL_INF( RMI_HANDLER_NAME, "%50s . . . . %d\r\n",          \
                                                     RMI_HANDLER_ERRORS_STR[ x ],                      \
                                                     pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )               { if( x < RMI_HANDLER_STATS_MAX )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )              { if( x < RMI_HANDLER_ERRORS_MAX )pxThis->pulErrorCounters[ x ]++; }
#define INC_ERROR_COUNTER_WITH_STATE( x )   { pxThis->xState = MODULE_STATE_ERROR; INC_ERROR_COUNTER( x ) }
#define SET_STAT_COUNTER( x, y )            { if( x < RMI_HANDLER_ERRORS_MAX )pxThis->pulStatCounters[ x ] = y; }


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    RMI_HANDLER_MSG_TYPES
 * @brief   Enumeration of mbox message types for this proxy
 */
typedef enum RMI_HANDLER_MSG_TYPES
{
    RMI_HANDLER_MSG_TYPE_GET_SDR_UPDATE = 0,
    RMI_HANDLER_MSG_TYPE_GET_ALL_SENSOR_DATA_UPDATE,
    RMI_HANDLER_MSG_TYPE_CONFIG_WRITES_UPDATE,
    RMI_HANDLER_MSG_TYPE_SEND_EVENTS_UPDATE,
    RMI_HANDLER_MSG_TYPE_RESPONSE_SUCCESS,
    RMI_HANDLER_MSG_TYPE_RESPONSE_FAILURE,

    MAX_RMI_HANDLER_MSG_TYPE

} RMI_HANDLER_MSG_TYPES;

/**
 * @enum    RMI_HANDLER_STATS
 * @brief   Enumeration of stats counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( RMI_HANDLER_STATS, RMI_HANDLER_STATS, RMI_HANDLER_STATS_STR )

/**
 * @enum    RMI_HANDLER_ERRORS
 * @brief   Enumeration of stats errors for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( RMI_HANDLER_ERRORS, RMI_HANDLER_ERRORS, RMI_HANDLER_ERRORS_STR )


/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/

/**
 * @struct  RMI_HANDLER_RX_DATA
 * @brief   Structure to hold rx'd data received from RMI
 */
typedef struct RMI_HANDLER_RX_DATA
{
    uint16_t usCid;
    union
    {
        RMI_HANDLER_SENSOR_REQUEST  xSensorRequest;
        RMI_HANDLER_RESULT          xResult;
    };

} RMI_HANDLER_RX_DATA;

/**
 * @struct  RMI_HANDLER_PRIVATE_DATA
 * @brief   Structure to hold this proxy driver's private data
 */
typedef struct RMI_HANDLER_PRIVATE_DATA
{
    uint32_t                ulUpperFirewall;

    int                     iInitialised;
    uint8_t                 ucMyId;

    EVL_RECORD             *pxEvlRecord;

    void                   *pvOsalMBoxReqHdl;
    void                   *pvOsalMBoxRespHdl;
    void                   *pvOsalMutexHdl;
    void                   *pvOsalTaskHdl;
    void                   *pvOsalSemaphoreHdl;

    MODULE_STATE            xState;

    RMI_HANDLER_RX_DATA     pxRxRespData[ RMI_HANDLER_RXDATA_SIZE ];
    RMI_HANDLER_RX_DATA     pxRxReqData[ RMI_HANDLER_RXDATA_SIZE ];

    uint32_t                pulStatCounters[ RMI_HANDLER_STATS_MAX ];
    uint32_t                pulErrorCounters[ RMI_HANDLER_ERRORS_MAX ];

    /* TODO request_response_data read this data to see if request has been successful req, size*/

    uint32_t                ulLowerFirewall;

} RMI_HANDLER_PRIVATE_DATA;

/**
 * @struct  RMI_HANDLER_MBOX_MSG
 * @brief   Data posted via the RMI Handler Proxy driver mailbox
 */
typedef struct RMI_HANDLER_MBOX_MSG
{
    RMI_HANDLER_MSG_TYPES xMsgType;
    uint16_t usDataSize;
    uintptr_t piAddress;
    union
    {
        RMI_HANDLER_RESULT xResult;
    };

} RMI_HANDLER_MBOX_MSG;


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static RMI_HANDLER_PRIVATE_DATA xLocalData =
{
    UPPER_FIREWALL,                 /* ulUpperFirewall    */
    FALSE,                          /* iInitialised       */
    0,                              /* ucMyId             */
    NULL,                           /* pxEvlRecord        */
    NULL,                           /* pvOsalMBoxReqHdl   */
    NULL,                           /* pvOsalMBoxRespHdl  */
    NULL,                           /* pvOsalMutexHdl     */
    NULL,                           /* pvOsalTaskHdl      */
    NULL,                           /* pvOsalSemaphoreHdl */
    MODULE_STATE_UNINITIALISED,     /* xState             */
    { { 0 } },                      /* pxRxRespData       */
    { { 0 } },                      /* pxRxReqData        */
    { 0 },                          /* pulStatCounters     */
    { 0 },                          /* pulErrorCounters    */
    LOWER_FIREWALL                  /* ulLowerFirewall    */
};
static RMI_HANDLER_PRIVATE_DATA *pxThis = &xLocalData;


/******************************************************************************/
/* Local Function declarations                                                */
/******************************************************************************/

/**
 * @brief   Handles the Request from RMI
 *
 * @param pucReq      Pointer to the address of the request
 * @param pusReqSize  Pointer to the length of the request
 * @param pucResp     Pointer to the address of the response
 * @param pusRespSize Pointer to the length of the response
 *
 * @return - Returns one of a series of error codes generated within the RMI
 *
 */
rmi_error_codes_t xRmiRequestHandler( uint8_t *pucReq, uint16_t *pusReqSize,
                                        uint8_t *pucResp, uint16_t *pusRespSize );

/**
 * @brief   Task declaration
 *
 * @param   pvArgs  Pointer to task args (unused)
 *
 * @return  N/A
 *
 */
static void vProxyDriverTask( void *pvArgs );


/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/* Init functions *************************************************************/

/**
 * @brief   Main initialisation point for the RMI Handler
 */
int iRMI_HANDLER_Initialise( uint8_t ucProxyId, uint32_t ulTaskPrio, uint32_t ulTaskStack )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FALSE == pxThis->iInitialised ) )
    {
        /* store parameters locally */
        pxThis->ucMyId = ucProxyId;

        /* initalise evl record*/
        if ( OK != iEVL_CreateRecord( &pxThis->pxEvlRecord ) )
        {
            PLL_ERR( RMI_HANDLER_NAME, "Error initialising EVL_RECORD\r\n" );
            INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_INIT_EVL_RECORD_FAILED );
        }
        else
        {
            /* Initalise OSAL items */
            if( OSAL_ERRORS_NONE != iOSAL_MBox_Create( &pxThis->pvOsalMBoxReqHdl, RMI_HANDLER_MBOX_SIZE,
                                                       sizeof( RMI_HANDLER_MBOX_MSG ), "RMI_Handler mbox" ) )
            {
                PLL_ERR( RMI_HANDLER_NAME, "Error creating mbox\r\n" );
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_INIT_MBOX_CREATE_FAILED );
            }
            else if( OSAL_ERRORS_NONE != iOSAL_MBox_Create( &pxThis->pvOsalMBoxRespHdl, RMI_HANDLER_MBOX_SIZE,
                                                       sizeof( RMI_HANDLER_MBOX_MSG ), "RMI_Handler mbox" ) )
            {
                PLL_ERR( RMI_HANDLER_NAME, "Error creating mbox\r\n" );
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_INIT_MBOX_CREATE_FAILED );
            }
            else if( OSAL_ERRORS_NONE != iOSAL_Mutex_Create( &pxThis->pvOsalMutexHdl, "Rmi_Handler mutex" ))
            {
                PLL_ERR( RMI_HANDLER_NAME, "Error initalising mutex\r\n" );
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_INIT_MUTEX_CREATE_FAILED )
            }
            else if( OSAL_ERRORS_NONE != iOSAL_Semaphore_Create( &pxThis->pvOsalSemaphoreHdl, 0,
                                                            1, "RMI_Handler Semaphore" ) )
            {
                PLL_ERR( RMI_HANDLER_NAME, "Error initalising semaphore\r\n" );
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_CREATE_SEMAPHORE_FAILED );
            }
            else if( OSAL_ERRORS_NONE != iOSAL_Task_Create( &pxThis->pvOsalTaskHdl,
                                                            vProxyDriverTask,
                                                            ulTaskStack,
                                                            NULL,
                                                            ulTaskPrio,
                                                            "RMI_Handler task" ) )
            {
                PLL_ERR( RMI_HANDLER_NAME, "Error initialising task\r\n" );
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_INIT_TASK_CREATE_FAILED );
            }
            else
            {
                INC_STAT_COUNTER( RMI_HANDLER_STATS_CREATE_MBOX );
                INC_STAT_COUNTER( RMI_HANDLER_STATS_CREATE_SEMAPHORE );
                INC_STAT_COUNTER( RMI_HANDLER_STATS_INIT_OVERALL_COMPLETE );
                INC_STAT_COUNTER( RMI_HANDLER_STATS_MUTEX_CREATE );
                pxThis->iInitialised = TRUE;
                pxThis->xState = MODULE_STATE_OK;
                iStatus = OK;
            }
        }
    }
    return iStatus;
}

/**
 * @brief   Bind into this proxy driver
 */

int iRMI_HANDLER_BindCallback( EVL_CALLBACK *pxCallback )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pxCallback ) &&
        ( NULL != pxThis->pxEvlRecord ) )
    {
        iStatus = iEVL_BindCallback( pxThis->pxEvlRecord, pxCallback );

        if( ERROR == iStatus )
        {
            INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_BIND_CB_FAILED );
        }
    }
    else
    {
        INC_ERROR_COUNTER( RMI_HANDLER_ERRORS_VALIDATION_FAILED );
    }

    return iStatus;
}

/* Set functions **************************************************************/

/**
 * @brief   Set the sensor response after the request completed
 */
int iRMI_HANDLER_SetSensorResponse( EVL_SIGNAL *pxSignal,
                                    RMI_HANDLER_RESULT xResult,
                                    uint16_t usResponseSize )
{
     int iStatus = ERROR;

     if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( NULL != pxSignal ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        RMI_HANDLER_MBOX_MSG xMsg = { 0 };
        xMsg.usDataSize = usResponseSize;
        xMsg.xResult = xResult;
        /* Set msgtype based on response */
        switch ( xResult )
        {
            case RMI_HANDLER_RESULT_SUCCESS:
                xMsg.xMsgType = RMI_HANDLER_MSG_TYPE_RESPONSE_SUCCESS;
                INC_STAT_COUNTER( RMI_HANDLER_STATS_RESPONSE_SUCCESS );
                break;
            case RMI_HANDLER_RESULT_FAILURE:
                xMsg.xMsgType = RMI_HANDLER_MSG_TYPE_RESPONSE_FAILURE;
                INC_STAT_COUNTER( RMI_HANDLER_STATS_RESPONSE_FAILURE );
                break;
            default:
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_SET_SENSOR_RESPONSE_FAILED );
                break;
        }
        if( OSAL_ERRORS_NONE == iOSAL_MBox_Post( pxThis->pvOsalMBoxRespHdl,
                                                 ( void* )&xMsg,
                                                 OSAL_TIMEOUT_NO_WAIT ) )
        {
            INC_STAT_COUNTER( RMI_HANDLER_STATS_SENSOR_MBOX_POST )
            iStatus = OK;
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_MAILBOX_POST_FAILED );
        }

    }
    return iStatus;
}

/* Get functions **************************************************************/

/**
 * @brief   Print all the stats gathered by the proxy driver
 */
int iRMI_HANDLER_PrintStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        int i = 0;
        PLL_INF( RMI_HANDLER_NAME, "============================================================\n\r" );
        PLL_INF( RMI_HANDLER_NAME, "RMI_HANDLER Proxy Statistics:\n\r" );
        for( i = 0; i < RMI_HANDLER_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( RMI_HANDLER_NAME, "------------------------------------------------------------\n\r" );
        PLL_INF( RMI_HANDLER_NAME, "RMI_HANDLER Proxy Errors:\n\r" );
        for( i = 0; i < RMI_HANDLER_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( RMI_HANDLER_NAME, "============================================================\n\r" );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( RMI_HANDLER_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Clear all the stats in the proxy driver
 */
int iRMI_HANDLER_ClearStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        pvOSAL_MemSet( pxThis->pulStatCounters, 0, sizeof( pxThis->pulStatCounters ) );
        pvOSAL_MemSet( pxThis->pulErrorCounters, 0, sizeof( pxThis->pulErrorCounters ) );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( RMI_HANDLER_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Get the RMI handler sensor request
 */
int iRMI_HANDLER_GetSensorRequest( EVL_SIGNAL *pxSignal,
                                   RMI_HANDLER_SENSOR_REQUEST *pxRequest )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pxSignal ) &&
        ( NULL != pxRequest ) )
        {
           INC_STAT_COUNTER( RMI_HANDLER_STATS_GET_SENSOR_REQUEST )

            if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvOsalMutexHdl,
                                                  OSAL_TIMEOUT_WAIT_FOREVER ) )
            {
                INC_STAT_COUNTER( RMI_HANDLER_STATS_TAKE_MUTEX )
                /* Populate the request with locally stored data */
                pxRequest->xPayloadAddress = pxThis->pxRxRespData->xSensorRequest.xPayloadAddress;
                pxRequest->usPayloadSize   = pxThis->pxRxRespData->xSensorRequest.usPayloadSize;
                pxRequest->xType           = pxThis->pxRxRespData->xSensorRequest.xType;
                pxRequest->xReq            = pxThis->pxRxRespData->xSensorRequest.xReq;
                iStatus = OK;

                if( OSAL_ERRORS_NONE != iOSAL_Mutex_Release( pxThis->pvOsalMutexHdl ) )
                {
                    INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_RELEASE_MUTEX_FAILED )
                    iStatus = ERROR;
                }
                else
                {
                    INC_STAT_COUNTER( RMI_HANDLER_STATS_RELEASE_MUTEX )
                }
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_TAKE_MUTEX_FAILED )
        }
    }
    else
    {
        INC_ERROR_COUNTER( RMI_HANDLER_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Gets the current state of the proxy
 */
int iRMI_HANDLER_GetState( MODULE_STATE *pxState )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( NULL != pxState ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvOsalMutexHdl, OSAL_TIMEOUT_TASK_WAIT_MS ) )
        {
            *pxState = pxThis->xState;

            INC_STAT_COUNTER( RMI_HANDLER_STATS_STATUS_RETRIEVAL );

            iStatus = OK;
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_TAKE_MUTEX_FAILED )
        }

        if( OSAL_ERRORS_NONE != iOSAL_Mutex_Release( pxThis->pvOsalMutexHdl ) )
        {
            INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_RELEASE_MUTEX_FAILED )
            iStatus = ERROR;
        }
        else
        {
            INC_STAT_COUNTER( RMI_HANDLER_STATS_RELEASE_MUTEX )
        }
    }
    else
    {
        INC_ERROR_COUNTER( RMI_HANDLER_ERRORS_VALIDATION_FAILED )
    }
    return iStatus;
}


/******************************************************************************/
/* Local Function implementations                                             */
/******************************************************************************/

/**
 * @brief   Handles the Request from RMI
 */
rmi_error_codes_t xRmiRequestHandler( uint8_t *pucReq, uint16_t *pusReqSize,
                                        uint8_t *pucResp, uint16_t *pusRespSize )
{
    rmi_error_codes_t xErr = eRMI_ERROR_GENERIC;
    uint8_t ucApiId = 0;
    if( ( NULL != pusReqSize ) &&
        ( NULL != pusReqSize ) &&
        ( apiREQUEST_HEADER_SIZE <= *pusReqSize) &&
        ( NULL != pusRespSize ) &&
        ( NULL != pucResp ) )
    {
        ucApiId = pucReq[ apiID_INDEX ];

        switch( ucApiId )
        {
            case apiCMD_GET_SDR_API_ID:
            {     
                RMI_HANDLER_MBOX_MSG xMsg = { 0 };
                xMsg.usDataSize = *pusReqSize;
                xMsg.xMsgType = RMI_HANDLER_MSG_TYPE_GET_SDR_UPDATE;
                xMsg.piAddress = ( uintptr_t ) pucReq;

                if( OSAL_ERRORS_NONE == iOSAL_MBox_Post( pxThis->pvOsalMBoxReqHdl,
                                            ( void* )&xMsg,
                                            OSAL_TIMEOUT_NO_WAIT ) )
                {
                    INC_STAT_COUNTER( RMI_HANDLER_STATS_SENSOR_MBOX_POST );
                }        
                break;
            }
            case apiCMD_GET_ALL_SENSOR_DATA_API_ID:
            {
                RMI_HANDLER_MBOX_MSG xMsg = { 0 };
                xMsg.usDataSize = *pusReqSize;
                xMsg.xMsgType = RMI_HANDLER_MSG_TYPE_GET_ALL_SENSOR_DATA_UPDATE;
                xMsg.piAddress = ( uintptr_t )pucReq;

                if( OSAL_ERRORS_NONE == iOSAL_MBox_Post( pxThis->pvOsalMBoxReqHdl,
                                        ( void* )&xMsg,
                                        OSAL_TIMEOUT_NO_WAIT ) )
                {
                    INC_STAT_COUNTER( RMI_HANDLER_STATS_SENSOR_MBOX_POST )
                }
                break;
            }
            default:
                xErr = eRMI_ERROR_INVALID_API_ID;
                break;
        }

        /* check have a timed out and got a response */
        uint32_t ulSemStartMs = ulOSAL_GetUptimeMs();
        if( OSAL_ERRORS_NONE == iOSAL_Semaphore_Pend( &pxThis->pvOsalSemaphoreHdl, RMI_HANDLER_TIMEOUT_MS ) )
        {
            INC_STAT_COUNTER( RMI_HANDLER_STATS_PEND_SEMAPHORE );
            if( ( RMI_HANDLER_TIMEOUT_MS > ulSemStartMs) && ( RMI_HANDLER_RESULT_SUCCESS == pxThis->pxRxRespData->xResult ) )
            {
                INC_STAT_COUNTER( RMI_HANDLER_STATS_HANDLER_SUCCESS );
            }
            else
            {
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_HANDLER_FAILURE );
            }
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_PEND_SEMAPHORE_FAILED );
        }
    }
    else
    {
        xErr = eRMI_ERROR_INVALID_ARGUMENT;
    }
    return xErr;
}

/*
 * @brief   RMI Handler task declaration
 */
static void vProxyDriverTask( void *pvArgs )
{
    RMI_HANDLER_MBOX_MSG xMBoxReqData = { 0 };
    uint32_t ulStartMs = 0;
    EVL_SIGNAL xNewSignal = { pxThis->ucMyId,
                            MAX_RMI_HANDLER_EVENTS,
                            0,
                            0 };

    FOREVER
    {
        ulStartMs = ulOSAL_GetUptimeMs();
        if( OSAL_ERRORS_NONE == iOSAL_MBox_Pend( pxThis->pvOsalMBoxReqHdl,
                                                 ( void* )&xMBoxReqData,
                                                 OSAL_TIMEOUT_NO_WAIT ) )
        {
            /* Request */
            switch( xMBoxReqData.xMsgType )
            {
                case RMI_HANDLER_MSG_TYPE_GET_SDR_UPDATE:
                {
                    xNewSignal.ucEventType = RMI_HANDLER_E_SENSOR_REQUEST;

                    /* raise event based on request */
                    if( ERROR == iEVL_RaiseEvent( pxThis->pxEvlRecord, &xNewSignal ) )
                    {
                        PLL_ERR( RMI_HANDLER_NAME, "Error attempting to raise event 0x%x\r\n", RMI_HANDLER_E_SENSOR_REQUEST );
                        INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_EVENT_SENSOR_REQUEST_FAILED )
                    }
                    else
                    {
                        /* Store request data locally */
                        pxThis->pxRxReqData->xSensorRequest.xPayloadAddress = xMBoxReqData.piAddress;                                
                        pxThis->pxRxReqData->xSensorRequest.usPayloadSize = xMBoxReqData.usDataSize;
                        /* TODO add Repo Type functionality */
                        pxThis->pxRxReqData->xSensorRequest.xType = 0;
                        pxThis->pxRxReqData->xSensorRequest.xReq = xMBoxReqData.xMsgType;
                    }
                    break;
                }
                case RMI_HANDLER_MSG_TYPE_GET_ALL_SENSOR_DATA_UPDATE:
                {
                    xNewSignal.ucEventType = RMI_HANDLER_E_SENSOR_REQUEST;

                    /* raise event based on request */
                    if( ERROR == iEVL_RaiseEvent( pxThis->pxEvlRecord, &xNewSignal ) )
                    {
                        PLL_ERR( RMI_HANDLER_NAME, "Error attempting to raise event 0x%x\r\n", RMI_HANDLER_E_SENSOR_REQUEST );
                        INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_EVENT_SENSOR_REQUEST_FAILED )
                    }
                    else
                    {
                        /* Store request data locally */
                        pxThis->pxRxReqData->xSensorRequest.xPayloadAddress = xMBoxReqData.piAddress;                                
                        pxThis->pxRxReqData->xSensorRequest.usPayloadSize = xMBoxReqData.usDataSize;
                        /* TODO add Repo Type functionality */
                        pxThis->pxRxReqData->xSensorRequest.xType = 0;
                        pxThis->pxRxReqData->xSensorRequest.xReq = xMBoxReqData.xMsgType;                           
                    }
                    break;
                }
                default:
                    break;
            } 
        }

        RMI_HANDLER_MBOX_MSG xMBoxRespData = { 0 };

        /* Check for new MBox data (tx path) */
        if( OSAL_ERRORS_NONE == iOSAL_MBox_Pend( pxThis->pvOsalMBoxRespHdl,
                                                 ( void* )&xMBoxRespData,
                                                 OSAL_TIMEOUT_NO_WAIT ) )
        {
            /* response handling */
            switch( xMBoxRespData.xMsgType )
            {
                case RMI_HANDLER_MSG_TYPE_RESPONSE_SUCCESS:
                {
                    pxThis->pxRxRespData->xResult = RMI_HANDLER_RESULT_SUCCESS; 
                    INC_STAT_COUNTER( RMI_HANDLER_STATS_RESPONSE_SUCCESS );
                    break;
                }
                case RMI_HANDLER_MSG_TYPE_RESPONSE_FAILURE:
                {
                    pxThis->pxRxRespData->xResult = RMI_HANDLER_RESULT_FAILURE; 
                    INC_STAT_COUNTER( RMI_HANDLER_STATS_RESPONSE_FAILURE );
                    break;
                }
                case RMI_HANDLER_MSG_TYPE_CONFIG_WRITES_UPDATE:
                    /* TODO */
                    break;

                case RMI_HANDLER_MSG_TYPE_SEND_EVENTS_UPDATE:
                    /* TODO */
                    break;

                default:
                    PLL_ERR( RMI_HANDLER_NAME, "Error unknown mailbox message type 0x%x\r\n", xMBoxReqData.xMsgType );
                    INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_UNKNOWN_MAILBOX_MSG );
                    break;
            }

            /* release semaphore after response invalid or valid */
            if( OSAL_ERRORS_NONE == iOSAL_Semaphore_Post( pxThis->pvOsalSemaphoreHdl ) )
            {
                INC_STAT_COUNTER( RMI_HANDLER_STATS_POST_SEMAPHORE );
            }
            else
            {
                INC_ERROR_COUNTER_WITH_STATE( RMI_HANDLER_ERRORS_POST_SEMAPHORE_FAILED );
            }
        }
    }

    pxThis->pulStatCounters[ RMI_HANDLER_STATS_TASK_TIME_MS ] = UTIL_ELAPSED_TIME_MS( ulStartMs );
    iOSAL_Task_SleepMs( RMI_HANDLER_TASK_SLEEP_MS );
}
