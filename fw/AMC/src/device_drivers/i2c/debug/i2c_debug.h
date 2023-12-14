/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the i2c driver debug access
 *
 * @file i2c_debug.h
 *
 */

#ifndef _I2C_DEBUG_H_
#define _I2C_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the i2c driver debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vI2C_DebugInit( DAL_HDL pxParentHandle );

#endif

