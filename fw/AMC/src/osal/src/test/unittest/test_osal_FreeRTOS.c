/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains unit tests for osal_FreeRTOS.c
 *
 * @file test_osal_FreeRTOS.c
 *
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Unit test includes */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#include "osal.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
#include "queue.h"

#include "../../freeRTOS/osal.c"

#define EVENT_BIT_0	                ( 1 << 0 )
#define EVENT_BIT_1	                ( 1 << 1 )
#define EVENT_BIT_2	                ( 1 << 2 )
#define EVENT_BIT_3	                ( 1 << 3 )
#define DEFAULT_TASK_PRIORITY       ( 5 )  
#define FABRIC_INTERRUPT_HANDLE     ( 1 )
#define OS_VERSION_MAJOR            ( tskKERNEL_VERSION_MAJOR )
#define OS_VERSION_MINOR            ( tskKERNEL_VERSION_MINOR )
#define OS_VERSION_BUILD            ( tskKERNEL_VERSION_BUILD )
#define PRINT_BUFFER_SIZE           ( 256 )
#define DEFAULT_TIMER_PERIOD_MS     ( 100 )
#define DEFAULT_TIMER_BLOCK_TIME_MS ( 1000 )


uint32_t ulPortYieldRequired = 0; /* local definition of extern variable */
static int lock = 0; /* used in Semaphore/Queue mock functions */

static void* pvPrintfMutexMockAddr = ( void* )0x11;
static void* pvMemSetMutexMockAddr = ( void* )0x22;
static void* pvMemCpyMutexMockAddr = ( void* )0x33;

static char cTestChar = 'a';

/*****************************************************************************/
/* Wrapping/Mocking functions                                                */         
/*****************************************************************************/

int __wrap_strcmp( const char *__s1, const char *__s2 )
{
    return( int )mock();
}


/*****************************************************************************/
/* Mocked Task functions                                                     */         
/*****************************************************************************/

BaseType_t __wrap_xTaskCreateStatic( TaskFunction_t pxTaskCode,
                               const char* const pcName, 
                               const configSTACK_DEPTH_TYPE usStackDepth,
                               void* const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t * const puxStackBuffer,
                               StaticTask_t * const pxTaskBuffer )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( pxTaskCode );
    check_expected( pcName );
    check_expected( usStackDepth );
    check_expected( pvParameters );
    check_expected( uxPriority );

    return( BaseType_t )mock();
}

void __wrap_vTaskDelete( TaskHandle_t xTaskToDelete )
{
    check_expected( xTaskToDelete );

    function_called();
}

void __wrap_vTaskSuspend( TaskHandle_t xTaskToSuspend )
{
    check_expected( xTaskToSuspend );

    function_called();
}

void __wrap_vTaskResume( TaskHandle_t xTaskToResume )
{
    check_expected( xTaskToResume );

    function_called();
}

void __wrap_vTaskDelay( const TickType_t xTicksToDelay )
{
    check_expected( xTicksToDelay );

    function_called();
}

void __wrap_vTaskStartScheduler( void )
{
    function_called();
}

TickType_t __wrap_xTaskGetTickCount( void )
{
    return( TickType_t )mock();
}

UBaseType_t __wrap_uxTaskGetNumberOfTasks( void ) 
{
    function_called();
}

UBaseType_t __wrap_uxTaskGetSystemState( TaskStatus_t * const pxTaskStatusArray,
                                  const UBaseType_t uxArraySize,
                                  configRUN_TIME_COUNTER_TYPE * const pulTotalRunTime )
{
    function_called();
}


/*****************************************************************************/
/* Mocked Semaphore/Mutex functions                                          */         
/*****************************************************************************/

QueueHandle_t __wrap_xQueueGenericCreate( const UBaseType_t uxQueueLength,
                                          const UBaseType_t uxItemSize,
                                          const uint8_t ucQueueType )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( uxQueueLength );
    check_expected( uxItemSize );
    check_expected( ucQueueType );

    return( QueueHandle_t )mock();
}

QueueHandle_t __wrap_xQueueCreateCountingSemaphore( const UBaseType_t uxMaxCount,
                                                    const UBaseType_t uxInitialCount )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( uxMaxCount );
    check_expected( uxInitialCount );

    return( QueueHandle_t )mock();
}

BaseType_t __wrap_xQueueSemaphoreTake( QueueHandle_t xQueue,
                                       TickType_t xTicksToWait )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    assert_int_not_equal( xQueue, NULL );
    check_expected( xQueue );
    check_expected( xTicksToWait );

    lock++;

    return( BaseType_t )mock();
}

BaseType_t __wrap_xQueueGenericSend( QueueHandle_t xQueue,
                                     const void * const pvItemToQueue,
                                     TickType_t xTicksToWait,
                                     const BaseType_t xCopyPosition )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    assert_int_not_equal( xQueue, NULL );
    check_expected( xQueue );
    check_expected( pvItemToQueue );
    check_expected( xTicksToWait );
    
    lock--;

    return( BaseType_t )mock();
}

BaseType_t __wrap_xQueueGiveFromISR( QueueHandle_t xQueue,
                                     BaseType_t * const pxHigherPriorityTaskWoken )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    assert_int_not_equal( xQueue, NULL );
    check_expected( xQueue );

    *pxHigherPriorityTaskWoken = pdTRUE;
    lock--;

    return( BaseType_t )mock(); 
}

QueueHandle_t __wrap_xQueueCreateMutex( const uint8_t ucQueueType )
{
    assert_int_equal( ucQueueType, queueQUEUE_TYPE_MUTEX);

    return( QueueHandle_t )mock();
}


/*****************************************************************************/
/* Mocked MBox/Queue functions                                               */         
/*****************************************************************************/

QueueHandle_t __wrap_xQueueCreate( UBaseType_t uxQueueLength,
                                   UBaseType_t uxItemSize )
{
    return( QueueHandle_t )mock();
}

void __wrap_vQueueDelete( QueueHandle_t xQueue )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    assert_int_not_equal( xQueue, NULL );
    check_expected( xQueue );

    function_called();
}

BaseType_t __wrap_xQueueReceive( QueueHandle_t xQueue,
                                 void *pvBuffer,
                                 TickType_t xTicksToWait )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    assert_int_not_equal( xQueue, NULL );
    check_expected( xQueue );
    check_expected( pvBuffer );
    check_expected( xTicksToWait );

    return( BaseType_t )mock();
}

BaseType_t __wrap_xQueueGenericSendFromISR( QueueHandle_t xQueue,
                                            const void* const pvItemToQueue,
                                            BaseType_t* const pxHigherPriorityTaskWoken,
                                            const BaseType_t xCopyPosition )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( xQueue );
    check_expected( pvItemToQueue );
    assert_int_not_equal( pxHigherPriorityTaskWoken, pdTRUE );

    *pxHigherPriorityTaskWoken = pdTRUE;
    return( BaseType_t )mock();
}


/*****************************************************************************/
/* Mocked Event functions                                                    */         
/*****************************************************************************/

EventGroupHandle_t __wrap_xEventGroupCreate( void )
{
    return( EventGroupHandle_t )mock();
}

void __wrap_vEventGroupDelete( EventGroupHandle_t xEventGroup )
{
    check_expected( xEventGroup );

    function_called();
}

EventBits_t __wrap_xEventGroupWaitBits( EventGroupHandle_t xEventGroup,
                                        const EventBits_t uxBitsToWaitFor,
                                        const BaseType_t xClearOnExit,
                                        const BaseType_t xWaitForAllBits,
                                        TickType_t xTicksToWait )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( xEventGroup );
    check_expected( uxBitsToWaitFor );
    assert_int_equal( xClearOnExit, pdTRUE );
    assert_int_equal( xWaitForAllBits, pdTRUE );
    check_expected( xTicksToWait );

    return( EventBits_t )uxBitsToWaitFor;
}

EventBits_t __wrap_xEventGroupSetBits( EventGroupHandle_t xEventGroup,
                                       const EventBits_t uxBitsToSet )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( xEventGroup );
    check_expected( uxBitsToSet );

    return( EventBits_t )uxBitsToSet;
}

BaseType_t __wrap_xEventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup,
                                             const EventBits_t uxBitsToSet,
                                             BaseType_t * pxHigherPriorityTaskWoken )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( xEventGroup );
    check_expected( uxBitsToSet );
    assert_int_not_equal( pxHigherPriorityTaskWoken, pdTRUE );

    *pxHigherPriorityTaskWoken = pdTRUE;
    return( BaseType_t )mock();
}


/*****************************************************************************/
/* Mocked Timer functions                                                    */         
/*****************************************************************************/

TimerHandle_t __wrap_xTimerCreate( const char * const pcTimerName,
                                   const TickType_t xTimerPeriodInTicks,
                                   const BaseType_t xAutoReload,
                                   void * const pvTimerID,
                                   TimerCallbackFunction_t pxCallbackFunction )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( pcTimerName );
    assert_int_equal( xTimerPeriodInTicks, pdMS_TO_TICKS( DEFAULT_TIMER_PERIOD_MS ) );
    check_expected( xAutoReload );
    assert_int_equal( pvTimerID, NULL );
    check_expected( pxCallbackFunction );
    
    return( TimerHandle_t )mock();
}

BaseType_t __wrap_xTimerGenericCommand( TimerHandle_t xTimer,
                                        const BaseType_t xCommandID,
                                        const TickType_t xOptionalValue,
                                        BaseType_t * const pxHigherPriorityTaskWoken,
                                        const TickType_t xTicksToWait )
{
    /* Check parameter values set by 'expect_*()' written in testcase */
    check_expected( xTimer );
    check_expected( xCommandID );
    check_expected( xOptionalValue );
    check_expected( pxHigherPriorityTaskWoken );
    check_expected( xTicksToWait );

    return( BaseType_t )mock();
}


/*****************************************************************************/
/* Mocked Interrupt functions                                                */         
/*****************************************************************************/

BaseType_t __wrap_xPortInstallInterruptHandler( uint8_t ucInterruptID, 
                                                XInterruptHandler pxHandler, 
                                                void *pvCallBackRef )
{
    return( BaseType_t )mock();
}

void __wrap_vPortEnableInterrupt( uint8_t ucInterruptID )
{
    function_called();
}

void __wrap_vPortDisableInterrupt( uint8_t ucInterruptID )
{
    function_called();
}


/*****************************************************************************/
/* Mocked Thread safe functions                                              */         
/*****************************************************************************/

void __wrap_vPortEnterCritical( void )
{
    function_called();
}

void __wrap_vPortExitCritical( void )
{
    function_called();
}

void* __wrap_pvPortMalloc( size_t xWantedSize )
{
    return malloc( xWantedSize );
}

void __wrap_vPortFree( void * pv )
{
    assert_int_not_equal( pv, NULL );

    if( pv != NULL )
	{
		free(pv);
	}	

    function_called();
}

void __wrap_xil_printf( const char8 *ctrl1, ...)
{
    assert_int_not_equal( ctrl1, NULL );

    if( NULL != ctrl1 )
    {
        char buffer[ PRINT_BUFFER_SIZE ] = { 0 };
        va_list args = { 0 };

        va_start( args, ctrl1 );
        vsnprintf( buffer, PRINT_BUFFER_SIZE, ctrl1, args );

        printf( "%s", buffer );
    }

    function_called();
}

char __wrap_inbyte( void )
{
    return( char )mock();
}


/*****************************************************************************/
/* Mocked Heap functions                                                     */         
/*****************************************************************************/

size_t __wrap_xPortGetFreeHeapSize( void ) { }

size_t __wrap_xPortGetMinimumEverFreeHeapSize( void ) { }


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
        printf( "Timer Callback Called" );
    }
}

/**
 * @brief   Interrupt handler callback function skeleton for setting up interrupts 
 */ 
void vInterruptHandler( void* pvCallBackRef )
{
    if( NULL != pvCallBackRef )
    {
        /* interrupt handler implementation */
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

void test_iOSAL_GetOsVersion_failure( void** state )
{
    ( void ) state; /* unused */

    char pcOs[ OSAL_OS_NAME_LEN ] = { 0 };
    uint8_t ucVersionMajor = 0;
    uint8_t ucVersionMinor = 0; 
    uint8_t ucVersionBuild = 0;

    /************************************/
    /* iOSAL_GetOsVersion Failure tests */
    /************************************/

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
}

void test_iOSAL_GetOsVersion_success( void** state )
{
    ( void ) state; /* unused */

    char pcOs[ OSAL_OS_NAME_LEN ] = { 0 };
    uint8_t ucVersionMajor = 0;
    uint8_t ucVersionMinor = 0; 
    uint8_t ucVersionBuild = 0;

    /************************************/
    /* iOSAL_GetOsVersion Success tests */
    /************************************/

    /* happy path */
    assert_int_equal( iOSAL_GetOsVersion( pcOs,
                                          &ucVersionMajor, 
                                          &ucVersionMinor,
                                          &ucVersionBuild ), OSAL_ERRORS_NONE );

    /* check values */
    assert_int_equal( ucVersionMajor, OS_VERSION_MAJOR );
    assert_int_equal( ucVersionMinor, OS_VERSION_MINOR );
    assert_int_equal( ucVersionBuild, OS_VERSION_BUILD );
    assert_string_equal( pcOs, "freeRTOS" );
}

void test_pvOSAL_MemAlloc_before_StartOS_failure( void** state )
{
    ( void ) state; /* unused */

    /**********************************/
    /* pvOSAL_MemAlloc Failure tests  */
    /**********************************/

    /* param 1 error */
    assert_int_equal( pvOSAL_MemAlloc( 0 ), NULL );
}

void test_pvOSAL_MemAlloc_before_StartOS_success( void** state )
{
    ( void ) state; /* unused */

    /**********************************/
    /* pvOSAL_MemAlloc Success tests  */
    /**********************************/

    /* happy path */
    assert_int_not_equal( pvOSAL_MemAlloc( 5 * sizeof( uint8_t ) ), NULL );
}

void test_pvOSAL_MemSet_before_StartOS_failure( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;

    /**********************************/
    /* pvOSAL_MemSet Failure tests    */
    /**********************************/

    /* param 1 error */
    assert_int_equal( pvOSAL_MemSet( pucMemBuff1, 1, 5 ), NULL );
}

void test_pvOSAL_MemSet_before_StartOS_success( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };

    /**********************************/
    /* pvOSAL_MemSet Success tests    */
    /**********************************/

    /* happy path */
    assert_int_not_equal( pvOSAL_MemSet( pucMemBuff2, 1, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 1 );
    assert_int_equal( pucMemBuff2[ 1 ], 1 );
    assert_int_equal( pucMemBuff2[ 2 ], 1 );
    assert_int_equal( pucMemBuff2[ 3 ], 1 );
    assert_int_equal( pucMemBuff2[ 4 ], 1 );
}

void test_pvOSAL_MemCpy_before_StartOS_failure( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;
    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };

    /*******************************/
    /* pvOSAL_MemCpy Failure tests */
    /*******************************/

    /* param 1 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff1, pucMemSourceBuff, 5 ), NULL );

    /* param 2 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff2, NULL, 5 ), NULL );
}

void test_pvOSAL_MemCpy_before_StartOS_success( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };

    /*******************************/
    /* pvOSAL_MemCpy Success tests */
    /*******************************/ 

    /* happy path */
    assert_int_not_equal( pvOSAL_MemCpy( pucMemBuff2, pucMemSourceBuff, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 0x55 );
    assert_int_equal( pucMemBuff2[ 1 ], 0x44 );
    assert_int_equal( pucMemBuff2[ 2 ], 0x33 );
    assert_int_equal( pucMemBuff2[ 3 ], 0x22 );
    assert_int_equal( pucMemBuff2[ 4 ], 0x11 );
}

void test_vOSAL_MemFree_before_StartOS_failure( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;

    /**********************************/
    /* vOSAL_MemFree Failure tests    */
    /**********************************/

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( NULL );

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( ( void** )&pucMemBuff1 );
}


void test_vOSAL_MemFree_before_StartOS_success( void** state )
{
    ( void ) state; /* unused */

    uint8_t *pucMemBuff2 = malloc( 5*sizeof( uint8_t ) );

    /**********************************/
    /* vOSAL_MemFree Success tests    */
    /**********************************/

    /* happy path */
    vOSAL_MemFree( ( void** )&pucMemBuff2 );
    assert_int_equal( pucMemBuff2, NULL );
}

void test_vOSAL_Printf_before_StartOS_failure( void** state )
{
    ( void ) state; /* unused */

    /**********************************/
    /* vOSAL_Printf Failure tests     */
    /**********************************/

    /* param 1 error - NULL */
    vOSAL_Printf( NULL );

    /* param 1 error - PRINT_BUFFER_SIZE < param 1 length */
    vOSAL_Printf( "257 character long string - aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n" );
}

void test_vOSAL_Printf_before_StartOS_success( void** state )
{
    ( void ) state; /* unused */
    int iTestParam = 2;

    /**********************************/
    /* pvOSAL_MemAlloc Success tests  */
    /**********************************/

    /* happy path */
    expect_function_call( __wrap_xil_printf );
    vOSAL_Printf( "test print happy path\r\n" );

    /* happy path 2 */
    expect_function_call( __wrap_xil_printf );
    vOSAL_Printf( "test print (%d) with a passed param\r\n", iTestParam );
}

void test_cOSAL_GetChar_before_StartOS_success( void** state )
{
    ( void ) state; /* unused */

    will_return( __wrap_inbyte, 1 );
    assert_int_equal( cOSAL_GetChar(), 1 );
}

void test_iOSAL_StartOS_failure( void** state )
{
    ( void ) state; /* unused */

    int      i                    = 0;
    void*    pvTaskHandle         = NULL;
    void**   ppvTaskHandle        = NULL;
    void*    pvStartOSTaskFunc    = NULL;
    uint16_t usStartTaskStackSize = 0x1000;

    /*******************************/
    /* iOSAL_StartOS Failure tests */
    /*******************************/

    /* param 1 error */
    assert_int_equal( iOSAL_StartOS( 3,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     0x1000, 
                                     1 ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_StartOS( TRUE,
                                     ppvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     0x1000, 
                                     1 ), OSAL_ERRORS_PARAMS );
    
    /* param 3 error */
    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     pvStartOSTaskFunc, 
                                     0x1000, 
                                     1 ), OSAL_ERRORS_PARAMS );

    /* param 4 error */
    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc,
                                     0, 
                                     1 ), OSAL_ERRORS_PARAMS );

    /* task create returns NULL */
    will_return( __wrap_xTaskCreateStatic, NULL );

    expect_value( __wrap_xTaskCreateStatic, pxTaskCode, &vStartOSTaskFunc );
    expect_string( __wrap_xTaskCreateStatic, pcName, "OSAL Main Task" );
    expect_value( __wrap_xTaskCreateStatic, usStackDepth, ( usStartTaskStackSize / sizeof( StackType_t ) ) );
    expect_value( __wrap_xTaskCreateStatic, pvParameters, NULL );
    expect_value( __wrap_xTaskCreateStatic, uxPriority, DEFAULT_TASK_PRIORITY );

    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     1 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invalid OS return value 2 */
    will_return( __wrap_xTaskCreateStatic, pdPASS );
    will_return( __wrap_xQueueCreateMutex, NULL );
    will_return( __wrap_xQueueCreateMutex, NULL );
    will_return( __wrap_xQueueCreateMutex, NULL );
    will_return( __wrap_xQueueCreateMutex, NULL );

    expect_value( __wrap_xTaskCreateStatic, pxTaskCode, &vStartOSTaskFunc );
    expect_string( __wrap_xTaskCreateStatic, pcName, "OSAL Main Task" );
    expect_value( __wrap_xTaskCreateStatic, usStackDepth, ( usStartTaskStackSize / sizeof( StackType_t ) ) );
    expect_value( __wrap_xTaskCreateStatic, pvParameters, NULL );
    expect_value( __wrap_xTaskCreateStatic, uxPriority, DEFAULT_TASK_PRIORITY );

    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     1 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* memory allocation fail */
    for( i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryUsed[ i ] = 1;
    }

    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     1 ), OSAL_ERRORS_OS_IMPLEMENTATION );

    for( i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryUsed[ i ] = MEM_UNUSED;
    }
}

void test_iOSAL_StartOS_success( void** state )
{
    ( void ) state; /* unused */

    void* pvTaskHandle = NULL;
    uint16_t usStartTaskStackSize = 0x1000;
    uint32_t ulStartTaskPriority = 1;

    /*******************************/
    /* iOSAL_StartOS Success tests */
    /*******************************/

    /* happy path - param 1 TRUE */
    will_return( __wrap_xTaskCreateStatic, pdPASS );
    will_return( __wrap_xQueueCreateMutex, pvPrintfMutexMockAddr );
    will_return( __wrap_xQueueCreateMutex, pvMemSetMutexMockAddr );
    will_return( __wrap_xQueueCreateMutex, pvMemCpyMutexMockAddr );
    will_return( __wrap_xQueueCreateMutex, pvMemCpyMutexMockAddr );
    expect_function_call( __wrap_vTaskStartScheduler );

    /* Check mock function parameters */
    expect_value( __wrap_xTaskCreateStatic, pxTaskCode, &vStartOSTaskFunc );
    expect_string( __wrap_xTaskCreateStatic, pcName, "OSAL Main Task" );
    expect_value( __wrap_xTaskCreateStatic, usStackDepth, ( usStartTaskStackSize / sizeof( StackType_t ) ) );
    expect_value( __wrap_xTaskCreateStatic, pvParameters, NULL );
    expect_value( __wrap_xTaskCreateStatic, uxPriority, DEFAULT_TASK_PRIORITY );

    assert_int_equal( iOSAL_StartOS( TRUE,
                                     &pvTaskHandle,
                                     &vStartOSTaskFunc, 
                                     usStartTaskStackSize, 
                                     ulStartTaskPriority ), OSAL_ERRORS_INSUFFICIENT_MEM );

    assert_int_not_equal( pvTaskHandle, NULL );
}

void test_ulOSAL_GetUptimeTicks_failure( void** state )
{
    ( void ) state; /* unused */

    /***************************************/
    /* ulOSAL_GetUptimeTicks Failure tests */
    /***************************************/

    /* invalid OS return value */
    will_return( __wrap_xTaskGetTickCount, NULL );

    assert_int_equal( ulOSAL_GetUptimeTicks( ), NULL );
}

void test_ulOSAL_GetUptimeTicks_success( void** state )
{
    ( void ) state; /* unused */

    /***************************************/
    /* ulOSAL_GetUptimeTicks Success tests */
    /***************************************/

    /* happy path */
    will_return( __wrap_xTaskGetTickCount, 500 );

    assert_int_equal( ulOSAL_GetUptimeTicks( ), 500 );
}

void test_ulOSAL_GetUptimeMs_failure( void** state )
{
    ( void ) state; /* unused */

    /************************************/
    /* ulOSAL_GetUptimeMs Failure tests */
    /************************************/

    /* invalid OS return value */
    will_return( __wrap_xTaskGetTickCount, NULL );

    assert_int_equal( ulOSAL_GetUptimeMs( ), NULL );
}

void test_ulOSAL_GetUptimeMs_success( void** state )
{
    ( void ) state; /* unused */

    /************************************/
    /* ulOSAL_GetUptimeMs Success tests */
    /************************************/

    /* happy path */
    will_return( __wrap_xTaskGetTickCount, 500 );
    /* check conversion value */
    assert_int_equal( ulOSAL_GetUptimeMs( ), 500 / pdMS_TO_TICKS( 1 ) );
}

/*****************************************************************************/
/* Task APIs                                                                 */
/*****************************************************************************/

void test_iOSAL_Task_Create_failure( void** state )
{
    ( void ) state; /* unused */

    int      i                    =  0;
    void*    pvTaskHandle         =  NULL;
    void**   ppvTaskHandle        =  NULL;
    uint16_t usStartTaskStackSize =  0x1000;
    void*    pvTaskParam          =  NULL;

    /***********************************/
    /* iOSAL_Task_Create Failure tests */
    /***********************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Task_Create( ppvTaskHandle,
                                         &vTaskFunc, 
                                         0x1000, 
                                         NULL,
                                         1, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );
    
    /* param 2 error */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         NULL, 
                                         0x1000, 
                                         NULL,
                                         1, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );

    /* param 3 error (value 0) */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         0, 
                                         NULL,
                                         1, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );

    /* param 3 error (not 32 bit aligned) */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         0x11, 
                                         NULL,
                                         1, 
                                         "Test Task" ), OSAL_ERRORS_PARAMS );

    /* param 6 error */
    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         0x1000, 
                                         NULL,
                                         1, 
                                         NULL ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xTaskCreateStatic, NULL );

    expect_value( __wrap_xTaskCreateStatic, pxTaskCode, &vTaskFunc );
    expect_string( __wrap_xTaskCreateStatic, pcName, "Test Task" );
    expect_value( __wrap_xTaskCreateStatic, usStackDepth, ( usStartTaskStackSize / sizeof( StackType_t ) ) );
    expect_value( __wrap_xTaskCreateStatic, pvParameters, pvTaskParam );
    expect_value( __wrap_xTaskCreateStatic, uxPriority, DEFAULT_TASK_PRIORITY );

    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         pvTaskParam,
                                         1, 
                                         "Test Task" ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* memory allocation fail */
    for( i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryUsed[ i ] = 1;
    }

    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         pvTaskParam,
                                         1, 
                                         "Test Task" ), OSAL_ERRORS_OS_IMPLEMENTATION );

    for( i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryUsed[ i ] = MEM_UNUSED;
    }
}

void test_iOSAL_Task_Create_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = NULL;
    void*  pvTaskHandle2 = NULL;
    void*  pvTaskHandle3 = NULL;
    uint16_t usStartTaskStackSize = 0x1000;
    void*  pvTaskParam = NULL;
    uint8_t pucTaskParamArray[ 5 ] = { 0 };

    /***********************************/
    /* iOSAL_Task_Create Success tests */
    /***********************************/

    /* happy path 1 */
    will_return( __wrap_xTaskCreateStatic, pdPASS );

    expect_value( __wrap_xTaskCreateStatic, pxTaskCode, &vTaskFunc );
    expect_string( __wrap_xTaskCreateStatic, pcName, "Test Task" );
    expect_value( __wrap_xTaskCreateStatic, usStackDepth, ( usStartTaskStackSize / sizeof( StackType_t ) ) );
    expect_value( __wrap_xTaskCreateStatic, pvParameters, pvTaskParam );
    expect_value( __wrap_xTaskCreateStatic, uxPriority, DEFAULT_TASK_PRIORITY );

    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         pvTaskParam,
                                         1, 
                                         "Test Task" ), OSAL_ERRORS_NONE );

    assert_int_not_equal( pvTaskHandle, NULL );

    /* happy path 2 - smaller stack size */
    usStartTaskStackSize = 0x20;
    will_return( __wrap_xTaskCreateStatic, pdPASS );

    expect_value( __wrap_xTaskCreateStatic, pxTaskCode, &vTaskFunc );
    expect_string( __wrap_xTaskCreateStatic, pcName, "Test Task 2" );
    expect_value( __wrap_xTaskCreateStatic, usStackDepth, ( usStartTaskStackSize / sizeof( StackType_t ) ) );
    expect_value( __wrap_xTaskCreateStatic, pvParameters, pvTaskParam );
    expect_value( __wrap_xTaskCreateStatic, uxPriority, DEFAULT_TASK_PRIORITY );

    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle2,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         pvTaskParam,
                                         1, 
                                         "Test Task 2" ), OSAL_ERRORS_NONE );

    assert_int_not_equal( pvTaskHandle2, NULL );

    /* happy path 3 - task param set */
    will_return( __wrap_xTaskCreateStatic, pdPASS );

    expect_value( __wrap_xTaskCreateStatic, pxTaskCode, &vTaskFunc );
    expect_string( __wrap_xTaskCreateStatic, pcName, "Test Task 3" );
    expect_value( __wrap_xTaskCreateStatic, usStackDepth, ( usStartTaskStackSize / sizeof( StackType_t ) ) );
    expect_value( __wrap_xTaskCreateStatic, pvParameters, ( void* )pucTaskParamArray );
    expect_value( __wrap_xTaskCreateStatic, uxPriority, DEFAULT_TASK_PRIORITY );

    assert_int_equal( iOSAL_Task_Create( &pvTaskHandle3,
                                         &vTaskFunc, 
                                         usStartTaskStackSize, 
                                         ( void* )pucTaskParamArray,
                                         1, 
                                         "Test Task 3" ), OSAL_ERRORS_NONE );

    assert_int_not_equal( pvTaskHandle3, NULL );
}

void test_iOSAL_Task_Delete_failure( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle2 = NULL;
    void** ppvTaskHandle = NULL;

    /***********************************/
    /* iOSAL_Task_Delete Failure tests */
    /***********************************/

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Task_Delete( ppvTaskHandle ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_Semaphore_Destroy( &pvTaskHandle2 ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Task_Delete_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = ( void* )0x10;
    void*  pvTaskHandle2 = ( void* )0x20;

    /***********************************/
    /* iOSAL_Task_Delete Success tests */
    /***********************************/

    /* happy path */
    pxOsStatsHandle->pxTaskHead->pvTask = pvTaskHandle;

    expect_function_call( __wrap_vTaskDelete );
    expect_value( __wrap_vTaskDelete, xTaskToDelete, pvTaskHandle );

    assert_int_equal( iOSAL_Task_Delete( &pvTaskHandle ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTaskHandle, NULL );

    /* happy path 2 */
    expect_function_call( __wrap_vTaskDelete );
    expect_value( __wrap_vTaskDelete, xTaskToDelete, pvTaskHandle2 );

    for( int i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryPool[ i ].xTaskHandle = pvTaskHandle2;
    }

    assert_int_equal( iOSAL_Task_Delete( &pvTaskHandle2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTaskHandle2, NULL );

    for( int i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryPool[ i ].xTaskHandle = 0;
    }
}

void test_iOSAL_Task_Suspend_failure( void** state )
{
    ( void ) state; /* unused */

    /************************************/
    /* iOSAL_Task_Suspend Failure tests */
    /************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Task_Suspend( NULL ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Task_Suspend_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = ( void* )0x10;
    void*  pvTaskHandle2 = ( void* )0x20;

    /************************************/
    /* iOSAL_Task_Suspend Success tests */
    /************************************/

    /* happy path */
    expect_function_call( __wrap_vTaskSuspend );
    expect_value( __wrap_vTaskSuspend, xTaskToSuspend, pvTaskHandle );
    
    assert_int_equal( iOSAL_Task_Suspend( pvTaskHandle ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    expect_function_call( __wrap_vTaskSuspend );
    expect_value( __wrap_vTaskSuspend, xTaskToSuspend, pvTaskHandle2 );

    pxOsStatsHandle->pxTaskHead->pvTask = pvTaskHandle2;

    assert_int_equal( iOSAL_Task_Suspend( pvTaskHandle2 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Task_Resume_failure( void** state )
{
    ( void ) state; /* unused */

    /************************************/
    /* iOSAL_Task_Resume Failure tests  */
    /************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Task_Resume( NULL ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Task_Resume_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = ( void* )0x10;
    void*  pvTaskHandle2 = ( void* )0x20;

    /***********************************/
    /* iOSAL_Task_Resume Success tests */
    /***********************************/

    /* happy path */
    expect_function_call( __wrap_vTaskResume );
    expect_value( __wrap_vTaskResume, xTaskToResume, pvTaskHandle );

    assert_int_equal( iOSAL_Task_Resume( pvTaskHandle ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    expect_function_call( __wrap_vTaskResume );
    expect_value( __wrap_vTaskResume, xTaskToResume, pvTaskHandle2 );

    pxOsStatsHandle->pxTaskHead->pvTask = pvTaskHandle2;

    assert_int_equal( iOSAL_Task_Resume( pvTaskHandle2 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Task_SleepTicks_failure( void** state )
{
    ( void ) state; /* unused */

    /***************************************/
    /* iOSAL_Task_SleepTicks Failure tests */
    /***************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Task_SleepTicks( 0 ), OSAL_ERRORS_PARAMS );
}

void test_iOSAL_Task_SleepTicks_success( void** state )
{
    ( void ) state; /* unused */
    uint32_t ulSleepTicks = 100;
    uint32_t ulSleepTicks2 = 500;

    /***************************************/
    /* iOSAL_Task_SleepTicks Success tests */
    /***************************************/

    /* happy path */
    expect_function_call( __wrap_vTaskDelay );
    expect_value( __wrap_vTaskDelay, xTicksToDelay, ulSleepTicks );
    
    assert_int_equal( iOSAL_Task_SleepTicks( ulSleepTicks ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    expect_function_call( __wrap_vTaskDelay );
    expect_value( __wrap_vTaskDelay, xTicksToDelay, ulSleepTicks2 );
    
    assert_int_equal( iOSAL_Task_SleepTicks( ulSleepTicks2 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Task_SleepMs_failure( void** state )
{
    ( void ) state; /* unused */

    /***************************************/
    /* iOSAL_Task_SleepTicks Failure tests */
    /***************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Task_SleepMs( 0 ), OSAL_ERRORS_PARAMS );
}

void test_iOSAL_Task_SleepMs_success( void** state )
{
    ( void ) state; /* unused */
    uint32_t ulSleepMs = 100;
    uint32_t ulSleepMs2 = 5;

    /***************************************/
    /* iOSAL_Task_SleepTicks Success tests */
    /***************************************/

    /* happy path */
    expect_function_call( __wrap_vTaskDelay );
    expect_value( __wrap_vTaskDelay, xTicksToDelay, pdMS_TO_TICKS( ulSleepMs ) );
    
    assert_int_equal( iOSAL_Task_SleepMs( ulSleepMs ), OSAL_ERRORS_NONE );

    /* happy path - small timeout value */
    expect_function_call( __wrap_vTaskDelay );
    expect_value( __wrap_vTaskDelay, xTicksToDelay, 0x32 );
    
    assert_int_equal( iOSAL_Task_SleepMs( ulSleepMs2 ), OSAL_ERRORS_NONE );
}

/*****************************************************************************/
/* Semaphore APIs                                                            */
/*****************************************************************************/

void test_iOSAL_Semaphore_Create_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvSemBinaryHandle = NULL;
    void* pvSemHandleSet = ( void* )0x10;

    /****************************************/
    /* iOSAL_Semaphore_Create Failure tests */
    /****************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_Create( NULL, 
                                              1,
                                              1,
                                              "Test Semaphore" ) , OSAL_ERRORS_PARAMS );

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemHandleSet, 
                                              1,
                                              1,
                                              "Test Semaphore" ) , OSAL_ERRORS_PARAMS );
    
    /* param 4 error */
    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle, 
                                              1,
                                              1,
                                              NULL ) , OSAL_ERRORS_PARAMS );

    /* invalid OS return value - counting semaphore */
    will_return( __wrap_xQueueCreateCountingSemaphore, NULL );
    expect_value( __wrap_xQueueCreateCountingSemaphore, uxMaxCount, 5 );
    expect_value( __wrap_xQueueCreateCountingSemaphore, uxInitialCount, 5 );

    assert_int_equal( iOSAL_Semaphore_Create( &pvSemBinaryHandle, 
                                              5,
                                              5,
                                              "Test Counting Semaphore" ) , OSAL_ERRORS_OS_IMPLEMENTATION );
}


void test_iOSAL_Semaphore_Create_success( void** state )
{
    ( void ) state; /* unused */

    void* pvSemBinaryHandle = NULL;
    void* pvSemBinaryHandle2 = NULL;
    void* pvSemCountingHandle = NULL;
    void* pvSemCountingHandle2 = NULL;

    /****************************************/
    /* iOSAL_Semaphore_Create Success tests */
    /****************************************/

    /* happy path - counting semaphore */
    will_return( __wrap_xQueueCreateCountingSemaphore, 0x10 );
    expect_value( __wrap_xQueueCreateCountingSemaphore, uxMaxCount, 5 );
    expect_value( __wrap_xQueueCreateCountingSemaphore, uxInitialCount, 5 );

    assert_int_equal( iOSAL_Semaphore_Create( &pvSemCountingHandle, 
                                              5,
                                              5,
                                              "Test Counting Semaphore" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvSemCountingHandle, NULL );

    /* happy path - counting semaphore (initial count differ to max count) */
    will_return( __wrap_xQueueCreateCountingSemaphore, 0x10 );
    expect_value( __wrap_xQueueCreateCountingSemaphore, uxMaxCount, 10 );
    expect_value( __wrap_xQueueCreateCountingSemaphore, uxInitialCount, 1 );

    assert_int_equal( iOSAL_Semaphore_Create( &pvSemCountingHandle2, 
                                              1,
                                              10,
                                              "Test Counting Semaphore" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvSemCountingHandle2, NULL );
}

void test_iOSAL_Semaphore_Destroy_failure( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle2 = NULL;

    /*****************************************/
    /* iOSAL_Semaphore_Destroy Failure tests */
    /*****************************************/

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Semaphore_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_Semaphore_Destroy( &pvSemHandle2 ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Semaphore_Destroy_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;
    void*  pvSemHandle2 = ( void* )0x200;

    /*****************************************/
    /* iOSAL_Semaphore_Destroy Success tests */
    /*****************************************/

    /* happy path */
    expect_function_call( __wrap_vQueueDelete );
    expect_value( __wrap_vQueueDelete, xQueue, pvSemHandle );

    assert_int_equal( iOSAL_Semaphore_Destroy( &pvSemHandle ), OSAL_ERRORS_NONE );
    assert_int_equal( pvSemHandle, NULL );

    /* happy path 2 */
    expect_function_call( __wrap_vQueueDelete );
    expect_value( __wrap_vQueueDelete, xQueue, pvSemHandle2 );

    assert_int_equal( iOSAL_Semaphore_Destroy( &pvSemHandle2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvSemHandle2, NULL );
}

void test_iOSAL_Semaphore_Pend_failure( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;

    /**************************************/
    /* iOSAL_Semaphore_Pend Failure tests */
    /**************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_Pend( NULL, 100 ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value */
    will_return( __wrap_xQueueSemaphoreTake, pdFAIL );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, pdMS_TO_TICKS( 100 ) );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 100 ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Semaphore_Pend_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;

    /**************************************/
    /* iOSAL_Semaphore_Pend Success tests */
    /**************************************/

    set_sem_lock(); /* set sem lock test value before happy path tests */

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, pdMS_TO_TICKS( 100 ) );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path 2 */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path 3 */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0 );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path 4 */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();
}

void test_iOSAL_Semaphore_Post_failure( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;

    /**************************************/
    /* iOSAL_Semaphore_Post Failure tests */
    /**************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_Post( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value */
    will_return( __wrap_xQueueGenericSend, pdFAIL );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    assert_int_equal( iOSAL_Semaphore_Post( pvSemHandle ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Semaphore_Post_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;
    void*  pvSemHandle2 = ( void* )0x200;

    /**************************************/
    /* iOSAL_Semaphore_Post Success tests */
    /**************************************/

    set_sem_lock(); /* set sem lock test value before happy path tests */

    /* happy path */
    /* pend semaphore */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* post semaphore */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    assert_int_equal( iOSAL_Semaphore_Post( pvSemHandle ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );

    /* happy path 2 */
    /* pend semaphore */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle2 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle2, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    
    /* post semaphore */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvSemHandle2 );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    assert_int_equal( iOSAL_Semaphore_Post( pvSemHandle2 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );
}

void test_iOSAL_Semaphore_PostFromISR_failure( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;

    /*********************************************/
    /* iOSAL_Semaphore_PostFromISR Failure tests */
    /*********************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Semaphore_PostFromISR( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value */
    will_return( __wrap_xQueueGiveFromISR, pdFAIL );
    expect_value( __wrap_xQueueGiveFromISR, xQueue, pvSemHandle );
    
    assert_int_equal( iOSAL_Semaphore_PostFromISR( pvSemHandle ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Semaphore_PostFromISR_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;
    void*  pvSemHandle2 = ( void* )0x200;

    /*********************************************/
    /* iOSAL_Semaphore_PostFromISR Success tests */
    /*********************************************/

    set_sem_lock(); /* set sem lock test value before happy path tests */

    /* happy path */
    /* pend semaphore */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* post semaphore */
    will_return( __wrap_xQueueGiveFromISR, pdPASS );
    expect_value( __wrap_xQueueGiveFromISR, xQueue, pvSemHandle );
    
    assert_int_equal( iOSAL_Semaphore_PostFromISR( pvSemHandle ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );

    /* happy path 2 */
    /* pend semaphore */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvSemHandle2 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_Semaphore_Pend( pvSemHandle2, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* post semaphore */
    will_return( __wrap_xQueueGiveFromISR, pdPASS );
    expect_value( __wrap_xQueueGiveFromISR, xQueue, pvSemHandle2 );
    
    assert_int_equal( iOSAL_Semaphore_PostFromISR( pvSemHandle2 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );
}

/*****************************************************************************/
/* Mutex APIs                                                                */
/*****************************************************************************/

void test_iOSAL_Mutex_Create_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandle = NULL;
    void* pvMutexHandleSet = ( void* )0x10;

    /************************************/
    /* iOSAL_Mutex_Create Failure tests */
    /************************************/

    /* param 1 error - pointer to handle set NULL */
    assert_int_equal( iOSAL_Mutex_Create( NULL, "test_mutex" ), OSAL_ERRORS_PARAMS );

    /* param 1 error - handle set to value */
    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandleSet, "test_mutex" ) , OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle, NULL ) , OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xQueueCreateMutex, NULL );

    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle, "test_mutex" ) , OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Mutex_Create_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandle = NULL;
    void* pvMutexHandle2 = NULL;

    /************************************/
    /* iOSAL_Mutex_Create Success tests */
    /************************************/

    /* happy path */
    will_return( __wrap_xQueueCreateMutex, 0x10 );

    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle, "test_mutex" ) , OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvMutexHandle, NULL );
    assert_int_equal( pvMutexHandle, 0x10 );

    /* happy path 2 */
    will_return( __wrap_xQueueCreateMutex, 0x200 );

    assert_int_equal( iOSAL_Mutex_Create( &pvMutexHandle2, "test_mutex" ) , OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvMutexHandle2, NULL );
    assert_int_equal( pvMutexHandle2, 0x200 );
}

void test_iOSAL_Mutex_Destroy_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandle = NULL;

    /*****************************************/
    /* iOSAL_Mutex_Destroy Failure tests     */
    /*****************************************/

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Mutex_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_Mutex_Destroy( &pvMutexHandle ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_Mutex_Destroy_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;
    void* pvMutexHandleSet2 = ( void* )0x200;

    /*****************************************/
    /* iOSAL_Mutex_Destroy Success tests     */
    /*****************************************/

    /* happy path */
    expect_function_call( __wrap_vQueueDelete );
    expect_value( __wrap_vQueueDelete, xQueue, pvMutexHandleSet );

    assert_int_equal( iOSAL_Mutex_Destroy( &pvMutexHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMutexHandleSet, NULL );

    /* happy path 2 */
    expect_function_call( __wrap_vQueueDelete );
    expect_value( __wrap_vQueueDelete, xQueue, pvMutexHandleSet2 );

    assert_int_equal( iOSAL_Mutex_Destroy( &pvMutexHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMutexHandleSet2, NULL );
}

void test_iOSAL_Mutex_Take_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;

    /**********************************/
    /* iOSAL_Mutex_Take Failure tests */
    /**********************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Mutex_Take( NULL, 100 ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value */
    will_return( __wrap_xQueueSemaphoreTake, pdFAIL );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, pdMS_TO_TICKS( 100 ) );
    
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 100 ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Mutex_Take_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;

    /**********************************/
    /* iOSAL_Mutex_Take Success tests */
    /**********************************/

    set_sem_lock(); /* set lock test value before happy path tests */

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, pdMS_TO_TICKS( 100 ) );
    
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0 );
    
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 5 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );
    set_sem_lock();
}

void test_iOSAL_Mutex_Release_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;

    /**************************************/
    /* iOSAL_Mutex_Release Failure tests  */
    /**************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Mutex_Release( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value */
    will_return( __wrap_xQueueGenericSend, pdFAIL );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    assert_int_equal( iOSAL_Mutex_Release( pvMutexHandleSet ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Mutex_Release_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMutexHandleSet = ( void* )0x10;
    void* pvMutexHandleSet2 = ( void* )0x200;

    /**************************************/
    /* iOSAL_Mutex_Release Success tests  */
    /**************************************/

    set_sem_lock(); /* set lock test value before happy path tests */

    /* happy path */
    /* take mutex */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, pdMS_TO_TICKS( 100 ) );
    
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* release mutex */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMutexHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    assert_int_equal( iOSAL_Mutex_Release( pvMutexHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );

    /* happy path 2 */
    /* take mutex */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMutexHandleSet2 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, pdMS_TO_TICKS( 100 ) );
    
    assert_int_equal( iOSAL_Mutex_Take( pvMutexHandleSet2, 100 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 1 );

    /* release mutex */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMutexHandleSet2 );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    assert_int_equal( iOSAL_Mutex_Release( pvMutexHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( get_sem_lock(), 0 );
}

/*****************************************************************************/
/* Mailbox APIs                                                              */
/*****************************************************************************/

void test_iOSAL_MBox_Create_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandle = NULL;
    void* pvMBoxHandleSet = ( void* )0x10;

    /***********************************/
    /* iOSAL_MBox_Create Failure tests */
    /***********************************/

    /* param 1 error - pointer to handle set NULL */
    assert_int_equal( iOSAL_MBox_Create( NULL, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_PARAMS );

    /* param 1 error - handle set to value */
    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandleSet, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_PARAMS );

    /* param 4 error */
    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle, 10, sizeof( uint32_t ), NULL ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xQueueGenericCreate, NULL );
    expect_value( __wrap_xQueueGenericCreate, uxQueueLength, 10 );
    expect_value( __wrap_xQueueGenericCreate, uxItemSize, sizeof( uint32_t ) );
    expect_value( __wrap_xQueueGenericCreate, ucQueueType, queueQUEUE_TYPE_BASE );

    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_MBox_Create_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandle = NULL;
    void* pvMBoxHandle2 = NULL;
    void* pvMBoxHandle3 = NULL;

    /************************************/
    /* iOSAL_Mutex_Create Success tests */
    /************************************/

    /* happy path */
    will_return( __wrap_xQueueGenericCreate, 0x10 );
    expect_value( __wrap_xQueueGenericCreate, uxQueueLength, 10 );
    expect_value( __wrap_xQueueGenericCreate, uxItemSize, sizeof( uint32_t ) );
    expect_value( __wrap_xQueueGenericCreate, ucQueueType, queueQUEUE_TYPE_BASE );

    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle, 10, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvMBoxHandle, NULL );
    assert_int_equal( pvMBoxHandle, 0x10 );

    /* happy path 2 - larger MBox length */
    will_return( __wrap_xQueueGenericCreate, 0x200 );
    expect_value( __wrap_xQueueGenericCreate, uxQueueLength, 100 );
    expect_value( __wrap_xQueueGenericCreate, uxItemSize, sizeof( uint32_t ) );
    expect_value( __wrap_xQueueGenericCreate, ucQueueType, queueQUEUE_TYPE_BASE );

    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle2, 100, sizeof( uint32_t ), "test_mbox" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvMBoxHandle2, NULL );
    assert_int_equal( pvMBoxHandle2, 0x200 );

    /* happy path 3 - smaller MBox item size */
    will_return( __wrap_xQueueGenericCreate, 0x300 );
    expect_value( __wrap_xQueueGenericCreate, uxQueueLength, 100 );
    expect_value( __wrap_xQueueGenericCreate, uxItemSize, sizeof( uint8_t ) );
    expect_value( __wrap_xQueueGenericCreate, ucQueueType, queueQUEUE_TYPE_BASE );

    assert_int_equal( iOSAL_MBox_Create( &pvMBoxHandle3, 100, sizeof( uint8_t ), "test_mbox" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvMBoxHandle3, NULL );
    assert_int_equal( pvMBoxHandle3, 0x300 );

}

void test_iOSAL_MBox_Destroy_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandle = NULL;

    /****************************************/
    /* iOSAL_MBox_Destroy Failure tests     */
    /****************************************/

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_MBox_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_MBox_Destroy( &pvMBoxHandle ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_MBox_Destroy_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandleSet = ( void* )0x10;
    void* pvMBoxHandleSet2 = ( void* )0x200;

    /****************************************/
    /* iOSAL_MBox_Destroy Success tests     */
    /****************************************/

    /* happy path */
    expect_function_call( __wrap_vQueueDelete );
    expect_value( __wrap_vQueueDelete, xQueue, pvMBoxHandleSet );

    assert_int_equal( iOSAL_MBox_Destroy( &pvMBoxHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMBoxHandleSet, NULL );

    /* happy path 2 */
    expect_function_call( __wrap_vQueueDelete );
    expect_value( __wrap_vQueueDelete, xQueue, pvMBoxHandleSet2 );

    assert_int_equal( iOSAL_MBox_Destroy( &pvMBoxHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvMBoxHandleSet2, NULL );
}

void test_iOSAL_MBox_Pend_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandleSet = ( void* )0x10;
    uint32_t ulTaskData = 0;

    /*********************************/
    /* iOSAL_MBox_Pend Failure tests */
    /*********************************/

    /* param 1 error */
    assert_int_equal( iOSAL_MBox_Pend( NULL, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, NULL, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xQueueReceive, pdFAIL );
    expect_value( __wrap_xQueueReceive, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueReceive, pvBuffer, ( void* )&ulTaskData );
    expect_value( __wrap_xQueueReceive, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_MBox_Pend_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandleSet = ( void* )0x10;
    uint32_t ulTaskData = 0;
    uint32_t ulTaskData2 = 100;

    /*********************************/
    /* iOSAL_MBox_Pend Success tests */
    /*********************************/

    /* happy path - all timeout value variations */

    /* happy path 1 */
    will_return( __wrap_xQueueReceive, pdPASS );
    expect_value( __wrap_xQueueReceive, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueReceive, pvBuffer, ( void* )&ulTaskData );
    expect_value( __wrap_xQueueReceive, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_xQueueReceive, pdPASS );
    expect_value( __wrap_xQueueReceive, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueReceive, pvBuffer, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueReceive, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, ( void* )&ulTaskData2, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 3 */
    will_return( __wrap_xQueueReceive, pdPASS );
    expect_value( __wrap_xQueueReceive, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueReceive, pvBuffer, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueReceive, xTicksToWait, 0 );
    
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, ( void* )&ulTaskData2, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );

    /* happy path 4 */
    will_return( __wrap_xQueueReceive, pdPASS );
    expect_value( __wrap_xQueueReceive, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueReceive, pvBuffer, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueReceive, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, ( void* )&ulTaskData2, 5 ), OSAL_ERRORS_NONE );

    /* happy path 5 */
    will_return( __wrap_xQueueReceive, pdPASS );
    expect_value( __wrap_xQueueReceive, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueReceive, pvBuffer, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueReceive, xTicksToWait, pdMS_TO_TICKS( 20 ) );
    
    assert_int_equal( iOSAL_MBox_Pend( pvMBoxHandleSet, ( void* )&ulTaskData2, 20 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_MBox_Post_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandleSet = ( void* )0x10;
    uint32_t ulTaskData = 0;

    /*********************************/
    /* iOSAL_MBox_Post Failure tests */
    /*********************************/

    /* param 1 error */
    assert_int_equal( iOSAL_MBox_Post( NULL, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, NULL, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xQueueGenericSend, pdFAIL );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, ( void* )&ulTaskData );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_OS_IMPLEMENTATION );
}


void test_iOSAL_MBox_Post_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandleSet = ( void* )0x10;
    uint32_t ulTaskData = 0;
    uint32_t ulTaskData2 = 100;

    /*********************************/
    /* iOSAL_MBox_Post Success tests */
    /*********************************/

    /* happy path - all timeout value variations */

    /* happy path 1 */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, ( void* )&ulTaskData );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData2, OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 3 */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, 0 );
    
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData2, OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );

    /* happy path 4 */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData2, 5 ), OSAL_ERRORS_NONE );

    /* happy path 5 */
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, ( void* )&ulTaskData2 );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, pdMS_TO_TICKS( 20 ) );
    
    assert_int_equal( iOSAL_MBox_Post( pvMBoxHandleSet, ( void* )&ulTaskData2, 20 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_MBox_PostFromISR_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandleSet = ( void* )0x10;
    uint32_t ulTaskData = 0;

    /****************************************/
    /* iOSAL_MBox_PostFromISR Failure tests */
    /****************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_MBox_PostFromISR( NULL, ( void* )&ulTaskData ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_MBox_PostFromISR( pvMBoxHandleSet, NULL ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xQueueGenericSendFromISR, pdFAIL );
    expect_value( __wrap_xQueueGenericSendFromISR, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSendFromISR, pvItemToQueue, ( void* )&ulTaskData );
    
    assert_int_equal( iOSAL_MBox_PostFromISR( pvMBoxHandleSet, ( void* )&ulTaskData ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_MBox_PostFromISR_success( void** state )
{
    ( void ) state; /* unused */

    void* pvMBoxHandleSet = ( void* )0x10;
    void* pvMBoxHandleSet2 = ( void* )0x200;
    uint32_t ulTaskData = 0;
    uint32_t ulTaskData2 = 100;

    /****************************************/
    /* iOSAL_MBox_PostFromISR Success tests */
    /****************************************/

    /* happy path 1 */
    will_return( __wrap_xQueueGenericSendFromISR, pdPASS );
    expect_value( __wrap_xQueueGenericSendFromISR, xQueue, pvMBoxHandleSet );
    expect_value( __wrap_xQueueGenericSendFromISR, pvItemToQueue, ( void* )&ulTaskData );
    
    assert_int_equal( iOSAL_MBox_PostFromISR( pvMBoxHandleSet, ( void* )&ulTaskData ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_xQueueGenericSendFromISR, pdPASS );
    expect_value( __wrap_xQueueGenericSendFromISR, xQueue, pvMBoxHandleSet2 );
    expect_value( __wrap_xQueueGenericSendFromISR, pvItemToQueue, ( void* )&ulTaskData2 );
    
    assert_int_equal( iOSAL_MBox_PostFromISR( pvMBoxHandleSet2, ( void* )&ulTaskData2 ), OSAL_ERRORS_NONE );
}

/*****************************************************************************/
/* Event APIs                                                                */
/*****************************************************************************/

void test_iOSAL_EventFlag_Create_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandle = NULL;
    void* pvEventFlagHandleSet = ( void* )0x10;

    /****************************************/
    /* iOSAL_EventFlag_Create Failure tests */
    /****************************************/

    /* param 1 error - pointer to handle set NULL */
    assert_int_equal( iOSAL_EventFlag_Create( NULL, "test_event" ), OSAL_ERRORS_PARAMS );

    /* param 1 error - handle set to value */
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandleSet, "test_event" ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle, NULL ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xEventGroupCreate, NULL );

    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle, "test_event" ), OSAL_ERRORS_OS_IMPLEMENTATION );
}


void test_iOSAL_EventFlag_Create_success( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandle = NULL;
    void* pvEventFlagHandle2 = NULL;

    /****************************************/
    /* iOSAL_EventFlag_Create Success tests */
    /****************************************/

    /* happy path 1 */
    will_return( __wrap_xEventGroupCreate, 0x10 );

    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle, "test_event" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvEventFlagHandle, NULL );
    assert_int_equal( pvEventFlagHandle, 0x10 );

    /* happy path 2 */
    will_return( __wrap_xEventGroupCreate, 0x200 );

    assert_int_equal( iOSAL_EventFlag_Create( &pvEventFlagHandle2, "test_event_2" ), OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvEventFlagHandle2, NULL );
    assert_int_equal( pvEventFlagHandle2, 0x200 );
}

void test_iOSAL_EventFlag_Destroy_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandle = NULL;

    /*****************************************/
    /* iOSAL_EventFlag_Destroy Failure tests */
    /*****************************************/

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_EventFlag_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandle ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_EventFlag_Destroy_success( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandleSet = ( void* )0x10;
    void* pvEventFlagHandleSet2 = ( void* )0x200;

    /*****************************************/
    /* iOSAL_EventFlag_Destroy Success tests */
    /*****************************************/

    /* happy path 1 */
    expect_function_call( __wrap_vEventGroupDelete );
    expect_value( __wrap_vEventGroupDelete, xEventGroup, pvEventFlagHandleSet );

    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvEventFlagHandleSet, NULL );

    /* happy path 2 */
    expect_function_call( __wrap_vEventGroupDelete );
    expect_value( __wrap_vEventGroupDelete, xEventGroup, pvEventFlagHandleSet2 );

    assert_int_equal( iOSAL_EventFlag_Destroy( &pvEventFlagHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvEventFlagHandleSet2, NULL );
}

void test_iOSAL_EventFlag_Pend_failure( void** state )
{
    ( void ) state; /* unused */

    /**************************************/
    /* iOSAL_EventFlag_Pend Failure tests */
    /**************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_EventFlag_Pend( NULL, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_EventFlag_Pend_success( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandleSet = ( void* )0x10;

    /**************************************/
    /* iOSAL_EventFlag_Pend Success tests */
    /**************************************/

    /* happy path - all timeout value variations */

    /* happy path 1 */
    expect_value( __wrap_xEventGroupWaitBits, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupWaitBits, uxBitsToWaitFor, ( EVENT_BIT_0 | EVENT_BIT_1 ) );
    expect_value( __wrap_xEventGroupWaitBits, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    expect_value( __wrap_xEventGroupWaitBits, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupWaitBits, uxBitsToWaitFor, ( EVENT_BIT_2 | EVENT_BIT_3 ) );
    expect_value( __wrap_xEventGroupWaitBits, xTicksToWait, portMAX_DELAY );
    
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            OSAL_TIMEOUT_WAIT_FOREVER ), OSAL_ERRORS_NONE );

    /* happy path 3 */
    expect_value( __wrap_xEventGroupWaitBits, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupWaitBits, uxBitsToWaitFor, ( EVENT_BIT_2 | EVENT_BIT_3 ) );
    expect_value( __wrap_xEventGroupWaitBits, xTicksToWait, 0 );
    
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            OSAL_TIMEOUT_NO_WAIT ), OSAL_ERRORS_NONE );

    /* happy path 4 */
    expect_value( __wrap_xEventGroupWaitBits, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupWaitBits, uxBitsToWaitFor, ( EVENT_BIT_2 | EVENT_BIT_3 ) );
    expect_value( __wrap_xEventGroupWaitBits, xTicksToWait, 0x32 );
    
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            5 ), OSAL_ERRORS_NONE );

    /* happy path 5 */
    expect_value( __wrap_xEventGroupWaitBits, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupWaitBits, uxBitsToWaitFor, ( EVENT_BIT_2 | EVENT_BIT_3 ) );
    expect_value( __wrap_xEventGroupWaitBits, xTicksToWait, pdMS_TO_TICKS( 20 ) );
    
    assert_int_equal( iOSAL_EventFlag_Pend( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ), 
                                            20 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_EventFlag_Post_failure( void** state )
{
    ( void ) state; /* unused */

    /**************************************/
    /* iOSAL_EventFlag_Post Failure tests */
    /**************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_EventFlag_Post( NULL, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_INVALID_HANDLE );
}

void test_iOSAL_EventFlag_Post_success( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandleSet = ( void* )0x10;
    void* pvEventFlagHandleSet2 = ( void* )0x200;

    /**************************************/
    /* iOSAL_EventFlag_Post Success tests */
    /**************************************/

    /* happy path 1 */
    expect_value( __wrap_xEventGroupSetBits, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupSetBits, uxBitsToSet, ( EVENT_BIT_0 | EVENT_BIT_1 ) );
    
    assert_int_equal( iOSAL_EventFlag_Post( pvEventFlagHandleSet, 
                                            ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    expect_value( __wrap_xEventGroupSetBits, xEventGroup, pvEventFlagHandleSet2 );
    expect_value( __wrap_xEventGroupSetBits, uxBitsToSet, ( EVENT_BIT_2 | EVENT_BIT_3 ) );
    
    assert_int_equal( iOSAL_EventFlag_Post( pvEventFlagHandleSet2, 
                                            ( EVENT_BIT_2 | EVENT_BIT_3 ) ), OSAL_ERRORS_NONE );
}

void test_iOSAL_EventFlag_PostFromISR_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandleSet = ( void* )0x10;

    /*********************************************/
    /* iOSAL_EventFlag_PostFromISR Failure tests */
    /*********************************************/

    /* param 1 error */
    assert_int_equal( iOSAL_EventFlag_PostFromISR( NULL, 
                                                   ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value */
    will_return( __wrap_xEventGroupSetBitsFromISR, pdFAIL );
    expect_value( __wrap_xEventGroupSetBitsFromISR, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupSetBitsFromISR, uxBitsToSet, ( EVENT_BIT_0 | EVENT_BIT_1 ) );
    
    assert_int_equal( iOSAL_EventFlag_PostFromISR( pvEventFlagHandleSet, 
                                                   ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_EventFlag_PostFromISR_success( void** state )
{
    ( void ) state; /* unused */

    void* pvEventFlagHandleSet = ( void* )0x10;
    void* pvEventFlagHandleSet2 = ( void* )0x200;

    /*********************************************/
    /* iOSAL_EventFlag_PostFromISR Success tests */
    /*********************************************/

    /* happy path 1 */
    will_return( __wrap_xEventGroupSetBitsFromISR, pdPASS );
    expect_value( __wrap_xEventGroupSetBitsFromISR, xEventGroup, pvEventFlagHandleSet );
    expect_value( __wrap_xEventGroupSetBitsFromISR, uxBitsToSet, ( EVENT_BIT_0 | EVENT_BIT_1 ) );
    
    assert_int_equal( iOSAL_EventFlag_PostFromISR( pvEventFlagHandleSet, 
                                                   ( EVENT_BIT_0 | EVENT_BIT_1 ) ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_xEventGroupSetBitsFromISR, pdPASS );
    expect_value( __wrap_xEventGroupSetBitsFromISR, xEventGroup, pvEventFlagHandleSet2 );
    expect_value( __wrap_xEventGroupSetBitsFromISR, uxBitsToSet, ( EVENT_BIT_2 | EVENT_BIT_3 ) );
    
    assert_int_equal( iOSAL_EventFlag_PostFromISR( pvEventFlagHandleSet2, 
                                                   ( EVENT_BIT_2 | EVENT_BIT_3 ) ), OSAL_ERRORS_NONE );
}

/*****************************************************************************/
/* Timer APIs                                                                */
/*****************************************************************************/

void test_iOSAL_Timer_Create_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    void* pvTimerHandle2 = NULL;
    void* pvTimerHandleSet = ( void* )0x10;

    /****************************************/
    /* iOSAL_Timer_Create Failure tests     */
    /****************************************/

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

    /* invalid OS return value */
    will_return( __wrap_xTimerCreate, NULL );
    expect_value( __wrap_xTimerCreate, pcTimerName, "test_timer" );
    expect_value( __wrap_xTimerCreate, xAutoReload, pdFALSE ); /* one shot */
    expect_value( __wrap_xTimerCreate, pxCallbackFunction, vTimerCallback );

    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Timer_Create_success( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    void* pvTimerHandle2 = NULL;

    /****************************************/
    /* iOSAL_Timer_Create Success tests     */
    /****************************************/

    /* happy path 1 */
    will_return( __wrap_xTimerCreate, 0x10 );
    expect_value( __wrap_xTimerCreate, pcTimerName, "test_timer" );
    expect_value( __wrap_xTimerCreate, xAutoReload, pdFALSE ); /* one shot */
    expect_value( __wrap_xTimerCreate, pxCallbackFunction, vTimerCallback );

    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle,
                                          OSAL_TIMER_CONFIG_ONE_SHOT, 
                                          vTimerCallback, 
                                          "test_timer" ) , OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvTimerHandle, NULL );
    assert_int_equal( pvTimerHandle, 0x10 );

    /* happy path 2 */
    will_return( __wrap_xTimerCreate, 0x200 );
    expect_value( __wrap_xTimerCreate, pcTimerName, "test_timer_2" );
    expect_value( __wrap_xTimerCreate, xAutoReload, pdTRUE ); /* periodic */
    expect_value( __wrap_xTimerCreate, pxCallbackFunction, vTimerCallback );

    assert_int_equal( iOSAL_Timer_Create( &pvTimerHandle2,
                                          OSAL_TIMER_CONFIG_PERIODIC, 
                                          vTimerCallback, 
                                          "test_timer_2" ) , OSAL_ERRORS_NONE );
    
    assert_int_not_equal( pvTimerHandle2, NULL );
    assert_int_equal( pvTimerHandle2, 0x200 );
}

void test_iOSAL_Timer_Destroy_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    void* pvTimerHandleSet = ( void* )0x10;

    /*****************************************/
    /* iOSAL_Timer_Destroy Failure tests     */
    /*****************************************/

    /* param 1 error - pointer to handle is NULL */
    assert_int_equal( iOSAL_Timer_Destroy( NULL ), OSAL_ERRORS_INVALID_HANDLE );

    /* param 1 error - handle is NULL */
    assert_int_equal( iOSAL_Timer_Destroy( &pvTimerHandle ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value */
    will_return( __wrap_xTimerGenericCommand, pdFAIL );
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_DELETE );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, NULL );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Destroy( &pvTimerHandleSet ), OSAL_ERRORS_OS_IMPLEMENTATION );
}


void test_iOSAL_Timer_Destroy_success( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandleSet = ( void* )0x10;
    void* pvTimerHandleSet2 = ( void* )0x200;

    /*****************************************/
    /* iOSAL_Timer_Destroy Success tests     */
    /*****************************************/

    /* happy path 1 */
    will_return( __wrap_xTimerGenericCommand, pdPASS );
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_DELETE );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, NULL );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Destroy( &pvTimerHandleSet ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTimerHandleSet, NULL );

    /* happy path 2 */
    will_return( __wrap_xTimerGenericCommand, pdPASS );
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet2 );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_DELETE );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, NULL );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Destroy( &pvTimerHandleSet2 ), OSAL_ERRORS_NONE );
    assert_int_equal( pvTimerHandleSet2, NULL );
}

void test_iOSAL_Timer_Start_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    void* pvTimerHandleSet = ( void* )0x10;
    uint32_t ulTimerDurationMs = 1000;
    TickType_t xTaskGetTickCountMockReturn = 100;

    /***********************************/
    /* iOSAL_Timer_Start Failure tests */
    /***********************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Timer_Start( pvTimerHandle, 1000 ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet, 0 ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value (xTimerChangePeriod) */
    will_return( __wrap_xTimerGenericCommand, pdFAIL ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, pdMS_TO_TICKS( ulTimerDurationMs ) );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invalid OS return value (xTimerStart) */
    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, pdMS_TO_TICKS( ulTimerDurationMs ) );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTimerGenericCommand, pdFAIL ); /* xTimerStart */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_START );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, xTaskGetTickCountMockReturn );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTaskGetTickCount, xTaskGetTickCountMockReturn ); /* xTaskGetTickCount */

    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Timer_Start_success( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandleSet = ( void* )0x10;
    void* pvTimerHandleSet2 = ( void* )0x200;
    uint32_t ulTimerDurationMs = 1000;
    uint32_t ulTimerDurationMs2 = 5;
    TickType_t xTaskGetTickCountMockReturn = 100;
    TickType_t xTaskGetTickCountMockReturn2 = 500;

    /***********************************/
    /* iOSAL_Timer_Start Success tests */
    /***********************************/

    /* happy path 1 */
    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, pdMS_TO_TICKS( ulTimerDurationMs ) );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerStart */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_START );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, xTaskGetTickCountMockReturn );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTaskGetTickCount, xTaskGetTickCountMockReturn ); /* xTaskGetTickCount */

    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_NONE );

    /* happy path 2 - duration < min timeout */
    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet2 );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, 0x32 );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerStart */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet2 );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_START );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, xTaskGetTickCountMockReturn2 );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTaskGetTickCount, xTaskGetTickCountMockReturn2 ); /* xTaskGetTickCount */

    assert_int_equal( iOSAL_Timer_Start( pvTimerHandleSet2, ulTimerDurationMs2 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Timer_Stop_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    void* pvTimerHandleSet = ( void* )0x10;

    /**********************************/
    /* iOSAL_Timer_Stop Failure tests */
    /**********************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandle ), OSAL_ERRORS_INVALID_HANDLE );

    /* invalid OS return value (xTimerStop) */
    will_return( __wrap_xTimerGenericCommand, pdFAIL );
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_STOP );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, NULL );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandleSet ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Timer_Stop_success( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandleSet = ( void* )0x10;
    void* pvTimerHandleSet2 = ( void* )0x200;

    /**********************************/
    /* iOSAL_Timer_Stop Success tests */
    /**********************************/

    /* happy path 1 */
    will_return( __wrap_xTimerGenericCommand, pdPASS );
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_STOP );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, NULL );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandleSet ), OSAL_ERRORS_NONE );

    /* happy path 2 */
    will_return( __wrap_xTimerGenericCommand, pdPASS );
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet2 );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_STOP );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, NULL );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Stop( pvTimerHandleSet2 ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Timer_Reset_failure( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandle = NULL;
    void* pvTimerHandleSet = ( void* )0x10;
    uint32_t ulTimerDurationMs = 1000;
    TickType_t xTaskGetTickCountMockReturn = 100;

    /***********************************/
    /* iOSAL_Timer_Reset Failure tests */
    /***********************************/

    /* param 1 error */
    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandle, 1000 ), OSAL_ERRORS_PARAMS );

    /* param 2 error */
    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet, 0 ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value (xTimerChangePeriod) */
    will_return( __wrap_xTimerGenericCommand, pdFAIL ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, pdMS_TO_TICKS( ulTimerDurationMs ) );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_OS_IMPLEMENTATION );

    /* invalid OS return value (xTimerReset) */
    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, pdMS_TO_TICKS( ulTimerDurationMs ) );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTimerGenericCommand, pdFAIL ); /* xTimerReset */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_RESET );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, xTaskGetTickCountMockReturn );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTaskGetTickCount, xTaskGetTickCountMockReturn ); /* xTaskGetTickCount */

    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Timer_Reset_success( void** state )
{
    ( void ) state; /* unused */

    void* pvTimerHandleSet = ( void* )0x10;
    void* pvTimerHandleSet2 = ( void* )0x200;
    uint32_t ulTimerDurationMs = 1000;
    uint32_t ulTimerDurationMs2 = 5;
    TickType_t xTaskGetTickCountMockReturn = 100;
    TickType_t xTaskGetTickCountMockReturn2 = 500;

    /***********************************/
    /* iOSAL_Timer_Reset Success tests */
    /***********************************/

    /* happy path 2 */
    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, pdMS_TO_TICKS( ulTimerDurationMs ) );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerReset */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_RESET );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, xTaskGetTickCountMockReturn );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTaskGetTickCount, xTaskGetTickCountMockReturn ); /* xTaskGetTickCount */

    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet, ulTimerDurationMs ), OSAL_ERRORS_NONE );

    /* happy path 2 - duration < min timeout */
    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerChangePeriod */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet2 );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_CHANGE_PERIOD );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, 0x32 );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTimerGenericCommand, pdPASS ); /* xTimerReset */
    expect_value( __wrap_xTimerGenericCommand, xTimer, pvTimerHandleSet2 );
    expect_value( __wrap_xTimerGenericCommand, xCommandID, tmrCOMMAND_RESET );
    expect_value( __wrap_xTimerGenericCommand, xOptionalValue, xTaskGetTickCountMockReturn2 );
    expect_value( __wrap_xTimerGenericCommand, pxHigherPriorityTaskWoken, NULL );
    expect_value( __wrap_xTimerGenericCommand, xTicksToWait, pdMS_TO_TICKS( DEFAULT_TIMER_BLOCK_TIME_MS ) );

    will_return( __wrap_xTaskGetTickCount, xTaskGetTickCountMockReturn2 ); /* xTaskGetTickCount */

    assert_int_equal( iOSAL_Timer_Reset( pvTimerHandleSet2, ulTimerDurationMs2 ), OSAL_ERRORS_NONE );
}

/*****************************************************************************/
/* Interrupt APIs                                                            */
/*****************************************************************************/

void test_iOSAL_Interrupt_Setup_failure( void** state )
{
    ( void ) state; /* unused */

    /***************************************/
    /* iOSAL_Interrupt_Setup Failure tests */
    /***************************************/

    /* param 2 error */
    assert_int_equal( iOSAL_Interrupt_Setup( FABRIC_INTERRUPT_HANDLE,
                                             NULL, 
                                             NULL ), OSAL_ERRORS_PARAMS );

    /* invalid OS return value */
    will_return( __wrap_xPortInstallInterruptHandler, pdFAIL );

    assert_int_equal( iOSAL_Interrupt_Setup( FABRIC_INTERRUPT_HANDLE,
                                             vInterruptHandler, 
                                             NULL ), OSAL_ERRORS_OS_IMPLEMENTATION );
}

void test_iOSAL_Interrupt_Setup_success( void** state )
{
    ( void ) state; /* unused */

    /***************************************/
    /* iOSAL_Interrupt_Setup Success tests */
    /***************************************/

    /* happy path */
    will_return( __wrap_xPortInstallInterruptHandler, pdPASS );

    assert_int_equal( iOSAL_Interrupt_Setup( FABRIC_INTERRUPT_HANDLE,
                                             vInterruptHandler, 
                                             NULL ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Interrupt_Enable_success( void** state )
{
    ( void ) state; /* unused */

    /****************************************/
    /* iOSAL_Interrupt_Enable Success tests */
    /****************************************/

    /* happy path */
    expect_function_call( __wrap_vPortEnableInterrupt );

    assert_int_equal( iOSAL_Interrupt_Enable( FABRIC_INTERRUPT_HANDLE ), OSAL_ERRORS_NONE );
}

void test_iOSAL_Interrupt_Disable_success( void** state )
{
    ( void ) state; /* unused */

    /*****************************************/
    /* iOSAL_Interrupt_Disable Success tests */
    /*****************************************/

    /* happy path */
    expect_function_call( __wrap_vPortDisableInterrupt );

    assert_int_equal( iOSAL_Interrupt_Disable( FABRIC_INTERRUPT_HANDLE ), OSAL_ERRORS_NONE );
}

/*****************************************************************************/
/* Thread safe APIs                                                          */
/*****************************************************************************/

void test_vOSAL_EnterCritical_success( void** state )
{
    ( void ) state; /* unused */

    /*************************************/
    /* vOSAL_EnterCritical Success tests */
    /*************************************/

    /* happy path */
    expect_function_call( __wrap_vPortEnterCritical );
    vOSAL_EnterCritical();
}

void test_vOSAL_ExitCritical_success( void** state )
{
    ( void ) state; /* unused */

    /*************************************/
    /* vOSAL_ExitCritical Success tests  */
    /*************************************/

    /* happy path */
    expect_function_call( __wrap_vPortExitCritical );
    vOSAL_ExitCritical();
}

void test_pvOSAL_MemAlloc_failure( void** state )
{
    ( void ) state; /* unused */

    /**********************************/
    /* pvOSAL_MemAlloc Failure tests  */
    /**********************************/

    /* param 1 error */
    assert_int_equal( pvOSAL_MemAlloc( 0 ), NULL );
}

void test_pvOSAL_MemAlloc_success( void** state )
{
    ( void ) state; /* unused */

    /**********************************/
    /* pvOSAL_MemAlloc Success tests  */
    /**********************************/

    /* happy path */
    /* allocate 5 bytes memory */
    assert_int_not_equal( pvOSAL_MemAlloc( 5*sizeof( uint8_t ) ), NULL );

    /* happy path 2 */
    /* allocate 10 bytes memory */
    assert_int_not_equal( pvOSAL_MemAlloc( 10*sizeof( uint8_t ) ), NULL );
}

void test_pvOSAL_MemSet_failure( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;
    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };

    /**********************************/
    /* pvOSAL_MemSet Failure tests    */
    /**********************************/

    /* param 1 error */
    assert_int_equal( pvOSAL_MemSet( pucMemBuff1, 1, 5 ), NULL );

    /* invalid OS return value ( iOSAL_Mutex_Take ) */
    will_return( __wrap_xQueueSemaphoreTake, pdFAIL );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, 0x33 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );

    assert_int_equal( pvOSAL_MemSet( pucMemBuff2, 1, 5 ), NULL );
}

void test_pvOSAL_MemSet_success( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };

    /**********************************/
    /* pvOSAL_MemSet Success tests    */
    /**********************************/

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, 0x33 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, 0x33 );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    assert_int_not_equal( pvOSAL_MemSet( pucMemBuff2, 1, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 1 );
    assert_int_equal( pucMemBuff2[ 1 ], 1 );
    assert_int_equal( pucMemBuff2[ 2 ], 1 );
    assert_int_equal( pucMemBuff2[ 3 ], 1 );
    assert_int_equal( pucMemBuff2[ 4 ], 1 );

    /* happy path 2 */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, 0x33 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, 0x33 );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    assert_int_not_equal( pvOSAL_MemSet( pucMemBuff2, 5, 5 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 5 );
    assert_int_equal( pucMemBuff2[ 1 ], 5 );
    assert_int_equal( pucMemBuff2[ 2 ], 5 );
    assert_int_equal( pucMemBuff2[ 3 ], 5 );
    assert_int_equal( pucMemBuff2[ 4 ], 5 );

    /* happy path 3*/
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, 0x33 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, 0x33 );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    assert_int_not_equal( pvOSAL_MemSet( pucMemBuff2, 1, 2 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 1 );
    assert_int_equal( pucMemBuff2[ 1 ], 1 );
    assert_int_equal( pucMemBuff2[ 2 ], 5 );
    assert_int_equal( pucMemBuff2[ 3 ], 5 );
    assert_int_equal( pucMemBuff2[ 4 ], 5 );
}

void test_pvOSAL_MemCpy_failure( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;
    uint8_t pucMemBuff2[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };


    /*******************************/
    /* pvOSAL_MemCpy Failure tests */
    /*******************************/

    /* param 1 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff1, pucMemSourceBuff, 5 ), NULL );

    /* param 2 error */
    assert_int_equal( pvOSAL_MemCpy( pucMemBuff2, NULL, 5 ), NULL );

    /* invalid OS return value (iOSAL_Mutex_Take) */
    will_return( __wrap_xQueueSemaphoreTake, pdFAIL );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMemCpyMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );

    assert_int_equal( pvOSAL_MemCpy( pucMemBuff2, pucMemSourceBuff, 5 ), NULL );
}

void test_pvOSAL_MemCpy_success( void** state )
{
    ( void ) state; /* unused */

    uint8_t pucMemBuff[ 5 ] = { 0, 0, 0, 0, 0 };
    uint8_t pucMemBuff2[ 10 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t pucMemBuff3[ 10 ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t pucMemSourceBuff[ 5 ] = { 0x55, 0x44, 0x33, 0x22, 0x11 };
    uint8_t pucMemSourceBuff2[ 10 ] = { 0x55, 0x44, 0x33, 0x22, 0x11, 
                                        0x11, 0x22, 0x33, 0x44, 0x55 };

    /*******************************/
    /* pvOSAL_MemCpy Success tests */
    /*******************************/ 

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMemCpyMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMemCpyMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    assert_int_not_equal( pvOSAL_MemCpy( pucMemBuff, pucMemSourceBuff, 5 ), NULL );

    assert_int_equal( pucMemBuff[ 0 ], 0x55 );
    assert_int_equal( pucMemBuff[ 1 ], 0x44 );
    assert_int_equal( pucMemBuff[ 2 ], 0x33 );
    assert_int_equal( pucMemBuff[ 3 ], 0x22 );
    assert_int_equal( pucMemBuff[ 4 ], 0x11 );

    /* happy path 2 */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMemCpyMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMemCpyMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    assert_int_not_equal( pvOSAL_MemCpy( pucMemBuff2, pucMemSourceBuff2, 10 ), NULL );

    assert_int_equal( pucMemBuff2[ 0 ], 0x55 );
    assert_int_equal( pucMemBuff2[ 1 ], 0x44 );
    assert_int_equal( pucMemBuff2[ 2 ], 0x33 );
    assert_int_equal( pucMemBuff2[ 3 ], 0x22 );
    assert_int_equal( pucMemBuff2[ 4 ], 0x11 );
    assert_int_equal( pucMemBuff2[ 5 ], 0x11 );
    assert_int_equal( pucMemBuff2[ 6 ], 0x22 );
    assert_int_equal( pucMemBuff2[ 7 ], 0x33 );
    assert_int_equal( pucMemBuff2[ 8 ], 0x44 );
    assert_int_equal( pucMemBuff2[ 9 ], 0x55 );

    /* happy path 3 */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvMemCpyMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvMemCpyMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    assert_int_not_equal( pvOSAL_MemCpy( pucMemBuff3, pucMemSourceBuff2, 5 ), NULL );

    assert_int_equal( pucMemBuff3[ 0 ], 0x55 );
    assert_int_equal( pucMemBuff3[ 1 ], 0x44 );
    assert_int_equal( pucMemBuff3[ 2 ], 0x33 );
    assert_int_equal( pucMemBuff3[ 3 ], 0x22 );
    assert_int_equal( pucMemBuff3[ 4 ], 0x11 );
    assert_int_equal( pucMemBuff3[ 5 ], 0 );
    assert_int_equal( pucMemBuff3[ 6 ], 0 );
    assert_int_equal( pucMemBuff3[ 7 ], 0 );
    assert_int_equal( pucMemBuff3[ 8 ], 0 );
    assert_int_equal( pucMemBuff3[ 9 ], 0 );
}

void test_vOSAL_MemFree_failure( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff1 = NULL;
    uint8_t* pucMemBuff2 = ( void* )0x10;

    /**********************************/
    /* vOSAL_MemFree Failure tests    */
    /**********************************/

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( NULL );

    /* param 1 error - expect no functions to be called */
    vOSAL_MemFree( ( void** )&pucMemBuff1 );
}

void test_vOSAL_MemFree_success( void** state )
{
    ( void ) state; /* unused */

    uint8_t* pucMemBuff = NULL;
    uint8_t* pucMemBuff2 = NULL;

    /**********************************/
    /* vOSAL_MemFree Success tests */
    /**********************************/

    /* happy path */
    pucMemBuff = pvOSAL_MemAlloc( 5*sizeof( uint8_t ) );

    expect_function_call( __wrap_vPortFree );
    vOSAL_MemFree( ( void** )&pucMemBuff );
    assert_int_equal( pucMemBuff, NULL );

    /* happy path 2 */
    pucMemBuff2 = pvOSAL_MemAlloc( 10*sizeof( uint8_t ) );

    expect_function_call( __wrap_vPortFree );
    vOSAL_MemFree( ( void** )&pucMemBuff2 );
    assert_int_equal( pucMemBuff2, NULL );
}

void test_vOSAL_Printf_failure( void** state )
{
    ( void ) state; /* unused */

    /**********************************/
    /* vOSAL_Printf Failure tests     */
    /**********************************/

    /* param 1 error - expect no functions to be called */
    vOSAL_Printf( NULL );
}

void test_vOSAL_Printf_success( void** state )
{
    ( void ) state; /* unused */

    int iTestParam = 2;

    /**********************************/
    /* pvOSAL_MemAlloc Success tests  */
    /**********************************/

    /* happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    expect_function_call( __wrap_xil_printf );

    vOSAL_Printf( "test print happy path\r\n" );

    /* happy path 2 */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    expect_function_call( __wrap_xil_printf );

    vOSAL_Printf( "test print (%d) with a passed param\r\n", iTestParam );
}

void test_cOSAL_GetChar_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_cOSAL_GetChar_success( void** state )
{
    ( void ) state; /* unused */

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, 0x22 );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, 0x22 );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_inbyte, 1 );
    assert_int_equal( cOSAL_GetChar(), 1 );
}

void test_pxAllocateTaskMemory_failure( void** state )
{
    ( void ) state; /* unused */
    
    int i = 0;

    for( i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryUsed[ i ] = 1;
    }

    assert_null( pxAllocateTaskMemory() );

    for( i = 0; i < OSAL_MAX_TASKS; i++ )
    {
        xTaskMemoryUsed[ i ] = MEM_UNUSED;
    }
}

void test_pxAllocateTaskMemory_success( void** state )
{
    ( void ) state; /* unused */

    assert_non_null( pxAllocateTaskMemory() );
}

void test_vDeallocateTaskMemory_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vDeallocateTaskMemory_success( void** state )
{
    ( void ) state; /* unused */

    OSAL_TASK_MEMORY *pxMemory = pxAllocateTaskMemory();

    vDeallocateTaskMemory( pxMemory );
}

void test_vPrint_Header_failure( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    vPrint_Header( NULL );
}

void test_vPrint_Header_success( void** state )
{
    ( void ) state; /* unused */

    /* Happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Header( "Semaphore" );
}

void test_vPrint_Footer_failure( void** state )
{
    ( void ) state; /* unused */

    int iSemCount = 0;

    /* param 2 error */
    vPrint_Footer( iSemCount, NULL );
}

void test_vPrint_Footer_success( void** state )
{
    ( void ) state; /* unused */

    int iSemCount = 0;

    /* Happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Footer( iSemCount, "Semaphore" );
}

void test_vPrint_OS_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_OS_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* Happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_OS_Stats();
}

void test_vPrint_Task_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_Task_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* OSAL_STATS_VERBOSITY_ACTIVE_ONLY ( 0 == pxCurrentTask->ulCpuUsagePercentage ) */
    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Task_Stats( OSAL_STATS_VERBOSITY_ACTIVE_ONLY );

    /* OSAL_STATS_VERBOSITY_ACTIVE_ONLY ( 0 != pxCurrentTask->ulCpuUsagePercentage ) */
    pxOsStatsHandle->pxTaskHead->ulCpuUsagePercentage = 1;

    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Task_Stats( OSAL_STATS_VERBOSITY_ACTIVE_ONLY );

    pxOsStatsHandle->pxTaskHead->ulCpuUsagePercentage = 0;

    /* OSAL_STATS_VERBOSITY_FULL (0 == pxCurrentTask->ulCpuUsagePercentage) */
    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Task_Stats( OSAL_STATS_VERBOSITY_FULL );

    /* OSAL_STATS_VERBOSITY_FULL (0 != pxCurrentTask->ulCpuUsagePercentage) */
    pxOsStatsHandle->pxTaskHead->ulCpuUsagePercentage = 1;

    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Task_Stats( OSAL_STATS_VERBOSITY_FULL );

    pxOsStatsHandle->pxTaskHead->ulCpuUsagePercentage = 0;

    /* default */
    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );
    
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Task_Stats( OSAL_STATS_VERBOSITY_COUNT_ONLY );
}

void test_vPrint_Sem_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_Sem_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* OSAL_STATS_VERBOSITY_ACTIVE_ONLY */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Sem_Stats( OSAL_STATS_VERBOSITY_ACTIVE_ONLY );

    /* OSAL_STATS_VERBOSITY_FULL */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Sem_Stats( OSAL_STATS_VERBOSITY_FULL );

    /* default */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Sem_Stats( OSAL_STATS_VERBOSITY_COUNT_ONLY );
}

void test_vPrint_Mutex_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_Mutex_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* OSAL_STATS_VERBOSITY_ACTIVE_ONLY */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Mutex_Stats( OSAL_STATS_VERBOSITY_ACTIVE_ONLY );

    /* OSAL_STATS_VERBOSITY_FULL */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Mutex_Stats( OSAL_STATS_VERBOSITY_FULL );

    /* default */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Mutex_Stats( OSAL_STATS_VERBOSITY_COUNT_ONLY );
}

void test_vPrint_Mailbox_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_Mailbox_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* OSAL_STATS_VERBOSITY_ACTIVE_ONLY */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Mailbox_Stats( OSAL_STATS_VERBOSITY_ACTIVE_ONLY );

    /* OSAL_STATS_VERBOSITY_FULL */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Mailbox_Stats( OSAL_STATS_VERBOSITY_FULL );

    /* default */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Mailbox_Stats( OSAL_STATS_VERBOSITY_COUNT_ONLY );
}

void test_vPrint_Event_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_Event_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* OSAL_STATS_VERBOSITY_ACTIVE_ONLY */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Event_Stats( OSAL_STATS_VERBOSITY_ACTIVE_ONLY );

    /* OSAL_STATS_VERBOSITY_FULL */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Event_Stats( OSAL_STATS_VERBOSITY_FULL );

    /* default */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Event_Stats( OSAL_STATS_VERBOSITY_COUNT_ONLY );
}

void test_vPrint_Timer_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_Timer_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* OSAL_STATS_VERBOSITY_ACTIVE_ONLY */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_strcmp, OK );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Timer_Stats( OSAL_STATS_VERBOSITY_ACTIVE_ONLY );

    /* OSAL_STATS_VERBOSITY_FULL */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    vPrint_Timer_Stats( OSAL_STATS_VERBOSITY_FULL );

    /* default */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Timer_Stats( OSAL_STATS_VERBOSITY_COUNT_ONLY );
}

void test_vPrint_Memory_Stats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vPrint_Memory_Stats_success( void** state )
{
    ( void ) state; /* unused */

    /* Happy path */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vPrint_Memory_Stats();
}

void test_pxFindTask_failure( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_null( pxFindTask( NULL ) );
}

void test_pxFindTask_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = ( void* )0x10;

    /* happy path */
    pxOsStatsHandle->pxTaskHead->pvTask = pvTaskHandle;

    assert_non_null( pxFindTask( pvTaskHandle ) );
}

void test_pxFindSem_failure( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_null( pxFindSem( NULL ) );
}

void test_pxFindSem_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvSemHandle = ( void* )0x10;

    /* happy path */
    pxOsStatsHandle->pxSemHead->pvSem = pvSemHandle;

    assert_non_null( pxFindSem( pvSemHandle ) );
}

void test_pxFindMutex_failure( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_null( pxFindMutex( NULL ) );
}

void test_pxFindMutex_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvMutexHandle = ( void* )0x10;

    /* happy path */
    pxOsStatsHandle->pxMutexHead->pvMutex = pvMutexHandle;

    assert_non_null( pxFindMutex( pvMutexHandle ) );
}

void test_pxFindMailbox_failure( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_null( pxFindMailbox( NULL ) );
}

void test_pxFindMailbox_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvMailboxHandle = ( void* )0x10;

    /* happy path */
    pxOsStatsHandle->pxMailboxHead->pvMailbox = pvMailboxHandle;

    assert_non_null( pxFindMailbox( pvMailboxHandle ) );
}

void test_pxFindEvent_failure( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_null( pxFindEvent( NULL ) );
}

void test_pxFindEvent_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvEventHandle = ( void* )0x10;

    /* happy path */
    pxOsStatsHandle->pxEventHead->pvEvent = pvEventHandle;

    assert_non_null( pxFindEvent( pvEventHandle ) );
}

void test_pxFindTimer_failure( void** state )
{
    ( void ) state; /* unused */

    /* param 1 error */
    assert_null( pxFindTimer( NULL ) );
}

void test_pxFindTimer_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvTimerHandle = ( void* )0x10;

    /* happy path */
    pxOsStatsHandle->pxTimerHead->pvTimer = pvTimerHandle;

    assert_non_null( pxFindTimer( pvTimerHandle ) );
}

void test_vCalculateStackWatermark_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vCalculateStackWatermark_success( void** state )
{
    ( void ) state; /* unused */

    void*  pvTaskHandle = ( void* )0x10;

    /* happy path */
    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );

    vCalculateStackWatermark();
}

void test_vOSAL_PrintAllStats_failure( void** state )
{
    ( void ) state; /* unused */
}

void test_vOSAL_PrintAllStats_success( void** state )
{
    ( void ) state; /* unused */

    /* OSAL_STATS_TYPE_OS */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_OS );

    /* OSAL_STATS_TYPE_TASK */
    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );
    
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_TASK );

    /* OSAL_STATS_TYPE_MUTEX */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_MUTEX );

    /* OSAL_STATS_TYPE_SEM */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_SEM );

    /* OSAL_STATS_TYPE_MAILBOX */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_MAILBOX );

    /* OSAL_STATS_TYPE_EVENT */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_EVENT );

    /* OSAL_STATS_TYPE_TIMER */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_TIMER );

    /* OSAL_STATS_TYPE_MEMORY */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, OSAL_STATS_TYPE_MEMORY );

    /* MAX_OSAL_STATS_TYPE_ALL */
/* OSAL_STATS_TYPE_OS */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );
    
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );


/* OSAL_STATS_TYPE_TASK */
    expect_function_call( __wrap_uxTaskGetNumberOfTasks );
    expect_function_call( __wrap_uxTaskGetSystemState );
    expect_function_call( __wrap_vPortFree );
    
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );


/* OSAL_STATS_TYPE_MUTEX */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );


/* OSAL_STATS_TYPE_SEM */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );


/* OSAL_STATS_TYPE_MAILBOX */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );


/* OSAL_STATS_TYPE_EVENT */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );


/* OSAL_STATS_TYPE_TIMER */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );


/* OSAL_STATS_TYPE_MEMORY */
    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    will_return( __wrap_xQueueSemaphoreTake, pdPASS );
    expect_value( __wrap_xQueueSemaphoreTake, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueSemaphoreTake, xTicksToWait, 0x32 );
    expect_function_call( __wrap_xil_printf );
    will_return( __wrap_xQueueGenericSend, pdPASS );
    expect_value( __wrap_xQueueGenericSend, xQueue, pvPrintfMutexMockAddr );
    expect_value( __wrap_xQueueGenericSend, pvItemToQueue, NULL );
    expect_value( __wrap_xQueueGenericSend, xTicksToWait, semGIVE_BLOCK_TIME );

    vOSAL_PrintAllStats( OSAL_STATS_VERBOSITY_COUNT_ONLY, MAX_OSAL_STATS_TYPE_ALL );
}

void test_vOSAL_ClearAllStats_failure( void** state )
{
     ( void ) state; /* unused */
}

void test_vOSAL_ClearAllStats_success( void** state )
{
    ( void ) state; /* unused */

    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );
    expect_function_call( __wrap_vPortFree );

    vOSAL_ClearAllStats();
}


/*****************************************************************************/
/* UT main entry point                                                       */         
/*****************************************************************************/

const struct CMUnitTest osal_FreeRTOS_tests[] = {
    cmocka_unit_test( test_iOSAL_GetOsVersion_failure ), 
    cmocka_unit_test( test_iOSAL_GetOsVersion_success ), 
    cmocka_unit_test( test_pvOSAL_MemAlloc_before_StartOS_failure ), 
    cmocka_unit_test( test_pvOSAL_MemAlloc_before_StartOS_success ), 
    cmocka_unit_test( test_pvOSAL_MemSet_before_StartOS_failure ), 
    cmocka_unit_test( test_pvOSAL_MemSet_before_StartOS_success ), 
    cmocka_unit_test( test_pvOSAL_MemCpy_before_StartOS_failure ), 
    cmocka_unit_test( test_pvOSAL_MemCpy_before_StartOS_success ), 
    cmocka_unit_test( test_vOSAL_MemFree_before_StartOS_failure ), 
    cmocka_unit_test( test_vOSAL_MemFree_before_StartOS_success ), 
    cmocka_unit_test( test_vOSAL_Printf_before_StartOS_failure ), 
    cmocka_unit_test( test_vOSAL_Printf_before_StartOS_success ), 
    cmocka_unit_test( test_cOSAL_GetChar_before_StartOS_success ), 
    cmocka_unit_test( test_iOSAL_StartOS_failure ), 
    cmocka_unit_test( test_iOSAL_StartOS_success ), 
    cmocka_unit_test( test_ulOSAL_GetUptimeTicks_failure ), 
    cmocka_unit_test( test_ulOSAL_GetUptimeTicks_success ), 
    cmocka_unit_test( test_ulOSAL_GetUptimeMs_failure ), 
    cmocka_unit_test( test_ulOSAL_GetUptimeMs_success ), 
    cmocka_unit_test( test_iOSAL_Task_Create_failure ), 
    cmocka_unit_test( test_iOSAL_Task_Create_success ), 
    cmocka_unit_test( test_iOSAL_Task_Delete_failure ), 
    cmocka_unit_test( test_iOSAL_Task_Delete_success ), 
    cmocka_unit_test( test_iOSAL_Task_Suspend_failure ), 
    cmocka_unit_test( test_iOSAL_Task_Suspend_success ), 
    cmocka_unit_test( test_iOSAL_Task_Resume_failure ), 
    cmocka_unit_test( test_iOSAL_Task_Resume_success ), 
    cmocka_unit_test( test_iOSAL_Task_SleepTicks_failure ), 
    cmocka_unit_test( test_iOSAL_Task_SleepTicks_success ), 
    cmocka_unit_test( test_iOSAL_Task_SleepMs_failure ), 
    cmocka_unit_test( test_iOSAL_Task_SleepMs_success ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Create_failure ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Create_success ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Destroy_failure ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Destroy_success ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Pend_failure ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Pend_success ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Post_failure ), 
    cmocka_unit_test( test_iOSAL_Semaphore_Post_success ), 
    cmocka_unit_test( test_iOSAL_Semaphore_PostFromISR_failure ), 
    cmocka_unit_test( test_iOSAL_Semaphore_PostFromISR_success ), 
    cmocka_unit_test( test_iOSAL_Mutex_Create_failure ),
    cmocka_unit_test( test_iOSAL_Mutex_Create_success ), 
    cmocka_unit_test( test_iOSAL_Mutex_Destroy_failure ), 
    cmocka_unit_test( test_iOSAL_Mutex_Destroy_success ), 
    cmocka_unit_test( test_iOSAL_Mutex_Take_failure ), 
    cmocka_unit_test( test_iOSAL_Mutex_Take_success ), 
    cmocka_unit_test( test_iOSAL_Mutex_Release_failure ), 
    cmocka_unit_test( test_iOSAL_Mutex_Release_success ), 
    cmocka_unit_test( test_iOSAL_MBox_Create_failure ),
    cmocka_unit_test( test_iOSAL_MBox_Create_success ), 
    cmocka_unit_test( test_iOSAL_MBox_Destroy_failure ), 
    cmocka_unit_test( test_iOSAL_MBox_Destroy_success ), 
    cmocka_unit_test( test_iOSAL_MBox_Pend_failure ), 
    cmocka_unit_test( test_iOSAL_MBox_Pend_success ), 
    cmocka_unit_test( test_iOSAL_MBox_Post_failure ), 
    cmocka_unit_test( test_iOSAL_MBox_Post_success ), 
    cmocka_unit_test( test_iOSAL_MBox_PostFromISR_failure ), 
    cmocka_unit_test( test_iOSAL_MBox_PostFromISR_success ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Create_failure ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Create_success ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Destroy_failure ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Destroy_success ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Pend_failure ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Pend_success ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Post_failure ), 
    cmocka_unit_test( test_iOSAL_EventFlag_Post_success ), 
    cmocka_unit_test( test_iOSAL_EventFlag_PostFromISR_failure ), 
    cmocka_unit_test( test_iOSAL_EventFlag_PostFromISR_success ), 
    cmocka_unit_test( test_iOSAL_Timer_Create_failure ), 
    cmocka_unit_test( test_iOSAL_Timer_Create_success ), 
    cmocka_unit_test( test_iOSAL_Timer_Destroy_failure ), 
    cmocka_unit_test( test_iOSAL_Timer_Destroy_success ), 
    cmocka_unit_test( test_iOSAL_Timer_Start_failure ), 
    cmocka_unit_test( test_iOSAL_Timer_Start_success ), 
    cmocka_unit_test( test_iOSAL_Timer_Stop_failure ), 
    cmocka_unit_test( test_iOSAL_Timer_Stop_success ), 
    cmocka_unit_test( test_iOSAL_Timer_Reset_failure ), 
    cmocka_unit_test( test_iOSAL_Timer_Reset_success ), 
    cmocka_unit_test( test_iOSAL_Interrupt_Setup_failure ), 
    cmocka_unit_test( test_iOSAL_Interrupt_Setup_success ), 
    cmocka_unit_test( test_iOSAL_Interrupt_Enable_success ), 
    cmocka_unit_test( test_iOSAL_Interrupt_Disable_success ), 
    cmocka_unit_test( test_vOSAL_EnterCritical_success ), 
    cmocka_unit_test( test_vOSAL_ExitCritical_success ), 
    cmocka_unit_test( test_pvOSAL_MemAlloc_failure ), 
    cmocka_unit_test( test_pvOSAL_MemAlloc_success ), 
    cmocka_unit_test( test_pvOSAL_MemSet_failure ), 
    cmocka_unit_test( test_pvOSAL_MemSet_success ), 
    cmocka_unit_test( test_pvOSAL_MemCpy_failure ), 
    cmocka_unit_test( test_pvOSAL_MemCpy_success ), 
    cmocka_unit_test( test_vOSAL_MemFree_failure ), 
    cmocka_unit_test( test_vOSAL_MemFree_success ), 
    cmocka_unit_test( test_vOSAL_Printf_failure ), 
    cmocka_unit_test( test_vOSAL_Printf_success ),
    cmocka_unit_test( test_cOSAL_GetChar_failure ), 
    cmocka_unit_test( test_cOSAL_GetChar_success ), 
    cmocka_unit_test( test_pxAllocateTaskMemory_failure ), 
    cmocka_unit_test( test_pxAllocateTaskMemory_success ), 
    cmocka_unit_test( test_vDeallocateTaskMemory_failure ), 
    cmocka_unit_test( test_vDeallocateTaskMemory_success ), 
    cmocka_unit_test( test_vPrint_Header_failure ), 
    cmocka_unit_test( test_vPrint_Header_success ), 
    cmocka_unit_test( test_vPrint_Footer_failure ), 
    cmocka_unit_test( test_vPrint_Footer_success ), 
    cmocka_unit_test( test_vPrint_OS_Stats_failure ), 
    cmocka_unit_test( test_vPrint_OS_Stats_success ), 
    cmocka_unit_test( test_vPrint_Task_Stats_failure ), 
    cmocka_unit_test( test_vPrint_Task_Stats_success ), 
    cmocka_unit_test( test_vPrint_Sem_Stats_failure ), 
    cmocka_unit_test( test_vPrint_Sem_Stats_success ), 
    cmocka_unit_test( test_vPrint_Mutex_Stats_failure ), 
    cmocka_unit_test( test_vPrint_Mutex_Stats_success ), 
    cmocka_unit_test( test_vPrint_Mailbox_Stats_failure ), 
    cmocka_unit_test( test_vPrint_Mailbox_Stats_success ), 
    cmocka_unit_test( test_vPrint_Event_Stats_failure ), 
    cmocka_unit_test( test_vPrint_Event_Stats_success ), 
    cmocka_unit_test( test_vPrint_Timer_Stats_failure ), 
    cmocka_unit_test( test_vPrint_Timer_Stats_success ), 
    cmocka_unit_test( test_vPrint_Memory_Stats_failure ), 
    cmocka_unit_test( test_vPrint_Memory_Stats_success ), 
    cmocka_unit_test( test_pxFindTask_failure ), 
    cmocka_unit_test( test_pxFindTask_success ), 
    cmocka_unit_test( test_pxFindSem_failure ), 
    cmocka_unit_test( test_pxFindSem_success ), 
    cmocka_unit_test( test_pxFindMutex_failure ), 
    cmocka_unit_test( test_pxFindMutex_success ), 
    cmocka_unit_test( test_pxFindMailbox_failure ), 
    cmocka_unit_test( test_pxFindMailbox_success ), 
    cmocka_unit_test( test_pxFindEvent_failure ), 
    cmocka_unit_test( test_pxFindEvent_success ), 
    cmocka_unit_test( test_pxFindTimer_failure ), 
    cmocka_unit_test( test_pxFindTimer_success ), 
    cmocka_unit_test( test_vCalculateStackWatermark_failure ), 
    cmocka_unit_test( test_vCalculateStackWatermark_success ),
    cmocka_unit_test( test_vOSAL_PrintAllStats_failure ), 
    cmocka_unit_test( test_vOSAL_PrintAllStats_success ), 
    cmocka_unit_test( test_vOSAL_ClearAllStats_failure ), 
    cmocka_unit_test( test_vOSAL_ClearAllStats_success )
};

int main( void )
{
    return cmocka_run_group_tests( osal_FreeRTOS_tests, NULL, NULL );
}
