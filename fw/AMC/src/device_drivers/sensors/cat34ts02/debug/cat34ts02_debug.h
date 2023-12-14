/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the CAT34TS02 debug access
 *
 * @file cat34ts02_debug.h
 *
 */

#ifndef _CAT34TS02_DEBUG_H_
#define _CAT34TS02_DEBUG_H_


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the CAT34TS02 debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vCAT34TS02_DebugInit( DAL_HDL pxParentHandle );

#endif
