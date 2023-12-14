/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the amc oo band telemetry implementation
 *
 * @file out_of_band_telemetry.c
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

/* common includes */
#include "standard.h"
#include "util.h"
#include "pll.h"
#include "event_id.h"
#include "asdm.h"

/* profile */
#include "profile_hal.h"

/* core_libs */
#include "evl.h"

/* proxy drivers */
#include "rmi_handler.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define OUT_OF_BAND_NAME                        "AMC_OUT_OF_BAND"

#define UPPER_FIREWALL                          ( 0xBABECAFE )
#define LOWER_FIREWALL                          ( 0xDEADFACE )

#define SENSOR_RESPONSE_VALUES                  ( 0x3 )
#define SENSOR_RESP_BUFFER_SIZE                 ( 512 )
#define INVALID_SENSOR_ID                       ( 0xFF )

/* Stat & Error definitions */
#define OUT_OF_BAND_STATS( DO )                             \
    DO( OUT_OF_BAND_STATS_INIT_OVERALL_COMPLETE )           \
    DO( OUT_OF_BAND_STATS_AMI_SENSOR_REQUEST )              \
    DO( OUT_OF_BAND_STATS_AMI_UNSUPPORTED_REQUEST )         \
    DO( OUT_OF_BAND_STATS_AMI_SENSOR_REQUEST_SUCCESS )      \
    DO( OUT_OF_BAND_STATS_AMI_EEPROM_RW_REQUEST )           \
    DO( OUT_OF_BAND_STATS_TAKE_MUTEX )                      \
    DO( OUT_OF_BAND_STATS_RELEASE_MUTEX )                   \
    DO( OUT_OF_BAND_STATS_RMI_HANDLER_UNSUPPORTED_REQUEST ) \
    DO( OUT_OF_BAND_STATS_MAX )

#define OUT_OF_BAND_ERRORS( DO )                                    \
    DO( OUT_OF_BAND_ERRORS_INIT_MUTEX_FAILED )                      \
    DO( OUT_OF_BAND_ERRORS_INIT_BIND_AMI_CB_FAILED )                \
    DO( OUT_OF_BAND_ERRORS_INIT_OVERALL_FAILED )                    \
    DO( OUT_OF_BAND_ERRORS_AMI_SENSOR_RESP_SIZE_TOO_SMALL )         \
    DO( OUT_OF_BAND_ERRORS_AMI_SENSOR_REQUEST_EMPTY_SDR )           \
    DO( OUT_OF_BAND_ERRORS_AMI_SENSOR_REQUEST_UNKNOWN_API )         \
    DO( OUT_OF_BAND_ERRORS_AMI_SENSOR_REQUEST_FAILED )              \
    DO( OUT_OF_BAND_ERRORS_AMI_UNSUPPORTED_REPO )                   \
    DO( OUT_OF_BAND_ERRORS_AMI_EEPROM_RW_UNKNOWN_REQ )              \
    DO( OUT_OF_BAND_ERRORS_MUTEX_RELEASE_FAILED )                   \
    DO( OUT_OF_BAND_ERRORS_MUTEX_TAKE_FAILED )                      \
    DO( OUT_OF_BAND_ERRORS_MALLOC_FAILED )                          \
    DO( OUT_OF_BAND_ERRORS_MAP_REQUEST_FAILED )                     \
    DO( OUT_OF_BAND_ERRORS_RMI_HANDLER_GET_SENSOR_REQUEST_FAILED )  \
    DO( OUT_OF_BAND_ERRORS_RMI_HANDLER_SENSOR_REQUEST_UNKNOWN_API ) \
    DO( OUT_OF_BAND_ERRORS_RMI_HANDLER_SENSOR_RESP_SIZE_TOO_SMALL ) \
    DO( OUT_OF_BAND_ERRORS_MAX )

#define PRINT_STAT_COUNTER( x )             PLL_INF( OUT_OF_BAND_NAME, "%50s . . . . %d\r\n",          \
                                                     OUT_OF_BAND_STATS_STR[ x ],                       \
                                                     pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )            PLL_INF( OUT_OF_BAND_NAME, "%50s . . . . %d\r\n",          \
                                                     OUT_OF_BAND_ERRORS_STR[ x ],                      \
                                                     pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )               { if( x < OUT_OF_BAND_STATS_MAX )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )              { if( x < OUT_OF_BAND_ERRORS_MAX )pxThis->pulErrorCounters[ x ]++; }


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    OUT_OF_BAND_STATS
 * @brief   Enumeration of stats counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( OUT_OF_BAND_STATS, OUT_OF_BAND_STATS, OUT_OF_BAND_STATS_STR )

/**
 * @enum    OUT_OF_BAND_ERRORS
 * @brief   Enumeration of stats errors for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( OUT_OF_BAND_ERRORS, OUT_OF_BAND_ERRORS, OUT_OF_BAND_ERRORS_STR )


/******************************************************************************/
/* Structs                                                                     */
/******************************************************************************/

/**
 * @struct  OUT_OF_BAND_PRIVATE_DATA
 * @brief   Structure to hold the out of band telemetry private data
 */
typedef struct OUT_OF_BAND_PRIVATE_DATA
{
    uint32_t                        ulUpperFirewall;
    int                             iInitialised;
    void                            *pvOsalMutexHdl;
    uint32_t                        pulStatCounters[ OUT_OF_BAND_STATS_MAX ];
    uint32_t                        pulErrorCounters[ OUT_OF_BAND_ERRORS_MAX ];
    uint32_t                        ulLowerFirewall;

} OUT_OF_BAND_PRIVATE_DATA;


/******************************************************************************/
/* Local Variables                                                            */
/******************************************************************************/

static OUT_OF_BAND_PRIVATE_DATA xLocalData =
{
    UPPER_FIREWALL, /* ulUpperFirewall      */
    FALSE,          /* iInitialised         */
    NULL,           /* pvOsalMutexHdl       */
    { 0 },          /* pulStatCounters       */
    { 0 },          /* pulErrorCounters      */
    LOWER_FIREWALL  /* ulLowerFirewall      */
};
static OUT_OF_BAND_PRIVATE_DATA *pxThis = &xLocalData;


/******************************************************************************/
/* EVL Callback Declarations                                                  */
/******************************************************************************/

/**
 * @brief   EVL Callbacks for binding to Proxy Drivers
 *
 * @param   pxSignal     Event raised
 *
 * @return  OK if no errors were raised in the callback
 *          ERROR if an error was raised in the callback
 */
static int iRmiHandlerCallback( EVL_SIGNAL *pxSignal );

/**
 * @brief   Map the request repo into the ASDM version
 *
 * @param   xRepo     The RMI Handler request repo
 * @param   pxRepo    The ASDM repo
 *
 * @return  OK or ERROR
 */
static int iMapRmiHandlerProxyRequestRepo( RMI_HANDLER_REPO_TYPE xRepo, ASDM_REPOSITORY_TYPE *pxRepo );


/******************************************************************************/
/* Function Implementations                                                   */
/******************************************************************************/

/**
 * @brief   Initialise the out of band telemetry application layer
 *          used to handle events and communication with RMI handler
 *
 */
int iOUT_OF_BAND_TELEMETRY_Initialise( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FALSE == pxThis->iInitialised ) )
    {
        /* Create mutex to protect ASDM access */
        if( OSAL_ERRORS_NONE != iOSAL_Mutex_Create( &pxThis->pvOsalMutexHdl, "out of band" ) )
        {
            INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_INIT_MUTEX_FAILED )
        }
        else
        {
            if( OK == iRMI_HANDLER_BindCallback( &iRmiHandlerCallback ) )
            {
                PLL_DBG( OUT_OF_BAND_NAME, "RMI Handler bound\r\n" );
                iStatus = OK;
            }
            else
            {
                INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_INIT_BIND_AMI_CB_FAILED )
                iStatus = ERROR;
            }

            if( OK == iStatus )
            {
                pxThis->iInitialised = TRUE;
                INC_STAT_COUNTER( OUT_OF_BAND_STATS_INIT_OVERALL_COMPLETE )
            }
            else
            {
                INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_INIT_OVERALL_FAILED )
            }
        }
    }

    return iStatus;
}

/**
 * @brief   Print all the stats gathered by the application
 *
 */
int iOUT_OF_BAND_TELEMETRY_PrintStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        int i = 0;
        PLL_INF( OUT_OF_BAND_NAME, "============================================================\n\r" );
        PLL_INF( OUT_OF_BAND_NAME, "Out of Band App Statistics:\n\r" );
        for( i = 0; i < OUT_OF_BAND_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( OUT_OF_BAND_NAME, "------------------------------------------------------------\n\r" );
        PLL_INF( OUT_OF_BAND_NAME, "Out of Band App Errors:\n\r" );
        for( i = 0; i < OUT_OF_BAND_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( OUT_OF_BAND_NAME, "============================================================\n\r" );

        iStatus = OK;
    }

    return iStatus;
}

/**
 * @brief   Clear all the stats in the application
 *
 */
int iOUT_OF_BAND_TELEMETRY_ClearStatistics( void )
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

    return iStatus;
}


/******************************************************************************/
/* EVL Callback Implementations                                               */
/******************************************************************************/

/**
 * @brief   RMI Handler EVL callback
 */
static int iRmiHandlerCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_RMI_HANDLER == pxSignal->ucModule ) )
    {
        if( MAX_RMI_HANDLER_EVENTS >= pxSignal->ucEventType )
        {
            switch( pxSignal->ucEventType )
            {
                case RMI_HANDLER_E_SENSOR_REQUEST:
                {
                    RMI_HANDLER_SENSOR_REQUEST xSensorRequest = { 0 };

                    iStatus = iRMI_HANDLER_GetSensorRequest( pxSignal, &xSensorRequest );
                    if( OK == iStatus )
                    {
                        ASDM_REPOSITORY_TYPE xRepo = 0;
                        int iResult = ERROR;
                        uint16_t usResponseSize = 0;
                        uint8_t ucRespBuffer[ SENSOR_RESP_BUFFER_SIZE ] = { 0 };
                        uint8_t *pucDestAdd = ( uint8_t* )( xSensorRequest.xPayloadAddress );

                        /* Reset iStatus */
                        iStatus = ERROR;

                        if( SENSOR_RESP_BUFFER_SIZE > xSensorRequest.usPayloadSize )
                        {
                            PLL_DBG( OUT_OF_BAND_NAME, "Response size 0x%x exceeding request size 0x%x",
                                    SENSOR_RESP_BUFFER_SIZE,
                                    xSensorRequest.usPayloadSize );
                            INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_RMI_HANDLER_SENSOR_RESP_SIZE_TOO_SMALL )
                        }
                        else
                        {
                            iStatus = iMapRmiHandlerProxyRequestRepo( xSensorRequest.xType, &xRepo );

                            if( OK == iStatus )
                            {
                                switch( xSensorRequest.xReq )
                                {
                                    case RMI_HANDLER_REQUEST_TYPE_GET_SDR:
                                        iStatus = iASDM_PopulateResponse( ASDM_API_ID_TYPE_GET_SDR_V2, xRepo, 0, ucRespBuffer, &usResponseSize );
                                        break;

                                    case RMI_HANDLER_REQUEST_TYPE_GET_ALL_SENSOR_DATA:
                                        iStatus = iASDM_PopulateResponse( ASDM_API_ID_TYPE_GET_ALL_SENSOR_DATA_V2, xRepo, 0, ucRespBuffer, &usResponseSize );
                                        break;

                                    case RMI_HANDLER_REQUEST_TYPE_CONFIG_WRITES:
                                    case RMI_HANDLER_REQUEST_TYPE_SEND_EVENTS:
                                    default:
                                        INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_RMI_HANDLER_SENSOR_REQUEST_UNKNOWN_API )
                                        iStatus = ERROR;
                                        break;
                                }
                            }
                        }

                        /* Check the response is not greater than the memory supplied */
                        if( OK == iStatus && ( usResponseSize <= xSensorRequest.usPayloadSize ) )
                        {
                            /* Copy the data into the supplied memory */
                            pvOSAL_MemCpy( pucDestAdd, ucRespBuffer, usResponseSize );

                            iResult = OK;
                            INC_STAT_COUNTER( OUT_OF_BAND_STATS_AMI_SENSOR_REQUEST_SUCCESS )
                        }
                        else
                        {
                            /* Single byte response for failure */
                            pucDestAdd[ ASDM_SDR_RESP_BYTE_CC ] = ASDM_SDR_COMPLETION_CODE_OPERATION_FAILED;
                            pucDestAdd[ ASDM_SDR_RESP_BYTE_SIZE ] = 1;
                            INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_AMI_SENSOR_REQUEST_FAILED )
                        }

                        iStatus = iRMI_HANDLER_SetSensorResponse( pxSignal, iResult, usResponseSize );
                    }
                    else
                    {
                        INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_RMI_HANDLER_GET_SENSOR_REQUEST_FAILED )
                    }
                    break;
                }

                case RMI_HANDLER_E_CONFIG_REQUEST:
                    /* TODO: handle request - more details rquired */
                    break;

                case RMI_HANDLER_E_SEND_EVENTS_REQUEST:
                    /* TODO: handle request - FUTURE WORK */
                    break;

                default:
                    INC_STAT_COUNTER( OUT_OF_BAND_STATS_RMI_HANDLER_UNSUPPORTED_REQUEST )
                    iStatus = OK;
                    break;
            }
        }
    }

    return iStatus;
}

/*
 * @brief   Map the request repo into the inband version
 */
static int iMapRmiHandlerProxyRequestRepo( RMI_HANDLER_REPO_TYPE xRepo, ASDM_REPOSITORY_TYPE *pxRepo )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( NULL != pxRepo ) )
    {
        switch( xRepo )
        {
        case RMI_HANDLER_REPO_TYPE_TEMPERATURE:
            *pxRepo = ASDM_REPOSITORY_TYPE_TEMP;
            iStatus = OK;
            break;
        case RMI_HANDLER_REPO_TYPE_VOLTAGE:
            *pxRepo = ASDM_REPOSITORY_TYPE_VOLTAGE;
            iStatus = OK;
            break;
        case RMI_HANDLER_REPO_TYPE_CURRENT:
            *pxRepo = ASDM_REPOSITORY_TYPE_CURRENT;
            iStatus = OK;
            break;
        case RMI_HANDLER_REPO_TYPE_POWER:
            /* return the total power repo as opposed to the individual power sensor values */
            *pxRepo = ASDM_REPOSITORY_TYPE_POWER;
            iStatus = OK;
            break;
        default:
            INC_ERROR_COUNTER( OUT_OF_BAND_ERRORS_AMI_UNSUPPORTED_REPO )
            break;
        }
    }

    return iStatus;
}
