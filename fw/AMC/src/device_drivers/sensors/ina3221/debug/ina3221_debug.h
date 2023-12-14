/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the INA3221 debug access
 *
 * @file ina3221_debug.h
 *
 */

#ifndef _INA3221_DEBUG_H_
#define _INA3221_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the INA3221 debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vINA3221_DebugInit( DAL_HDL pxParentHandle );

#endif
