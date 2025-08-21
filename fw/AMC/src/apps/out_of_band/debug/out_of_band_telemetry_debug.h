/**
 * Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the in band telemetry defintions
 *
 * @file out_of_band_telemetry.h
 *
 */

#ifndef _OUT_OF_BAND_TELEMETRY_DEBUG_H_
#define _OUT_OF_BAND_TELEMETRY_DEBUG_H_


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/**
 * @brief   Initialise the out of band telemetry application debug access
 *
 * @param   pxParentHandle  optional handle to the parent directory
 *
 * @return  N/A
 */
void vOUT_OF_BAND_TELEMETRY_DebugInit( DAL_HDL pxParentHandle );

#endif /* _OUT_OF_BAND_TELEMETRY_DEBUG_H_ */
