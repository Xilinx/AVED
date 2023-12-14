/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Clock Control (ACC) debug access
 *
 * @file acc_proxy_driver_debug.h
 *
 */

#ifndef _ACC_PROXY_DRIVER_DEBUG_H_
#define _ACC_PROXY_DRIVER_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the ACC debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vACC_DebugInit( DAL_HDL pxParentHandle );

#endif

