/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF OSPI debug access
 *
 * @file fw_if_ospi_debug.h
 *
 */

#ifndef _FW_IF_OSPI_DEBUG_H_
#define _FW_IF_OSPI_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the FW IF OSPI debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vFW_IF_OSPI_DebugInit( DAL_HDL pxParentHandle );

#endif
