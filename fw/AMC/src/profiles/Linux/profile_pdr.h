/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the PLDM PDR profile for Linux
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

#define TOTAL_PDR_TEMPERATURE          ( 5 )
#define TOTAL_PDR_VOLTAGE              ( 9 )
#define TOTAL_PDR_CURRENT              ( 9 )
#define TOTAL_PDR_POWER                ( 5 )
#define TOTAL_PDR_NUMERIC_ASCI_SENSORS ( TOTAL_PDR_TEMPERATURE + TOTAL_PDR_VOLTAGE + TOTAL_PDR_CURRENT + \
                                         TOTAL_PDR_POWER )

#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD ( 68 )
#define LOGICAL_PLDM_ENTITY_ADDIN_CARD  ( 32835 )

/* Macros to set bits for 'ucucRangeFieldSupport' field in PDR */

#define NORMAL_MIN_MAX  ( 0x3 << 1 )
#define RATED_MIN_MAX   ( 0x3 << 3 )
#define HIGH_THRESHOLDS ( 0x5 << 3 )
#define LOW_THRESHOLDS  ( 0x5 << 4 )
#define NOMINAL_VALUE   ( 0x1 )

/* Macros to set bits for 'ucucSupportedThresholds' field in PDR */

#define HAS_LOW_THRESHOLDS  ( 0x7 << 3 )
#define HAS_HIGH_THRESHOLDS ( 0x7 )

/*
 *  AMC Device ID Defines
 */

#define VR_12V_PEX_DEVICE_ID       ( 5 )
#define VR_3V3_PEX_DEVICE_ID       ( 10 )
#define VR_3V3_QSFP_DEVICE_ID      ( 15 )
#define VR_1V5_VCC_AUX_DEVICE_ID   ( 20 )
#define VR_12V_AUX0_DEVICE_ID      ( 25 )
#define VR_12V_AUX1_DEVICE_ID      ( 30 )
#define VR_VCCINT_DEVICE_ID        ( 35 )
#define VR_VCC_HBM_DEVICE_ID       ( 40 )
#define VR_12V_VCC0_DIMM_DEVICE_ID ( 45 )
#define VR_1V2_GTXAVTT_DEVICE_ID   ( 50 )
#define FPGA_DEVICE_ID             ( 55 )
#define QSFP_MODULE_0_DEVICE_ID    ( 60 )
#define QSFP_MODULE_1_DEVICE_ID    ( 65 )
#define QSFP_MODULE_2_DEVICE_ID    ( 70 )
#define QSFP_MODULE_3_DEVICE_ID    ( 75 )

/*
 *  AMC Reading Channel Defines
 */

#define VR_12V_PEX_VOLTAGE_READING_CHANNEL         ( 1 )
#define VR_12V_PEX_CURRENT_READING_CHANNEL         ( 2 )
#define VR_12V_PEX_POWER_READING_CHANNEL           ( 3 )
#define VR_3V3_PEX_VOLTAGE_READING_CHANNEL         ( 1 )
#define VR_3V3_PEX_CURRENT_READING_CHANNEL         ( 2 )
#define VR_3V3_PEX_POWER_READING_CHANNEL           ( 3 )
#define VR_1V5_VCC_AUX_TEMPERATURE_READING_CHANNEL ( 0 )
#define VR_1V5_VCC_AUX_VOLTAGE_READING_CHANNEL     ( 1 )
#define VR_1V5_VCC_AUX_CURRENT_READING_CHANNEL     ( 2 )
#define VR_12V_AUX0_VOLTAGE_READING_CHANNEL        ( 1 )
#define VR_12V_AUX0_CURRENT_READING_CHANNEL        ( 2 )
#define VR_12V_AUX0_POWER_READING_CHANNEL          ( 3 )
#define VR_12V_AUX1_VOLTAGE_READING_CHANNEL        ( 1 )
#define VR_12V_AUX1_CURRENT_READING_CHANNEL        ( 2 )
#define VR_12V_AUX1_POWER_READING_CHANNEL          ( 3 )
#define VR_VCCINT_TEMPERATURE_READING_CHANNEL      ( 0 )
#define VR_VCCINT_VOLTAGE_READING_CHANNEL          ( 1 )
#define VR_VCCINT_CURRENT_READING_CHANNEL          ( 2 )
#define VR_VCC_HBM_TEMPERATURE_READING_CHANNEL     ( 0 )
#define VR_VCC_HBM_VOLTAGE_READING_CHANNEL         ( 1 )
#define VR_VCC_HBM_CURRENT_READING_CHANNEL         ( 2 )
#define VR_12V_VCC0_DIMM_VOLTAGE_READING_CHANNEL   ( 1 )
#define VR_12V_VCC0_DIMM_CURRENT_READING_CHANNEL   ( 2 )
#define VR_12V_VCC0_DIMM_POWER_READING_CHANNEL     ( 3 )
#define VR_1V2_GTXAVTT_TEMPERATURE_READING_CHANNEL ( 0 )
#define VR_1V2_GTXAVTT_VOLTAGE_READING_CHANNEL     ( 1 )
#define VR_1V2_GTXAVTT_CURRENT_READING_CHANNEL     ( 2 )
#define FPGA_TEMPERATURE_READING_CHANNEL           ( 0 )

/*
 * PLDM Type 2 (DSP0248) Numeric Sensor ID Defines
 */

#define VR_12V_PEX_VOLTAGE_SENSOR_ID         ( VR_12V_PEX_DEVICE_ID + ( VR_12V_PEX_VOLTAGE_READING_CHANNEL << 8 ) )
#define VR_12V_PEX_CURRENT_SENSOR_ID         ( VR_12V_PEX_DEVICE_ID + ( VR_12V_PEX_CURRENT_READING_CHANNEL << 8 ) )
#define VR_12V_PEX_POWER_SENSOR_ID           ( VR_12V_PEX_DEVICE_ID + ( VR_12V_PEX_POWER_READING_CHANNEL << 8 ) )
#define VR_3V3_PEX_VOLTAGE_SENSOR_ID         ( VR_3V3_PEX_DEVICE_ID + ( VR_3V3_PEX_VOLTAGE_READING_CHANNEL << 8 ) )
#define VR_3V3_PEX_CURRENT_SENSOR_ID         ( VR_3V3_PEX_DEVICE_ID + ( VR_3V3_PEX_CURRENT_READING_CHANNEL << 8 ) )
#define VR_3V3_PEX_POWER_SENSOR_ID           ( VR_3V3_PEX_DEVICE_ID + ( VR_3V3_PEX_POWER_READING_CHANNEL << 8 ) )
#define VR_1V5_VCC_AUX_TEMPERATURE_SENSOR_ID ( VR_1V5_VCC_AUX_DEVICE_ID + \
                                               ( VR_1V5_VCC_AUX_TEMPERATURE_READING_CHANNEL << 8 ) )
#define VR_1V5_VCC_AUX_VOLTAGE_SENSOR_ID     ( VR_1V5_VCC_AUX_DEVICE_ID + ( VR_1V5_VCC_AUX_VOLTAGE_READING_CHANNEL << \
                                                                            8 ) )
#define VR_1V5_VCC_AUX_CURRENT_SENSOR_ID     ( VR_1V5_VCC_AUX_DEVICE_ID + ( VR_1V5_VCC_AUX_CURRENT_READING_CHANNEL << \
                                                                            8 ) )
#define VR_12V_AUX0_VOLTAGE_SENSOR_ID        ( VR_12V_AUX0_DEVICE_ID + ( VR_12V_AUX0_VOLTAGE_READING_CHANNEL << 8 ) )
#define VR_12V_AUX0_CURRENT_SENSOR_ID        ( VR_12V_AUX0_DEVICE_ID + ( VR_12V_AUX0_CURRENT_READING_CHANNEL << 8 ) )
#define VR_12V_AUX0_POWER_SENSOR_ID          ( VR_12V_AUX0_DEVICE_ID + ( VR_12V_AUX0_POWER_READING_CHANNEL << 8 ) )
#define VR_12V_AUX1_VOLTAGE_SENSOR_ID        ( VR_12V_AUX1_DEVICE_ID + ( VR_12V_AUX1_VOLTAGE_READING_CHANNEL << 8 ) )
#define VR_12V_AUX1_CURRENT_SENSOR_ID        ( VR_12V_AUX1_DEVICE_ID + ( VR_12V_AUX1_CURRENT_READING_CHANNEL << 8 ) )
#define VR_12V_AUX1_POWER_SENSOR_ID          ( VR_12V_AUX1_DEVICE_ID + ( VR_12V_AUX1_POWER_READING_CHANNEL << 8 ) )
#define VR_VCCINT_TEMPERATURE_SENSOR_ID      ( VR_VCCINT_DEVICE_ID + ( VR_VCCINT_TEMPERATURE_READING_CHANNEL << 8 ) )
#define VR_VCCINT_VOLTAGE_SENSOR_ID          ( VR_VCCINT_DEVICE_ID + ( VR_VCCINT_VOLTAGE_READING_CHANNEL << 8 ) )
#define VR_VCCINT_CURRENT_SENSOR_ID          ( VR_VCCINT_DEVICE_ID + ( VR_VCCINT_CURRENT_READING_CHANNEL << 8 ) )
#define VR_VCC_HBM_TEMPERATURE_SENSOR_ID     ( VR_VCC_HBM_DEVICE_ID + ( VR_VCC_HBM_TEMPERATURE_READING_CHANNEL << 8 ) )
#define VR_VCC_HBM_VOLTAGE_SENSOR_ID         ( VR_VCC_HBM_DEVICE_ID + ( VR_VCC_HBM_VOLTAGE_READING_CHANNEL << 8 ) )
#define VR_VCC_HBM_CURRENT_SENSOR_ID         ( VR_VCC_HBM_DEVICE_ID + ( VR_VCC_HBM_CURRENT_READING_CHANNEL << 8 ) )
#define VR_12V_VCC0_DIMM_VOLTAGE_SENSOR_ID   ( VR_12V_VCC0_DIMM_DEVICE_ID + \
                                               ( VR_12V_VCC0_DIMM_VOLTAGE_READING_CHANNEL << 8 ) )
#define VR_12V_VCC0_DIMM_CURRENT_SENSOR_ID   ( VR_12V_VCC0_DIMM_DEVICE_ID + \
                                               ( VR_12V_VCC0_DIMM_CURRENT_READING_CHANNEL << 8 ) )
#define VR_12V_VCC0_DIMM_POWER_SENSOR_ID     ( VR_12V_VCC0_DIMM_DEVICE_ID + ( VR_12V_VCC0_DIMM_POWER_READING_CHANNEL << \
                                                                              8 ) )
#define VR_1V2_GTXAVTT_TEMPERATURE_SENSOR_ID ( VR_1V2_GTXAVTT_DEVICE_ID + \
                                               ( VR_1V2_GTXAVTT_TEMPERATURE_READING_CHANNEL << 8 ) )
#define VR_1V2_GTXAVTT_VOLTAGE_SENSOR_ID     ( VR_1V2_GTXAVTT_DEVICE_ID + ( VR_1V2_GTXAVTT_VOLTAGE_READING_CHANNEL << \
                                                                            8 ) )
#define VR_1V2_GTXAVTT_CURRENT_SENSOR_ID     ( VR_1V2_GTXAVTT_DEVICE_ID + ( VR_1V2_GTXAVTT_CURRENT_READING_CHANNEL << \
                                                                            8 ) )
#define FPGA_TEMPERATURE_SENSOR_ID           ( FPGA_DEVICE_ID + ( FPGA_TEMPERATURE_READING_CHANNEL << 8 ) )

/*
 * PLDM Type 2 (DSP2061&DSP0241) Entity Type and Number
 */

#define LOGICAL_PLDM_ENTITY_ADDIN_CARD_1_VR_12V_AUX0_CURRENT          ( 1 )
#define LOGICAL_PLDM_ENTITY_ADDIN_CARD_2_VR_12V_AUX0_POWER            ( 2 )
#define LOGICAL_PLDM_ENTITY_ADDIN_CARD_3_VR_12V_AUX1_CURRENT          ( 3 )
#define LOGICAL_PLDM_ENTITY_ADDIN_CARD_4_VR_12V_AUX1_POWER            ( 4 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_1_VR_12V_PEX_VOLTAGE          ( 1 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_2_VR_12V_PEX_CURRENT          ( 2 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_3_VR_12V_PEX_POWER            ( 3 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_4_VR_3V3_PEX_VOLTAGE          ( 4 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_5_VR_3V3_PEX_CURRENT          ( 5 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_6_VR_3V3_PEX_POWER            ( 6 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_10_VR_1V5_VCC_AUX_TEMPERATURE ( 10 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_11_VR_1V5_VCC_AUX_VOLTAGE     ( 11 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_12_VR_1V5_VCC_AUX_CURRENT     ( 12 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_13_VR_12V_AUX0_VOLTAGE        ( 13 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_14_VR_12V_AUX1_VOLTAGE        ( 14 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_15_VR_VCCINT_TEMPERATURE      ( 15 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_16_VR_VCCINT_VOLTAGE          ( 16 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_17_VR_VCCINT_CURRENT          ( 17 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_18_VR_VCC_HBM_TEMPERATURE     ( 18 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_19_VR_VCC_HBM_VOLTAGE         ( 19 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_20_VR_VCC_HBM_CURRENT         ( 20 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_21_VR_12V_VCC0_DIMM_VOLTAGE   ( 21 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_22_VR_12V_VCC0_DIMM_CURRENT   ( 22 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_23_VR_12V_VCC0_DIMM_POWER     ( 23 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_24_VR_1V2_GTXAVTT_TEMPERATURE ( 24 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_25_VR_1V2_GTXAVTT_VOLTAGE     ( 25 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_26_VR_1V2_GTXAVTT_CURRENT     ( 26 )
#define PHYSICAL_PLDM_ENTITY_ADDIN_CARD_27_FPGA_TEMPERATURE           ( 27 )

/*
 * VR_12V_PEX Voltage PDR Defines
 */

#define VR_12V_PEX_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_PEX Current PDR Defines
 */

#define VR_12V_PEX_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_PEX Power PDR Defines
 */

#define VR_12V_PEX_POWER_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_3V3_PEX Voltage PDR Defines
 */

#define VR_3V3_PEX_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_3V3_PEX Current PDR Defines
 */

#define VR_3V3_PEX_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_3V3_PEX Power PDR Defines
 */

#define VR_3V3_PEX_POWER_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_3V3_QSFP Voltage PDR Defines
 */

#define VR_3V3_QSFP_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_3V3_QSFP Current PDR Defines
 */

#define VR_3V3_QSFP_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_3V3_QSFP Power PDR Defines
 */

#define VR_3V3_QSFP_POWER_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_1V5_VCC_AUX Temperature PDR Defines
 */

#define VR_1V5_VCC_AUX_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_1V5_VCC_AUX Voltage PDR Defines
 */

#define VR_1V5_VCC_AUX_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_1V5_VCC_AUX Current PDR Defines
 */

#define VR_1V5_VCC_AUX_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_AUX0 Voltage PDR Defines
 */

#define VR_12V_AUX0_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_AUX0 Current PDR Defines
 */

#define VR_12V_AUX0_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_AUX0 Power PDR Defines
 */

#define VR_12V_AUX0_POWER_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_AUX1 Voltage PDR Defines
 */

#define VR_12V_AUX1_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_AUX1 Current PDR Defines
 */

#define VR_12V_AUX1_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_AUX1 Power PDR Defines
 */

#define VR_12V_AUX1_POWER_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_VCCINT Temperature PDR Defines
 */

#define VR_VCCINT_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_VCCINT Voltage PDR Defines
 */

#define VR_VCCINT_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_VCCINT Current PDR Defines
 */

#define VR_VCCINT_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_VCC_HBM Temperature PDR Defines
 */

#define VR_VCC_HBM_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_VCC_HBM Voltage PDR Defines
 */

#define VR_VCC_HBM_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_VCC_HBM Current PDR Defines
 */

#define VR_VCC_HBM_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_VCC0_DIMM Voltage PDR Defines
 */

#define VR_12V_VCC0_DIMM_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_VCC0_DIMM Current PDR Defines
 */

#define VR_12V_VCC0_DIMM_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_12V_VCC0_DIMM Power PDR Defines
 */

#define VR_12V_VCC0_DIMM_POWER_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_1V2_GTXAVTT Temperature PDR Defines
 */

#define VR_1V2_GTXAVTT_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_1V2_GTXAVTT Voltage PDR Defines
 */

#define VR_1V2_GTXAVTT_VOLTAGE_UPDATE_INTERVAL ( 1.0 )

/*
 * VR_1V2_GTXAVTT Current PDR Defines
 */

#define VR_1V2_GTXAVTT_CURRENT_UPDATE_INTERVAL ( 1.0 )

/*
 * FPGA Temperature PDR Defines
 */

#define FPGA_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * QSFP Module 0 Temperature PDR Defines
 */

#define QSFP_MODULE_0_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * QSFP Module 1 Temperature PDR Defines
 */

#define QSFP_MODULE_1_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * QSFP Module 2 Temperature PDR Defines
 */

#define QSFP_MODULE_2_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

/*
 * QSFP Module 3 Temperature PDR Defines
 */

#define QSFP_MODULE_3_TEMPERATURE_UPDATE_INTERVAL ( 1.0 )

PLDM_NUMERIC_SENSOR_PDR pxPdrTemperatureSensors[ TOTAL_PDR_TEMPERATURE ] =
{
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_1V5_VCC_AUX_TEMPERATURE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_10_VR_1V5_VCC_AUX_TEMPERATURE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_DEGREES_C,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_1V5_VCC_AUX_TEMPERATURE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_VCCINT_TEMPERATURE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_15_VR_VCCINT_TEMPERATURE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_DEGREES_C,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_VCCINT_TEMPERATURE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_VCC_HBM_TEMPERATURE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_18_VR_VCC_HBM_TEMPERATURE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_DEGREES_C,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_VCC_HBM_TEMPERATURE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_1V2_GTXAVTT_TEMPERATURE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_24_VR_1V2_GTXAVTT_TEMPERATURE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_DEGREES_C,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_1V2_GTXAVTT_TEMPERATURE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = FPGA_TEMPERATURE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_27_FPGA_TEMPERATURE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_DEGREES_C,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = FPGA_TEMPERATURE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
};

PLDM_NUMERIC_SENSOR_PDR pxPdrVoltageSensors[ TOTAL_PDR_VOLTAGE ] =
{
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_PEX_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_1_VR_12V_PEX_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_PEX_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_3V3_PEX_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_4_VR_3V3_PEX_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_3V3_PEX_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_1V5_VCC_AUX_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_11_VR_1V5_VCC_AUX_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_1V5_VCC_AUX_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_AUX0_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_13_VR_12V_AUX0_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_AUX0_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_AUX1_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_14_VR_12V_AUX1_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_AUX1_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_VCCINT_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_16_VR_VCCINT_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_VCCINT_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_VCC_HBM_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_19_VR_VCC_HBM_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_VCC_HBM_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_VCC0_DIMM_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_21_VR_12V_VCC0_DIMM_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_VCC0_DIMM_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_1V2_GTXAVTT_VOLTAGE_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_25_VR_1V2_GTXAVTT_VOLTAGE,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_VOLTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_1V2_GTXAVTT_VOLTAGE_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    }
};

PLDM_NUMERIC_SENSOR_PDR pxPdrCurrentSensors[ TOTAL_PDR_CURRENT ] =
{
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_PEX_CURRENT_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_2_VR_12V_PEX_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_PEX_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_3V3_PEX_CURRENT_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_5_VR_3V3_PEX_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_3V3_PEX_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_1V5_VCC_AUX_CURRENT_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_12_VR_1V5_VCC_AUX_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_1V5_VCC_AUX_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_AUX0_CURRENT_SENSOR_ID,
        .usEntityType                       = LOGICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = LOGICAL_PLDM_ENTITY_ADDIN_CARD_1_VR_12V_AUX0_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_AUX0_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_AUX1_CURRENT_SENSOR_ID,
        .usEntityType                       = LOGICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = LOGICAL_PLDM_ENTITY_ADDIN_CARD_3_VR_12V_AUX1_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_AUX1_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_VCCINT_CURRENT_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_17_VR_VCCINT_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_VCCINT_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_VCC_HBM_CURRENT_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_20_VR_VCC_HBM_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_VCC_HBM_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_VCC0_DIMM_CURRENT_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_22_VR_12V_VCC0_DIMM_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_VCC0_DIMM_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_1V2_GTXAVTT_CURRENT_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_26_VR_1V2_GTXAVTT_CURRENT,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_AMPS,
        .cUnitModifier                      = PDR_PARAM_NONE,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_1V2_GTXAVTT_CURRENT_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    }
};

PLDM_NUMERIC_SENSOR_PDR pxPdrPowerSensors[ TOTAL_PDR_POWER ] =
{
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_PEX_POWER_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_3_VR_12V_PEX_POWER,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_WATTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_PEX_POWER_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_3V3_PEX_POWER_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_6_VR_3V3_PEX_POWER,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_WATTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_3V3_PEX_POWER_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_AUX0_POWER_SENSOR_ID,
        .usEntityType                       = LOGICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = LOGICAL_PLDM_ENTITY_ADDIN_CARD_2_VR_12V_AUX0_POWER,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_WATTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_AUX0_POWER_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_AUX1_POWER_SENSOR_ID,
        .usEntityType                       = LOGICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = LOGICAL_PLDM_ENTITY_ADDIN_CARD_4_VR_12V_AUX1_POWER,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_WATTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_AUX1_POWER_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    },
    {
        .ucSensorDataSize                   = EDataTypeSInt16,
        .xRange.ucRangeFieldFormat          = EDataTypeSInt16,
        .usPldmTerminusHandle               = PLDM_TERMINUS_HANDLE,
        .usSensorId                         = VR_12V_VCC0_DIMM_POWER_SENSOR_ID,
        .usEntityType                       = PHYSICAL_PLDM_ENTITY_ADDIN_CARD,
        .usEntityInstanceNumber             = PHYSICAL_PLDM_ENTITY_ADDIN_CARD_23_VR_12V_VCC0_DIMM_POWER,
        .usContainerId                      = CONTAINED_ENTITY_CONTAINER_ID,
        .ucSensorInit                       = PLDM_PDR_INIT_STATUS_NO_INIT,
        .ucSensorAuxiliaryNamesPdr          = TRUE,
        .ucBaseUnit                         = PLDM_PDR_BASE_UNIT_WATTS,
        .cUnitModifier                      = UNITS_IN_MILLI,
        .ucRateUnit                         = PLDM_PDR_RATE_UNIT_NONE,
        .ucBaseOemUnitHandle                = PDR_PARAM_NONE,
        .ucAuxUnit                          = PLDM_PDR_RATE_UNIT_NONE,
        .cAuxUnitModifier                   = PDR_PARAM_NONE,
        .ucAuxRateUnit                      = PLDM_PDR_RATE_UNIT_NONE,
        .ucRel                              = PDR_PARAM_NONE,
        .ucAuxOemUnitHandle                 = PDR_PARAM_NONE,
        .ucIsLinear                         = TRUE,
        .fResolution                        = PDR_PARAM_NONE,
        .fOffset                            = PDR_PARAM_NONE,
        .usAccuracy                         = PDR_PARAM_NONE,
        .ucPlusTolerance                    = PDR_PARAM_NONE,
        .ucMinusTolerance                   = PDR_PARAM_NONE,
        .sHysteresis                        = PDR_PARAM_NONE,
        .ucSupportedThresholds              = PDR_PARAM_NONE,
        .ucThresholdAndHysteresisVolatility = PDR_PARAM_NONE,
        .fStateTransitionInterval           = PDR_PARAM_NONE,
        .fUpdateInterval                    = VR_12V_VCC0_DIMM_POWER_UPDATE_INTERVAL,
        .sMaxReadable                       = PDR_PARAM_NONE,
        .sMinReadable                       = PDR_PARAM_NONE,
        .xRange.ucRangeFieldSupport         = PDR_PARAM_NONE,
        .xRange.sNominalValue               = PDR_PARAM_NONE,
        .xRange.sNormalMax                  = PDR_PARAM_NONE,
        .xRange.sNormalMin                  = PDR_PARAM_NONE,
        .xRange.sWarningHigh                = PDR_PARAM_NONE,
        .xRange.sWarningLow                 = PDR_PARAM_NONE,
        .xRange.sCriticalHigh               = PDR_PARAM_NONE,
        .xRange.sCriticalLow                = PDR_PARAM_NONE,
        .xRange.sFatalHigh                  = PDR_PARAM_NONE,
        .xRange.sFatalLow                   = PDR_PARAM_NONE
    }
};

/* Numeric sensor names PDRs */

PLDM_NUMERIC_SENSOR_NAME_PDR pxPdrSensorNames[ TOTAL_PDR_NUMERIC_ASCI_SENSORS ] =
{
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_PEX_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 80, 0, 69, 0, 88, 0, 32, 0, 86, 0, 111, 0, 108, 0, 116, 0,
          97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_PEX_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 80, 0, 69, 0, 88, 0, 32, 0, 67, 0, 117, 0, 114, 0, 114, 0,
          101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_PEX_POWER_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 80, 0, 69, 0, 88, 0, 32, 0, 80, 0, 111, 0, 119, 0, 101, 0,
          114, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_3V3_PEX_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 51, 0, 86, 0, 51, 0, 95, 0, 80, 0, 69, 0, 88, 0, 32, 0, 86, 0, 111, 0, 108, 0, 116, 0,
          97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_3V3_PEX_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 51, 0, 86, 0, 51, 0, 95, 0, 80, 0, 69, 0, 88, 0, 32, 0, 67, 0, 117, 0, 114, 0, 114, 0,
          101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_3V3_PEX_POWER_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 51, 0, 86, 0, 51, 0, 95, 0, 80, 0, 69, 0, 88, 0, 32, 0, 80, 0, 111, 0, 119, 0, 101, 0,
          114, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_1V5_VCC_AUX_TEMPERATURE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 86, 0, 53, 0, 95, 0, 86, 0, 67, 0, 67, 0, 95, 0, 65, 0, 85, 0, 88, 0, 32, 0,
          84, 0, 101, 0, 109, 0, 112, 0, 101, 0, 114, 0, 97, 0, 116, 0, 117, 0, 114, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_1V5_VCC_AUX_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 86, 0, 53, 0, 95, 0, 86, 0, 67, 0, 67, 0, 95, 0, 65, 0, 85, 0, 88, 0, 32, 0,
          86, 0, 111, 0, 108, 0, 116, 0, 97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_1V5_VCC_AUX_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 86, 0, 53, 0, 95, 0, 86, 0, 67, 0, 67, 0, 95, 0, 65, 0, 85, 0, 88, 0, 32, 0,
          67, 0, 117, 0, 114, 0, 114, 0, 101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_AUX0_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 65, 0, 85, 0, 88, 0, 48, 0, 32, 0, 86, 0, 111, 0, 108, 0,
          116, 0, 97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_AUX0_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 65, 0, 85, 0, 88, 0, 48, 0, 32, 0, 67, 0, 117, 0, 114, 0,
          114, 0, 101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_AUX0_POWER_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 65, 0, 85, 0, 88, 0, 48, 0, 32, 0, 80, 0, 111, 0, 119, 0,
          101, 0, 114, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_AUX1_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 65, 0, 85, 0, 88, 0, 49, 0, 32, 0, 86, 0, 111, 0, 108, 0,
          116, 0, 97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_AUX1_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 65, 0, 85, 0, 88, 0, 49, 0, 32, 0, 67, 0, 117, 0, 114, 0,
          114, 0, 101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_AUX1_POWER_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 65, 0, 85, 0, 88, 0, 49, 0, 32, 0, 80, 0, 111, 0, 119, 0,
          101, 0, 114, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_VCCINT_TEMPERATURE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 86, 0, 67, 0, 67, 0, 73, 0, 78, 0, 84, 0, 32, 0, 84, 0, 101, 0, 109, 0, 112, 0, 101,
          0, 114, 0, 97, 0, 116, 0, 117, 0, 114, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_VCCINT_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 86, 0, 67, 0, 67, 0, 73, 0, 78, 0, 84, 0, 32, 0, 86, 0, 111, 0, 108, 0, 116, 0, 97, 0,
          103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_VCCINT_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 86, 0, 67, 0, 67, 0, 73, 0, 78, 0, 84, 0, 32, 0, 67, 0, 117, 0, 114, 0, 114, 0, 101,
          0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_VCC_HBM_TEMPERATURE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 86, 0, 67, 0, 67, 0, 95, 0, 72, 0, 66, 0, 77, 0, 32, 0, 84, 0, 101, 0, 109, 0, 112, 0,
          101, 0, 114, 0, 97, 0, 116, 0, 117, 0, 114, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_VCC_HBM_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 86, 0, 67, 0, 67, 0, 95, 0, 72, 0, 66, 0, 77, 0, 32, 0, 86, 0, 111, 0, 108, 0, 116, 0,
          97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_VCC_HBM_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 86, 0, 67, 0, 67, 0, 95, 0, 72, 0, 66, 0, 77, 0, 32, 0, 67, 0, 117, 0, 114, 0, 114, 0,
          101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_VCC0_DIMM_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 86, 0, 67, 0, 67, 0, 48, 0, 95, 0, 68, 0, 73, 0, 77, 0,
          77, 0, 32, 0, 86, 0, 111, 0, 108, 0, 116, 0, 97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_VCC0_DIMM_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 86, 0, 67, 0, 67, 0, 48, 0, 95, 0, 68, 0, 73, 0, 77, 0,
          77, 0, 32, 0, 67, 0, 117, 0, 114, 0, 114, 0, 101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_12V_VCC0_DIMM_POWER_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 50, 0, 86, 0, 95, 0, 86, 0, 67, 0, 67, 0, 48, 0, 95, 0, 68, 0, 73, 0, 77, 0,
          77, 0, 32, 0, 80, 0, 111, 0, 119, 0, 101, 0, 114, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_1V2_GTXAVTT_TEMPERATURE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 86, 0, 50, 0, 95, 0, 71, 0, 84, 0, 88, 0, 65, 0, 86, 0, 84, 0, 84, 0, 32, 0,
          84, 0, 101, 0, 109, 0, 112, 0, 101, 0, 114, 0, 97, 0, 116, 0, 117, 0, 114, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_1V2_GTXAVTT_VOLTAGE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 86, 0, 50, 0, 95, 0, 71, 0, 84, 0, 88, 0, 65, 0, 86, 0, 84, 0, 84, 0, 32, 0,
          86, 0, 111, 0, 108, 0, 116, 0, 97, 0, 103, 0, 101, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = VR_1V2_GTXAVTT_CURRENT_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 86, 0, 82, 0, 95, 0, 49, 0, 86, 0, 50, 0, 95, 0, 71, 0, 84, 0, 88, 0, 65, 0, 86, 0, 84, 0, 84, 0, 32, 0,
          67, 0, 117, 0, 114, 0, 114, 0, 101, 0, 110, 0, 116, 0, 0 }

    },
    {
        .usPldmTerminusHandle = PLDM_TERMINUS_HANDLE,
        .usSensorId           = FPGA_TEMPERATURE_SENSOR_ID,
        .ucSensorCount        = 1,
        .ucNameStringCount    =
        { 1 },
        .ucNameLanguageTag =
        { 117, 115, 45, 101, 110, 0 },
        .ucSensorNameBe16 =
        { 0, 70, 0, 80, 0, 71, 0, 65, 0, 32, 0, 84, 0, 101, 0, 109, 0, 112, 0, 101, 0, 114, 0, 97, 0, 116, 0, 117, 0,
          114, 0, 101, 0, 0 }

    },
};

#endif
