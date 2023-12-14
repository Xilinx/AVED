/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Printing and Logging Library (PLL) debug access
 *
 * @file pll_debug.h
 *
 */

#ifndef _PLL_DEBUG_H_
#define _PLL_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the PLL debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vPLL_DebugInit( DAL_HDL pxParentHandle );

#endif

