/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the EMMC driver debug access
 *
 * @file emmc_debug.h
 *
 */

#ifndef _EMMC_DEBUG_H_
#define _EMMC_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the EMMC driver debug access
 *
 * @param   pxParentHandle optional handle to the parent directory
 *
 * @return  N/A
 */
void vEMMC_DebugInit( DAL_HDL pxParentHandle );

#endif
