/**
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*
* This header file containing the FW IF SMBus V70-specific definitions.
*
* @file fw_if_smbus_v70.h
*
*/

#ifndef _FW_IF_SMBUS_V70_H_
#define _FW_IF_SMBUS_V70_H_

/*****************************************************************************/
/* enums                                                                     */
/*****************************************************************************/

/**
 * @enum     FW_IF_SMBUS_V70_COMMAND
 * @brief    Command codes for V70
 */
typedef enum _FW_IF_SMBUS_V70_COMMAND
{
    FW_IF_SMBUS_V70_FRU_DATA_READ = 0x0,
    FW_IF_SMBUS_V70_MAX_DIMM_TEMP = 0x1,
    FW_IF_SMBUS_V70_BOARD_TEMP = 0x2,
    FW_IF_SMBUS_V70_BOARD_POWER_CONSUMPTION = 0x3,
    FW_IF_SMBUS_V70_SC_FW_VER = 0x4,
    FW_IF_SMBUS_V70_FPGA_TEMP = 0x5,
    FW_IF_SMBUS_V70_MAX_QSFP_TEMP = 0x6,
    FW_IF_SMBUS_V70_FPGA_RESET = 0x0F,
    FW_IF_SMBUS_V70_FRU_DATA_WRITE = 0x80,

    FW_IF_SMBUS_V70_MAX

} FW_IF_SMBUS_V70_COMMAND;

#endif /* FW_IF_SMBUS_V70_H */