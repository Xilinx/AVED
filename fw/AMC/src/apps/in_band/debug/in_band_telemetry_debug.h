/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the in band telemetry debug access
 *
 * @file in_band_telemetry_debug.h
 *
 */

#ifndef _IN_BAND_TELEMETRY_DEBUG_H_
#define _IN_BAND_TELEMETRY_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the in band telemetry application debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vIN_BAND_TELEMETRY_DebugInit( DAL_HDL pxParentHandle );

#endif

