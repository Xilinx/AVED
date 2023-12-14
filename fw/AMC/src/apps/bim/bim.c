/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the implementation of the Built in Monitoring (BIM) Application.
 *
 * @file bim.c
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include <string.h>

#include "standard.h"
#include "util.h"
#include "osal.h"
#include "event_id.h"

#include "pll.h"
#include "evl.h"

#include "bim.h"

#include "profile_hal.h"

#include "asc_proxy_driver.h"
#include "ami_proxy_driver.h"
#include "apc_proxy_driver.h"
#include "acc_proxy_driver.h"
#include "axc_proxy_driver.h"
#include "rmi_handler.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define UPPER_FIREWALL       ( 0xBABECAFE )
#define LOWER_FIREWALL       ( 0xDEADFACE )

#define BIM_NAME             "BIM"

/* Stat & Error definitions */
#define BIM_STATS( DO )                              \
    DO( BIM_STATS_INIT_OVERALL_COMPLETE )            \
    DO( BIM_STATS_CREATE_MUTEX )                     \
    DO( BIM_STATS_TAKE_MUTEX )                       \
    DO( BIM_STATS_RELEASE_MUTEX )                    \
    DO( BIM_STATS_STATUS_RETRIEVAL )                 \
    DO( BIM_STATS_STATUS_CHANGE )                    \
    DO( BIM_STATS_ASC_EVENT )                        \
    DO( BIM_STATS_AMI_EVENT )                        \
    DO( BIM_STATS_APC_EVENT )                        \
    DO( BIM_STATS_ACC_EVENT )                        \
    DO( BIM_STATS_AXC_EVENT )                        \
    DO( BIM_STATS_RMI_HANDLER_EVENT )                \
    DO( BIM_STATS_ASC_SENSOR_UPPER_WARNING_EVENT )   \
    DO( BIM_STATS_ASC_SENSOR_UPPER_CRITICAL_EVENT )  \
    DO( BIM_STATS_ASC_SENSOR_UPPER_FATAL_EVENT )     \
    DO( BIM_STATS_MAX )

#define BIM_ERRORS( DO )                          \
    DO( BIM_ERRORS_INIT_MUTEX_CREATE_FAILED )     \
    DO( BIM_ERRORS_MUTEX_RELEASE_FAILED )         \
    DO( BIM_ERRORS_MUTEX_TAKE_FAILED )            \
    DO( BIM_ERRORS_VALIDATION_FAILED )            \
    DO( BIM_ERRORS_STATUS_SET_FAILED )            \
    DO( BIM_ERRORS_ASC_UNKNOWN_EVENT )            \
    DO( BIM_ERRORS_AMI_UNKNOWN_EVENT )            \
    DO( BIM_ERRORS_APC_UNKNOWN_EVENT )            \
    DO( BIM_ERRORS_ACC_UNKNOWN_EVENT )            \
    DO( BIM_ERRORS_AXC_UNKNOWN_EVENT )            \
    DO( BIM_ERRORS_RMI_HANDLER_UNKNOWN_EVENT )    \
    DO( BIM_ERRORS_HANDLE_EVENT_ERROR )           \
    DO( BIM_ERRORS_MEM_ALLOC_FAILED )             \
    DO( BIM_ERRORS_GET_STATE_FAILED )             \
    DO( BIM_ERRORS_ACC_EXCEED_LIMIT_FAILED )      \
    DO( BIM_ERRORS_MAX )


#define PRINT_STAT_COUNTER( x )     PLL_INF( BIM_NAME, "%50s . . . . %d\r\n",          \
                                             BIM_STATS_STR[ x ],                       \
                                             pxThis->ulStats[ x ] )
#define PRINT_ERROR_COUNTER( x )    PLL_INF( BIM_NAME, "%50s . . . . %d\r\n",          \
                                             BIM_ERRORS_STR[ x ],                      \
                                             pxThis->ulErrors[ x ] )

#define INC_STAT_COUNTER( x )       { if( x < BIM_STATS_MAX )pxThis->ulStats[ x ]++; }
#define INC_ERROR_COUNTER( x )      { if( x < BIM_ERRORS_MAX )pxThis->ulErrors[ x ]++; }


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    BIM_STATS
 * @brief   Enumeration of stats counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( BIM_STATS, BIM_STATS, BIM_STATS_STR )

/**
 * @enum    BIM_ERRORS
 * @brief   Enumeration of stats errors for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( BIM_ERRORS, BIM_ERRORS, BIM_ERRORS_STR )


/******************************************************************************/
/* Structs/Unions                                                             */
/******************************************************************************/

/**
 * @struct  BIM_PRIVATE_DATA
 * @brief   Locally held private data
 */
typedef struct BIM_PRIVATE_DATA
{
    uint32_t        ulUpperFirewall;

    int             iIsInitialised;

    BIM_MODULES     *pxModuleData;
    BIM_STATUS      xHealthStatus;
    MODULE_STATE    pxModuleStates[ MAX_AMC_EVENT_UNIQUE_ID ];

    void            *pvMtxHdl;

    uint32_t        ulStats[ BIM_STATS_MAX ];
    uint32_t        ulErrors[ BIM_ERRORS_MAX ];

    uint32_t        ulLowerFirewall;

} BIM_PRIVATE_DATA;


/******************************************************************************/
/* Local data                                                                 */
/******************************************************************************/

static BIM_PRIVATE_DATA xLocalData =
{
    UPPER_FIREWALL, /* ulUpperFirewall */

    FALSE,          /* iIsInitialised  */

    NULL,           /* pxModuleData    */
    0,              /* xHealthStatus   */
    { 0 },          /* pxModuleStates  */

    NULL,           /* pvMtxHdl        */

    { 0 },          /* ulStats         */
    { 0 },          /* ulErrors        */

    LOWER_FIREWALL  /* ulLowerFirewall */
};

static BIM_PRIVATE_DATA *pxThis = &xLocalData;

/* BIM_STATUS string mapping */
static const char *pcBimStatusStr[ ] = { "HEALTHY", "DEGRADED", "CRITICAL", "FATAL" };


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
 *
 */
#if ( 0 != HAL_AMC_CLOCK_CONTROL )
static int iAccCallback( EVL_SIGNAL *pxSignal );
#endif

static int iAxcCallback( EVL_SIGNAL *pxSignal );
static int iApcCallback( EVL_SIGNAL *pxSignal );
static int iAmiCallback( EVL_SIGNAL *pxSignal );
static int iAscCallback( EVL_SIGNAL *pxSignal );
static int iRmiHandlerCallback( EVL_SIGNAL *pxSignal );


/******************************************************************************/
/* Local Function Declarations                                                */
/******************************************************************************/

/**
 * @brief   Determines AMC health status based on event raised
 *
 * @param   pxSignal Pointer to event being raised
 *
 * @return  OK if event handled successfully
 *          ERROR if event not handled successfully
 *
 */
static int iHandleEvent( EVL_SIGNAL *pxSignal );

/**
 * @brief   Helper function to map BIM_STATUS to a readable string
 *
 * @param   xStatus BIM_STATUS health status
 *
 * @return  String equivalent of BIM_STATUS
 */
static const char *pcHealthStatusToStr( BIM_STATUS xStatus );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the Built in Monitoring (BIM) Application.
 */
int iBIM_Initialise( BIM_MODULES *pxModuleData )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FALSE == pxThis->iIsInitialised ) &&
        ( NULL != pxModuleData ) )
    {
        if( OSAL_ERRORS_NONE != iOSAL_Mutex_Create( &pxThis->pvMtxHdl, "BIM Mutex" ) )
        {
            PLL_ERR( BIM_NAME, "Error initialising mutex\r\n" );
            INC_ERROR_COUNTER( BIM_ERRORS_INIT_MUTEX_CREATE_FAILED )
        }
        else
        {
            INC_STAT_COUNTER( BIM_STATS_CREATE_MUTEX )

            iStatus = OK;

#if ( 0 != HAL_AMC_CLOCK_CONTROL )
            if( OK == iACC_BindCallback( &iAccCallback ) )
            {
                PLL_DBG( BIM_NAME, "ACC Proxy Driver bound\r\n" );
            }
            else
            {
                iStatus = ERROR;
                PLL_ERR( BIM_NAME, "Error binding to ACC Proxy Driver\r\n" );
            }
#endif

            if( OK == iAXC_BindCallback( &iAxcCallback ) )
            {
                PLL_DBG( BIM_NAME, "AXC Proxy Driver bound\r\n" );
            }
            else
            {
                iStatus = ERROR;
                PLL_ERR( BIM_NAME, "Error binding to AXC Proxy Driver\r\n" );
            }

            if( OK == iAPC_BindCallback( &iApcCallback ) )
            {
                PLL_DBG( BIM_NAME, "APC Proxy Driver bound\r\n" );
            }
            else
            {
                iStatus = ERROR;
                PLL_ERR( BIM_NAME, "Error binding to APC Proxy Driver\r\n" );
            }

            if( OK == iAMI_BindCallback( &iAmiCallback ) )
            {
                PLL_DBG( BIM_NAME, "AMI Proxy Driver bound\r\n" );
            }
            else
            {
                iStatus = ERROR;
                PLL_ERR( BIM_NAME, "Error binding to AMI Proxy Driver\r\n" );
            }

            if( OK == iASC_BindCallback( &iAscCallback ) )
            {
                PLL_DBG( BIM_NAME, "ASC Proxy Driver bound\r\n" );
            }
            else
            {
                iStatus = ERROR;
                PLL_ERR( BIM_NAME, "Error binding to ASC Proxy Driver\r\n" );
            }

            if( OK == iRMI_HANDLER_BindCallback( &iRmiHandlerCallback ) )
            {
                PLL_DBG( BIM_NAME, "RMI Handler bound\r\n" );
            }
            else
            {
                iStatus = ERROR;
                PLL_ERR( BIM_NAME, "Error binding to RMI Handler\r\n" );
            }

            pxThis->pxModuleData = ( BIM_MODULES* )pvOSAL_MemAlloc( sizeof ( BIM_MODULES ) * MAX_AMC_EVENT_UNIQUE_ID );

            if( NULL != pxThis->pxModuleData )
            {
                pvOSAL_MemCpy( pxThis->pxModuleData, pxModuleData, sizeof( BIM_MODULES ) * MAX_AMC_EVENT_UNIQUE_ID );

                pxThis->iIsInitialised = TRUE;

                INC_STAT_COUNTER( BIM_STATS_INIT_OVERALL_COMPLETE )
            }
            else
            {
                iStatus = ERROR;

                PLL_ERR( BIM_NAME, "pvOSAL_MemAlloc failed\r\n" );
                INC_ERROR_COUNTER( BIM_ERRORS_MEM_ALLOC_FAILED )
            }
        }
    }
    else
    {
        INC_ERROR_COUNTER( BIM_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Returns the current health status of the AMC.
 */
int iBIM_GetOverallHealthStatus( BIM_STATUS *pxStatus )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iIsInitialised )            &&
        ( NULL != pxStatus ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvMtxHdl, OSAL_TIMEOUT_TASK_WAIT_MS ) )
        {
            INC_STAT_COUNTER( BIM_STATS_TAKE_MUTEX );

            *pxStatus = pxThis->xHealthStatus;

            INC_STAT_COUNTER( BIM_STATS_STATUS_RETRIEVAL );

            if( OSAL_ERRORS_NONE == iOSAL_Mutex_Release( pxThis->pvMtxHdl ) )
            {
                INC_STAT_COUNTER( BIM_STATS_RELEASE_MUTEX );
                iStatus = OK;
            }
            else
            {
                INC_ERROR_COUNTER( BIM_ERRORS_MUTEX_RELEASE_FAILED );
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_MUTEX_TAKE_FAILED );
        }
    }
    else
    {
        INC_ERROR_COUNTER( BIM_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Updates the overall health status of the AMC
 */
int iBIM_SetOverallHealthStatus( BIM_STATUS xStatus )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iIsInitialised ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvMtxHdl, OSAL_TIMEOUT_TASK_WAIT_MS ) )
        {
            INC_STAT_COUNTER( BIM_STATS_TAKE_MUTEX );

            pxThis->xHealthStatus = xStatus;

            const char *pcStatusStr = "UNKNOWN";
            if( ( 0 <= pxThis->xHealthStatus ) &&
                ( MAX_BIM_STATUS > pxThis->xHealthStatus ) )
            {
                pcStatusStr = pcBimStatusStr[ pxThis->xHealthStatus ];
            }

            /* Log change in health status */
            PLL_LOG( BIM_NAME, "AMC health status: %s \r\n", pcStatusStr ) ;
            INC_STAT_COUNTER( BIM_STATS_STATUS_CHANGE );

            if( OSAL_ERRORS_NONE == iOSAL_Mutex_Release( pxThis->pvMtxHdl ) )
            {
                INC_STAT_COUNTER( BIM_STATS_RELEASE_MUTEX );
                iStatus = OK;
            }
            else
            {
                INC_ERROR_COUNTER( BIM_ERRORS_MUTEX_RELEASE_FAILED );
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_MUTEX_TAKE_FAILED );
        }
    }
    else
    {
        INC_ERROR_COUNTER( BIM_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Display the current stats/errors
 */
int iBIM_PrintStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        int i = 0;

        PLL_INF( BIM_NAME,  "============================================================\n\r" );
        PLL_INF( BIM_NAME,  "BIM Statistics:\n\r" );
        for( i = 0; i < BIM_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( BIM_NAME,  "------------------------------------------------------------\n\r" );
        PLL_INF( BIM_NAME,  "BIM Errors:\n\r" );
        for( i = 0; i < BIM_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( BIM_NAME, "============================================================\n\r" );
        PLL_INF( BIM_NAME, "Health Stats\n\r" );
        PLL_INF( BIM_NAME, "============================================================\n\r" );
        PLL_INF( BIM_NAME, "ACC Health Status: %s\r\n", pcHealthStatusToStr( pxThis->pxModuleData[ AMC_EVENT_UNIQUE_ID_ACC ].xCurrentStatus ) );
        PLL_INF( BIM_NAME, "AXC Health Status: %s\r\n", pcHealthStatusToStr( pxThis->pxModuleData[ AMC_EVENT_UNIQUE_ID_AXC ].xCurrentStatus ) );
        PLL_INF( BIM_NAME, "APC Health Status: %s\r\n", pcHealthStatusToStr( pxThis->pxModuleData[ AMC_EVENT_UNIQUE_ID_APC ].xCurrentStatus ) );
        PLL_INF( BIM_NAME, "ASC Health Status: %s\r\n", pcHealthStatusToStr( pxThis->pxModuleData[ AMC_EVENT_UNIQUE_ID_ASC ].xCurrentStatus ) );
        PLL_INF( BIM_NAME, "AMI Health Status: %s\r\n", pcHealthStatusToStr( pxThis->pxModuleData[ AMC_EVENT_UNIQUE_ID_AMI ].xCurrentStatus ) );
        PLL_INF( BIM_NAME, "RMI Handler Health Status: %s\r\n", pcHealthStatusToStr (pxThis->pxModuleData[ AMC_EVENT_UNIQUE_ID_RMI_HANDLER ].xCurrentStatus ) );
        PLL_INF( BIM_NAME, "============================================================\n\r" );
        PLL_INF( BIM_NAME, "Overall AMC Health Status: %s\r\n", pcHealthStatusToStr( pxThis->xHealthStatus ) );
        PLL_INF( BIM_NAME, "============================================================\n\r" );

        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( BIM_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Set all stats/error values back to zero
 */
int iBIM_ClearStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iIsInitialised ) )
    {
        pvOSAL_MemSet( pxThis->ulStats, 0, sizeof( pxThis->ulStats ) );
        pvOSAL_MemSet( pxThis->ulErrors, 0, sizeof( pxThis->ulErrors ) );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( BIM_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}


/******************************************************************************/
/* EVL Callback Implementations                                               */
/******************************************************************************/

/**
 * @brief   ACC Proxy Driver EVL callback
 */
#if ( 0 != HAL_AMC_CLOCK_CONTROL )
static int iAccCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_ACC == pxSignal->ucModule ) )
    {
        if( MAX_ACC_PROXY_DRIVER_EVENTS >= pxSignal->ucEventType )
        {
            INC_STAT_COUNTER( BIM_STATS_ACC_EVENT )

            switch( pxSignal->ucEventType )
            {
                case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ENABLED:
                case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_DISABLED:
                case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ACTIVATED:
                case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_CLOCK_REENABLED:
                    break;

                default:
                {
                    INC_ERROR_COUNTER( BIM_ERRORS_ACC_UNKNOWN_EVENT );
                    break;
                }
            }

            iStatus = OK;

            if( OK != iHandleEvent( pxSignal ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_HANDLE_EVENT_ERROR );
            }

            /* call into the ACC Proxy to check the module state */
            MODULE_STATE xCurrentState = pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_ACC ];
            if( OK != iACC_GetState( &pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_ACC ] ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_GET_STATE_FAILED )
            }
            else
            {
                if( pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_ACC ] > xCurrentState )
                {
                    PLL_LOG( BIM_NAME, "ACC Proxy in ERROR state \r\n" );
                }
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_ACC_UNKNOWN_EVENT );
        }
    }

    return iStatus;
}
#endif

/**
 * @brief   AXC Proxy Driver EVL callback
 */
static int iAxcCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_AXC == pxSignal->ucModule ) )
    {
        if( MAX_AXC_PROXY_DRIVER_EVENTS >= pxSignal->ucEventType )
        {
            INC_STAT_COUNTER( BIM_STATS_AXC_EVENT )

            switch( pxSignal->ucEventType )
            {
                case AXC_PROXY_DRIVER_E_QSFP_PRESENT:
                {
                    PLL_LOG( BIM_NAME, "Event QSFP PRESENT (0x%02X)\r\n", pxSignal->ucEventType );
                    break;
                }
                case AXC_PROXY_DRIVER_E_QSFP_NOT_PRESENT:
                {
                    PLL_LOG( BIM_NAME, "Event QSFP NOT PRESENT (0x%02X)\r\n", pxSignal->ucEventType );
                    break;
                }

                default:
                {
                    INC_ERROR_COUNTER( BIM_ERRORS_AXC_UNKNOWN_EVENT );
                    break;
                }
            }

            iStatus = OK;

            if( OK != iHandleEvent( pxSignal ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_HANDLE_EVENT_ERROR );
            }

            /* call into the AXC to check the module state */
            MODULE_STATE xCurrentState = pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_AXC ];
            if( OK != iAXC_GetState( &pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_AXC ] ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_GET_STATE_FAILED )
            }
            else
            {
                if( pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_AXC ] > xCurrentState )
                {
                    PLL_LOG( BIM_NAME, "AXC Proxy in ERROR state \r\n" );
                }
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_AXC_UNKNOWN_EVENT );
        }
    }

    return iStatus;
}

/**
 * @brief   APC Proxy Driver EVL callback
 */
static int iApcCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_APC == pxSignal->ucModule ) )
    {
        if( MAX_APC_PROXY_DRIVER_EVENTS >= pxSignal->ucEventType )
        {
            INC_STAT_COUNTER( BIM_STATS_APC_EVENT )

            switch( pxSignal->ucEventType )
            {
                case APC_PROXY_DRIVER_E_DOWNLOAD_STARTED:
                case APC_PROXY_DRIVER_E_DOWNLOAD_COMPLETE:
                case APC_PROXY_DRIVER_E_DOWNLOAD_FAILED:
                case APC_PROXY_DRIVER_E_DOWNLOAD_BUSY:
                case APC_PROXY_DRIVER_E_COPY_STARTED:
                case APC_PROXY_DRIVER_E_COPY_COMPLETE:
                case APC_PROXY_DRIVER_E_COPY_FAILED:
                case APC_PROXY_DRIVER_E_COPY_BUSY:
                case APC_PROXY_DRIVER_E_PARTITION_SELECTED:
                case APC_PROXY_DRIVER_E_PARTITION_SELECTION_FAILED:
                    break;

                default:
                {
                    INC_ERROR_COUNTER( BIM_ERRORS_APC_UNKNOWN_EVENT );
                    break;
                }
            }

            iStatus = OK;

            if( OK != iHandleEvent( pxSignal ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_HANDLE_EVENT_ERROR );
            }

            /* call into the APC Proxy to check the module state */
            MODULE_STATE xCurrentState = pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_APC ];
            if( OK != iAPC_GetState( &pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_APC ] ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_GET_STATE_FAILED )
            }
            else
            {
                if( pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_APC ] > xCurrentState )
                {
                    PLL_LOG( BIM_NAME, "APC Proxy in ERROR state \r\n" );
                }
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_APC_UNKNOWN_EVENT );
        }
    }

    return iStatus;
}

/**
 * @brief   AMI Proxy Driver EVL callback
 */
static int iAmiCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_AMI == pxSignal->ucModule ) )
    {
        if( MAX_AMI_PROXY_DRIVER_EVENTS >= pxSignal->ucEventType )
        {
            INC_STAT_COUNTER( BIM_STATS_AMI_EVENT )

            switch( pxSignal->ucEventType )
            {
                case AMI_PROXY_DRIVER_E_PDI_DOWNLOAD_START:
                case AMI_PROXY_DRIVER_E_PDI_COPY_START:
                case AMI_PROXY_DRIVER_E_SENSOR_READ:
                case AMI_PROXY_DRIVER_E_GET_IDENTITY:
                case AMI_PROXY_DRIVER_E_BOOT_SELECT:
                case AMI_PROXY_DRIVER_E_HEARTBEAT:
                case AMI_PROXY_DRIVER_E_EEPROM_READ_WRITE:
                    break;

                default:
                {
                    INC_ERROR_COUNTER( BIM_ERRORS_AMI_UNKNOWN_EVENT );
                    break;
                }
            }

            iStatus = OK;

            if( OK != iHandleEvent( pxSignal ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_HANDLE_EVENT_ERROR );
            }

            /* call into the AMI Proxy to check the module state */
            MODULE_STATE xCurrentState = pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_AMI ];
            if( OK != iAMI_GetState( &pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_AMI ] ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_GET_STATE_FAILED )
            }
            else
            {
                if( pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_AMI ] > xCurrentState )
                {
                    PLL_LOG( BIM_NAME, "AMI Proxy in ERROR state \r\n" );
                }
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_AMI_UNKNOWN_EVENT );
        }
    }

    return iStatus;
}

/**
 * @brief   ASC Proxy Driver EVL callback
 */
static int iAscCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_ASC == pxSignal->ucModule ) )
    {
        if( MAX_ASC_PROXY_DRIVER_EVENTS >= pxSignal->ucEventType )
        {
            INC_STAT_COUNTER( BIM_STATS_ASC_EVENT )

            switch( pxSignal->ucEventType )
            {
                case ASC_PROXY_DRIVER_E_SENSOR_UPDATE_COMPLETE:
                case ASC_PROXY_DRIVER_E_SENSOR_UNAVAILABLE:
                case ASC_PROXY_DRIVER_E_SENSOR_COMMS_FAILURE:
                case ASC_PROXY_DRIVER_E_SENSOR_WARNING:
                case ASC_PROXY_DRIVER_E_SENSOR_CRITICAL:
                case ASC_PROXY_DRIVER_E_SENSOR_FATAL:
                case ASC_PROXY_DRIVER_E_SENSOR_LOWER_WARNING:
                case ASC_PROXY_DRIVER_E_SENSOR_LOWER_CRITICAL:
                case ASC_PROXY_DRIVER_E_SENSOR_LOWER_FATAL:
                    break;
                case ASC_PROXY_DRIVER_E_SENSOR_UPPER_WARNING:
                    INC_STAT_COUNTER( BIM_STATS_ASC_SENSOR_UPPER_WARNING_EVENT )

                    /* Call into ACC with the sensor id and the sensor type */
                    if( OK != iACC_WarningLimitExceeded( pxSignal->ucInstance, pxSignal->ucAdditionalData ) )
                    {
                        INC_ERROR_COUNTER( BIM_ERRORS_ACC_EXCEED_LIMIT_FAILED )
                    }
                    break;
                case ASC_PROXY_DRIVER_E_SENSOR_UPPER_CRITICAL:
                    INC_STAT_COUNTER( BIM_STATS_ASC_SENSOR_UPPER_CRITICAL_EVENT )

                    /* Call into ACC with the sensor id and the sensor type */
                    if( OK != iACC_CriticalLimitExceeded( pxSignal->ucInstance, pxSignal->ucAdditionalData ) )
                    {
                        INC_ERROR_COUNTER( BIM_ERRORS_ACC_EXCEED_LIMIT_FAILED )
                    }
                    break;
                case ASC_PROXY_DRIVER_E_SENSOR_UPPER_FATAL:
                    INC_STAT_COUNTER( BIM_STATS_ASC_SENSOR_UPPER_FATAL_EVENT )

                    /* Call into ACC with the sensor id and the sensor type */
                    if( OK != iACC_FatalLimitExceeded( pxSignal->ucInstance, pxSignal->ucAdditionalData ) )
                    {
                        INC_ERROR_COUNTER( BIM_ERRORS_ACC_EXCEED_LIMIT_FAILED )
                    }
                    break;

                default:
                {
                    INC_ERROR_COUNTER( BIM_ERRORS_ASC_UNKNOWN_EVENT );
                    break;
                }
            }

            iStatus = OK;

            if( OK != iHandleEvent( pxSignal ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_HANDLE_EVENT_ERROR );
            }

            /* call into the ASC Proxy to check the module state */
            MODULE_STATE xCurrentState = pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_ASC ];
            if( OK != iASC_GetState( &pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_ASC ] ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_GET_STATE_FAILED )
            }
            else
            {
                if( pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_ASC ] > xCurrentState )
                {
                    PLL_LOG( BIM_NAME, "ASC Proxy in ERROR state \r\n" );
                }
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_ASC_UNKNOWN_EVENT );
        }
    }

    return iStatus;
}

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
            INC_STAT_COUNTER( BIM_STATS_RMI_HANDLER_EVENT )

            /* TODO: Handle events when the are implemented */
            switch( pxSignal->ucEventType )
            {
                default:
                {
                    INC_ERROR_COUNTER( BIM_ERRORS_RMI_HANDLER_UNKNOWN_EVENT );
                    break;
                }
            }

            iStatus = OK;

            if( OK != iHandleEvent( pxSignal ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_HANDLE_EVENT_ERROR );
            }

            /* call into the RMI Handler to check the module state */
            MODULE_STATE xCurrentState = pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_RMI_HANDLER ];
            if( OK != iRMI_HANDLER_GetState( &pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_RMI_HANDLER ] ) )
            {
                iStatus = ERROR;
                INC_ERROR_COUNTER( BIM_ERRORS_GET_STATE_FAILED )
            }
            else
            {
                if( pxThis->pxModuleStates[ AMC_EVENT_UNIQUE_ID_RMI_HANDLER ] > xCurrentState )
                {
                    PLL_LOG( BIM_NAME, "RMI Handler in ERROR state \r\n" );
                }
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_RMI_HANDLER_UNKNOWN_EVENT );
        }
    }

    return iStatus;
}


/******************************************************************************/
/* Local function implementations                                             */
/******************************************************************************/

/**
 * @brief   Determines AMC health status based on event raised
 */
static int iHandleEvent( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) &&
        ( NULL != pxThis->pxModuleData ) )
    {
        BIM_MODULES *pxModule = &pxThis->pxModuleData[ pxSignal->ucModule ];

        if( ( NULL != pxModule ) &&
            ( pxModule->xModuleId == pxSignal->ucModule ) )
        {
            if( pxModule->ulEventsLen >= pxModule->pxEvents[ pxSignal->ucEventType ].ulEventId )
            {
                iStatus = OK;

                /* increment event counter */
                pxModule->pxEvents[ pxSignal->ucEventType ].ulCallCount++;

                /* only update module status if it is more severe than the current one */
                if( pxModule->pxEvents[ pxSignal->ucEventType ].xContextStatus > pxModule->xCurrentStatus )
                {
                    /* only update overall status if it is more severe than the current one */
                    if( pxModule->pxEvents[ pxSignal->ucEventType ].xContextStatus > pxThis->xHealthStatus )
                    {
                        /* if a module fails (raises a critical or more severe event), update AMC health status to the module's context status */
                        if( pxModule->pxEvents[ pxSignal->ucEventType ].xContextStatus >= BIM_STATUS_CRITICAL )
                        {
                            if( OK != iBIM_SetOverallHealthStatus( pxModule->xContextStatus ) )
                            {
                                INC_ERROR_COUNTER( BIM_ERRORS_STATUS_SET_FAILED )
                                iStatus = ERROR;
                            }
                        }
                        /* else, update AMC health status to the individual event's context status */
                        else
                        {
                            if( OK != iBIM_SetOverallHealthStatus( pxModule->pxEvents[ pxSignal->ucEventType ].xContextStatus ) )
                            {
                                INC_ERROR_COUNTER( BIM_ERRORS_STATUS_SET_FAILED )
                                iStatus = ERROR;
                            }
                        }
                    }

                    pxModule->xCurrentStatus = pxModule->pxEvents[ pxSignal->ucEventType ].xContextStatus;
                }
            }
        }
        else
        {
            INC_ERROR_COUNTER( BIM_ERRORS_VALIDATION_FAILED )
        }
    }
    else
    {
        INC_ERROR_COUNTER( BIM_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Helper function to map BIM_STATUS to a readable string
 */
static const char *pcHealthStatusToStr( BIM_STATUS xStatus )
{
    const char *pcStatusStr = "UNKNOWN";
    if( ( 0 <= pxThis->xHealthStatus ) &&
        ( MAX_BIM_STATUS > pxThis->xHealthStatus ) )
    {
        pcStatusStr = pcBimStatusStr[ xStatus ];
    }

    return pcStatusStr;
}
