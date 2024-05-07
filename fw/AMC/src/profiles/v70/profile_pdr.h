/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the PLDM PDR profile for the V70
 *
 * @file profile_pdr.h
 *
 */

#ifndef _PROFILE_PDR_H_
#define _PROFILE_PDR_H_

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include "util.h"
#include "asc_proxy_driver.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

/* AMC Device ID Defines */

#define FPGA_DEVICE_ID       ( 40 )
#define VR_VCCINT_DEVICE_ID  ( 35 )
#define VR_12V_PEX_DEVICE_ID ( 5 )
#define VR_3V3_PEX_DEVICE_ID ( 10 )
#define VR_3V3_AUX_DEVICE_ID ( 15 )

/* AMC Reading Channel Defines */

#define FPGA_TEMPERATURE_READING_CHANNEL      ( 0 )
#define VR_VCCINT_TEMPERATURE_READING_CHANNEL ( 0 )
#define VR_VCCINT_VOLTAGE_READING_CHANNEL     ( 1 )
#define VR_VCCINT_CURRENT_READING_CHANNEL     ( 2 )
#define VR_12V_PEX_VOLTAGE_READING_CHANNEL    ( 1 )
#define VR_12V_PEX_CURRENT_READING_CHANNEL    ( 2 )
#define VR_12V_PEX_POWER_READING_CHANNEL      ( 3 )
#define VR_3V3_PEX_VOLTAGE_READING_CHANNEL    ( 1 )
#define VR_3V3_PEX_CURRENT_READING_CHANNEL    ( 2 )
#define VR_3V3_PEX_POWER_READING_CHANNEL      ( 3 )
#define VR_3V3_AUX_VOLTAGE_READING_CHANNEL    ( 1 )

/* FPGA Temperature PDR Defines */

#define FPGA_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/* VR_VCCINT Temperature PDR Defines */

#define VR_VCCINT_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/* VR_VCCINT Voltage PDR Defines */

#define VR_VCCINT_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/* VR_VCCINT Current PDR Defines */

#define VR_VCCINT_CURRENT_UPDATE_INTERVAL ( 1.0 )

/* VR_12V_PEX Voltage PDR Defines */

#define VR_12V_PEX_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/* VR_12V_PEX Current PDR Defines */

#define VR_12V_PEX_CURRENT_UPDATE_INTERVAL ( 1.0 )

/* VR_12V_PEX Power PDR Defines */

#define VR_12V_PEX_POWER_UPDATE_INTERVAL ( 1.0 )

/* VR_3v3_PEX Voltage PDR Defines */

#define VR_3V3_PEX_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/* VR_3v3_PEX Current PDR Defines */

#define VR_3V3_PEX_CURRENT_UPDATE_INTERVAL ( 1.0 )

/* VR_3v3_PEX Power PDR Defines */

#define VR_3V3_PEX_POWER_UPDATE_INTERVAL ( 1.0 )

/* VR_3v3_AUX Voltage PDR Defines */

#define VR_3V3_AUX_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

#define TOTAL_PDR_TEMPERATURE          ( 0 )
#define TOTAL_PDR_VOLTAGE              ( 0 )
#define TOTAL_PDR_CURRENT              ( 0 )
#define TOTAL_PDR_POWER                ( 0 )
#define TOTAL_PDR_NUMERIC_ASCI_SENSORS ( 0 )

PLDM_NUMERIC_SENSOR_PDR pxPdrTemperatureSensors[ TOTAL_PDR_TEMPERATURE ] =
{};
PLDM_NUMERIC_SENSOR_PDR pxPdrVoltageSensors[ TOTAL_PDR_VOLTAGE ] =
{};
PLDM_NUMERIC_SENSOR_PDR pxPdrCurrentSensors[ TOTAL_PDR_CURRENT ] =
{};
PLDM_NUMERIC_SENSOR_PDR pxPdrPowerSensors[ TOTAL_PDR_POWER ] =
{};
PLDM_NUMERIC_SENSOR_NAME_PDR pxPdrSensorNames[ TOTAL_PDR_NUMERIC_ASCI_SENSORS ] =
{};

#endif
