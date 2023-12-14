/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the ASDM debug access
 *
 * @file asdm_debug.h
 *
 */

#ifndef _ASDM_DEBUG_H_
#define _ASDM_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the ASDM application debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vASDM_DebugInit( DAL_HDL pxParentHandle );

#endif

