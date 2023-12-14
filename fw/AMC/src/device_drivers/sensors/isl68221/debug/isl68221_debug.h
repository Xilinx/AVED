/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the ISL68221 debug access
 *
 * @file isl68221_debug.h
 *
 */

#ifndef _ISL68221_DEBUG_H_
#define _ISL68221_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the ISL68221 debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vISL68221_DebugInit( DAL_HDL pxParentHandle );

#endif
