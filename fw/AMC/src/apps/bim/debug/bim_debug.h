/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Built in Monitoring (BIM) debug access
 *
 * @file bim_debug.h
 *
 */

#ifndef _BIM_DEBUG_H_
#define _BIM_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the BIM debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vBIM_DebugInit( DAL_HDL pxParentHandle );

#endif

