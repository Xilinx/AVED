/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF SMBUS debug access
 *
 * @file fw_if_smbus_debug.h
 *
 */

#ifndef _FW_IF_SMBUS_DEBUG_H_
#define _FW_IF_SMBUS_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the FW IF SMBUS debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vFW_IF_SMBUS_DebugInit( DAL_HDL pxParentHandle );

#endif

