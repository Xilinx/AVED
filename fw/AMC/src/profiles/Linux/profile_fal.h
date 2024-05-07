/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the fal profile for the Linux platform
 *
 * @file profile_fal.h
 *
 */

#ifndef _PROFILE_FAL_H_
#define _PROFILE_FAL_H_

#include "util.h"
#include "pll.h"
#include "fw_if.h"
#include "amc_cfg.h"

#ifndef PLL_PRINTF
#define vPLL_Printf( ... ) printf( __VA_ARGS__ )
#define PLL_PRINTF( ... )  printf( __VA_ARGS__ )
#endif

/* QSFP */
#define FAL_QSFP_MAX_DATA ( 256 )

/* FAL objects */
extern FW_IF_CFG xGcqIf;
extern FW_IF_CFG *pxOspiIf;
extern FW_IF_CFG *pxEmmcIf;
extern FW_IF_CFG xQsfpIf1;
extern FW_IF_CFG xQsfpIf2;
extern FW_IF_CFG xQsfpIf3;
extern FW_IF_CFG xQsfpIf4;
extern FW_IF_CFG xDimmIf;
extern FW_IF_CFG *pxSMBusIf;

/**
 * @brief   Initialise FAL layer
 *
 * @param   pullAmcInitStatus   Pointer to the uint64_t bitmask of the
 *                              initialisation status of each component
 *
 * @return  OK if all FAL objects initialised and created successfully
 *          ERROR if any or all FAL objects not initialised or created
 *
 */
int iFAL_Initialise( uint64_t *pullAmcInitStatus );

/**
 * @brief   Initialise FAL Debug monitoring
 *
 * @return  none.
 *
 */
void vFAL_DebugInitialise( void );

#endif
