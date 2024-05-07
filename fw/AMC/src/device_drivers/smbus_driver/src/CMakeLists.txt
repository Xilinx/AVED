# Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required( VERSION 3.5.0 )

project( smbus-driver )

set( SOURCE_FILES
     smbus_action.c  
     smbus_circular_log.c  
     smbus_driver.c  
     smbus_event_buffer.c  
     smbus_event.c  
     smbus_hardware_access.c  
     smbus_interrupt_handler.c  
     smbus_state.c
)

add_library( _smbus_driver STATIC ${SOURCE_FILES} )
