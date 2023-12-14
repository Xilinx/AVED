/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Debug Access Library (DAL) debug access
 *
 * @file dal_debug.h
 *
 */

#ifndef _DAL_DEBUG_H_
#define _DAL_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the DAL debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vDAL_DebugInit( DAL_HDL pxParentHandle );

#endif

