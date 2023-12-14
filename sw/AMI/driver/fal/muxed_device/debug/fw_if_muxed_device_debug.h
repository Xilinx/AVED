/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF MUXED_DEVICE debug access.
 *
 * @file fw_if_muxed_device_debug.h
 *
 */

#ifndef _FW_IF_MUXED_DEVICE_DEBUG_H_
#define _FW_IF_MUXED_DEVICE_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the FW IF MUXED_DEVICE debug access
 *
 * @param   pxParentHandle optional handle to the parent directory
 *
 * @return  N/A
 */
void vFW_IF_MUXED_DEVICE_DebugInit( DAL_HDL pxParentHandle );

#endif
