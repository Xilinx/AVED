/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the System Monitoring (SYS_MON) debug access
 *
 * @file sys_mon_debug.h
 *
 */

#ifndef _SYS_MON_DEBUG_H_
#define _SYS_MON_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the SYS_MON debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vSYS_MON_DebugInit( DAL_HDL pxParentHandle );

#endif

