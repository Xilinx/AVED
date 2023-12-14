/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Event Library (EVL) debug access
 *
 * @file evl_debug.h
 *
 */

#ifndef _EVL_DEBUG_H_
#define _EVL_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the EVL debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vEVL_DebugInit( DAL_HDL pxParentHandle );

#endif

