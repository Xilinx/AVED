/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the OSPI driver debug access
 *
 * @file ospi_debug.h
 *
 */

#ifndef _OSPI_DEBUG_H_
#define _OSPI_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the OSPI driver debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vOSPI_DebugInit( DAL_HDL pxParentHandle );

#endif

