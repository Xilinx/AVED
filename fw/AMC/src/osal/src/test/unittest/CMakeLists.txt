# Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required( VERSION 3.5.0 )

project( amc )

include( CTest )
enable_testing()

remove_definitions( -DRUN_TIME )

include_directories( ../../../../../amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/include )
include_directories( ../../../../../amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/lib )
include_directories( ../../ )
include_directories( ./ )

find_package( Threads REQUIRED )
add_executable( test_osal_Linux 
                test_osal_Linux.c
                ../../linux/osal.c 
)

target_link_libraries( test_osal_Linux
                       cmocka 
                       Threads::Threads 
                       rt
                       -Wl,--wrap=pthread_create
                       -Wl,--wrap=pthread_attr_init
                       -Wl,--wrap=pthread_attr_init
                       -Wl,--wrap=pthread_attr_setschedparam
                       -Wl,--wrap=pthread_attr_setstacksize
                       -Wl,--wrap=pthread_mutex_init
                       -Wl,--wrap=clock_gettime
                       -Wl,--wrap=malloc
                       -Wl,--wrap=usleep
                       -Wl,--wrap=printf
                       -Wl,--wrap=sem_open
                       -Wl,--wrap=sem_close
                       -Wl,--wrap=sem_unlink
                       -Wl,--wrap=sem_wait
                       -Wl,--wrap=sem_trywait
                       -Wl,--wrap=sem_timedwait
                       -Wl,--wrap=sem_post
                       -Wl,--wrap=free
                       -Wl,--wrap=pthread_mutex_init
                       -Wl,--wrap=pthread_mutex_destroy
                       -Wl,--wrap=pthread_mutex_timedlock
                       -Wl,--wrap=pthread_mutex_trylock
                       -Wl,--wrap=pthread_mutex_lock
                       -Wl,--wrap=pthread_mutex_unlock
                       -Wl,--wrap=pthread_cond_destroy
                       -Wl,--wrap=pthread_cond_broadcast
                       -Wl,--wrap=pthread_cond_wait
                       -Wl,--wrap=pthread_cond_timedwait
                       -Wl,--wrap=timer_create
                       -Wl,--wrap=timer_delete
                       -Wl,--wrap=timer_settime
                       -Wl,--wrap=fopen
)

# add_test( NAME test_osal_Linux
#           COMMAND test_osal_Linux
# )

add_executable( test_osal_FreeRTOS
                test_osal_FreeRTOS.c
)

target_link_libraries( test_osal_FreeRTOS 
                       cmocka
                       -Wl,--wrap=xTaskCreateStatic
                       -Wl,--wrap=vTaskDelete
                       -Wl,--wrap=vTaskSuspend
                       -Wl,--wrap=vTaskResume
                       -Wl,--wrap=vTaskDelay
                       -Wl,--wrap=vTaskStartScheduler
                       -Wl,--wrap=xTaskGetTickCount
                       -Wl,--wrap=xSemaphoreCreateBinary
                       -Wl,--wrap=xSemaphoreCreateCounting
                       -Wl,--wrap=xQueueGenericCreate
                       -Wl,--wrap=xQueueCreateCountingSemaphore
                       -Wl,--wrap=vSemaphoreDelete
                       -Wl,--wrap=xSemaphoreTake
                       -Wl,--wrap=xQueueSemaphoreTake
                       -Wl,--wrap=xSemaphoreGive
                       -Wl,--wrap=xQueueGenericSend
                       -Wl,--wrap=xSemaphoreGiveFromISR
                       -Wl,--wrap=xQueueGiveFromISR
                       -Wl,--wrap=xSemaphoreCreateMutex
                       -Wl,--wrap=xQueueCreateMutex
                       -Wl,--wrap=xQueueCreate
                       -Wl,--wrap=vQueueDelete
                       -Wl,--wrap=xQueueReceive
                       -Wl,--wrap=xQueueSend
                       -Wl,--wrap=xQueueSendFromISR
                       -Wl,--wrap=xQueueGenericSendFromISR
                       -Wl,--wrap=xEventGroupCreate
                       -Wl,--wrap=vEventGroupDelete
                       -Wl,--wrap=xEventGroupWaitBits
                       -Wl,--wrap=xEventGroupSetBits
                       -Wl,--wrap=xEventGroupSetBitsFromISR
                       -Wl,--wrap=xTimerCreate
                       -Wl,--wrap=xTimerDelete
                       -Wl,--wrap=xTimerChangePeriod
                       -Wl,--wrap=xTimerStop
                       -Wl,--wrap=xTimerReset
                       -Wl,--wrap=xTimerGenericCommand
                       -Wl,--wrap=xPortInstallInterruptHandler
                       -Wl,--wrap=xPortGetFreeHeapSize
                       -Wl,--wrap=xPortGetMinimumEverFreeHeapSize
                       -Wl,--wrap=uxTaskGetNumberOfTasks
                       -Wl,--wrap=uxTaskGetSystemState
                       -Wl,--wrap=vPortEnableInterrupt
                       -Wl,--wrap=vPortDisableInterrupt
                       -Wl,--wrap=vPortEnterCritical
                       -Wl,--wrap=vPortExitCritical
                       -Wl,--wrap=pvPortMalloc
                       -Wl,--wrap=vPortFree
                       -Wl,--wrap=xil_printf
                       -Wl,--wrap=inbyte
                       -Wl,--wrap=strcmp
)

add_test( NAME test_osal_FreeRTOS
          COMMAND test_osal_FreeRTOS
)

# unit test coverage setup

if( COVERAGE_ENABLE )
    include( CodeCoverage.cmake )
    APPEND_COVERAGE_COMPILER_FLAGS()

    set( COVERAGE_EXCLUDES
         test_osal_Linux.c
         test_osal_FreeRTOS.c
    )

    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME test_coverage
        EXECUTABLE ctest
        DEPENDENCIES 
            test_osal_Linux
            test_osal_FreeRTOS
    )
endif()
