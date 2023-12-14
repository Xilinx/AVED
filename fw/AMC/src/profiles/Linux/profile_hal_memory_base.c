/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains definitions of memory base variables, required by 
 * the AMC Linux build
 *
 * @file profile_hal_memory_base.c
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

/* hardware definitions */
#include "profile_hal.h"

/******************************************************************************/
/* Global variables                                                           */
/******************************************************************************/

uint8_t HAL_BASE_LOGIC_GCQ_M2R_S01_AXI_BASEADDR[ HAL_GCQ_BASE_ADDR_SIZE ]  = { 0 };
uint64_t HAL_USER_CLOCK_CONTROL_BASE                                       = 0;
const uint64_t HAL_USER_CLOCK_CONTROL_BASE_ADDERSS                         = ( uint64_t )&HAL_USER_CLOCK_CONTROL_BASE;
uint8_t HAL_RPU_SHARED_MEMORY_BASE_ARRAY[ HAL_RPU_SHARED_MEMORY_SIZE ]     = { 0 };
const uint64_t HAL_RPU_SHARED_MEMORY_BASE_ADDR                             = ( uint64_t )&HAL_RPU_SHARED_MEMORY_BASE_ARRAY;
