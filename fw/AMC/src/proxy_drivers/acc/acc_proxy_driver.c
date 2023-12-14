/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the implementation for the Alveo Clock Control ( ACC )
 * proxy driver.
 *
 * @file acc_proxy_driver.c
 *
 */


/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include "util.h"
#include "pll.h"
#include "osal.h"
#include "acc_proxy_driver.h"

/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define UPPER_FIREWALL      ( 0xBABECAFE )
#define LOWER_FIREWALL      ( 0xDEADFACE )

#define ACC_TASK_SLEEP_MS   ( 100 )

#define ACC_NAME            "ACC"

#define ACC_MAX_MSG_SIZE    ( 64 )
#define ACC_MBOX_SIZE       ( 10 )
#define ACC_SENSOR_NAME_MAX                             ( 20 )
#define ACTIVITY_MAX                                    ( 128 )
#define CLOCK_THROTTLING_AVERAGE_SIZE                   ( 10 )
#define MASK_CLOCKTHROTTLING_DISABLE_THROTTLING         ( 0 << 20 )
#define MASK_CLOCKTHROTTLING_ENABLE_THROTTLING          ( 1 << 20 )
#define MASK_CLEAR_LATCHEDSHUTDOWNCLOCKS                ( 1 << 16 )

#define REDUCE_GAPPING_DEMAND_RATE_TO_FIVE_PERCENTAGE   ( 0x07 )
#define GAPPING_DEMAND_FULL_RATE                        ( 0x80 )

/* Stat & Error definitions */
#define ACC_PROXY_STATS( DO )               \
    DO( ACC_PROXY_STATS_INITIALISATION )    \
    DO( ACC_PROXY_STATS_MAILBOX_CREATE )    \
    DO( ACC_PROXY_STATS_MAILBOX_POST )      \
    DO( ACC_PROXY_STATS_MAILBOX_PEND )      \
    DO( ACC_PROXY_STATS_MUTEX_CREATE )      \
    DO( ACC_PROXY_STATS_MUTEX_TAKE )        \
    DO( ACC_PROXY_STATS_MUTEX_RELEASE )     \
    DO( ACC_PROXY_STATS_TASK_CREATE )       \
    DO( ACC_PROXY_STATS_EVENT_RAISE )       \
    DO( ACC_PROXY_STATS_CLOCK_SHUTDOWN )    \
    DO( ACC_PROXY_STATS_TASK_TIME_MS )      \
    DO( ACC_PROXY_STATS_ENABLE_SHUTDOWN )   \
    DO( ACC_PROXY_STATS_DISABLE_SHUTDOWN )  \
    DO( ACC_PROXY_STATS_STATUS_RETRIEVAL )  \
    DO( ACC_PROXY_STATS_MAX )

#define ACC_PROXY_ERRORS( DO )                   \
    DO( ACC_PROXY_ERRORS_INITIALISATION )        \
    DO( ACC_PROXY_ERRORS_MAILBOX_CREATE )        \
    DO( ACC_PROXY_ERRORS_MAILBOX_POST )          \
    DO( ACC_PROXY_ERRORS_MAILBOX_PEND )          \
    DO( ACC_PROXY_ERRORS_MUTEX_CREATE )          \
    DO( ACC_PROXY_ERRORS_MUTEX_TAKE )            \
    DO( ACC_PROXY_ERRORS_MUTEX_RELEASE )         \
    DO( ACC_PROXY_ERRORS_TASK_CREATE )           \
    DO( ACC_PROXY_ERRORS_EVENT_RAISE )           \
    DO( ACC_PROXY_ERRORS_UNKNOWN_MAILBOX_MSG )   \
    DO( ACC_PROXY_STATS_ENABLE_SHUTDOWN_FAILED ) \
    DO( ACC_PROXY_STATS_DISABLE_SHUTDOWN_FAILED )\
    DO( ACC_PROXY_ERRORS_VALIDATION_FAILED )     \
    DO( ACC_PROXY_ERRORS_INIT_EVL_RECORD_FAILED )\
    DO( ACC_PROXY_ERRORS_MAX )


#define PRINT_STAT_COUNTER( x )             PLL_INF( ACC_NAME, "%50s . . . . %d\r\n",          \
                                                     ACC_PROXY_STATS_STR[ x ],       \
                                                     pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )            PLL_INF( ACC_NAME, "%50s . . . . %d\r\n",          \
                                                     ACC_PROXY_ERRORS_STR[ x ],      \
                                                     pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )               { if( x < ACC_PROXY_STATS_MAX )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )              { if( x < ACC_PROXY_ERRORS_MAX )pxThis->pulErrorCounters[ x ]++; }
#define INC_ERROR_COUNTER_WITH_STATE( x )   { pxThis->xState = MODULE_STATE_ERROR; INC_ERROR_COUNTER( x ) }
#define SET_STAT_COUNTER( x, y )            { if( x < ACC_PROXY_ERRORS_MAX )pxThis->pulStatCounters[ x ] = y; }


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    ACC_PROXY_STATS
 * @brief   Enumeration of stats counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( ACC_PROXY_STATS, ACC_PROXY_STATS, ACC_PROXY_STATS_STR )

/**
 * @enum    ACC_PROXY_ERRORS
 * @brief   Enumeration of stats errors for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( ACC_PROXY_ERRORS, ACC_PROXY_ERRORS, ACC_PROXY_ERRORS_STR )

/**
 * @enum    ACC_MSG_TYPES
 * @brief   Enumeration of mbox message types for this proxy
 */
typedef enum
{
    ACC_MSG_TYPE_SHUTDOWN_FATAL_THRESHOLD_CROSS = 0,
    ACC_MSG_TYPE_SHUTDOWN_CRITICAL_THRESHOLD_CROSS,
    ACC_MSG_TYPE_SHUTDOWN_WARNING_THRESHOLD_CROSS,
    ACC_MSG_TYPE_SHUTDOWN_FEATURE_ENABLED,
    ACC_MSG_TYPE_SHUTDOWN_FEATURE_DISABLED,
    ACC_MSG_TYPE_SHUTDOWN_REENABLE_CLOCK,


    /* ...etc... */

    MAX_ACC_MSG_TYPE
} ACC_MSG_TYPES;

/**
 * @enum    ACC_SHUTDOWN_LEVEL
 * @brief   Enumeration of shutdown level
 */
typedef enum
{
    ACC_SHUTDOWN_LEVEL_NONE = 0,
    ACC_SHUTDOWN_LEVEL_WARNING,
    ACC_SHUTDOWN_LEVEL_CRITICAL,
    ACC_SHUTDOWN_LEVEL_FATAL,

    /* ...etc... */
    ACC_SHUTDOWN_LEVEL_MAX

} ACC_SHUTDOWN_LEVEL;


/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/

/**
 * @struct  ACC_PRIVATE_DATA
 * @brief   Structure to hold ths proxy driver's private data
 */
typedef struct ACC_PRIVATE_DATA
{
    uint32_t                                ulUpperFirewall;

    int                                     iInitialised;
    int                                     iMetaDataInitialised;
    uint8_t                                 ucMyId;

    EVL_RECORD                              *pxEvlRecord;

    void *                                  pvOsalMutexHdl;
    void *                                  pvOsalMBoxHdl;
    void *                                  pvOsalTaskHdl;
    uint8_t                                 ucInstance;

    uint32_t                                pulStatCounters[ACC_PROXY_STATS_MAX];
    uint32_t                                pulErrorCounters[ACC_PROXY_ERRORS_MAX];

    ACC_SHUTDOWN_LEVEL                      xClockShutdownLevel;
    int                                     iClockShutdownEnabled;
    ACC_PROXY_DRIVER_SHUTDOWN_SENSOR_DATA   xClockShutdownTable;

    MODULE_STATE                            xState;

    uint32_t                                ulLowerFirewall;
} ACC_PRIVATE_DATA;


/**
 * @struct  ACC_MBOX_MSG
 * @brief   Data posted via the ACC Proxy driver mailbox
 */
typedef struct ACC_MBOX_MSG
{
    ACC_MSG_TYPES   eMsgType;
    union
    {
        ACC_PROXY_DRIVER_SHUTDOWN_SENSOR        xSensor;

    };
} ACC_MBOX_MSG;


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static ACC_PRIVATE_DATA xLocalData =
{
    UPPER_FIREWALL,                 /* ulUpperFirewall */
    FALSE,                          /* iInitialised */
    FALSE,                          /* iMetaDataInitialised */
    0,                              /* ucMyId */

    NULL,                           /* pxEvlRecord */

    NULL,                           /* pvOsalMutexHdl */
    NULL,                           /* pvOsalMBoxHdl */
    NULL,                           /* pvOsalTaskHdl */
    0,                              /* ucInstance */

    { 0 },                          /* pulStatCounters */
    { 0 },                          /* pulErrorCounters */

    ACC_SHUTDOWN_LEVEL_NONE,        /* xClockShutdownLevel */
    FALSE,                          /* iClockShutdownEnabled */
    { 0 },                          /* xClockShutdownTable */

    MODULE_STATE_UNINITIALISED,     /* xState */

    LOWER_FIREWALL                  /* ulLowerFirewall */
};
static ACC_PRIVATE_DATA *pxThis                     = &xLocalData;


/******************************************************************************/
/* Local Function declarations                                                */
/******************************************************************************/

/**
 * @brief   Task declaration
 *
 * @param   pvArgs  Pointer to task args ( unused )
 *
 * @return  N/A
 *
 */
static void vProxyDriverTask( void *pvArgs );

/**
 * @brief   Check if clock shutdown is enabled and act on the clock if it is
 *
 * @param   ucSensorId      The ID to identify the sensor
 * @param   ucSensorType    The type of sensor, temp, current, voltage or power
 *
 * @return  N/A
 *
 */
static void vClockShutdown( uint8_t ucSensorId, uint8_t ucSensorType, ACC_SHUTDOWN_LEVEL xClockShutdownLevel );

/**
 * @brief   Reset the current state and re-enable clock if required
 *
 * @return  N/A
 *
 */
static void vReenableClock( void );

/**
 * @brief   Get text string for shutdown level
 *
 * @param   xClockShutdownLevel    The shutdown level
 *
 * @return  string representation of the ACC_SHUTDOWN_LEVEL passed in
 */
static const char *pucGetLevelString( ACC_SHUTDOWN_LEVEL xClockShutdownLevel );

/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/* Init functions *************************************************************/

/**
 * @brief   Main initialisation point for the ACC Proxy Driver
 */
int iACC_Initialise( uint8_t ucProxyId, uint32_t ulTaskPrio, uint32_t ulTaskStack,
                     ACC_PROXY_DRIVER_SHUTDOWN_SENSOR_DATA *pxShutdownData,
                     void *pvUccBaseAddress )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( NULL != pxShutdownData ) &&
        ( NULL != pvUccBaseAddress ) &&
        ( FALSE == pxThis->iInitialised ) )
    {
        /* Store parameters locally */
        pxThis->ucMyId     = ucProxyId;

        /* initalise evl record*/
        if ( OK != iEVL_CreateRecord( &pxThis->pxEvlRecord ) )
        {
            PLL_ERR( ACC_NAME, "Error initialising EVL_RECORD\r\n" );
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_INIT_EVL_RECORD_FAILED );
        }
        else       
        {
            /* Initialise OSAL items */
            INC_STAT_COUNTER( ACC_PROXY_STATS_MUTEX_CREATE );
            if( OSAL_ERRORS_NONE != iOSAL_Mutex_Create( &pxThis->pvOsalMutexHdl,
                                                            "acc_proxy mutex" ) )
            {
                INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MUTEX_CREATE );
            }
            else
            {
                INC_STAT_COUNTER( ACC_PROXY_STATS_MAILBOX_CREATE );
                if( OSAL_ERRORS_NONE != iOSAL_MBox_Create( &pxThis->pvOsalMBoxHdl, ACC_MBOX_SIZE,
                                                sizeof( ACC_MBOX_MSG ), "acc_proxy mbox" ) )
                {
                    INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MAILBOX_CREATE );
                }
                else
                {
                    INC_STAT_COUNTER( ACC_PROXY_STATS_TASK_CREATE );
                    if( OSAL_ERRORS_NONE != iOSAL_Task_Create( &pxThis->pvOsalTaskHdl,
                                                                    vProxyDriverTask,
                                                                    ulTaskStack,
                                                                    NULL,
                                                                    ulTaskPrio,
                                                                    "acc_proxy task" ) )
                    {
                        INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_TASK_CREATE );
                    }
                    else
                    {
                        /* Read in the list of sensors required for the throttling algorithm */
                        pxThis->xClockShutdownTable.ucNumSensors = pxShutdownData->ucNumSensors;

                        pvOSAL_MemCpy( &(pxThis->xClockShutdownTable), pxShutdownData, sizeof( ACC_PROXY_DRIVER_SHUTDOWN_SENSOR_DATA ) );

                        INC_STAT_COUNTER( ACC_PROXY_STATS_INITIALISATION );
                        pxThis->iInitialised = TRUE;
                        pxThis->xState = MODULE_STATE_OK;
                        iStatus = OK;
                    }
                }
            }
        }
    }   

    if( OK != iStatus )
    {
        INC_ERROR_COUNTER( ACC_PROXY_ERRORS_INITIALISATION );
    }
    return iStatus;
}

/**
 * @brief   Bind into this proxy driver
 */

int iACC_BindCallback( EVL_CALLBACK *pxCallback )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pxCallback ) &&
        ( NULL != pxThis->pxEvlRecord ) )
    {
        iStatus = iEVL_BindCallback( pxThis->pxEvlRecord, pxCallback );
    }

    return iStatus;
}

/* Set functions **************************************************************/

/**
 * @brief   Report to ACC that a sensor has exceeded fatal limit
 */
int iACC_FatalLimitExceeded( uint8_t ucSensorId, uint8_t ucSensorType  )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        ACC_MBOX_MSG xMsg = { 0 };
        xMsg.eMsgType = ACC_MSG_TYPE_SHUTDOWN_FATAL_THRESHOLD_CROSS;
        xMsg.xSensor.ucShutdownId = ucSensorId;
        xMsg.xSensor.xShutdownType = ucSensorType;

        if( OSAL_ERRORS_NONE == iOSAL_MBox_Post( pxThis->pvOsalMBoxHdl, ( void* )&xMsg, OSAL_TIMEOUT_NO_WAIT ) )
        {
            INC_STAT_COUNTER( ACC_PROXY_STATS_MAILBOX_POST );
            iStatus = OK;
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MAILBOX_POST );
        }

    }

    return iStatus;
}

/**
 * @brief   Report to ACC that a sensor has exceeded a critical limit
 */
int iACC_CriticalLimitExceeded( uint8_t ucSensorId, uint8_t ucSensorType  )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        ACC_MBOX_MSG xMsg = { 0 };
        xMsg.eMsgType = ACC_MSG_TYPE_SHUTDOWN_CRITICAL_THRESHOLD_CROSS;
        xMsg.xSensor.ucShutdownId = ucSensorId;
        xMsg.xSensor.xShutdownType = ucSensorType;

        if( OSAL_ERRORS_NONE == iOSAL_MBox_Post( pxThis->pvOsalMBoxHdl, ( void* )&xMsg, OSAL_TIMEOUT_NO_WAIT ) )
        {
            INC_STAT_COUNTER( ACC_PROXY_STATS_MAILBOX_POST );
            iStatus = OK;
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MAILBOX_POST );
        }

    }

    return iStatus;
}

/**
 * @brief   Report to ACC that a sensor has exceeded a warning limit
 */
int iACC_WarningLimitExceeded( uint8_t ucSensorId, uint8_t ucSensorType  )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        ACC_MBOX_MSG xMsg = { 0 };
        xMsg.eMsgType = ACC_MSG_TYPE_SHUTDOWN_WARNING_THRESHOLD_CROSS;
        xMsg.xSensor.ucShutdownId = ucSensorId;
        xMsg.xSensor.xShutdownType = ucSensorType;

        if( OSAL_ERRORS_NONE == iOSAL_MBox_Post( pxThis->pvOsalMBoxHdl, ( void* )&xMsg, OSAL_TIMEOUT_NO_WAIT ) )
        {
            INC_STAT_COUNTER( ACC_PROXY_STATS_MAILBOX_POST );
            iStatus = OK;
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MAILBOX_POST );
        }

    }

    return iStatus;
}

/**
 * @brief   Re-enable the clock if previously shutdown
 */
int iACC_ResetLimit( void  )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        ACC_MBOX_MSG xMsg = { 0 };
        xMsg.eMsgType = ACC_MSG_TYPE_SHUTDOWN_REENABLE_CLOCK;

        if( OSAL_ERRORS_NONE == iOSAL_MBox_Post( pxThis->pvOsalMBoxHdl, ( void* )&xMsg, OSAL_TIMEOUT_NO_WAIT ) )
        {
            INC_STAT_COUNTER( ACC_PROXY_STATS_MAILBOX_POST );
            iStatus = OK;
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MAILBOX_POST );
        }

    }

    return iStatus;
}

/**
 * @brief   Display the current stats
 */
int iACC_PrintStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        int i = 0;
        PLL_INF( ACC_NAME, "============================================================\n\r" );
        PLL_INF( ACC_NAME, "ACC Proxy Statistics:\n\r" );
        for( i = 0; i < ACC_PROXY_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( ACC_NAME, "------------------------------------------------------------\n\r" );
        PLL_INF( ACC_NAME, "ACC Proxy Errors:\n\r" );
        for( i = 0; i < ACC_PROXY_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( ACC_NAME, "============================================================\n\r" );

        iStatus = OK;
    }

    return iStatus;
}

/**
 * @brief   Set all stats values back to zero
 */
int iACC_ClearStatistics( void )
{
    int iStatus = ERROR;
    int i       = 0;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        for( i = 0; i < ACC_PROXY_STATS_MAX; i++ )
        {
            pxThis->pulStatCounters[i] = 0;

        }
        for( i = 0; i < ACC_PROXY_ERRORS_MAX; i++ )
        {

            pxThis->pulErrorCounters[i] = 0;
        }
        iStatus = OK;
    }

    return iStatus;
}


/**
 * @brief   Enable Clock Shutdown for threshold crossing
 *
 */
int iACC_EnableShutdown( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        pxThis->iClockShutdownEnabled = TRUE;

        iStatus = OK;
    }

    return iStatus;
}

/**
 * @brief   Disable Clock Shutdown for threshold crossing
 *
 */
int iACC_DisableShutdown( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        pxThis->iClockShutdownEnabled = FALSE;

        iStatus = OK;
    }

    return iStatus;
}

/**
 * @brief   Gets the current state of the proxy
 */
int iACC_GetState( MODULE_STATE *pxState )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( NULL != pxState ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvOsalMutexHdl, OSAL_TIMEOUT_TASK_WAIT_MS ) )
        {
            *pxState = pxThis->xState;

            INC_STAT_COUNTER( ACC_PROXY_STATS_STATUS_RETRIEVAL )

            iStatus = OK;
        }
        else
        {
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MUTEX_TAKE )
        }

        if( OSAL_ERRORS_NONE != iOSAL_Mutex_Release( pxThis->pvOsalMutexHdl ) )
        {
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_MUTEX_RELEASE )
            iStatus = ERROR;
        }
        else
        {
            INC_STAT_COUNTER( ACC_PROXY_STATS_MUTEX_RELEASE )
        }
    }
    else
    {
        INC_ERROR_COUNTER( ACC_PROXY_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}


/* Get functions **************************************************************/


/******************************************************************************/
/* Local Function implementations                                             */
/******************************************************************************/

/**
 * @brief   Driver task
 */
static void vProxyDriverTask( void *pvArgs )
{
    ACC_MBOX_MSG xMBoxData  = { 0 };
    uint32_t ulStartMs      = 0;

    FOREVER
    {
        ulStartMs = ulOSAL_GetUptimeMs();

        /* Get inputs from SET commands */
        if( OSAL_ERRORS_NONE == iOSAL_MBox_Pend( pxThis->pvOsalMBoxHdl, ( void* )&xMBoxData, OSAL_TIMEOUT_NO_WAIT ) )
        {
            INC_STAT_COUNTER( ACC_PROXY_STATS_MAILBOX_PEND );
            switch( xMBoxData.eMsgType )
            {
                case ACC_MSG_TYPE_SHUTDOWN_FATAL_THRESHOLD_CROSS:
                {
                    vClockShutdown( xMBoxData.xSensor.ucShutdownId, xMBoxData.xSensor.xShutdownType,
                                    ACC_SHUTDOWN_LEVEL_FATAL );
                }
                break;

                case ACC_MSG_TYPE_SHUTDOWN_CRITICAL_THRESHOLD_CROSS:
                {
                    vClockShutdown( xMBoxData.xSensor.ucShutdownId, xMBoxData.xSensor.xShutdownType,
                                    ACC_SHUTDOWN_LEVEL_CRITICAL );
                }
                break;

                case ACC_MSG_TYPE_SHUTDOWN_WARNING_THRESHOLD_CROSS:
                {
                    /* Do nothing for warning */
                }
                break;

                case ACC_MSG_TYPE_SHUTDOWN_FEATURE_ENABLED:
                {
                    if( OK != iACC_EnableShutdown() )
                    {
                        INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_STATS_ENABLE_SHUTDOWN_FAILED );
                    }
                    else
                    {
                        INC_STAT_COUNTER( ACC_PROXY_STATS_ENABLE_SHUTDOWN );
                    }
                }
                break;

                case ACC_MSG_TYPE_SHUTDOWN_FEATURE_DISABLED:
                {
                    if( OK != iACC_DisableShutdown() )
                    {
                        INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_STATS_DISABLE_SHUTDOWN_FAILED );
                    }
                    else
                    {
                        INC_STAT_COUNTER( ACC_PROXY_STATS_DISABLE_SHUTDOWN );
                    }
                }
                break;

                case ACC_MSG_TYPE_SHUTDOWN_REENABLE_CLOCK:
                {
                    vReenableClock();
                }
                break;

                default:
                    INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_UNKNOWN_MAILBOX_MSG );
                    PLL_ERR( ACC_NAME, "Error unknown mailbox message type 0x%x\r\n", xMBoxData.eMsgType );
                    break;
            }
        }

        pxThis->pulStatCounters[ ACC_PROXY_STATS_TASK_TIME_MS ] = UTIL_ELAPSED_TIME_MS( ulStartMs )
        iOSAL_Task_SleepMs( ACC_TASK_SLEEP_MS );
    }
}

/**
 * @brief   Check if clock shutdown is enabled and act on the clock if it is
 */
static void vClockShutdown( uint8_t ucSensorId, uint8_t ucSensorType, ACC_SHUTDOWN_LEVEL xClockShutdownLevel )
{
    int i = 0;
    int iStatus = ERROR;

    if( TRUE == pxThis->iClockShutdownEnabled )
    {
        for( i = 0; i < pxThis->xClockShutdownTable.ucNumSensors; i++ )
        {
            if( ( ucSensorId    == pxThis->xClockShutdownTable.xSensor[ i ].ucShutdownId ) &&
                ( ucSensorType  == pxThis->xClockShutdownTable.xSensor[ i ].xShutdownType ) )
            {
                /* If current active level is lower than the new level */
                if( pxThis->xClockShutdownLevel < xClockShutdownLevel )
                {
                    pxThis->xClockShutdownLevel = xClockShutdownLevel;

                    PLL_DBG( ACC_NAME, "Clockshutdown Level is %s\n\r",pucGetLevelString( xClockShutdownLevel ) );

                    /* TODO - Add this code in when available */
                    /* uint32_t ulClockshutdownOutput = ( REDUCE_GAPPING_DEMAND_RATE_TO_FIVE_PERCENTAGE |
                                                            MASK_CLOCKTHROTTLING_ENABLE_THROTTLING );
                    IO_SYNC_WRITE32( ulClockshutdownOutput, XPAR_SHELL_UTILS_UCC_0_BASEADDR ); */

                    /* Send an event up to AMC to report clock shutdown */
                    EVL_SIGNAL xNewSignal = { pxThis->ucMyId,
                                                ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ACTIVATED,
                                                pxThis->ucInstance++,
                                                0 };

                    iStatus = iEVL_RaiseEvent( pxThis->pxEvlRecord, &xNewSignal );
                    if( ERROR == iStatus )
                    {
                        INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_EVENT_RAISE );
                    }
                    else
                    {
                        INC_STAT_COUNTER( ACC_PROXY_STATS_EVENT_RAISE );
                    }

                    INC_STAT_COUNTER( ACC_PROXY_STATS_CLOCK_SHUTDOWN );
                }
            }
        }
    }
}

/**
 * @brief   Reset the current state and re-enable clock if required
 */
static void vReenableClock( void )
{
    int iStatus = ERROR;

    if( TRUE == pxThis->iClockShutdownEnabled )
    {
        /* TODO - Add this code in when available */
        /* uint32_t ulClockshutdownOutput = ( GAPPING_DEMAND_FULL_RATE |
                                                MASK_CLOCKTHROTTLING_DISABLE_THROTTLING );
        IO_SYNC_WRITE32( ulClockshutdownOutput, XPAR_SHELL_UTILS_UCC_0_BASEADDR ); */

        /* Send an event up to AMC to report clock has been reenabled */
        EVL_SIGNAL xNewSignal = { pxThis->ucMyId,
                                    ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_CLOCK_REENABLED,
                                    pxThis->ucInstance++,
                                    0 };

        iStatus = iEVL_RaiseEvent( pxThis->pxEvlRecord, &xNewSignal );
        if( ERROR == iStatus )
        {
            INC_ERROR_COUNTER_WITH_STATE( ACC_PROXY_ERRORS_EVENT_RAISE );
        }
        else
        {
            INC_STAT_COUNTER( ACC_PROXY_STATS_EVENT_RAISE );
        }

        pxThis->xClockShutdownLevel = ACC_SHUTDOWN_LEVEL_NONE;
        PLL_DBG( ACC_NAME, "Clock Re-Enabled \n\r" );
        INC_STAT_COUNTER( ACC_PROXY_STATS_CLOCK_SHUTDOWN );
    }
}

/**
 * @brief   Get text string for shutdown level
 */
const char *pucGetLevelString( ACC_SHUTDOWN_LEVEL xClockShutdownLevel )
{
    const char *pucLevelString = "ACC_SHUTDOWN_LEVEL_NONE";

    switch ( xClockShutdownLevel )
    {
        case ACC_SHUTDOWN_LEVEL_NONE:
            pucLevelString = "ACC_SHUTDOWN_LEVEL_NONE";
            break;
        case ACC_SHUTDOWN_LEVEL_WARNING:
            pucLevelString = "ACC_SHUTDOWN_LEVEL_WARNING";
            break;
        case ACC_SHUTDOWN_LEVEL_CRITICAL:
            pucLevelString = "ACC_SHUTDOWN_LEVEL_CRITICAL";
            break;
        case ACC_SHUTDOWN_LEVEL_FATAL:
            pucLevelString = "ACC_SHUTDOWN_LEVEL_FATAL";
            break;

        default:
            break;
    }

    return pucLevelString;
}