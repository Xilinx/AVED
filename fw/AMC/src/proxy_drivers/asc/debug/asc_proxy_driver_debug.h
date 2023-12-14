/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Sensor Control (ASC) debug access
 *
 * @file asc_proxy_driver_debug.h
 *
 */

#ifndef _ASC_PROXY_DRIVER_DEBUG_H_
#define _ASC_PROXY_DRIVER_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the ASC debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vASC_DebugInit( DAL_HDL pxParentHandle );

#endif

