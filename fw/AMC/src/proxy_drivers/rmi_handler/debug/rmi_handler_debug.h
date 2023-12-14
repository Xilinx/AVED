/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Remote Management Interface (RMI) handler debug access
 *
 * @file    rmi_handler_debug.h
 *
 */

#ifndef _RMI_HANDLER_DEBUG_H_
#define _RMI_HANDLER_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the RMI_HANDLER debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vRMI_HANDLER_DebugInit( DAL_HDL pxParentHandle );

#endif

