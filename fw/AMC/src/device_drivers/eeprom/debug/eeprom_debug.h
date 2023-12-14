/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the EEPROM debug access
 *
 * @file eeprom_debug.h
 *
 */

#ifndef _EEPROM_DEBUG_H_
#define _EEPROM_DEBUG_H_


#include "dal.h"

/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the EEPROM debug access
 * 
 * @param   pxParentHandle optional handle to the parent directory
 * 
 * @return  N/A
 */
void vEeprom_DebugInit( DAL_HDL pxParentHandle );

#endif
