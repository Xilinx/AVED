/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF UART debug access
 *
 * @file fw_if_uart_debug.h
 *
 */

#ifndef _FW_IF_UART_DEBUG_H_
#define _FW_IF_UART_DEBUG_H_

#include "dal.h"


/******************************************************************************/
/*  Functions                                                                 */
/******************************************************************************/

/**
 * @brief   Initialize the FW IF UART debug access
 *
 * @param   pxParentHandle optional handle to the parent directory
 */
void vFW_IF_UART_DebugInit( DAL_HDL pxParentHandle );

#endif
