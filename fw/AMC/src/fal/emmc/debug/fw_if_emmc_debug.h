/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF EMMC debug access
 *
 * @file fw_if_emmc_debug.h
 *
 */

#ifndef _FW_IF_EMMC_DEBUG_H_
#define _FW_IF_EMMC_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the FW IF EMMC debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vFW_IF_EMMC_DebugInit( DAL_HDL pxParentHandle );

#endif

