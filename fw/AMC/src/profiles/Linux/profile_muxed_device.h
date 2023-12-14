/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the muxed device profile for the Linux platform
 *
 * @file profile_muxed_device.h
 *
 */

#ifndef _PROFILE_MUXED_DEVICE_H_
#define _PROFILE_MUXED_DEVICE_H_

/* 4 QSFPs mocked for linux build and 1 DIMM mocked for linux build */
#define MAX_NUM_EXTERNAL_DEVICES_AVAILABLE ( 5 )

#define QSFP_1_REG_BIT_OFFSET              ( 1 << 0 )
#define QSFP_2_REG_BIT_OFFSET              ( 1 << 1 )
#define QSFP_3_REG_BIT_OFFSET              ( 1 << 2 )
#define QSFP_4_REG_BIT_OFFSET              ( 1 << 3 )

#define MUXED_DEVICE_MUX_1_ADDR            ( 0x70 )
#define MUXED_DEVICE_MUX_2_ADDR            ( 0x71 )
#define MUXED_DEVICE_MUX_3_ADDR            ( 0x72 )
#define MUXED_DEVICE_MUX_REG_BIT_1         ( 1 << 0 )
#define MUXED_DEVICE_MUX_REG_BIT_2         ( 1 << 1 )
#define MUXED_DEVICE_MUX_REG_BIT_3         ( 1 << 2 )
#define MUXED_DEVICE_MUX_REG_BIT_4         ( 1 << 3 )

#define QSFP_POWER_IO_EXPANDER_I2C_ADDR    ( 0x21 )
#define QSFP_IO_EXPANDER_I2C_ADDR          ( 0x20 )
#define QSFP_I2C_ADDR                      ( 0x50 )
#define DIMM_I2C_ADDR                      ( 0x18 )

#endif
