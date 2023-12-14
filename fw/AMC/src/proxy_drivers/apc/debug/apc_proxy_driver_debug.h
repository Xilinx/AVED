/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Programming Control (APC) debug access
 *
 * @file apc_proxy_driver_debug.h
 *
 */

#ifndef _APC_PROXY_DRIVER_DEBUG_H_
#define _APC_PROXY_DRIVER_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the APC debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vAPC_DebugInit( DAL_HDL pxParentHandle );

#endif

