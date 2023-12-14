/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the clocks profile for the V80
 *
 * @file profile_clocks.h
 *
 */

#ifndef _PROFILES_CLOCKS_H_
#define _PROFILES_CLOCKS_H_

#include "acc_proxy_driver.h"

#define CLOCK_PROFILE_NUM_SHUTDOWN_SENSORS                  ( 2 )

ACC_PROXY_DRIVER_SHUTDOWN_SENSOR_DATA xShutdownSensors =
{
    CLOCK_PROFILE_NUM_SHUTDOWN_SENSORS, /* 2  sensors */
    {
    { 13, 0 },                          /* Device (FPGA), Temperature */
    { 14, 0 },                          /* VCCINT       , Temperature */
    {  0, 0 },
    {  0, 0 },
    {  0, 0 },
    {  0, 0 },
    {  0, 0 },
    {  0, 0 },
    {  0, 0 },
    {  0, 0 }
    }

};

#endif
