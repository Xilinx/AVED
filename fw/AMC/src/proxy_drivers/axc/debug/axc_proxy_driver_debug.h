/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED External Device (AXC) debug access
 *
 * @file axc_proxy_driver_debug.h
 *
 */

#ifndef _AXC_PROXY_DRIVER_DEBUG_H_
#define _AXC_PROXY_DRIVER_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the AXC debug access
 *
 * @param   pxParentHandle optional handle to the parent directory
 *
 * @return  N/A
 */
void vAXC_DebugInit( DAL_HDL pxParentHandle );

#endif

