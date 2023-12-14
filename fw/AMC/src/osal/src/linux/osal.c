/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the (OSAL) API implementation for Linux.
 *
 * @file osal_Linux.c
 *
 */


/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

/* Linux includes */
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <assert.h>   

/* AMC includes */
#include "osal.h"
#include "standard.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define DEFAULT_TASK_PRIORITY                ( 5 )  
#define MAX_TASK_NAME_LEN                    ( 30 )
#define NULL_CHARACTER_LEN                   ( 1 )
#define NAME_OFFSET                          ( 1 )
#define CHECK_32BIT_ALIGNMENT( x )           ( 0 == ( x & 0x3 ) )
#define PRINT_BUFFER_SIZE                    ( 256 )
#define MINIMUM_TIMEOUT_MS                   ( 1000 )
#define EXPECTED_NUM_VERSION_COMPONENTS      ( 3 )

#define BINARY_SEM_BUCKET_SIZE               ( 1 )
#define SEMAPHORE_PERMISSIONS                ( 0644 )
#define SEM_ERROR                            ( -1 )
#define TIMER_START_OFFSET                   ( 5 )
#define PTHREAD_STACK_MIN                    ( 16384 )

#define SECONDS_TO_TICKS_FACTOR              100
#define MILLISECONDS_TO_MICROSECONS_FACTOR   1000
#define SECONDS_TO_MILLISECONDS_FACTOR       1000
#define TICKS_TO_MICROSECONDS_FACTOR         10000
#define NANOSECONDS_TO_MILLISECONDS_FACTOR   1000000
#define NANOSECONDS_TO_TICKS_FACTOR          10000000
#define NANOSECONDS_TO_SECONDS_FACTOR        1000000000

#define DEFAULT_OS_NAME                      ( "Linux" )

#define RETURN_IF_OS_NOT_STARTED             if( TRUE != iOsStarted ) return OSAL_ERRORS_OS_NOT_STARTED

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

static pthread_mutex_t xPrintfMutexHandle           = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xGetCharMutexHandle          = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xMemSetMutexHandle           = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xMemCpyMutexHandle           = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xMemFreeMutexHandle          = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xMemAllocMutexHandle         = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t xCriticalSectionMutexHandle  = PTHREAD_MUTEX_INITIALIZER;

static int iOsStarted = FALSE;

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * @struct  OSAL_SEM_STRUCT
 * @brief   Stores the OSAL Semaphore information
 */
typedef struct OSAL_SEM_STRUCT
{
    sem_t* pxSem;
    char* pcName;

} OSAL_SEM_STRUCT;

/**
 * @struct  OSAL_MUTEX_STRUCT
 * @brief   Stores the OSAL Mutex information
 */
typedef struct OSAL_MUTEX_STRUCT
{
    char cName[ MAX_TASK_NAME_LEN + NULL_CHARACTER_LEN ];
    pthread_mutex_t xMutex;

} OSAL_MUTEX_STRUCT;

/**
 * @struct  OSAL_TASK_STRUCT
 * @brief   Stores the OSAL Task information
 */
typedef struct OSAL_TASK_STRUCT
{
    pthread_t xThread;
    pthread_mutex_t xMutex;

} OSAL_TASK_STRUCT;

/**
 * @struct  OSAL_MAILBOX_ITEM
 * @brief   Stores the OSAL Mailbox Item information
 */
typedef struct OSAL_MAILBOX_ITEM 
{
    void *pvItem;
    struct OSAL_MAILBOX_ITEM *pxNext;

} OSAL_MAILBOX_ITEM;

/**
 * @struct  OSAL_MAILBOX
 * @brief   Stores the OSAL Mailbox information
 */
typedef struct OSAL_MAILBOX
{
    OSAL_SEM_STRUCT *pxEmpty;
    OSAL_SEM_STRUCT *pxFull;
    OSAL_MUTEX_STRUCT *xMutex;
    char cName[ MAX_TASK_NAME_LEN ];
    OSAL_MAILBOX_ITEM *pxHead;
    OSAL_MAILBOX_ITEM *pxTail;
    size_t ulItemSize; 

} OSAL_MAILBOX;

/**
 * @struct  OSAL_EVENT_STRUCT
 * @brief   Stores the OSAL Event information
 */
typedef struct OSAL_EVENT_STRUCT
{
   pthread_cond_t xCond;
   pthread_mutex_t xMutex;
   uint32_t ulFlags;

} OSAL_EVENT_STRUCT;

/**
 * @struct  OSAL_TIMER_STRUCT
 * @brief   Stores the OSAL Timer information
 */
typedef struct OSAL_TIMER_STRUCT
{
    timer_t xTimerId;
    void ( *pvCallback )( void* );

} OSAL_TIMER_STRUCT;

/**
 * @struct  OSAL_GLOBAL_START_TIME
 * @brief   Stores the global start time of the OS task
 */
static struct timespec OSAL_GLOBAL_START_TIME = { 0 };

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/**
 * @brief   Initialises the system timer.
 */
static void vOSAL_InitUptime( void )
{
    clock_gettime( CLOCK_MONOTONIC, &OSAL_GLOBAL_START_TIME );
}

/**
 * @brief   Wrapper for the Timer Callback function.
 */
void vTimerCallbackWrapper( union sigval xSv )
{
    void ( *pvCallback )( void * ) = NULL;
    void *pvHandle = xSv.sival_ptr;

    struct OSAL_TIMER_STRUCT* pxTimerHandle = ( struct OSAL_TIMER_STRUCT* ) pvHandle;

    pvCallback = pxTimerHandle->pvCallback;
    pvCallback( pvHandle );
}

/*****************************************************************************/
/* Public APIs                                                               */
/*****************************************************************************/

/**
 * @brief   This function will return OS type and version information for the OSAL
 *          implementation being used.
 */
int iOSAL_GetOsVersion( char pcOs[ OSAL_OS_NAME_LEN ],
                        uint8_t* pucVersionMajor,
                        uint8_t* pucVersionMinor,
                        uint8_t* pucVersionBuild )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    if( ( NULL != pcOs ) &&
        ( NULL != pucVersionMajor ) &&
        ( NULL != pucVersionMinor ) &&
        ( NULL != pucVersionBuild ) )
    {
        strncpy( pcOs, DEFAULT_OS_NAME, OSAL_OS_NAME_LEN );

        /* Linux Kernel vesion information is sorted here */
        FILE *pxFp = fopen( "/proc/version", "r" );
        if( pxFp != NULL )
        {
            int iVersionMajor = 0, iVersionMinor = 0, iVersionBuild = 0;
            if( EXPECTED_NUM_VERSION_COMPONENTS == fscanf( pxFp, "Linux version %d.%d.%d", &iVersionMajor, &iVersionMinor, &iVersionBuild ) )
            {
                *pucVersionMajor = iVersionMajor;
                *pucVersionMinor = iVersionMinor;
                *pucVersionBuild = iVersionBuild;

                iStatus = OSAL_ERRORS_NONE;
            }

            fclose( pxFp );
        }
        else
        {
            /* Could not read version information from file */
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/*****************************************************************************/
/* Scheduler APIs                                                            */
/*****************************************************************************/

/**
 * @brief   Start the Linux scheduler.
 */
int iOSAL_StartOS( int         iRoundRobinEnabled,
                   void**      ppvTaskHandle,
                   void        ( *pvStartTask )( void ), 
                   uint16_t    usStartTaskStackSize,
                   uint32_t    ulStartTaskPriority )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    if( ( NULL != ppvTaskHandle      ) && 
        ( NULL != pvStartTask        ) && 
        ( 0 != usStartTaskStackSize  ) && 
        ( CHECK_32BIT_ALIGNMENT( usStartTaskStackSize ) ) &&
        ( FALSE == iOsStarted ) )
    {
        OSAL_TASK_STRUCT* pxTask = ( OSAL_TASK_STRUCT* ) malloc( sizeof( OSAL_TASK_STRUCT ) );

        if( NULL != pxTask )
        {
            /* Creating pthread attribute object */
            pthread_attr_t xAttr = { { 0 } };
            assert( OK == pthread_attr_init( &xAttr ) );
            
            /* Creating scheduling parameter struct */
            struct sched_param xParam = { 0 };
            xParam.sched_priority = ulStartTaskPriority;
            
            if( PTHREAD_STACK_MIN > usStartTaskStackSize )
            {
                usStartTaskStackSize = PTHREAD_STACK_MIN;
            }

            /* Setting pthread attributes */
            assert( OK == pthread_attr_setschedpolicy( &xAttr, SCHED_FIFO ) );
            assert( OK == pthread_attr_setschedparam( &xAttr, &xParam ) );
            assert( OK == pthread_attr_setstacksize( &xAttr, (size_t)usStartTaskStackSize ) );
            
            if( OSAL_ERRORS_NONE == pthread_create( &pxTask->xThread, 
                                        &xAttr,
                                        ( void* )*pvStartTask,
                                        "main" ) )
            {
                /* thread created successfully */
                iOsStarted = TRUE;
                iStatus = OSAL_ERRORS_NONE;

                *ppvTaskHandle = ( void* )pxTask;
                pthread_attr_destroy( &xAttr );
   
                /* Initialising global timer */
                vOSAL_InitUptime();

                /* mimicing freeRTOS vTaskStartScheduler blocking call - stops main thread from exiting */
                while( TRUE )
                {
                    iOSAL_Task_SleepMs( 1000 );
                }
            }
            else
            {
                /* The task was not created. */ 
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Returns tick count since OS task was initialised.
 */
uint32_t ulOSAL_GetUptimeTicks( void )
{
    struct timespec xNow = { 0 };
    uint64_t uptime_ticks = 0;

    if( OK == clock_gettime( CLOCK_MONOTONIC, &xNow ) )
    {
        /* 10ms/tick */ 
        uptime_ticks = ( xNow.tv_sec - OSAL_GLOBAL_START_TIME.tv_sec ) * SECONDS_TO_TICKS_FACTOR
                            + ( xNow.tv_nsec - OSAL_GLOBAL_START_TIME.tv_nsec ) / NANOSECONDS_TO_TICKS_FACTOR;
    }
    
    return ( uint32_t )uptime_ticks;
}

/**
 * @brief   Returns ms count since OS task was initialised.
 */
uint32_t ulOSAL_GetUptimeMs( void )
{
    struct timespec xNow = { 0 };
    uint64_t ulUptime_ms = 0;

    if( OK == clock_gettime( CLOCK_MONOTONIC, &xNow ) )
    {
        ulUptime_ms = ( xNow.tv_sec - OSAL_GLOBAL_START_TIME.tv_sec ) * SECONDS_TO_MILLISECONDS_FACTOR
                            + ( xNow.tv_nsec - OSAL_GLOBAL_START_TIME.tv_nsec ) / NANOSECONDS_TO_MILLISECONDS_FACTOR;
    }

    return ( uint32_t )ulUptime_ms;
}

/*****************************************************************************/
/* Task APIs                                                                 */
/*****************************************************************************/

/**
 * @brief   Create a new OSAL task.
 */
int iOSAL_Task_Create( void**      ppvTaskHandle,
                       void        ( *pvTaskFunction )( void* pvTaskParam ),  
                       uint16_t    usTaskStackSize,
                       void*       pvTaskParam, 
                       uint32_t    ulTaskPriority, 
                       const char* pcTaskName )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvTaskHandle  ) && 
        ( NULL != pvTaskFunction ) && 
        ( 0 != usTaskStackSize   ) && 
        ( CHECK_32BIT_ALIGNMENT( usTaskStackSize ) ) &&
        ( NULL != pcTaskName ) )
    {
        OSAL_TASK_STRUCT* pxTask = ( OSAL_TASK_STRUCT* ) pvOSAL_MemAlloc( sizeof( OSAL_TASK_STRUCT ) );
        
        if( NULL != pxTask )
        { 
            /* Creating pthread attribute object */
            pthread_attr_t xAttr = { { 0 } };
            assert( OK == pthread_attr_init( &xAttr ) );

            /* Creating scheduling parameter struct */
            struct sched_param xParam = { 0 };
            xParam.sched_priority = ulTaskPriority;

            if( PTHREAD_STACK_MIN > usTaskStackSize )
            {
                usTaskStackSize = PTHREAD_STACK_MIN;
            }

            /* Setting pthread attributes */
            assert( OK == pthread_attr_setschedpolicy( &xAttr, SCHED_FIFO ) );
            assert( OK == pthread_attr_setschedparam( &xAttr, &xParam ) );
            assert( OK == pthread_attr_setstacksize( &xAttr, usTaskStackSize ) );

            if( OSAL_ERRORS_NONE == pthread_create( &pxTask->xThread,
                                        &xAttr,
                                        ( void* )*pvTaskFunction,
                                        pvTaskParam ) )
            {
                *ppvTaskHandle = ( void* )pxTask;
                assert( OK == pthread_attr_destroy( &xAttr ) );

                iStatus = OSAL_ERRORS_NONE;
            }
            else
            {
                /* The task was not created. */ 
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Remove OSAL task.
 */
int iOSAL_Task_Delete( void** ppvTaskHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( NULL != ppvTaskHandle )
    {
        if( NULL != *ppvTaskHandle )
        {
            *ppvTaskHandle = NULL;
        }

        iStatus = OSAL_ERRORS_NONE;
    }

    return iStatus;
}

/**
 * @brief   Suspend OSAL task.
 */
int iOSAL_Task_Suspend( void* pvTaskHandle )
{
    int iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;

    return iStatus;
}

/**
 * @brief   Resume OSAL task.
 */
int iOSAL_Task_Resume( void* pvTaskHandle )
{
    int iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;

    return iStatus;
}

/**
 * @brief   Delay a task for a given number of ticks.
 */
int iOSAL_Task_SleepTicks( uint32_t ulSleepTicks )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( 0 < ulSleepTicks )
    {
        /* Convert ticks to microseconds */
        usleep( ulSleepTicks * TICKS_TO_MICROSECONDS_FACTOR );
        iStatus = OSAL_ERRORS_NONE;
    }

    return iStatus;
}

/**
 * @brief   Delay a task for a given number of ms.
 */
int iOSAL_Task_SleepMs( uint32_t ulSleepMs )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( MINIMUM_TIMEOUT_MS <= ulSleepMs )
    {
        /* Convert milliseconds to microseconds */
        usleep( ulSleepMs * MILLISECONDS_TO_MICROSECONS_FACTOR );
        iStatus = OSAL_ERRORS_NONE;
    }

    return iStatus;
}

/*****************************************************************************/
/* Semaphore APIs                                                            */
/*****************************************************************************/

/**
 * @brief   Creates a binary or counting semaphore, and sets OS Semaphore Handle by which the semaphore can be referenced.
 */
int iOSAL_Semaphore_Create( void** ppvSemHandle, 
                            uint32_t ullCount, 
                            uint32_t ullBucket, 
                            const char* pcSemName )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvSemHandle  ) &&
        ( NULL == *ppvSemHandle ) &&
        ( NULL != pcSemName ) )
    {
        sem_t *pxSem = NULL;
        if( BINARY_SEM_BUCKET_SIZE == ullBucket )
        {
            /* Create binary semaphore */
            pxSem = sem_open( pcSemName, O_CREAT, SEMAPHORE_PERMISSIONS, 0 );
        }
        else
        {
            /* Create counting semaphore */
            pxSem = sem_open( pcSemName, O_CREAT, SEMAPHORE_PERMISSIONS, ullCount );
        }
    
        if( pxSem == SEM_FAILED )
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
        else 
        {
            OSAL_SEM_STRUCT* pxSemData = ( OSAL_SEM_STRUCT* ) pvOSAL_MemAlloc( sizeof( OSAL_SEM_STRUCT ) );
            if( NULL != pxSemData )
            {
                pxSemData->pxSem = pxSem;
                pxSemData->pcName = strdup( pcSemName );
            
                *ppvSemHandle = pxSemData;

                /* ppvSemHandle has already been null checked*/

                /* Semaphore created successfully */
                iStatus = OSAL_ERRORS_NONE;
            }
            else
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
        }
    }

    return iStatus;
}

/**
 * @brief   Deletes the binary or counting semaphore, to which the handle refers. 
 */
int iOSAL_Semaphore_Destroy( void** ppvSemHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvSemHandle  ) &&
        ( NULL != *ppvSemHandle ) )
    {
        OSAL_SEM_STRUCT* pxSemData = ( OSAL_SEM_STRUCT* )*ppvSemHandle;

        if( ( SEM_ERROR == sem_close( pxSemData->pxSem ) ) || 
            ( SEM_ERROR == sem_unlink( pxSemData->pcName ) ) )
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
        else
        {
            iStatus = OSAL_ERRORS_NONE;
        }

        free( pxSemData->pcName );
        free( pxSemData );

        if( NULL != *ppvSemHandle )
        {
            *ppvSemHandle = NULL;
        }
    }

    return iStatus;
}

/**
 * @brief   Pends to / obtains a previously created semaphore, to which the handle refers. 
 */
int iOSAL_Semaphore_Pend( void* pvSemHandle, uint32_t ulTimeoutMs )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( NULL != pvSemHandle )
    {
        OSAL_SEM_STRUCT* pxSemData = ( OSAL_SEM_STRUCT* )pvSemHandle;

        if( OSAL_TIMEOUT_WAIT_FOREVER == ulTimeoutMs )
        {
            if( SEM_ERROR == sem_wait( pxSemData->pxSem ) )
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
            else
            {
                iStatus = OSAL_ERRORS_NONE;
            }
        }
        else if( OSAL_TIMEOUT_NO_WAIT == ulTimeoutMs )
        {
            if( SEM_ERROR == sem_trywait( pxSemData->pxSem ) )
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
            else
            {
                iStatus = OSAL_ERRORS_NONE;
            }
        }
        else
        {
            /* sem_timedwait() takes a deadline (absolute time in the future) rather than a timeout */
            struct timespec xTimeoutTime = { 0 };

            if( OK == clock_gettime( CLOCK_REALTIME, &xTimeoutTime ) )
            {
            
                if( ulTimeoutMs < MINIMUM_TIMEOUT_MS )
                {
                    ulTimeoutMs = MINIMUM_TIMEOUT_MS;
                }

                xTimeoutTime.tv_sec += ulTimeoutMs / SECONDS_TO_MILLISECONDS_FACTOR;
                xTimeoutTime.tv_nsec += ( ulTimeoutMs % SECONDS_TO_MILLISECONDS_FACTOR ) * NANOSECONDS_TO_MILLISECONDS_FACTOR;

                if( SEM_ERROR == sem_timedwait( pxSemData->pxSem, &xTimeoutTime ) )
                {
                    iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
                }
                else
                {
                    iStatus = OSAL_ERRORS_NONE;
                }
            }
            else
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }          
        }
    }

    return iStatus;
}

/**
 * @brief   Posts / Releases a previously created semaphore, to which the handle refers. 
 */
int iOSAL_Semaphore_Post( void* pvSemHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( NULL != pvSemHandle )
    {
        OSAL_SEM_STRUCT* pxSemData = ( OSAL_SEM_STRUCT* )pvSemHandle;
        if( SEM_ERROR == sem_post( pxSemData->pxSem ) )
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
        else
        {
            iStatus = OSAL_ERRORS_NONE;
        }
    }
    
    return iStatus;
}

/**
 * @brief   A version of iOSAL_Semaphore_Post() that can be called from an ISR.
 */
int iOSAL_Semaphore_PostFromISR( void* pvSemHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( NULL != pvSemHandle )
    {
        iStatus = iOSAL_Semaphore_Post( pvSemHandle );
    }

    return iStatus;
}

/*****************************************************************************/
/* Mutex APIs                                                                */
/*****************************************************************************/

/**
 * @brief   Creates a Mutex, and sets OS Mutex Handle by which the Mutex can be referenced.
 */
int iOSAL_Mutex_Create( void** ppvMutexHandle, const char* pcMutexName )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvMutexHandle  ) && 
        ( NULL == *ppvMutexHandle ) &&
        ( NULL != pcMutexName ) )
    {
        OSAL_MUTEX_STRUCT *pxMutexData = pvOSAL_MemAlloc( sizeof( OSAL_MUTEX_STRUCT ) );

        if( NULL != pxMutexData )
        {
            strncpy( pxMutexData->cName, pcMutexName, MAX_TASK_NAME_LEN );

            if( OK == pthread_mutex_init( &pxMutexData->xMutex, NULL ) )
            {
                *ppvMutexHandle = pxMutexData;

                /* ppvMutexHandle has already been null checked */

                /* Mutex created successfully */
                iStatus = OSAL_ERRORS_NONE;
            }
            else
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }        
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Deletes a Mutex, to which the handle refers.
 */
int iOSAL_Mutex_Destroy( void** ppvMutexHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;
    
    if( ( NULL != ppvMutexHandle ) &&
        ( NULL != *ppvMutexHandle ) )
    {
        OSAL_MUTEX_STRUCT *pxMutexData = ( OSAL_MUTEX_STRUCT* ) *ppvMutexHandle;
        pthread_mutex_destroy( &pxMutexData->xMutex );
        free( pxMutexData );

        if( NULL != *ppvMutexHandle )
        {
            *ppvMutexHandle = NULL;
        }

        iStatus = OSAL_ERRORS_NONE;
    }

    return iStatus;
}

/**
 * @brief   Obtains a previously created Mutex, to which the handle refers.
 */
int iOSAL_Mutex_Take( void* pvMutexHandle, uint32_t ulTimeout )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( NULL != pvMutexHandle )
    {
        OSAL_MUTEX_STRUCT* pxMutexData = ( OSAL_MUTEX_STRUCT* )pvMutexHandle;
        struct timespec xTimeoutTime = { 0 };

        if( OSAL_TIMEOUT_WAIT_FOREVER == ulTimeout )
        {
            if( OK != pthread_mutex_lock( &pxMutexData->xMutex ) )
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
            else
            {
                iStatus = OSAL_ERRORS_NONE;
            }
        }
        else if( OSAL_TIMEOUT_NO_WAIT == ulTimeout )
        {
            if( OK != pthread_mutex_trylock( &pxMutexData->xMutex ) )
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
            else
            {
                iStatus = OSAL_ERRORS_NONE;
            }
        }
        else
        {
            if( ulTimeout < MINIMUM_TIMEOUT_MS )
            {
                ulTimeout = MINIMUM_TIMEOUT_MS;
            }

            if( OK == clock_gettime( CLOCK_REALTIME, &xTimeoutTime ) )
            {
                xTimeoutTime.tv_sec += ulTimeout / SECONDS_TO_MILLISECONDS_FACTOR;
                xTimeoutTime.tv_nsec += ( ulTimeout % SECONDS_TO_MILLISECONDS_FACTOR ) * NANOSECONDS_TO_MILLISECONDS_FACTOR;

                if( OK != pthread_mutex_timedlock( &pxMutexData->xMutex, &xTimeoutTime ) )
                {
                    iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
                }
                else
                {
                    iStatus = OSAL_ERRORS_NONE;
                }
            }
            else
            {   
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
        }
    }

    return iStatus;
}

/**
 * @brief   Releases a previously created Mutex, to which the handle refers. 
 */
int iOSAL_Mutex_Release( void* pvMutexHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;
    
    if( NULL != pvMutexHandle )
    {
        OSAL_MUTEX_STRUCT *pxMutexData = ( OSAL_MUTEX_STRUCT* ) pvMutexHandle;
        assert( OK == pthread_mutex_unlock( &pxMutexData->xMutex ) );
        iStatus = OSAL_ERRORS_NONE;
    }

    return iStatus;
}

/*****************************************************************************/
/* Mailbox APIs                                                              */
/*****************************************************************************/

/**
 * @brief   Creates a new MailBox, and sets OS MailBox Handle by which the MailBox can be referenced.
 */
int iOSAL_MBox_Create( void **ppvMBoxHandle, uint32_t ulMBoxLength, uint32_t ulItemSize, const char *pcMBoxName )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvMBoxHandle  ) &&
        ( NULL == *ppvMBoxHandle ) &&
        ( NULL != pcMBoxName     ) )
    {
        OSAL_MAILBOX* pxMailbox = ( OSAL_MAILBOX* )pvOSAL_MemAlloc( sizeof( OSAL_MAILBOX ) );

        if( NULL != pxMailbox)
        {
            pxMailbox->ulItemSize = ulItemSize;

            pxMailbox->pxEmpty = NULL;
            pxMailbox->pxFull = NULL;
            pxMailbox->xMutex = NULL;

            if( ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Create( ( void** )&pxMailbox->pxEmpty, ulMBoxLength, ulMBoxLength, "empty_sem" ) ) &&
                ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Create( ( void** )&pxMailbox->pxFull, 0, ulMBoxLength, "full_sem" ) ) &&
                ( OSAL_ERRORS_NONE == iOSAL_Mutex_Create( ( void** )&pxMailbox->xMutex, "mailbox mutex" ) ) )
            {
                strncpy( pxMailbox->cName, pcMBoxName, sizeof( pxMailbox->cName ) - NAME_OFFSET );

                pxMailbox->pxHead = NULL;
                pxMailbox->pxTail = NULL;

                *ppvMBoxHandle = pxMailbox;

                /* ppvMBoxHandle has already been null checked */
                
                /* MBox created successfully */
                iStatus = OSAL_ERRORS_NONE;
            }
            else
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Resets a MailBox, to which the handle refers. 
 */
int iOSAL_MBox_Destroy( void **ppvMBoxHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvMBoxHandle ) &&
        ( NULL != *ppvMBoxHandle ) )
    {
        OSAL_MAILBOX *pxMailbox = ( OSAL_MAILBOX* )*ppvMBoxHandle;

        if( ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Destroy( ( void** )&pxMailbox->pxEmpty ) ) &&
            ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Destroy( ( void** )&pxMailbox->pxFull ) ) &&
            ( OSAL_ERRORS_NONE == iOSAL_Mutex_Destroy( ( void* ) &pxMailbox->xMutex ) ) )
        {
            while ( pxMailbox->pxHead )
            {
                OSAL_MAILBOX_ITEM* pxMailboxItem = pxMailbox->pxHead;
                pxMailbox->pxHead = pxMailbox->pxHead->pxNext;

                free( pxMailboxItem->pvItem );
                free( pxMailboxItem );
            }

            free( pxMailbox );
            *ppvMBoxHandle = NULL;

            iStatus = OSAL_ERRORS_NONE;
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Recieve an item from a message Mailbox, to which the handle refers. 
 */
int iOSAL_MBox_Pend( void *pvMBoxHandle, void *pvMBoxBuffer, uint32_t ulTimeoutMs )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != pvMBoxHandle ) &&
        ( NULL != pvMBoxBuffer ) )
    {
        OSAL_MAILBOX *pxMailbox = ( OSAL_MAILBOX* )pvMBoxHandle;

        if( ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Pend( ( void* )pxMailbox->pxFull, ulTimeoutMs ) ) &&
            ( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( ( void* )pxMailbox->xMutex, ulTimeoutMs ) ) )
        {
            OSAL_MAILBOX_ITEM *pxMailboxItem = pxMailbox->pxHead;

            if( ( NULL != pxMailboxItem ) && 
                ( NULL != pxMailboxItem->pvItem ) )
            {
                pvOSAL_MemCpy( pvMBoxBuffer, pxMailboxItem->pvItem, pxMailbox->ulItemSize );
                free( pxMailboxItem->pvItem );
            }

            if( pxMailbox->pxHead == pxMailbox->pxTail )
            {
                pxMailbox->pxHead = NULL;
                pxMailbox->pxTail = NULL;
            }
            else
            {
                pxMailbox->pxHead = pxMailbox->pxHead->pxNext;
            }
            
            if( ( OSAL_ERRORS_NONE == iOSAL_Mutex_Release( ( void* )pxMailbox->xMutex ) ) &&
                ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Post( ( void* )pxMailbox->pxEmpty ) ) )
            {
                iStatus = OSAL_ERRORS_NONE;
            }
            else
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }

            if( NULL != pxMailboxItem )
            {
                free( pxMailboxItem );
            }
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Posts an item onto a MailBox, to which the handle refers. 
 */
int iOSAL_MBox_Post( void *pvMBoxHandle, void *pvMBoxItem, uint32_t ulTimeoutMs )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != pvMBoxHandle ) &&
        ( NULL != pvMBoxItem ) )
    {
        OSAL_MAILBOX *pxMailbox = ( OSAL_MAILBOX* )pvMBoxHandle;
        OSAL_MAILBOX_ITEM *pxNewMailboxItem = ( OSAL_MAILBOX_ITEM* )pvOSAL_MemAlloc( sizeof( OSAL_MAILBOX_ITEM ) );

        if( ( NULL != pxMailbox ) && 
            ( NULL != pxNewMailboxItem ) )
        {
            pxNewMailboxItem->pvItem = pvOSAL_MemAlloc( pxMailbox->ulItemSize );
            pvOSAL_MemCpy( pxNewMailboxItem->pvItem, pvMBoxItem, pxMailbox->ulItemSize );
            pxNewMailboxItem->pxNext = NULL;

            if( ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Pend( ( void* )pxMailbox->pxEmpty, ulTimeoutMs ) ) &&
                ( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( ( void* )pxMailbox->xMutex, ulTimeoutMs ) ) )
            {
                if( pxMailbox->pxHead == NULL )
                {
                    pxMailbox->pxHead = pxNewMailboxItem;
                    pxMailbox->pxTail = pxMailbox->pxHead;
                }
                else
                {
                    pxMailbox->pxTail->pxNext = pxNewMailboxItem;
                    pxMailbox->pxTail = pxNewMailboxItem;
                }

                if( ( OSAL_ERRORS_NONE == iOSAL_Mutex_Release( ( void* )pxMailbox->xMutex ) ) &&
                    ( OSAL_ERRORS_NONE == iOSAL_Semaphore_Post( ( void* )pxMailbox->pxFull ) ) )
                {
                    iStatus = OSAL_ERRORS_NONE;
                }
                else
                {
                    iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
                }   
            }
            else
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/*****************************************************************************/
/* Event APIs                                                                */
/*****************************************************************************/

/**
 * @brief   Creates a new Event Flag group, and sets OS Handle by which the Event Flag group can be referenced.
 * @note    The default number of bits (or flags) implemented within an event group is 24.
 */
int iOSAL_EventFlag_Create( void** ppvEventFlagHandle, const char* pcEventFlagName )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvEventFlagHandle ) &&
        ( NULL == *ppvEventFlagHandle ) &&
        ( NULL != pcEventFlagName ) )
    {
        OSAL_EVENT_STRUCT* pxEvent = pvOSAL_MemAlloc ( sizeof( *pxEvent ) );
        if( NULL != pxEvent )
        {
            pthread_condattr_t xCattr  = { { 0 } };
            pthread_mutexattr_t xMattr = { { 0 } };

            /* Only proceed when condition attribute and mutex initialised successfuly */
            if( ( OK == pthread_condattr_init( &xCattr ) ) &&
                ( OK == pthread_condattr_setclock( &xCattr, CLOCK_MONOTONIC ) ) &&
                ( OK == pthread_cond_init( &pxEvent->xCond, &xCattr ) ) &&
                ( OK == pthread_mutexattr_init( &xMattr ) ) &&
                ( OK == pthread_mutexattr_setprotocol( &xMattr, PTHREAD_PRIO_INHERIT ) ) &&
                ( OK == pthread_mutex_init( &pxEvent->xMutex, &xMattr ) ) )
            {
                pxEvent->ulFlags = 0;

                *ppvEventFlagHandle = pxEvent;
                iStatus = OSAL_ERRORS_NONE;
            }
            else
            {
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Deletes an Event Flag group, to which the handle refers.
 */
int iOSAL_EventFlag_Destroy( void** ppvEventFlagHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvEventFlagHandle ) &&
        ( NULL != *ppvEventFlagHandle ) )
    {
        OSAL_EVENT_STRUCT* pxEvent = *ppvEventFlagHandle;

        *ppvEventFlagHandle = NULL;

        if( ( OK == pthread_cond_destroy( &pxEvent->xCond ) ) &&
            ( OK == pthread_mutex_destroy( &pxEvent->xMutex ) ) )
        {
            iStatus = OSAL_ERRORS_NONE;
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }

        free( pxEvent );
    }

    return iStatus;
}

/**
 * @brief   Pend task to Wait for a bit or group of bits to become set.
 */
int iOSAL_EventFlag_Pend( void* pvEventFlagHandle, uint32_t ulFlagWait, uint32_t ulTimeoutMs )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    if( NULL != pvEventFlagHandle )
    {
        OSAL_EVENT_STRUCT* pxEvent = ( OSAL_EVENT_STRUCT* )pvEventFlagHandle;
        struct timespec xTs = { 0 };

        uint64_t ulNsec = ( uint64_t )ulTimeoutMs * NANOSECONDS_TO_MILLISECONDS_FACTOR;

        if( ulTimeoutMs != OSAL_TIMEOUT_WAIT_FOREVER )
        {
            if( OK == clock_gettime( CLOCK_MONOTONIC, &xTs ) )
            {
                ulNsec += xTs.tv_nsec;

                xTs.tv_sec += ulNsec / NANOSECONDS_TO_SECONDS_FACTOR;
                xTs.tv_nsec = ulNsec % NANOSECONDS_TO_SECONDS_FACTOR;
            }
        }

        if( OK == pthread_mutex_lock( &pxEvent->xMutex ) )
        {
            while( 0 == ( pxEvent->ulFlags & ulFlagWait ) )
            {
                if( ulTimeoutMs != OSAL_TIMEOUT_WAIT_FOREVER )
                {
                    if( OK != pthread_cond_timedwait( &pxEvent->xCond, &pxEvent->xMutex, &xTs ) )
                    {
                        assert( OK == pthread_mutex_unlock( &pxEvent->xMutex ) );
                        iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
                    }
                    else
                    {
                        iStatus = OSAL_ERRORS_NONE;
                    }
                }
                else
                {
                    if( OK != pthread_cond_wait( &pxEvent->xCond, &pxEvent->xMutex ) )
                    {
                        assert( OK == pthread_mutex_unlock( &pxEvent->xMutex ) );
                        iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
                    }
                    else
                    {
                        iStatus = OSAL_ERRORS_NONE;
                    }
                }
            }

            assert( OK == pthread_mutex_unlock( &pxEvent->xMutex ) );
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/**
 * @brief   Sets or clears event flag bits, to which the handle refers.
 */
int iOSAL_EventFlag_Post( void* pvEventFlagHandle, uint32_t ulFlagSet )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    if( NULL != pvEventFlagHandle )
    {
        OSAL_EVENT_STRUCT* pxEvent = ( OSAL_EVENT_STRUCT* )pvEventFlagHandle;

        if( OK == pthread_mutex_lock( &pxEvent->xMutex ) )
        {
            pxEvent->ulFlags |= ulFlagSet;
            assert( OK == pthread_mutex_unlock( &pxEvent->xMutex ) );
        }

        if( OK == pthread_cond_broadcast( &pxEvent->xCond ) )
        {
            iStatus = OSAL_ERRORS_NONE;
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }

    return iStatus;
}

/*****************************************************************************/
/* Timer APIs                                                                */
/*****************************************************************************/

/**
 * @brief   Creates a new software timer instance, and sets OS Handle by which the Timer can be referenced.
 */
int iOSAL_Timer_Create( void** ppvTimerHandle,
                        OSAL_TIMER_CONFIG xTimerConfig,
                        void ( *pvTimerCallback )( void* ),
                        const char* pcTimerName )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvTimerHandle  ) &&
        ( NULL == *ppvTimerHandle ) &&
        ( MAX_OSAL_TIMER_CONFIG > xTimerConfig ) &&
        ( NULL != pvTimerCallback ) &&
        ( NULL != pcTimerName ) )
    {
        struct OSAL_TIMER_STRUCT* pxTimerHandle = pvOSAL_MemAlloc( sizeof( struct OSAL_TIMER_STRUCT ) );
        
        if( NULL != pxTimerHandle )
        {
            struct sigevent xSev = { { 0 } };

            memset( &xSev, 0, sizeof( struct sigevent ) );
            xSev.sigev_notify = SIGEV_THREAD;
            xSev.sigev_notify_function = vTimerCallbackWrapper;
            xSev.sigev_value.sival_ptr = pxTimerHandle;

            if( OK != timer_create( CLOCK_MONOTONIC, &xSev, &pxTimerHandle->xTimerId ) )
            {
                free( pxTimerHandle );
                iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
            }
            else
            {
                iStatus =  OSAL_ERRORS_NONE;

                /* Storing original callback */
                pxTimerHandle->pvCallback = pvTimerCallback;
                *ppvTimerHandle = pxTimerHandle;
            }
        }
        else
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
    }
    
    return iStatus;
}

/**
 * @brief   Deletes a Timer, to which the handle refers.
 */
int iOSAL_Timer_Destroy( void** ppvTimerHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != ppvTimerHandle ) &&
        ( NULL != *ppvTimerHandle ) )
    {
        struct OSAL_TIMER_STRUCT* pxTimerHandle = *ppvTimerHandle;

        if( OK != timer_delete( pxTimerHandle->xTimerId ) )
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
        else
        {
            free( pxTimerHandle );
            *ppvTimerHandle = NULL;

            iStatus = OSAL_ERRORS_NONE;
        }
    }

    return iStatus;
}

/**
 * @brief   Starts a Timer, to which the handle refers.
 */
int iOSAL_Timer_Start( void* pvTimerHandle, uint32_t ulDurationMs )
{
    int iStatus = OSAL_ERRORS_PARAMS;

    RETURN_IF_OS_NOT_STARTED;

    if( ( NULL != pvTimerHandle ) &&
        ( TIMER_START_OFFSET <= ulDurationMs ) )
    {
        struct OSAL_TIMER_STRUCT* pxTimerHandle = pvTimerHandle;
        struct itimerspec xIts = { { 0 } };

        /* Offset to account for callback function - Simulating RTOS Kernel */
        ulDurationMs -= TIMER_START_OFFSET;

        xIts.it_value.tv_sec = ulDurationMs / SECONDS_TO_MILLISECONDS_FACTOR;
        xIts.it_value.tv_nsec = ( ulDurationMs % SECONDS_TO_MILLISECONDS_FACTOR ) * NANOSECONDS_TO_MILLISECONDS_FACTOR;
        xIts.it_interval.tv_sec = xIts.it_value.tv_sec;
        xIts.it_interval.tv_nsec = xIts.it_value.tv_nsec;

        if( OK != timer_settime( pxTimerHandle->xTimerId, 0, &xIts, NULL ) )
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
        else
        {
            iStatus = OSAL_ERRORS_NONE;
        }
    }

    return iStatus;
}

/**
 * @brief   Stops a Timer, to which the handle refers.
 */
int iOSAL_Timer_Stop( void* pvTimerHandle )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( NULL != pvTimerHandle )
    {
        struct OSAL_TIMER_STRUCT* pxTimerHandle = pvTimerHandle;
        struct itimerspec xIts = { { 0 } };

        xIts.it_value.tv_sec = 0;
        xIts.it_value.tv_nsec = 0;
        xIts.it_interval.tv_sec = 0;
        xIts.it_interval.tv_nsec = 0;

        if( OK != timer_settime( pxTimerHandle->xTimerId, 0, &xIts, NULL ) )
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
        else
        {
            iStatus =  OSAL_ERRORS_NONE;
        }
    }

    return iStatus;
}

/**
 * @brief   Re-Starts a Timer, to which the handle refers.
 */
int iOSAL_Timer_Reset( void* pvTimerHandle, uint32_t ulDurationMs )
{
    int iStatus = OSAL_ERRORS_INVALID_HANDLE;

    RETURN_IF_OS_NOT_STARTED;

    if( NULL != pvTimerHandle )
    {
        /* No reset functionality in POSIX timers - Must restart */
        if( OSAL_ERRORS_NONE != iOSAL_Timer_Start( pvTimerHandle, ulDurationMs ) )
        {
            iStatus = OSAL_ERRORS_OS_IMPLEMENTATION;
        }
        else
        {
            iStatus = OSAL_ERRORS_NONE;
        }
    }
    
    return iStatus;
}

/*****************************************************************************/
/* Thread safe APIs                                                          */
/*****************************************************************************/

/**
 * @brief   Mark the start of critical code region.
 */
void vOSAL_EnterCritical( void )
{
    assert( OK == pthread_mutex_lock( &xCriticalSectionMutexHandle ) );
}

/**
 * @brief   Mark the end of critical code region.
 */
void vOSAL_ExitCritical( void )
{   
    assert( OK == pthread_mutex_unlock( &xCriticalSectionMutexHandle ) );
}

/**
 * @brief   OSAL wrapper for task/thread safe memory allocation.
 */
void* pvOSAL_MemAlloc( uint16_t usSize )
{
    void* pvMemory = NULL;

    if( 0 != usSize )
    {
        if( TRUE == iOsStarted ) 
        { 
            if( OK == pthread_mutex_lock( &xMemAllocMutexHandle ) )
            {
                pvMemory = malloc( usSize );
                assert( OK == pthread_mutex_unlock( &xMemAllocMutexHandle ) );
            }
        }
        else
        {
            /* OS not started or Mutex for memory allocation not created, allocation not thread safe */
            pvMemory = malloc( usSize );
        }
    }

    return pvMemory;
}

/**
 * @brief   OSAL wrapper for task/thread safe memory deallocation.
 */
void vOSAL_MemFree( void** ppv )
{
    if( ( NULL != ppv ) &&
        ( NULL != *ppv ) )
    {
        if( TRUE == iOsStarted )
        { 
            if( OK == pthread_mutex_lock( &xMemFreeMutexHandle ) )
            {
                free( *ppv );
                *ppv = NULL;

                assert( OK == pthread_mutex_unlock( &xMemFreeMutexHandle ) );
            }
        }
        else
        {
            /* Not thread safe */
            free( *ppv );
            *ppv = NULL;
        }
    }
}

/**
 * @brief   OSAL wrapper for task/thread safe memory set.
 */
void* pvOSAL_MemSet( void* pvDestination, int iValue, uint16_t usSize )
{
    void* pvSetMemory = NULL;

    if( NULL != pvDestination )
    {
        if(TRUE == iOsStarted )  
        {  
            if( OK == pthread_mutex_lock( &xMemSetMutexHandle ) )
            {
                pvSetMemory = memset( pvDestination, iValue, ( size_t )usSize );
                assert( OK == pthread_mutex_unlock( &xMemSetMutexHandle ) );
            }
        }
        else
        {
            /* Not thread safe */
            pvSetMemory = memset( pvDestination, iValue, ( size_t )usSize );        
        }
    }

    return pvSetMemory;
}

/**
 * @brief   OSAL wrapper for task/thread safe memory copy.
 */
void* pvOSAL_MemCpy( void* pvDestination, const void* pvSource, uint16_t usSize )
{
    void* pvSetMemory = NULL;

    if( ( NULL != pvDestination ) &&
        ( NULL != pvSource ) )
    {
        if( TRUE == iOsStarted ) 
        { 
            if( OK == pthread_mutex_lock( &xMemCpyMutexHandle ) )
            {
                pvSetMemory = memcpy( pvDestination, pvSource, ( size_t )usSize );
                assert( OK == pthread_mutex_unlock( &xMemCpyMutexHandle ) );
            }
        }
        else
        {
            /* Not thread safe */
            pvSetMemory = memcpy( pvDestination, pvSource, ( size_t )usSize );
        }
    }

    return pvSetMemory;
}

/**
 * @brief   OSAL wrapper for task/thread safe prints.
 */
void vOSAL_Printf( const char* pcFormat, ... )
{
    if( ( NULL != pcFormat ) &&
        ( PRINT_BUFFER_SIZE >= strlen( pcFormat ) ) )
    {
        char pcBuffer[PRINT_BUFFER_SIZE] = { 0 };
        va_list args = { { 0 } };
        va_start( args, pcFormat );
        vsnprintf( pcBuffer, PRINT_BUFFER_SIZE, pcFormat, args );

        if( TRUE == iOsStarted )
        { 
            if( OK == pthread_mutex_lock( &xPrintfMutexHandle ) )
            {
                printf( "%s", pcBuffer );
                assert( OK == pthread_mutex_unlock( &xPrintfMutexHandle ) );
            }
        }
        else
        {
            /* Not thread safe */
            printf( "%s", pcBuffer );
        }
        va_end( args );
    }
}

/**
 * @brief   OSAL wrapper for task/thread safe char reading.
 */
char cOSAL_GetChar( void )
{
    char cInput = '\0';

    if( TRUE == iOsStarted )
    { 
        /* take Mutex */
        if( OK == pthread_mutex_lock( &xGetCharMutexHandle ) )
        {
            /* Mutex taken successfully, read character */
            cInput =  ( char )getchar(); 

            /* release Mutex */
            assert( OK == pthread_mutex_unlock( &xGetCharMutexHandle ) );
        }
    }
    else
    {
        /* read character, note: Not thread safe */
        cInput = ( char )getchar(); 
    }
    
    return cInput;
}

/*****************************************************************************/
/* Stubs                                                                     */
/*****************************************************************************/

/**
 * @brief   Sets up interrupt handler callback with appropriate interrupt ID
 */
int iOSAL_Interrupt_Setup( uint8_t ucInterruptID, 
                           void    ( *pvInterruptHandler )( void* pvCallBackRef ),
                           void*   pvCallBackRef )
{
    return OSAL_ERRORS_INVALID_HANDLE;
}

/**
 * @brief   Enable OSAL interrupts
 */
int iOSAL_Interrupt_Enable( uint8_t ucInterruptID )
{
    return OSAL_ERRORS_INVALID_HANDLE;
}

/**
 * @brief   Disable OSAL interrupts
 */
int iOSAL_Interrupt_Disable( uint8_t ucInterruptID )
{
    return OSAL_ERRORS_INVALID_HANDLE;
}

/*****************************************************************************/
/* Debug stats functions                                                     */
/*****************************************************************************/


/**
* @brief   Prints All debug stats.
*/
void vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY eVerbosity, OSAL_STATS_TYPE eStatType )
{
    /* TODO implement print stats */
}

/**
* @brief   Clears debug stats and frees associated memory.
*/
void vOSAL_ClearAllStats( void )
{
    /* TODO implement clear stats */
}
