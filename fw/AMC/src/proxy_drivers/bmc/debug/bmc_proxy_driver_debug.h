/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Board Management Controller (BMC) proxy debug access
 *
 * @file bmc_proxy_driver_debug.h
 *
 */

#ifndef _BMC_PROXY_DRIVER_DEBUG_H_
#define _BMC_PROXY_DRIVER_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the BMC debug access
 *
 * @param   pxParentHandle optional handle to the parent directory
 *
 * @return  N/A
 */
void vBMC_DebugInit( DAL_HDL pxParentHandle );

#endif
