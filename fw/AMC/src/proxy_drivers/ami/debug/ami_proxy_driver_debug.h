/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Management Interface (AMI) debug access
 *
 * @file ami_proxy_driver_debug.h
 *
 */

#ifndef _AMI_PROXY_DRIVER_DEBUG_H_
#define _AMI_PROXY_DRIVER_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the AMI debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vAMI_DebugInit( DAL_HDL pxParentHandle );

#endif

