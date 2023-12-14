/**
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*
* This header file contains the FW IF EMMC abstraction definitions.
*
* @file fw_if_emmc.h
*
*/

#ifndef _FW_IF_EMMC_H_
#define _FW_IF_EMMC_H_

/*****************************************************************************/
/* includes                                                                  */
/*****************************************************************************/

#include "fw_if.h"


/*****************************************************************************/
/* enums                                                                     */
/*****************************************************************************/

/**
 * @enum FW_IF_EMMC_ERRORS
 * @brief Enumeration of EMMC return values
 */
typedef enum FW_IF_EMMC_ERRORS
{
    FW_IF_EMMC_ERRORS_INVALID_STATE = MAX_FW_IF_ERROR,
    FW_IF_EMMC_ERRORS_DRIVER_FAILURE,

    MAX_FW_IF_EMMC_ERRORS

} FW_IF_EMMC_ERRORS;

/**
 * @enum    FW_IF_EMMC_IOCTRL
 * @brief   ioctrl options for EMMC interfaces (generic across all EMMC interfaces)
 */
typedef enum FW_IF_EMMC_IOCTRL
{
    FW_IF_EMMC_IOCTRL_PRINT_INSTANCE_DETAILS = MAX_FW_IF_COMMON_IOCTRL_OPTION,
                            
    MAX_FW_IF_EMMC_IOCTRL_OPTION
                                    
} FW_IF_EMMC_IOCTRL;


/*****************************************************************************/
/* structs                                                                   */
/*****************************************************************************/

/* TODO: add to these structs */

/**
 * @struct  FW_IF_EMMC_INIT_CFG
 * @brief   config options for EMMC initialisation (generic across all EMMC interfaces)
 */
typedef struct FW_IF_EMMC_INIT_CFG
{
    uint32_t        ulDriverId;
    char            *pcDriverName;

} FW_IF_EMMC_INIT_CFG;

/**
 * @struct  FW_IF_EMMC_CFG
 * @brief   config options for EMMC interfaces (generic across all EMMC interfaces)
 */
typedef struct FW_IF_EMMC_CFG
{
    uint32_t        ulIfId;                 /* Interface ID */
    char            *pcIfName;              /* Interface name */
    uint8_t         ucEraseBeforeWriteFlag; /* Erase before write */
    uint16_t        ucDeviceId;             /* The EMMC device ID */

} FW_IF_EMMC_CFG;


/*****************************************************************************/
/* public functions                                                          */
/*****************************************************************************/

/**
 * @brief   initialisation function for EMMC interfaces (generic across all EMMC interfaces)
 *
 * @param   pxInitCfg   pointer to the config to initialise the driver with
 *
 * @return  See FW_IF_ERRORS
 */
extern uint32_t ulFW_IF_EMMC_Init( FW_IF_EMMC_INIT_CFG *pxInitCfg );

/**
 * @brief   creates an instance of the EMMC interface
 *
 * @param   pxFwIf       fw_if handle to the interface instance
 * @param   pxEmmcCfg    unique data of this instance
 *
 * @return  See FW_IF_ERRORS
 */
extern uint32_t ulFW_IF_EMMC_Create( FW_IF_CFG *pxFwIf, FW_IF_EMMC_CFG *pxEmmcCfg );

/**
 *
 * @brief    Print all the stats gathered by the interface
 *
 * @return   OK                  Stats retrieved from fw_if_emmc successfully
 *           ERROR               Stats not retrieved successfully
 */
extern int iFW_IF_EMMC_PrintStatistics( void );

/**
 *
 * @brief    Clears all the stats gathered by the interface
 *
 * @return   OK                  Stats cleared successfully
 *           ERROR               Stats not cleared successfully
 */
extern int iFW_IF_EMMC_ClearStatistics( void );

#endif
