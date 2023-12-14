/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains unit tests for osal_FreeRTOS.c
 *
 * @file test_osal_Linux.c
 *
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Unit test includes */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdbool.h>

/* Linux includes */
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <assert.h>   
#include <sys/utsname.h>

/* AMC includes */
#include "osal.h"
#include "standard.h"

#define PTHREAD_STACK_MIN                    ( 16384 )
#define DEFAULT_OS_TIMEOUT                   ( 1000000 )
#define MAX_TASK_NAME_LEN                    ( 30 )

#define SECONDS_TO_TICKS_FACTOR              ( 100 )
#define MILLISECONDS_TO_MICROSECONS_FACTOR   ( 1000 )
#define SECONDS_TO_MILLISECONDS_FACTOR       ( 1000 )
#define TICKS_TO_MICROSECONDS_FACTOR         ( 10000 )
#define NANOSECONDS_TO_MILLISECONDS_FACTOR   ( 1000000 )
#define NANOSECONDS_TO_TICKS_FACTOR          ( 10000000 )
#define NANOSECONDS_TO_SECONDS_FACTOR        ( 1000000000 )

#define EVENT_BIT_0	                ( 1 << 0 )
#define EVENT_BIT_1	                ( 1 << 1 )
#define EVENT_BIT_2	                ( 1 << 2 )
#define EVENT_BIT_3	                ( 1 << 3 )

static int lock = 0; /* used in Semaphore/Queue mock functions */

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
    char cName[ MAX_TASK_NAME_LEN ];
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

struct timespec OSAL_GLOBAL_START_TIME = { 0 };

/*****************************************************************************/
/* Wrapping/Mocking functions                                                */         
/*****************************************************************************/

int __wrap_pthread_create( pthread_t *pxThread, const pthread_attr_t *pxAttr,
                         void *( *pvStartRoutine ) ( void* ), void *pvArgs )
{
    return ( int )mock();
}

int __wrap_pthread_attr_init( pthread_attr_t *pxAttr )
{
    check_expected( pxAttr );

    return mock_type( int );
}

int __wrap_pthread_attr_setschedpolicy( pthread_attr_t *pxAttr, int iPolicy )
{
    check_expected( pxAttr );
    check_expected( iPolicy );

    return mock_type( int );
}

int __wrap_pthread_attr_setschedparam( pthread_attr_t *pxAttr, const struct sched_param *pxParam )
{
    check_expected( pxAttr );
    check_expected( pxParam );
 
    return mock_type( int );
}

int __wrap_pthread_attr_setstacksize( pthread_attr_t *pxAttr, size_t xStackSize )
{
    check_expected( pxAttr );
    check_expected( xStackSize );

    return mock_type( int );
}

int __real_usleep( useconds_t xUsec );

int __wrap_usleep( useconds_t xUsec )
{
    check_expected( xUsec );
    if ( mock_type( int ) )
    {
        return -1; 
    }
    else
    {
        return __real_usleep( xUsec );
    }
}

int __wrap_printf( const char *pcFormat, ... )
{
    char* pcPrinted = NULL;
    va_list xArgs;
    va_start( xArgs, pcFormat );
    pcPrinted = va_arg( xArgs, char* );
    va_end( xArgs );
    
    check_expected_ptr( pcFormat );
    check_expected_ptr( pcPrinted );

    int result = mock_type( int );

    return result;
}

void* __real_malloc( size_t xSize );

void* __wrap_malloc( size_t xSize )
{
    if ( mock_type( int ) )
    {
        return NULL; 
    }
    else
    {
        return __real_malloc( xSize );
    }
}

int __wrap_clock_gettime( clockid_t xClk_id, struct timespec *pxTs )
{
    check_expected( xClk_id );

    pxTs->tv_sec = mock_type( time_t );
    pxTs->tv_nsec = mock_type( long );

    return mock_type( int );

}

sem_t* __real_sem_open( const char *pcName, int iOflag,
                       mode_t xMode, unsigned int uiValue );

sem_t* __wrap_sem_open( const char *pcName, int iOflag,
                       mode_t xMode, unsigned int uiValue )
{
    if ( mock_type( sem_t* ) )
    {
        return SEM_FAILED; 
    }
    else
    {
        return __real_sem_open( pcName, iOflag, xMode, uiValue );
    }
}

int __wrap_sem_close( sem_t *pxSem )
{
    return mock_type( int );
}

int __wrap_sem_unlink( const char *pcName )
{
    return mock_type( int );
}

int __wrap_sem_wait( sem_t *pxSem )
{
    lock++;
    return mock_type( int );
}

int __wrap_sem_trywait( sem_t *pxSem )
{
    lock++;
    return mock_type( int );
}

int __wrap_sem_timedwait( sem_t *pxSem, const struct timespec *pxAbstime )
{
    lock++;
    return mock_type( int );
}

int __wrap_sem_post( sem_t *pxSem )
{
    lock--;
    return mock_type( int );
}

int __real_pthread_mutex_init( pthread_mutex_t *pxMutex, 
    const pthread_mutexattr_t *pxAttr );

int __wrap_pthread_mutex_init( pthread_mutex_t *pxMutex, 
    const pthread_mutexattr_t *pxAttr )
{
    pxMutex = ( pthread_mutex_t* )0x100;
    return mock_type( int );
}

int __wrap_pthread_mutex_destroy( pthread_mutex_t *pxMutex )
{
    return mock_type( int );
}

int __wrap_pthread_mutex_timedlock( pthread_mutex_t *pxMutex,
       const struct timespec *pxAbstime )
{
    lock++;
    return mock_type( int );
}

int __wrap_pthread_mutex_lock( pthread_mutex_t *pxMutex )
{
    lock++;
    return mock_type( int );
}

int __wrap_pthread_mutex_trylock( pthread_mutex_t *pxMutex )
{
    lock++;
    return mock_type( int );
}

int __wrap_pthread_mutex_unlock( pthread_mutex_t *pxMutex )
{
    lock--;
    return mock_type( int );
}

int __wrap_pthread_cond_destroy( pthread_cond_t *pxCond )
{
    return mock_type( int );
}

int __wrap_pthread_cond_broadcast( pthread_cond_t *pxCond )
{
    return mock_type( int );
}

int __wrap_pthread_cond_wait( pthread_cond_t *pxCond, pthread_mutex_t *pxMutex )
{
    return mock_type( int );
}

int __wrap_pthread_cond_timedwait( pthread_cond_t *pxCond, 
    pthread_mutex_t *pxMutex, const struct timespec *pxAbstime )
{
    return mock_type( int );
}

void __wrap_free( void *pvPtr )
{
}

int __wrap_timer_create( clockid_t xClockid, struct sigevent *pxEvp,
                         timer_t *pxTimerid )
{
    return mock_type( int );
}

int __wrap_timer_delete ( timer_t xTimerid )
{
    return mock_type( int );
}

int __wrap_timer_settime( timer_t xTimerid, int iFlags,
                  const struct itimerspec *pxNewValue,
                  struct itimerspec *pxOldValue )
{
    return mock_type( int );
}

FILE* __real_fopen( const char *pcName, const char *pcOperation );

FILE* __wrap_fopen( const char *pcName, const char *pcOperation )
{
    if ( mock_type( int ) )
    {
        return NULL; 
    }
    else
    {
        return __real_fopen( pcName, pcOperation );
    }
}

/*****************************************************************************/
/* Helper Functions                                                          */         
/*****************************************************************************/

/**
 * @brief   wrapper for will_return statement of the clock_gettime function
 */
void vExpectClockGettime()
{
    /* clock_gettime gets called in 3s */
    expect_any( __wrap_clock_gettime, xClk_id );
    will_return( __wrap_clock_gettime, 0 );
    will_return( __wrap_clock_gettime, 0 );
    will_return( __wrap_clock_gettime, 0 );
}

/*****************************************************************************/
/* UT Callback functions                                                     */         
/*****************************************************************************/

/**
 * @brief   Task function for Start OS UTs.
 */
void vStartOSTaskFunc( void )
{

}

/**
 * @brief   Generic task function for Task Create Test suite.
 */
void vTaskFunc( void* pvArg )
{
    while( 1 )
    {
        iOSAL_Task_SleepMs( 1000 );
    }
}

/**
 * @brief   Callback function for OSAL Timers  
 */ 
void vTimerCallback( void* pvTimerHandle )
{
    if( NULL != pvTimerHandle )
    {
    }
}

/**
 * @brief   Interrupt handler callback function skeleton for setting up interrupts 
 */ 
void vInterruptHandler( void* pvCallBackRef )
{
    if( NULL != pvCallBackRef )
    {
    }
}

/*****************************************************************************/
/* UT internal functions                                                     */         
/*****************************************************************************/

/* Used to assert lock value in testcase*/
int get_sem_lock()
{
    return lock;
}

/* Used to reset lock value in testcase*/
int set_sem_lock()
{
    lock = 0;
}

/*****************************************************************************/
/* Test Cases                                                                */         
/*****************************************************************************/

/*****************************************************************************/
/* Thread-safe API                                                           */         
/*****************************************************************************/

void test_iOSAL_GetOsVersion_failure_tests( void** state )
{
    ( void ) state; /* unused */

    char pcOs[ OSAL_OS_NAME_LEN ] = { 0 };
    uint8_t ucVersionMajor = 0;
    uint8_t ucVersionMinor = 0; 
    uint8_t ucVersionBuild = 0;

    /* param 1 error */
    assert_int_equal( iOSAL_GetOsVersion( NULL,
                                          &ucVersionMajor, 
                                          &ucVersionMinor,
                                          &ucVersionBuild ), OSAL_ERRORS_PARAMS );
    
    /* param 2 error */
    assert_int_equal( iOSAL_GetOsVersion( pcOs,
                                          NULL, 
                                          &ucVersionMinor,
                                          &ucVersionBuild ), OSAL_ERRORS_PARAMS );

    /* param 3 error */
    assert_int_equal( iOSAL_GetOsVersion( pcOs,
                                          &ucVersionMajor, 
                                          NULL,
                                          &ucVersionBuild ), OSAL_ERRORS_PARAMS );

    /* param 4 error */
    assert_int_equal( iOSAL_GetOsVersion( pcOs,
                                          &ucVersionMajor, 
                                          &ucVersionMinor,
                                          NULL ), OSAL_ERRORS_PARAMS );
    /* os 1 error */
    will_return( __wrap_fopen, -1 );
    assert_int_equal( iOSAL_GetOsVersion( pcOs,
                                          &ucVersionMajor,
                                          &ucVersionMinor,
                                          &ucVersionBuild ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_GetOsVersion_success_tests( void** state )
{
    ( void ) state; /* unused */

    char pcOs[ OSAL_OS_NAME_LEN ] = { 0 };

    uint8_t ucVersionMajor = 0;
    uint8_t ucVersionMinor = 0;
    uint8_t ucVersionBuild = 0;

    uint8_t ucExpectedVersionMajor = 0;
    uint8_t ucExpectedVersionMinor = 0;
    uint8_t ucExpectedVersionBuild = 0;

    struct utsname unameData = { 0 };
    uname( &unameData );

    /* parse unameData.release to get Linux version numbers */
    sscanf( unameData.release, "%hhd.%hhd.%hhd", &ucExpectedVersionMajor, &ucExpectedVersionMinor, &ucExpectedVersionBuild );

    /* happy path */
    will_return( __wrap_fopen, 0 );
    assert_int_equal( iOSAL_GetOsVersion( pcOs,
                                          &ucVersionMajor,
                                          &ucVersionMinor,
                                          &ucVersionBuild ), OSAL_ERRORS_NONE );

    assert_int_equal( ucExpectedVersionMajor, ucVersionMajor );
    assert_int_equal( ucExpectedVersionMinor, ucVersionMinor );
    assert_int_equal( ucExpectedVersionBuild, ucVersionBuild );
    assert_string_equal( pcOs, "Linux" );
}

void test_pvOSAL_MemAlloc_before_StartOS_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_int_equal( pvOSAL_MemAlloc( 0 ), NULL );
}

void test_pvOSAL_MemAlloc_before_StartOS_success_tests( void** state )
{
    ( void ) state; /* unused */

    /* happy path */
    will_return( __wrap_malloc, 0 );
    assert_int_not_equal( pvOSAL_MemAlloc( 5*sizeof( uint8_t ) ), NULL );
}

void test_pvOSAL_MemSet_before_StartOS_failure_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;

    /* param 1 error */
    assert_int_equal( pvOSAL_MemSet( pucMemBuff1, 1, 5 ), NULL );
}

void test_pvOSAL_MemSet_before_StartOS_success_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };

    /* happy path */
    assert_int_not_equal( pvOSAL_MemSet( pucMemBuff2, 1, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 1 );
    assert_int_equal( pucMemBuff2[ 1 ], 1 );
    assert_int_equal( pucMemBuff2[ 2 ], 1 );
    assert_int_equal( pucMemBuff2[ 3 ], 1 );
    assert_int_equal( pucMemBuff2[ 4 ], 1 );
}

void test_pvOSAL_MemCpy_before_StartOS_failure_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;
    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };

    /* param 1 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff1, pucMemSourceBuff, 5 ), NULL );

    /* param 2 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff2, NULL, 5 ), NULL );
}

void test_pvOSAL_MemCpy_before_StartOS_success_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };

    /* happy path */
    assert_int_not_equal( pvOSAL_MemCpy( pucMemBuff2, pucMemSourceBuff, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 0x55 );
    assert_int_equal( pucMemBuff2[ 1 ], 0x44 );
    assert_int_equal( pucMemBuff2[ 2 ], 0x33 );
    assert_int_equal( pucMemBuff2[ 3 ], 0x22 );
    assert_int_equal( pucMemBuff2[ 4 ], 0x11 );
}

void test_vOSAL_MemFree_before_StartOS_failure_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( NULL );

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( ( void** )&pucMemBuff1 );
}

void test_vOSAL_MemFree_before_StartOS_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    uint8_t *pucMemBuff2 = malloc( 5*sizeof( uint8_t ) );

    /* happy path */
    vOSAL_MemFree( ( void** )&pucMemBuff2 );
    assert_int_equal( pucMemBuff2, NULL );
}

void test_vOSAL_Printf_before_StartOS_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error - expect no functions to be called */
    vOSAL_Printf( NULL );
}

void test_vOSAL_Printf_before_StartOS_success_tests ( void **state )
{
    ( void ) state; /* unused */

    char *pcText = "\ntest print\n";

    /* We expect __wrap_printf to be called with "%s" and `text` */
    expect_string( __wrap_printf, pcFormat, "%s" );
    expect_string( __wrap_printf, pcPrinted, pcText );
    
    will_return( __wrap_printf, strlen( pcText ) );

    vOSAL_Printf( pcText );
}

/*****************************************************************************/
/* Scheduler API                                                             */
/*****************************************************************************/

void test_iOSAL_StartOS_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = NULL;
    void** ppvTaskHandle = NULL;
    void*  pvStartOSTaskFunc = NULL;
    uint16_t usStartTaskStackSize = 0x1000;
    uint32_t ulTaskPriority = 1;
    pthread_attr_t pxTaskAttr = { 0 };
    struct sched_param pxTaskParam = { 0 };

    /*******************************/
    /* iOSAL_StartOS Failure tests */
    /*******************************/

    /* param 1 error */
    assert_int_equal( iOSAL_StartOS( TRUE,
                                     ppvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     ulTaskPriority ), OSAL_ERRORS_PARAMS );
    
    /* param 2 error */
    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     pvStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     ulTaskPriority ), OSAL_ERRORS_PARAMS );

    /* param 3 error */
    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc,
                                     0, 
                                     ulTaskPriority ), OSAL_ERRORS_PARAMS );
    
    /* invaild OS return value 1 - malloc fails */
    will_return( __wrap_malloc, -1 );

    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     ulTaskPriority ), OSAL_ERRORS_OS_IMPLEMENTATION );
    
    
    /* invaild OS return value 2 - pthread_create fails */

    will_return( __wrap_malloc, 0 );
    
    expect_any( __wrap_pthread_attr_init, pxAttr );
    will_return( __wrap_pthread_attr_init, 0 );

    expect_any( __wrap_pthread_attr_setschedparam, pxAttr );
    expect_any( __wrap_pthread_attr_setschedparam, pxParam );
    will_return( __wrap_pthread_attr_setschedparam, 0 );

    expect_any( __wrap_pthread_attr_setstacksize, pxAttr );
    expect_value( __wrap_pthread_attr_setstacksize, xStackSize, PTHREAD_STACK_MIN );
    will_return( __wrap_pthread_attr_setstacksize, 0 );

    will_return( __wrap_pthread_create, -1 );

    assert_int_equal( iOSAL_StartOS( 1,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     1 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 3 - mutex_init fails */

    will_return( __wrap_malloc, 0 );

    expect_any( __wrap_pthread_attr_init, pxAttr );
    will_return( __wrap_pthread_attr_init, 0 );

    expect_any( __wrap_pthread_attr_setschedparam, pxAttr );
    expect_any( __wrap_pthread_attr_setschedparam, pxParam );
    will_return( __wrap_pthread_attr_setschedparam, 0 );

    expect_any( __wrap_pthread_attr_setstacksize, pxAttr );
    expect_value( __wrap_pthread_attr_setstacksize, xStackSize, PTHREAD_STACK_MIN );
    will_return( __wrap_pthread_attr_setstacksize, 0 );
    
    will_return( __wrap_pthread_create, 0 );

    will_return( __wrap_pthread_mutex_init, -1 );

    assert_int_equal( iOSAL_StartOS( 1,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     1 ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_StartOS_success_tests( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = NULL;
    void** ppvTaskHandle = NULL;
    void*  pvStartOSTaskFunc = NULL;
    uint16_t usStartTaskStackSize = 0x1000;
    uint32_t ulTaskPriority = 1;

    pthread_attr_t pxTaskAttr = { 0 };
    struct sched_param pxTaskParam = { 0 };

    will_return( __wrap_malloc, 0 );

    expect_any( __wrap_pthread_attr_init, pxAttr );
    will_return( __wrap_pthread_attr_init, 0 );

    expect_any( __wrap_pthread_attr_setschedparam, pxAttr );
    expect_any( __wrap_pthread_attr_setschedparam, pxParam );
    will_return( __wrap_pthread_attr_setschedparam, 0 );

    expect_any( __wrap_pthread_attr_setstacksize, pxAttr );
    expect_value( __wrap_pthread_attr_setstacksize, xStackSize, PTHREAD_STACK_MIN );
    will_return( __wrap_pthread_attr_setstacksize, 0 );
    
    will_return( __wrap_pthread_create, 0 );  

    vExpectClockGettime();
    will_return( __wrap_usleep, 0 );
    expect_value( __wrap_usleep, xUsec, DEFAULT_OS_TIMEOUT );

    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     ulTaskPriority ), OSAL_ERRORS_NONE );
}

void test_ulOSAL_GetUptimeTicks_failure_tests( void **state )
{
    ( void ) state; /* unused */

    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC );

    will_return_count( __wrap_clock_gettime, 0, 2 );
    will_return( __wrap_clock_gettime, -1 );

    uint32_t uptime_ticks = ulOSAL_GetUptimeTicks();

    assert_int_equal( uptime_ticks, 0 );
}

void test_ulOSAL_GetUptimeTicks_success_tests( void **state )
{
    ( void ) state; /* unused */

    OSAL_GLOBAL_START_TIME.tv_sec = 0;
    OSAL_GLOBAL_START_TIME.tv_nsec = 0;

    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC );

    will_return( __wrap_clock_gettime, 10000 );
    will_return_count( __wrap_clock_gettime, 0, 2 );

    uint32_t uptime_ticks = ulOSAL_GetUptimeTicks( );

    assert_int_equal( uptime_ticks, 10000 * SECONDS_TO_TICKS_FACTOR );
}

void test_ulOSAL_GetUptimeMs_failure_tests( void **state )
{
    ( void ) state; /* unused */

    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC);

    will_return_count( __wrap_clock_gettime, 0, 2 );
    will_return( __wrap_clock_gettime, -1 );

    uint32_t uptime_ms = ulOSAL_GetUptimeMs();

    assert_int_equal( uptime_ms, 0 );
}

void test_ulOSAL_GetUptimeMs_success_tests( void **state )
{
    ( void ) state; /* unused */

    OSAL_GLOBAL_START_TIME.tv_sec = 0;
    OSAL_GLOBAL_START_TIME.tv_nsec = 0;

    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC );

    will_return( __wrap_clock_gettime, 10000 );
    will_return_count( __wrap_clock_gettime, 0, 2 );

    uint32_t uptime_ms = ulOSAL_GetUptimeMs( );

    assert_int_equal( uptime_ms, 10000 * SECONDS_TO_MILLISECONDS_FACTOR );
}

/*****************************************************************************/
/* Task API                                                                  */
/*****************************************************************************/


void test_iOSAL_Task_Create_failure_tests( void** state )
{
   ( void ) state; /* unused */

    void*  pvTaskHandle = NULL;
    void** ppvTaskHandle = NULL;
    uint16_t usStartTaskStackSize = 0x1000;
    void*  pvTaskParam = NULL;
    uint32_t ulTaskPriority = 1;

    /* param 1 error */
    assert_int_equal( iOSAL_Task_Create( ppvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         NULL,
                                         ulTaskPriority, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );
    
    /* param 2 error */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         NULL, 
                                         usStartTaskStackSize, 
                                         NULL,
                                         ulTaskPriority, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );

    /* param 3 error (value 0 ) */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         0, 
                                         NULL,
                                         ulTaskPriority, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );

    /* param 3 error (not 32 bit aligned) */
    usStartTaskStackSize = 0x11;
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         NULL,
                                         ulTaskPriority, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );

    /* param 6 error */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         NULL,
                                         ulTaskPriority, 
                                         NULL ), OSAL_ERRORS_PARAMS );

    /* invaild OS return value 1 - malloc fails*/
    usStartTaskStackSize = 0x1000;

    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, -1 );
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         pvTaskParam,
                                         ulTaskPriority, 
                                         "Test Task" ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 2 - pthread_create fails*/
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, 0 );

    expect_any( __wrap_pthread_attr_init, pxAttr );
    will_return( __wrap_pthread_attr_init, 0 );

    expect_any( __wrap_pthread_attr_setschedparam, pxAttr );
    expect_any( __wrap_pthread_attr_setschedparam, pxParam );
    will_return( __wrap_pthread_attr_setschedparam, 0 );

    expect_any( __wrap_pthread_attr_setstacksize, pxAttr );
    expect_value( __wrap_pthread_attr_setstacksize, xStackSize, PTHREAD_STACK_MIN );
    will_return( __wrap_pthread_attr_setstacksize, 0 );
    will_return( __wrap_pthread_create, -1 );

    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         pvTaskParam,
                                         ulTaskPriority, 
                                         "Test Task" ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Task_Create_success_test( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = NULL;
    void*  pvTaskHandle2 = NULL;
    void*  pvTaskHandle3 = NULL;
    uint16_t usStartTaskStackSize = 0x1000;
    void*  pvTaskParam = NULL;
    uint8_t pucTaskParamArray[ 5 ] = { 0 };
    uint32_t ulTaskPriority = 1;

    /* Happy path 1 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, 0 );

    expect_any( __wrap_pthread_attr_init, pxAttr );
    will_return( __wrap_pthread_attr_init, 0 );

    expect_any( __wrap_pthread_attr_setschedparam, pxAttr );
    expect_any( __wrap_pthread_attr_setschedparam, pxParam );
    will_return( __wrap_pthread_attr_setschedparam, 0 );

    expect_any( __wrap_pthread_attr_setstacksize, pxAttr );
    expect_value( __wrap_pthread_attr_setstacksize, xStackSize, PTHREAD_STACK_MIN );
    will_return( __wrap_pthread_attr_setstacksize, 0 );
    will_return( __wrap_pthread_create, 0 );

    /* valid parameters */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc,
                                         usStartTaskStackSize,
                                         pvTaskParam,
                                         ulTaskPriority,
                                         "Test Task" ), OSAL_ERRORS_NONE );

    /* Assert that the task handle was properly set */
    assert_non_null( pvTaskHandle );

    /* Happy path 2 - smaller stack size */

    usStartTaskStackSize = 0x20;
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, 0 );

    expect_any( __wrap_pthread_attr_init, pxAttr );
    will_return( __wrap_pthread_attr_init, 0 );

    expect_any( __wrap_pthread_attr_setschedparam, pxAttr );
    expect_any( __wrap_pthread_attr_setschedparam, pxParam );
    will_return( __wrap_pthread_attr_setschedparam, 0 );

    expect_any( __wrap_pthread_attr_setstacksize, pxAttr );
    expect_value( __wrap_pthread_attr_setstacksize, xStackSize, PTHREAD_STACK_MIN );
    will_return( __wrap_pthread_attr_setstacksize, 0 );
    will_return( __wrap_pthread_create, 0 );
    
    /* valid parameters */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc,
                                         usStartTaskStackSize,
                                         pvTaskParam,
                                         ulTaskPriority,
                                         "Test Task" ), OSAL_ERRORS_NONE );

    /* Assert that the task handle was properly set */
    assert_non_null( pvTaskHandle );

    /* Happy path 3 - Passing TaskParams */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, 0 );
    expect_any( __wrap_pthread_attr_init, pxAttr );
    will_return( __wrap_pthread_attr_init, 0 );

    expect_any( __wrap_pthread_attr_setschedparam, pxAttr );
    expect_any( __wrap_pthread_attr_setschedparam, pxParam );
    will_return( __wrap_pthread_attr_setschedparam, 0 );

    expect_any( __wrap_pthread_attr_setstacksize, pxAttr );
    expect_value( __wrap_pthread_attr_setstacksize, xStackSize, PTHREAD_STACK_MIN );
    will_return( __wrap_pthread_attr_setstacksize, 0 );  
    will_return( __wrap_pthread_create, 0 );

    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle3,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         ( void* )pucTaskParamArray,
                                         ulTaskPriority, 
                                         "Test Task 3" ), OSAL_ERRORS_NONE );
    
    /* Assert that the task handle was properly set */
    assert_non_null( pvTaskHandle );
}

void test_iOSAL_Task_Delete_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle2 = NULL;
    void** ppvTaskHandle = NULL;

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Task_Delete( ppvTaskHandle ), OSAL_ERRORS_INVALID_HANDLE );

}

void test_iOSAL_Task_Delete_success_tests( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = ( void* )0x10;
    void*  pvTaskHandle2 = ( void* )0x20;

    /* happy path */
    assert_int_equal( iOSAL_Task_Delete( &pvTaskHandle ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTaskHandle, NULL );

    /* happy path 2 */
    assert_int_equal( iOSAL_Task_Delete( &pvTaskHandle2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTaskHandle2, NULL );
}

void test_iOSAL_Task_Suspend_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_int_equal( iOSAL_Task_Suspend( NULL ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Task_Resume_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_int_equal( iOSAL_Task_Resume( NULL ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Task_SleepTicks_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_int_equal( iOSAL_Task_SleepTicks( 0 ), OSAL_ERRORS_PARAMS );
}

void test_iOSAL_Task_SleepTicks_success_tests( void** state )
{
    ( void ) state; /* unused */
    uint32_t ulSleepTicks = 1000;

    /* happy path */
    expect_value( __wrap_usleep, xUsec, ulSleepTicks * TICKS_TO_MICROSECONDS_FACTOR );
    will_return( __wrap_usleep, 0 );

    assert_int_equal( iOSAL_Task_SleepTicks( ulSleepTicks ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Task_SleepMs_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_int_equal( iOSAL_Task_SleepMs( 0 ), OSAL_ERRORS_PARAMS );
}

void test_iOSAL_Task_SleepMs_success_tests( void** state )
{
    ( void ) state; /* unused */
    uint32_t ulSleepMs = 1000;

    /* happy path */
    expect_value( __wrap_usleep, xUsec, ulSleepMs * MILLISECONDS_TO_MICROSECONS_FACTOR );
    will_return( __wrap_usleep, 0 );

    assert_int_equal( iOSAL_Task_SleepMs( ulSleepMs ), OSAL_ERRORS_NONE );
}

/*****************************************************************************/
/* Semaphore APIs                                                            */
/*****************************************************************************/

void test_iOSAL_Semaphore_Create_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvSemBinaryHandle = NULL;
    void* pvSemHandleSet = ( void* )0x10;

    uint32_t ullBinarySemCount = 1;
    uint32_t ullBinarySemBucket = 1;

    uint32_t ullCountingSemCount = 5;
    uint32_t ullCountingSemBucket = 5;

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_Create( NULL, 
                                              ullBinarySemCount,
                                              ullBinarySemBucket,
                                              "Test Semaphore" ) , OSAL_ERRORS_PARAMS );

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemHandleSet, 
                                              ullBinarySemCount,
                                              ullBinarySemBucket,
                                              "Test Semaphore" ) , OSAL_ERRORS_PARAMS );
    
    /* param 4 error */
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle, 
                                              ullBinarySemCount,
                                              ullBinarySemBucket,
                                              NULL ) , OSAL_ERRORS_PARAMS );

    /* invaild OS return value 1 - binary semaphore */
    will_return( __wrap_sem_open, -1 );
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle, 
                                              ullBinarySemCount,
                                              ullBinarySemBucket,
                                              "Test Binary Semaphore" ) , OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 2 - counting semaphore */
    will_return( __wrap_sem_open, -1 );
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle, 
                                                ullCountingSemCount,
                                                ullCountingSemBucket,
                                                "Test Binary Semaphore" ) , OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 3 - malloc fails */
    will_return( __wrap_sem_open, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, -1 );
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle, 
                                              ullBinarySemCount,
                                              ullBinarySemBucket,
                                              "Test Binary Semaphore" ) , OSAL_ERRORS_OS_IMPLEMENTATION );

}

void test_iOSAL_Semaphore_Create_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 4 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 4 );

    void* pvSemBinaryHandle = NULL;
    void* pvSemBinaryHandle2 = NULL;
    void* pvSemCountingHandle = NULL;
    void* pvSemCountingHandle2 = NULL;

    /* happy path - binary semaphore */
    will_return( __wrap_sem_open, 0 );
    will_return( __wrap_malloc, 0 );
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle, 
                                              1,
                                              1,
                                              "Test Binary Semaphore" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvSemBinaryHandle, NULL );

    /* happy path - binary semaphore (count value > 1) */
    will_return( __wrap_sem_open, 0 );
    will_return( __wrap_malloc, 0 );
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle2, 
                                              5,
                                              1,
                                              "Test Binary Semaphore" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvSemBinaryHandle2, NULL );

    /* happy path - counting semaphore */
    will_return( __wrap_sem_open, 0 );
    will_return( __wrap_malloc, 0 );
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemCountingHandle, 
                                              5,
                                              5,
                                              "Test Counting Semaphore" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvSemCountingHandle, NULL );

    /* happy path - counting semaphore (initial count differ to max count) */
    will_return( __wrap_sem_open, 0 );
    will_return( __wrap_malloc, 0 );
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemCountingHandle2, 
                                              1,
                                              10,
                                              "Test Counting Semaphore" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvSemCountingHandle2, NULL );
}

void test_iOSAL_Semaphore_Destroy_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 2 );

    OSAL_SEM_STRUCT* pvSemHandle = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle );
    OSAL_SEM_STRUCT* pvSemHandle1 = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle1 );

    void*  pvSemHandle2 = NULL;

    pvSemHandle->pcName = strdup( "test sem" );
    pvSemHandle1->pcName = strdup( "test sem 1" );

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Semaphore_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_Semaphore_Destroy( ( void* )&pvSemHandle2 ), OSAL_ERRORS_INVALID_HANDLE );

    /* OS 1 error - sem_close fails */
    will_return( __wrap_sem_close, -1 ) ;
    assert_int_equal( iOSAL_Semaphore_Destroy( ( void* )&pvSemHandle ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* OS 1 error - sem_unlink fails */
    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, -1 );
    assert_int_equal( iOSAL_Semaphore_Destroy( ( void* )&pvSemHandle1 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvSemHandle );
    free( pvSemHandle1 );
}

void test_iOSAL_Semaphore_Destroy_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 2 );

    OSAL_SEM_STRUCT* pvSemHandle = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle );
    OSAL_SEM_STRUCT* pvSemHandle1 = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle1 );

    pvSemHandle->pcName = strdup( "test sem" );
    pvSemHandle1->pcName = strdup( "test sem 1" );

    /* happy path 1 */
    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, 0 );
    assert_int_equal( iOSAL_Semaphore_Destroy( ( void* )&pvSemHandle ), OSAL_ERRORS_NONE );
    assert_int_equal( pvSemHandle, NULL );

    /* happy path 2 */
    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, 0 );
    assert_int_equal( iOSAL_Semaphore_Destroy( ( void* )&pvSemHandle1 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvSemHandle1, NULL );

    free( pvSemHandle );
    free( pvSemHandle1 );
}

void test_iOSAL_Semaphore_Pend_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    OSAL_SEM_STRUCT* pvSemHandle = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle );
    
    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_Pend( NULL, 100 ), OSAL_ERRORS_INVALID_HANDLE );

    /* invaild OS return value 1 - clock_gettime fails */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, -1, 3 );

    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 100 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 2 - wait fails */
    will_return( __wrap_sem_wait, -1 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 3 trywait fails */
    will_return( __wrap_sem_trywait, -1 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 4 - timedwait fails */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_sem_timedwait, -1 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 5 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvSemHandle );
}

void test_iOSAL_Semaphore_Pend_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    OSAL_SEM_STRUCT* pvSemHandle = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle );

    set_sem_lock(); /* set sem lock test value before happy path tests */

    /* happy path */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_sem_timedwait, 0 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path 2 */
    will_return( __wrap_sem_wait, 0 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path 3 */
    will_return( __wrap_sem_trywait, 0 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path 4 */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_sem_timedwait, 0 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path 5 */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_sem_timedwait, 0 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, SECONDS_TO_MILLISECONDS_FACTOR + 500 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    free( pvSemHandle );
}

void test_iOSAL_Semaphore_PostFromISR_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_PostFromISR( NULL ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Semaphore_PostFromISR_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 2 );

    OSAL_SEM_STRUCT* pvSemHandle = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle );
    OSAL_SEM_STRUCT* pvSemHandle2 = ( OSAL_SEM_STRUCT* ) malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvSemHandle2 );

    set_sem_lock(); /* set sem lock test value before happy path tests */

    /* happy path */

    /* pend semaphore */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_sem_timedwait, 0 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* post semaphore */
    will_return( __wrap_sem_post, 0 );
    assert_int_equal( iOSAL_Semaphore_PostFromISR( pvSemHandle ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );

    /* happy path 2 */

    /* pend semaphore */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_sem_timedwait, 0 );
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle2, 10 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* post semaphore */    
    will_return( __wrap_sem_post, 0 );
    assert_int_equal( iOSAL_Semaphore_PostFromISR( pvSemHandle2 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );

    free( pvSemHandle );
    free( pvSemHandle2 );
}
/*****************************************************************************/
/* Mutex APIs                                                                */
/*****************************************************************************/

void test_iOSAL_Mutex_Create_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 2 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 2 );

    void* pvMutexHandle = NULL;
    void* pvMutexHandleSet = ( void* )0x10;

    /* param 1 error - pointer to handle set NULL */
    assert_int_equal( iOSAL_Mutex_Create( NULL, "test_mutex" ), OSAL_ERRORS_PARAMS );

    /* param 1 error - handle set to value */
    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandleSet, "test_mutex" ) , OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle, NULL ) , OSAL_ERRORS_PARAMS );

    /* invaild OS return value 1 - malloc fails */
    will_return( __wrap_malloc, -1 );

    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle, "test_mutex" ) , OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 2 - mutex_init fails */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, -1 );

    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle, "test_mutex" ) , OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Mutex_Create_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 2 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 2 );

    void* pvMutexHandle = NULL;
    void* pvMutexHandle2 = NULL;

    /* happy path */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, 0 );
    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle, "test_mutex" ) , OSAL_ERRORS_NONE );
    assert_int_not_equal( pvMutexHandle, NULL );

    /* happy path 2 */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, 0 );
    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle2, "test_mutex" ) , OSAL_ERRORS_NONE );
    assert_int_not_equal( pvMutexHandle2, NULL );
}

void test_iOSAL_Mutex_Destroy_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandle = NULL;

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Mutex_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_Mutex_Destroy( &pvMutexHandle ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Mutex_Destroy_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 2 );

    OSAL_MUTEX_STRUCT *pvMutexHandleSet = malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMutexHandleSet );
    OSAL_MUTEX_STRUCT *pvMutexHandleSet2 = malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMutexHandleSet2 );

    /* happy path */
    will_return( __wrap_pthread_mutex_destroy, 0 );
    assert_int_equal( iOSAL_Mutex_Destroy( ( void*)&pvMutexHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMutexHandleSet, NULL );

    /* happy path 2 */
    will_return( __wrap_pthread_mutex_destroy, 0 );
    assert_int_equal( iOSAL_Mutex_Destroy( ( void*)&pvMutexHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMutexHandleSet2, NULL );

    free( pvMutexHandleSet );
    free( pvMutexHandleSet2 );
}

void test_iOSAL_Mutex_Take_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;

    /* param 1 error */
    assert_int_equal( iOSAL_Mutex_Take( NULL, 100 ), OSAL_ERRORS_INVALID_HANDLE );

    /* invaild OS return value 1 - clock_gettime fails */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, -1, 3 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 100 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 2 - pthread_mutex_lock fails */
    will_return( __wrap_pthread_mutex_lock, -1 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 3 - pthread_mutex_trylock fails */
    will_return( __wrap_pthread_mutex_trylock, -1 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 4 - pthread_mutex_timedlock fails */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_pthread_mutex_timedlock, -1 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 5 ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Mutex_Take_success_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;

    set_sem_lock(); /* set lock test value before happy path tests */

    /* happy path */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_pthread_mutex_timedlock, 0 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path */
    will_return( __wrap_pthread_mutex_lock, 0 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path */
    will_return( __wrap_pthread_mutex_trylock, 0 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_pthread_mutex_timedlock, 0 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, NANOSECONDS_TO_SECONDS_FACTOR * 2 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();
}

void test_iOSAL_Mutex_Release_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;

    /* param 1 error */
    assert_int_equal( iOSAL_Mutex_Release( NULL ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Mutex_Release_success_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;
    void* pvMutexHandleSet2 = ( void* )0x200;

    set_sem_lock(); /* set lock test value before happy path tests */

    /* happy path */

    /* take mutex */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_pthread_mutex_timedlock, 0 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* release mutex */
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_equal( iOSAL_Mutex_Release( pvMutexHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );

    /* happy path 2 */

    /* take mutex */
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_REALTIME );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    will_return( __wrap_pthread_mutex_timedlock, 0 );
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet2, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* release mutex */
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_equal( iOSAL_Mutex_Release( pvMutexHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );
}

/*****************************************************************************/
/* Mailbox APIs                                                              */
/*****************************************************************************/

void test_iOSAL_MBox_Create_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 2 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 2 );

    void* pvMBoxHandle = NULL;
    void* pvMBoxHandleSet = ( void* )0x10;

    /* param 1 error - pointer to handle set NULL */
    assert_int_equal( iOSAL_MBox_Create( NULL, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_PARAMS );

    /* param 1 error - handle set to value */
    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandleSet, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_PARAMS );

    /* param 4 error */
    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle, 10, sizeof( uint32_t ), NULL ), OSAL_ERRORS_PARAMS );

    /* OS Error 1 - malloc fails */
    will_return( __wrap_malloc, -1 );
    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* OS Error 2 - Semaphore_Create fails */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_open, -1 );
    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_MBox_Create_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 12 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 12 );

    void* pvMBoxHandle = NULL;
    void* pvMBoxHandle2 = NULL;
    void* pvMBoxHandle3 = NULL;

    /* happy path */
    will_return( __wrap_malloc, 0 );
    
    /* sem 1 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_open, 0 );

    /* sem 2 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_open, 0 );

    /* mutex 1 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, 0 );

    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle, 10, sizeof( uint32_t ), "test_mbox1" ), OSAL_ERRORS_NONE );
    assert_int_not_equal( pvMBoxHandle, NULL );

    /* happy path 2 - larger MBox length */
    will_return( __wrap_malloc, 0 );
    
    /* sem 1 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_open, 0 );

    /* sem 2 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_open, 0 );

    /* mutex 1 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, 0 );

    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle2, 100, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_NONE );
    assert_int_not_equal( pvMBoxHandle2, NULL );

    /* happy path 3 - smaller MBox item size */
    will_return( __wrap_malloc, 0 );
    
    /* sem 1 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_open, 0 );

    /* sem 2 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_open, 0 );

    /* mutex 1 create */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, 0 );

    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle3, 100, sizeof( uint8_t ), "test_mbox" ), OSAL_ERRORS_NONE );
    assert_int_not_equal( pvMBoxHandle3, NULL );
}

void test_iOSAL_MBox_Destroy_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 4 );

    void* pvMBoxHandle = NULL;
    OSAL_MAILBOX* pvMBoxHandleSet = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet );
    
    /* Test mailbox 1 setup */
    pvMBoxHandleSet->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxFull );
    pvMBoxHandleSet->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxEmpty );
    pvMBoxHandleSet->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->xMutex );

    pvMBoxHandleSet->pxHead = NULL;
    pvMBoxHandleSet->pxFull->pcName = strdup( "sem full" );
    pvMBoxHandleSet->pxEmpty->pcName = strdup( "sem empty" );
    
    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_MBox_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_MBox_Destroy( &pvMBoxHandle ), OSAL_ERRORS_INVALID_HANDLE );

    /* OS Error 1 */
    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, -1 );

    assert_int_equal( iOSAL_MBox_Destroy( ( void* )&pvMBoxHandleSet ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvMBoxHandleSet->xMutex );
    free( pvMBoxHandleSet->pxEmpty );
    free( pvMBoxHandleSet->pxFull );
    free( pvMBoxHandleSet );
}

void test_iOSAL_MBox_Destroy_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 12 );

    OSAL_MAILBOX* pvMBoxHandleSet = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet );
    OSAL_MAILBOX* pvMBoxHandleSet2 = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet2 );

    /* Test mailbox 1 setup */
    pvMBoxHandleSet->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxFull );
    pvMBoxHandleSet->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxEmpty );
    pvMBoxHandleSet->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->xMutex );

    pvMBoxHandleSet->pxHead = NULL;
    pvMBoxHandleSet->pxFull->pcName = strdup( "sem full" );
    pvMBoxHandleSet->pxEmpty->pcName = strdup( "sem empty" );
    
    /* Test mailbox 2 setup */
    pvMBoxHandleSet2->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxFull );
    pvMBoxHandleSet2->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxEmpty );
    pvMBoxHandleSet2->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->xMutex );
    
    pvMBoxHandleSet2->pxHead = NULL;
    pvMBoxHandleSet2->pxFull->pcName = strdup( "sem full" );
    pvMBoxHandleSet2->pxEmpty->pcName = strdup( "sem empty" );

    /* Add an item to mailbox 1 */
    OSAL_MAILBOX_ITEM* pxMailboxItem = ( OSAL_MAILBOX_ITEM*) malloc( sizeof( OSAL_MAILBOX_ITEM ) );

    pxMailboxItem->pvItem = malloc( sizeof( uint32_t ) );
    assert_non_null( pxMailboxItem->pvItem );
    pxMailboxItem->pxNext = NULL;
    pvMBoxHandleSet->pxHead = pxMailboxItem;

    /* Add an item to mailbox 2 */
    OSAL_MAILBOX_ITEM* pxMailboxItem2 = ( OSAL_MAILBOX_ITEM* ) malloc( sizeof( OSAL_MAILBOX_ITEM ) );
    pxMailboxItem2->pvItem = malloc( sizeof( uint32_t ) );
    assert_non_null( pxMailboxItem2->pvItem );
    pxMailboxItem2->pxNext = NULL;
    pvMBoxHandleSet2->pxHead = pxMailboxItem2;

    /* happy path */
    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, 0 );

    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, 0 );

    will_return( __wrap_pthread_mutex_destroy, 0 );
    assert_int_equal( iOSAL_MBox_Destroy( ( void* )&pvMBoxHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMBoxHandleSet, NULL );

    /* happy path 2 */
    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, 0 );

    will_return( __wrap_sem_close, 0 );
    will_return( __wrap_sem_unlink, 0 );

    will_return( __wrap_pthread_mutex_destroy, 0 );
    assert_int_equal( iOSAL_MBox_Destroy( ( void* )&pvMBoxHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMBoxHandleSet2, NULL );

    /* Memory cleanup done as part of iOSAL_MBox_Destroy function */
}

void test_iOSAL_MBox_Pend_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 6 );

    void* pvMBoxHandleSet = ( void* )0x10;
    uint32_t ulTaskData = 0;

    OSAL_MAILBOX* pvMBoxHandleSet2 = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet2 ); 

    pvMBoxHandleSet2->ulItemSize = sizeof( uint32_t ) ;

    /* Test mailbox setup */
    pvMBoxHandleSet2->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxFull );
    
    pvMBoxHandleSet2->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxEmpty );
    
    pvMBoxHandleSet2->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->xMutex );

    OSAL_MAILBOX_ITEM *pxNewMailboxItem = ( OSAL_MAILBOX_ITEM* )malloc( sizeof( OSAL_MAILBOX_ITEM ) );
    assert_non_null( pxNewMailboxItem );
    
    pxNewMailboxItem->pvItem = malloc( pvMBoxHandleSet2->ulItemSize );
    assert_non_null( pxNewMailboxItem->pvItem );

    *( ( uint32_t* )pxNewMailboxItem->pvItem ) = ulTaskData;

    /* param 1 error */
    assert_int_equal( iOSAL_MBox_Pend( NULL, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );


    /* param 2 error */
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, NULL, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );


    /* invalid OS return value 1 - sem_pend fails */
    will_return( __wrap_sem_wait, -1 );
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet2, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );


    /* invalid OS return value 2 - sem_post fails */
    will_return( __wrap_sem_wait, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, -1 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );

    pvMBoxHandleSet2->pxHead = pxNewMailboxItem;
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet2, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvMBoxHandleSet2->xMutex );
    free( pvMBoxHandleSet2->pxEmpty );
    free( pvMBoxHandleSet2->pxFull );
    free( pxNewMailboxItem );
    free( pvMBoxHandleSet2 );
}

void test_iOSAL_MBox_Pend_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 12 );

    OSAL_MAILBOX* pvMBoxHandleSet = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet );
    OSAL_MAILBOX* pvMBoxHandleSet2 = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet2 );

    uint32_t ulTaskData = 0;
    uint32_t ulTaskData2 = 100;

    /* Test mailbox setup */
    pvMBoxHandleSet->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxFull );
    pvMBoxHandleSet->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxEmpty );
    pvMBoxHandleSet->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->xMutex );

    OSAL_MAILBOX_ITEM *pxNewMailboxItem = ( OSAL_MAILBOX_ITEM* )malloc( sizeof( OSAL_MAILBOX_ITEM ) );
    assert_non_null( pxNewMailboxItem );
    
    pxNewMailboxItem->pvItem = malloc( pvMBoxHandleSet->ulItemSize );
    assert_non_null( pxNewMailboxItem->pvItem );

    pvMBoxHandleSet->pxHead = pxNewMailboxItem;

    /* Test mailbox 2 setup */
    pvMBoxHandleSet2->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxFull );
    pvMBoxHandleSet2->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxEmpty );
    pvMBoxHandleSet2->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->xMutex );

    OSAL_MAILBOX_ITEM *pxNewMailboxItem2 = ( OSAL_MAILBOX_ITEM* )malloc( sizeof( OSAL_MAILBOX_ITEM ) );
    assert_non_null( pxNewMailboxItem );
    
    pxNewMailboxItem2->pvItem = malloc( pvMBoxHandleSet2->ulItemSize );
    assert_non_null( pxNewMailboxItem2->pvItem );

    pvMBoxHandleSet2->pxTail = pxNewMailboxItem2;
    pvMBoxHandleSet2->pxHead = pvMBoxHandleSet2->pxTail;

    /* happy path - all timeout value variations */

    /* happy path 1 */
    will_return( __wrap_sem_wait, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_sem_wait, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet2, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    free( pxNewMailboxItem );

    free( pvMBoxHandleSet->pxFull );
    free( pvMBoxHandleSet->pxEmpty );
    free( pvMBoxHandleSet->xMutex );
    free( pvMBoxHandleSet );

    free( pvMBoxHandleSet2->pxFull );
    free( pvMBoxHandleSet2->pxEmpty );
    free( pvMBoxHandleSet2->xMutex );
    free( pvMBoxHandleSet2 );
}

void test_iOSAL_MBox_Post_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 5 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 5 );
    will_return_count( __wrap_malloc, 0, 4 );

    void* pvMBoxHandleSet = ( void* )0x10;
    uint32_t ulTaskData = 0;

    OSAL_MAILBOX* pvMBoxHandleSet2 = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet );

    pvMBoxHandleSet2->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxFull );
    pvMBoxHandleSet2->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->pxEmpty );
    pvMBoxHandleSet2->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet2->xMutex );

    pvMBoxHandleSet2->ulItemSize = sizeof( ulTaskData ); 

    pvMBoxHandleSet2->pxHead = NULL;
    pvMBoxHandleSet2->pxTail = NULL;

    /* param 1 error */
    assert_int_equal( iOSAL_MBox_Post( NULL, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, NULL, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );

    /* invaild OS return value 1 - malloc fails */
    will_return( __wrap_malloc, -1 );
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet2, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );
    
    /* invaild OS return value 2 - sem_pend fails */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_wait, -1 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, 0 );
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet2, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 3 - sem_post fails */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_sem_wait, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, -1 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, 0 );
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet2, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvMBoxHandleSet2->pxFull );
    free( pvMBoxHandleSet2->pxEmpty );
    free( pvMBoxHandleSet2->xMutex );
    free( pvMBoxHandleSet2 );
}

void test_iOSAL_MBox_Post_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 12 );
    will_return_count( __wrap_pthread_mutex_lock, 0, 12 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 12 );

    OSAL_MAILBOX* pvMBoxHandleSet = ( OSAL_MAILBOX* )malloc( sizeof( OSAL_MAILBOX ) );
    assert_non_null( pvMBoxHandleSet );
    uint32_t ulTaskData = 0;
    uint32_t ulTaskData2 = 100;

    /* Test Mailbox setup */
    pvMBoxHandleSet->pxFull = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxFull );
    pvMBoxHandleSet->pxEmpty = ( OSAL_SEM_STRUCT* )malloc( sizeof( OSAL_SEM_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->pxEmpty );
    pvMBoxHandleSet->xMutex = ( OSAL_MUTEX_STRUCT* )malloc( sizeof( OSAL_MUTEX_STRUCT ) );
    assert_non_null( pvMBoxHandleSet->xMutex );

    pvMBoxHandleSet->ulItemSize = sizeof( ulTaskData ); 

    pvMBoxHandleSet ->pxHead = NULL;
    pvMBoxHandleSet ->pxTail = NULL;

    /* happy path - all timeout value variations */

    /* happy path 1 */
    will_return( __wrap_sem_wait, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, 0 );
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_sem_wait, 0 );
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, 0 );
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData2, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 3 */
    vExpectClockGettime();
    will_return( __wrap_sem_trywait, 0 );
    vExpectClockGettime();
    will_return( __wrap_pthread_mutex_trylock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, 0 );
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData2, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );

    /* happy path 4 */
    will_return( __wrap_sem_timedwait, 0 );
    will_return( __wrap_pthread_mutex_timedlock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_sem_post, 0 );
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData2, 5 ), OSAL_ERRORS_NONE );

    free( pvMBoxHandleSet->pxFull );
    free( pvMBoxHandleSet->pxEmpty );
    free( pvMBoxHandleSet->xMutex );
    free( pvMBoxHandleSet );
}

/*****************************************************************************/
/* Event APIs                                                                */
/*****************************************************************************/

void test_iOSAL_EventFlag_Create_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 2 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 2 );

    void* pvEventFlagHandle = NULL;
    void* pvEventFlagHandleSet = ( void* )0x10;

    /* param 1 error - pointer to handle set NULL */
    assert_int_equal( iOSAL_EventFlag_Create( NULL, "test_event" ), OSAL_ERRORS_PARAMS );

    /* param 1 error - handle set to value */
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandleSet, "test_event" ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle, NULL ), OSAL_ERRORS_PARAMS );

    /* invaild OS return value 1 - malloc fails */
    will_return( __wrap_malloc, -1 );
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle, "test_event" ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 2 - mutex_init fails */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, -1 );
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle, "test_event" ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_EventFlag_Create_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 2 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 2 );

    void* pvEventFlagHandle = NULL;
    void* pvEventFlagHandle2 = NULL;

    /* happy path 1 */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, 0 );
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle, "test_event" ), OSAL_ERRORS_NONE );
    assert_int_not_equal( pvEventFlagHandle, NULL );

    /* happy path 2 */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_pthread_mutex_init, 0 );
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle2, "test_event_2" ), OSAL_ERRORS_NONE );
    assert_int_not_equal( pvEventFlagHandle2, NULL );
}

void test_iOSAL_EventFlag_Destroy_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandle = NULL;

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_EventFlag_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandle ), OSAL_ERRORS_INVALID_HANDLE );

    /* os error 1 - pthread_cond_destroy fails */
    pvEventFlagHandle = ( void* )0x200;

    will_return( __wrap_pthread_cond_destroy, -1 );
    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandle ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* os error 2 - pthread_mutex_destroy fails */
    pvEventFlagHandle = ( void* )0x200;

    will_return( __wrap_pthread_cond_destroy, 0 );
    will_return( __wrap_pthread_mutex_destroy, -1 );
    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandle ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_EventFlag_Destroy_success_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandleSet = ( void* )0x10;
    void* pvEventFlagHandleSet2 = ( void* )0x200;

    /* happy path 1 */
    will_return( __wrap_pthread_cond_destroy, 0 );
    will_return( __wrap_pthread_mutex_destroy, 0 );
    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvEventFlagHandleSet, NULL );

    /* happy path 2 */
    will_return( __wrap_pthread_cond_destroy, 0 );
    will_return( __wrap_pthread_mutex_destroy, 0 );
    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvEventFlagHandleSet2, NULL );
}

void test_iOSAL_EventFlag_Pend_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    OSAL_EVENT_STRUCT* pvEventFlagHandleSet  = malloc( sizeof( struct OSAL_EVENT_STRUCT ) );
    assert_non_null( pvEventFlagHandleSet );

    /* param 1 error */
    assert_int_equal( iOSAL_EventFlag_Pend( NULL, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_INVALID_HANDLE );

    /* OS error 1 */
    will_return( __wrap_pthread_mutex_lock, -1 );
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );
    
    /* OS error 2 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_cond_wait , -1 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );
    /* OS error 3 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_cond_timedwait , -1 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvEventFlagHandleSet );
}

void test_iOSAL_EventFlag_Pend_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    OSAL_EVENT_STRUCT* pvEventFlagHandleSet  = malloc( sizeof( struct OSAL_EVENT_STRUCT ) );
    assert_non_null( pvEventFlagHandleSet );

    /* happy path - all timeout value variations */

    /* happy path 1 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_cond_wait , 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_cond_wait , 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );

    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 3 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_cond_timedwait , 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );

    /* happy path 4 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_cond_timedwait , 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            5 ), OSAL_ERRORS_NONE );

    /* happy path 5 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_cond_timedwait , 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    expect_value( __wrap_clock_gettime, xClk_id, CLOCK_MONOTONIC );
    will_return_count( __wrap_clock_gettime, 0, 3 );
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            20 ), OSAL_ERRORS_NONE );

    free( pvEventFlagHandleSet );
}

void test_iOSAL_EventFlag_Post_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    OSAL_EVENT_STRUCT* pvEventFlagHandleSet = malloc( sizeof( struct OSAL_EVENT_STRUCT ) );
    assert_non_null( pvEventFlagHandleSet );

    /* param 1 error */
    assert_int_equal( iOSAL_EventFlag_Post( NULL, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_INVALID_HANDLE );

    /* OS Error 1 - pthread_cond_broadcast fails */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_pthread_cond_broadcast, -1 );
    assert_int_equal( iOSAL_EventFlag_Post( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvEventFlagHandleSet );
}

void test_iOSAL_EventFlag_Post_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_malloc, 0, 2 );

    OSAL_EVENT_STRUCT* pvEventFlagHandleSet = malloc( sizeof( struct OSAL_EVENT_STRUCT ) );
    assert_non_null( pvEventFlagHandleSet );
    OSAL_EVENT_STRUCT* pvEventFlagHandleSet2 = malloc( sizeof( struct OSAL_EVENT_STRUCT ) );
    assert_non_null( pvEventFlagHandleSet2 );

    /* happy path 1 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_pthread_cond_broadcast, 0 );
    assert_int_equal( iOSAL_EventFlag_Post( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_pthread_cond_broadcast, 0 );
    assert_int_equal( iOSAL_EventFlag_Post( pvEventFlagHandleSet2, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ) ), OSAL_ERRORS_NONE );

    free( pvEventFlagHandleSet );
    free( pvEventFlagHandleSet2 );
}

/*****************************************************************************/
/* Timer APIs                                                                */
/*****************************************************************************/

void test_iOSAL_Timer_Create_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 2 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 2 );

    void* pvTimerHandle = NULL;
    void* pvTimerHandle2 = NULL;
    void* pvTimerHandleSet = ( void* )0x10;

    /* param 1 error - pointer to handle set NULL */
    assert_int_equal( iOSAL_Timer_Create( NULL,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_PARAMS );

    /* param 1 error - handle set to value */
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandleSet,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          MAX_OSAL_TIMER_CONFIG, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_PARAMS );

    /* param 3 error */
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          NULL, 
                                          "test_timer" ) , OSAL_ERRORS_PARAMS );

    /* param 4 error */
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          NULL ) , OSAL_ERRORS_PARAMS );

    /* invaild OS return value 1 - malloc fails */
    will_return( __wrap_malloc, -1 );
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invaild OS return value 2 - timer create fails */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_timer_create, -1 );
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_OS_IMPLEMENTATION );
    
}

void test_iOSAL_Timer_Create_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return_count( __wrap_pthread_mutex_lock, 0, 2 );
    will_return_count( __wrap_pthread_mutex_unlock, 0, 2 );

    void* pvTimerHandle = NULL;
    void* pvTimerHandle2 = NULL;

    /* happy path 1 */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_timer_create, 0 );
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvTimerHandle, NULL );

    /* happy path 2 */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_timer_create, 0 );
    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle2,
                                          OSAL_TIMER_CONFIG_PERIODIC, 
                                          vTimerCallback, 
                                          "test_timer_2" ) , OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvTimerHandle2, NULL );
}

void test_iOSAL_Timer_Destroy_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    
    will_return( __wrap_malloc, 0 );
    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Timer_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_Timer_Destroy( &pvTimerHandle ), OSAL_ERRORS_INVALID_HANDLE );

    /* invaild OS return value */
    pvTimerHandleSet->pvCallback = vTimerCallback;

    will_return( __wrap_timer_delete, -1 );
    assert_int_equal( iOSAL_Timer_Destroy( ( void* )&pvTimerHandleSet ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvTimerHandleSet );
}


void test_iOSAL_Timer_Destroy_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_malloc, 0 );

    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );
    struct OSAL_TIMER_STRUCT* pvTimerHandleSet2 = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet2 );

    /* happy path 1 */
    will_return( __wrap_timer_delete, 0 );
    pvTimerHandleSet->pvCallback = vTimerCallback;
    assert_int_equal( iOSAL_Timer_Destroy( ( void* )&pvTimerHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTimerHandleSet, NULL );

    /* happy path 2 */
    will_return( __wrap_timer_delete, 0 );
    pvTimerHandleSet2->pvCallback = vTimerCallback;
    assert_int_equal( iOSAL_Timer_Destroy( ( void* )&pvTimerHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTimerHandleSet2, NULL );

    free( pvTimerHandleSet );
    free( pvTimerHandleSet2 );
}

void test_iOSAL_Timer_Start_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    uint32_t ulTimerDurationMs = 1000;

    will_return( __wrap_malloc, 0 );

    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );

    /* param 1 error */
    assert_int_equal( iOSAL_Timer_Start( pvTimerHandle, ulTimerDurationMs ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    pvTimerHandleSet->pvCallback = vTimerCallback;
    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet, 0 ), OSAL_ERRORS_PARAMS );

    /* invaild OS return value (xTimerChangePeriod) */
    will_return( __wrap_timer_settime, -1 );
    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvTimerHandleSet );
}

void test_iOSAL_Timer_Start_success_tests( void** state )
{
    ( void ) state; /* unused */

    will_return( __wrap_malloc, 0 );
    will_return( __wrap_malloc, 0 );

    uint32_t ulTimerDurationMs = 1000;
    uint32_t ulTimerDurationMs2 = 5;

    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );
    struct OSAL_TIMER_STRUCT* pvTimerHandleSet2 = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet2 );

    /* happy path 1 */
    pvTimerHandleSet->pvCallback = vTimerCallback;

    will_return( __wrap_timer_settime, 0 );
    
    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_NONE );

    /* happy path 2 - duration < min timeout */
    pvTimerHandleSet2->pvCallback = vTimerCallback;

    will_return( __wrap_timer_settime, 0 );
    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet2, ulTimerDurationMs2 ), OSAL_ERRORS_NONE );

    free( pvTimerHandleSet );
    free( pvTimerHandleSet2 );
}

void test_iOSAL_Timer_Stop_failure_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    void* pvTimerHandle = NULL;
    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );

    /* param 1 error */
    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandle ), OSAL_ERRORS_INVALID_HANDLE );

    /* os error 1 */
    pvTimerHandleSet->pvCallback = vTimerCallback;
    will_return( __wrap_timer_settime, -1 );
    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandleSet ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvTimerHandleSet );
}

void test_iOSAL_Timer_Stop_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );
    will_return( __wrap_malloc, 0 );

    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );
    struct OSAL_TIMER_STRUCT* pvTimerHandleSet2 = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet2 );

    /* happy path 1 */
    pvTimerHandleSet->pvCallback = vTimerCallback;
    will_return( __wrap_timer_settime, 0 );
    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandleSet ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    pvTimerHandleSet2->pvCallback = vTimerCallback;
    will_return( __wrap_timer_settime, 0 );
    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandleSet2 ), OSAL_ERRORS_NONE );

    free( pvTimerHandleSet );
    free( pvTimerHandleSet2 );
}

void test_iOSAL_Timer_Reset_failure_tests( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    
    will_return( __wrap_malloc, 0 );
    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );
    uint32_t ulTimerDurationMs = 1000;

    /* param 1 error */
    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandle, 1000 ), OSAL_ERRORS_INVALID_HANDLE );

    /* OS error 1 */
    will_return( __wrap_timer_settime, -1 );
    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet, 100 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    free( pvTimerHandleSet );
}

void test_iOSAL_Timer_Reset_success_tests( void** state )
{
    ( void ) state; /* unused */

    will_return( __wrap_malloc, 0 );
    will_return( __wrap_malloc, 0 );

    struct OSAL_TIMER_STRUCT* pvTimerHandleSet = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet );
    struct OSAL_TIMER_STRUCT* pvTimerHandleSet2 = malloc( sizeof( struct OSAL_TIMER_STRUCT ) );
    assert_non_null( pvTimerHandleSet2 );

    uint32_t ulTimerDurationMs = 1000;
    uint32_t ulTimerDurationMs2 = 5;

    /* happy path 2 */
    will_return( __wrap_timer_settime, 0 );
    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_NONE );

    /* happy path 2 - duration < min timeout */
    will_return( __wrap_timer_settime, 0 );
    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet2, ulTimerDurationMs2 ), OSAL_ERRORS_NONE );

    free( pvTimerHandleSet );
    free( pvTimerHandleSet2 );
}

void test_vTimerCallbackWrapper_success_test( void** state )
{
    ( void ) state; /* unused */

    struct OSAL_TIMER_STRUCT mock_osal_timer_struct;
    mock_osal_timer_struct.pvCallback = vTimerCallback;

    union sigval xSv;
    xSv.sival_ptr = &mock_osal_timer_struct;

    vTimerCallbackWrapper( xSv );
}

/*****************************************************************************/
/* Thread-safe API                                                           */         
/*****************************************************************************/

void test_pvOSAL_MemAlloc_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_int_equal( pvOSAL_MemAlloc( 0 ), NULL );
}

void test_pvOSAL_MemAlloc_success_tests( void** state )
{
    ( void ) state; /* unused */

    /* happy path */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    will_return( __wrap_malloc, 0 );
    assert_int_not_equal( pvOSAL_MemAlloc( 5*sizeof( uint8_t ) ), NULL );
}

void test_pvOSAL_MemSet_failure_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;

    /* param 1 error */
    assert_int_equal( pvOSAL_MemSet( pucMemBuff1, 1, 5 ), NULL );
}

void test_pvOSAL_MemSet_success_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };

    /* happy path */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_not_equal( pvOSAL_MemSet( pucMemBuff2, 1, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 1 );
    assert_int_equal( pucMemBuff2[ 1 ], 1 );
    assert_int_equal( pucMemBuff2[ 2 ], 1 );
    assert_int_equal( pucMemBuff2[ 3 ], 1 );
    assert_int_equal( pucMemBuff2[ 4 ], 1 );
}

void test_pvOSAL_MemCpy_failure_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;
    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };

    /* param 1 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff1, pucMemSourceBuff, 5 ), NULL );

    /* param 2 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff2, NULL, 5 ), NULL );
}

void test_pvOSAL_MemCpy_success_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };

    /* happy path */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    assert_int_not_equal( pvOSAL_MemCpy( pucMemBuff2, pucMemSourceBuff, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 0x55 );
    assert_int_equal( pucMemBuff2[ 1 ], 0x44 );
    assert_int_equal( pucMemBuff2[ 2 ], 0x33 );
    assert_int_equal( pucMemBuff2[ 3 ], 0x22 );
    assert_int_equal( pucMemBuff2[ 4 ], 0x11 );
}

void test_vOSAL_MemFree_failure_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( NULL );

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( ( void** )&pucMemBuff1 );
}

void test_vOSAL_MemFree_success_tests( void** state )
{
    ( void ) state; /* unused */
    will_return( __wrap_malloc, 0 );

    uint8_t *pucMemBuff2 = malloc( 5 * sizeof( uint8_t ) );
    assert_non_null( pucMemBuff2 );

    /* happy path */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    vOSAL_MemFree( ( void** )&pucMemBuff2 );
    assert_int_equal( pucMemBuff2, NULL );
}

void test_vOSAL_Printf_failure_tests( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error - expect no functions to be called */
    vOSAL_Printf( NULL );
}

void test_vOSAL_Printf_success_tests ( void **state )
{
    ( void ) state; /* unused */

    char *pcText = "\ntest print\n";

    /* We expect __wrap_printf to be called with "%s" and `text` */
    will_return( __wrap_pthread_mutex_lock, 0 );
    will_return( __wrap_pthread_mutex_unlock, 0 );
    expect_string( __wrap_printf, pcFormat, "%s" );
    expect_string( __wrap_printf, pcPrinted, pcText );
    
    will_return( __wrap_printf, strlen( pcText ) );

    vOSAL_Printf( pcText );
}

void test_vOSAL_EnterCritical_success_tests( void** state )
{
    ( void ) state; /* unused */

    /* happy path */
    will_return( __wrap_pthread_mutex_lock, 0 );
    vOSAL_EnterCritical();
}

void test_vOSAL_ExitCritical_success_tests( void** state )
{
    ( void ) state; /* unused */

    /* happy path */
    will_return( __wrap_pthread_mutex_unlock, 0 );
    vOSAL_ExitCritical();
}

void test_iOSAL_Interrupt_Setup_success_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t ucInterruptID = 1;
    void* pvCallBackRef = NULL;
    void ( *pvInterruptHandler )( void* ) = NULL;

    assert_int_equal( iOSAL_Interrupt_Setup( ucInterruptID, pvInterruptHandler, pvCallBackRef ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Interrupt_Enable_success_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t ucInterruptID = 1;

    assert_int_equal( iOSAL_Interrupt_Enable( ucInterruptID ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Interrupt_Disable_success_tests( void** state )
{
    ( void ) state; /* unused */

    uint8_t ucInterruptID = 1;

    assert_int_equal( iOSAL_Interrupt_Disable( ucInterruptID ), OSAL_ERRORS_INVALID_HANDLE );
}

/*****************************************************************************/
/* UT main entry point                                                       */         
/*****************************************************************************/

const struct CMUnitTest THREAD_SAFE_API_OS_NOT_STARTED_TESTS[] = {
    cmocka_unit_test( test_pvOSAL_MemAlloc_before_StartOS_failure_tests ),
    cmocka_unit_test( test_pvOSAL_MemAlloc_before_StartOS_success_tests ),
    cmocka_unit_test( test_pvOSAL_MemSet_before_StartOS_failure_tests ),
    cmocka_unit_test( test_pvOSAL_MemSet_before_StartOS_success_tests ),
    cmocka_unit_test( test_pvOSAL_MemCpy_before_StartOS_failure_tests ),
    cmocka_unit_test( test_pvOSAL_MemCpy_before_StartOS_success_tests ),
    cmocka_unit_test( test_vOSAL_MemFree_before_StartOS_success_tests ),
    cmocka_unit_test( test_vOSAL_Printf_before_StartOS_failure_tests ),
    cmocka_unit_test( test_vOSAL_Printf_before_StartOS_success_tests )

};

const struct CMUnitTest SCHEDULER_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_GetOsVersion_failure_tests ),
    cmocka_unit_test( test_iOSAL_GetOsVersion_success_tests ),
    cmocka_unit_test( test_iOSAL_StartOS_failure_tests ),
    cmocka_unit_test( test_iOSAL_StartOS_success_tests ),
    cmocka_unit_test( test_ulOSAL_GetUptimeTicks_failure_tests ),
    cmocka_unit_test( test_ulOSAL_GetUptimeTicks_success_tests ), 
    cmocka_unit_test( test_ulOSAL_GetUptimeMs_failure_tests ),
    cmocka_unit_test( test_ulOSAL_GetUptimeMs_success_tests )

};

const struct CMUnitTest TASK_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_Task_Create_failure_tests ),
    cmocka_unit_test( test_iOSAL_Task_Create_success_test ),
    cmocka_unit_test( test_iOSAL_Task_Delete_failure_tests ),
    cmocka_unit_test( test_iOSAL_Task_Delete_success_tests ),
    cmocka_unit_test( test_iOSAL_Task_Suspend_failure_tests ),
    cmocka_unit_test( test_iOSAL_Task_Resume_failure_tests ),
    cmocka_unit_test( test_iOSAL_Task_SleepTicks_failure_tests ),
    cmocka_unit_test( test_iOSAL_Task_SleepTicks_success_tests ),
    cmocka_unit_test( test_iOSAL_Task_SleepMs_failure_tests ),
    cmocka_unit_test( test_iOSAL_Task_SleepMs_success_tests )

};

const struct CMUnitTest SEMAPHORE_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_Semaphore_Create_failure_tests ),
    cmocka_unit_test( test_iOSAL_Semaphore_Create_success_tests ),
    cmocka_unit_test( test_iOSAL_Semaphore_Destroy_failure_tests ),
    cmocka_unit_test( test_iOSAL_Semaphore_Destroy_success_tests ),
    cmocka_unit_test( test_iOSAL_Semaphore_Pend_failure_tests ),
    cmocka_unit_test( test_iOSAL_Semaphore_Pend_success_tests ),
    cmocka_unit_test( test_iOSAL_Semaphore_PostFromISR_failure_tests ),
    cmocka_unit_test( test_iOSAL_Semaphore_PostFromISR_success_tests )

};

const struct CMUnitTest MUTEX_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_Mutex_Create_failure_tests ),
    cmocka_unit_test( test_iOSAL_Mutex_Create_success_tests ),
    cmocka_unit_test( test_iOSAL_Mutex_Destroy_failure_tests ),
    cmocka_unit_test( test_iOSAL_Mutex_Destroy_success_tests ),
    cmocka_unit_test( test_iOSAL_Mutex_Take_failure_tests ),
    cmocka_unit_test( test_iOSAL_Mutex_Take_success_tests ),
    cmocka_unit_test( test_iOSAL_Mutex_Release_failure_tests ),
    cmocka_unit_test( test_iOSAL_Mutex_Release_success_tests )

};

const struct CMUnitTest MAILBOX_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_MBox_Create_failure_tests ),
    cmocka_unit_test( test_iOSAL_MBox_Create_success_tests ),
    cmocka_unit_test( test_iOSAL_MBox_Destroy_failure_tests ),
    cmocka_unit_test( test_iOSAL_MBox_Destroy_success_tests ),
    cmocka_unit_test( test_iOSAL_MBox_Pend_failure_tests ),
    cmocka_unit_test( test_iOSAL_MBox_Pend_success_tests ),
    cmocka_unit_test( test_iOSAL_MBox_Post_failure_tests ),
    cmocka_unit_test( test_iOSAL_MBox_Post_success_tests )

};

const struct CMUnitTest EVENT_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_EventFlag_Create_failure_tests ),
    cmocka_unit_test( test_iOSAL_EventFlag_Create_success_tests ),
    cmocka_unit_test( test_iOSAL_EventFlag_Destroy_failure_tests ),
    cmocka_unit_test( test_iOSAL_EventFlag_Destroy_success_tests ),
    cmocka_unit_test( test_iOSAL_EventFlag_Pend_failure_tests ),
    cmocka_unit_test( test_iOSAL_EventFlag_Pend_success_tests ),
    cmocka_unit_test( test_iOSAL_EventFlag_Post_failure_tests ),
    cmocka_unit_test( test_iOSAL_EventFlag_Post_success_tests )

};

const struct CMUnitTest TIMER_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_Timer_Create_failure_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Create_success_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Destroy_failure_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Destroy_success_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Start_failure_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Start_success_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Stop_failure_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Stop_success_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Reset_failure_tests ),
    cmocka_unit_test( test_iOSAL_Timer_Reset_success_tests ),
    cmocka_unit_test( test_vTimerCallbackWrapper_success_test )

};

const struct CMUnitTest INTERRUPT_API_TESTS[] = {
    cmocka_unit_test( test_iOSAL_Interrupt_Setup_success_tests ),
    cmocka_unit_test( test_iOSAL_Interrupt_Enable_success_tests ),
    cmocka_unit_test( test_iOSAL_Interrupt_Disable_success_tests )

};

const struct CMUnitTest THREAD_SAFE_API_OS_STARTED_TESTS[] = {
    cmocka_unit_test( test_pvOSAL_MemAlloc_failure_tests ),
    cmocka_unit_test( test_pvOSAL_MemAlloc_success_tests ),
    cmocka_unit_test( test_pvOSAL_MemSet_failure_tests ),
    cmocka_unit_test( test_pvOSAL_MemSet_success_tests ),
    cmocka_unit_test( test_pvOSAL_MemCpy_failure_tests ),
    cmocka_unit_test( test_pvOSAL_MemCpy_success_tests ),
    cmocka_unit_test( test_vOSAL_MemFree_success_tests ),
    cmocka_unit_test( test_vOSAL_Printf_failure_tests ),
    cmocka_unit_test( test_vOSAL_Printf_success_tests ),
    cmocka_unit_test( test_vOSAL_EnterCritical_success_tests ),
    cmocka_unit_test( test_vOSAL_ExitCritical_success_tests )

};

int main( void )
{
    int iStatus = 0;

    iStatus += cmocka_run_group_tests( THREAD_SAFE_API_OS_NOT_STARTED_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( SCHEDULER_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( TASK_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( SEMAPHORE_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( MUTEX_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( MAILBOX_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( EVENT_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( TIMER_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( INTERRUPT_API_TESTS, NULL, NULL );
    iStatus += cmocka_run_group_tests( THREAD_SAFE_API_OS_STARTED_TESTS, NULL, NULL );

    return iStatus;
}
