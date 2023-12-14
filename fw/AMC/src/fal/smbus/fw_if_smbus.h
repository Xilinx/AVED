/**
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*
* This header file containing the FW IF SMBus abstraction definitions.
*
* @file fw_if_smbus.h
*
*/

#ifndef _FW_IF_SMBUS_H_
#define _FW_IF_SMBUS_H_

/*****************************************************************************/
/* includes                                                                  */
/*****************************************************************************/

#include "fw_if.h"


/*****************************************************************************/
/* defines                                                                   */
/*****************************************************************************/

#define FW_IF_SMBUS_MAX_DATA    ( 256 )

#define FW_IF_SMBUS_UDID_LEN    ( 16 )

/*****************************************************************************/
/* enums                                                                     */
/*****************************************************************************/

/**
 * @enum    FW_IF_SMBUS_IOCTRL_OPTION
 * @brief   ioctrl options for smbus interfaces (generic across all smbus interfaces)
 */
typedef enum _FW_IF_SMBUS_IOCTRL_OPTIONS
{
    FW_IF_SMBUS_IOCTRL_SET_CONTROLLER = MAX_FW_IF_COMMON_IOCTRL_OPTION,
    FW_IF_SMBUS_IOCTRL_SET_TARGET,
    FW_IF_SMBUS_IOCTRL_ENABLE_PEC,
                            
    MAX_FW_IF_SMBUS_IOCTRL_OPTION
                                    
} FW_IF_SMBUS_IOCTRL_OPTIONS;


/**
 * @enum    FW_IF_SMBUS_EVENTS
 * @brief   smbus events raised in the callback (generic across all smbus interface)
 */
typedef enum _FW_IF_SMBUS_EVENTS
{
    FW_IF_SMBUS_EVENT_ADDRESS_CHANGE = MAX_FW_IF_COMMON_EVENT,

    MAX_FW_IF_SMBUS_EVENT
                            
} FW_IF_SMBUS_EVENTS;

/**
 * @enum    FW_IF_SMBUS_ROLE
 * @brief   Controller or Target
 */
typedef enum _FW_IF_SMBUS_ROLE
{
    FW_IF_SMBUS_ROLE_CONTROLLER = 0,
    FW_IF_SMBUS_ROLE_TARGET,

    MAX_FW_IF_SMBUS_ROLE

} FW_IF_SMBUS_ROLE;

/**
 * @enum    FW_IF_SMBUS_ARP
 * @brief   ARP Capability
 */
typedef enum _FW_IF_SMUBUS_ARP
{
    FW_IF_SMBUS_ARP_CAPABILITY = 0,
    FW_IF_SMBUS_ARP_FIXED_DISCOVERABLE,
    FW_IF_SMBUS_ARP_FIXED_NOT_DISCOVERABLE,
    FW_IF_SMBUS_ARP_NON_ARP_CAPABLE,

    MAX_FW_IF_SMBUS_ARP

} FW_IF_SMBUS_ARP;

/*****************************************************************************/
/* structs                                                                   */
/*****************************************************************************/

/**
 * @struct  FW_IF_SMBUS_INIT_CFG
 * @brief   config options for smbus initialisation (generic across all smbus interfaces)
 */
typedef struct _FW_IF_SMBUS_INIT_CFG
{
    uint32_t            baseAddr;
    uint32_t            baudRate;

} FW_IF_SMBUS_INIT_CFG;

/**
 * @enum     FW_IF_SMBUS_PROTOCOL
 * @brief    Full SMBus protocol or raw i2c
 */
typedef enum _FW_IF_SMBUS_PROTOCOL
{
    FW_IF_SMBUS_PROTOCOL_SMBUS,
    FW_IF_SMBUS_PROTOCOL_I2C,

    MAX_FW_IF_SMBUS_PROTOCOL

} FW_IF_SMBUS_PROTOCOL;

/**
 * @struct  FW_IF_SMBUS_CFG
 * @brief   config options for smbus interfaces (generic across all smbus interfaces)
 */
typedef struct _FW_IF_SMBUS_CFG
{
    uint32_t                port;
    FW_IF_SMBUS_ROLE        role;
    FW_IF_SMBUS_ARP         arpCapability;
    FW_IF_SMBUS_PROTOCOL    protocol;

    uint8_t                 udid[ FW_IF_SMBUS_UDID_LEN ];

} FW_IF_SMBUS_CFG;


/*****************************************************************************/
/* public functions                                                          */
/*****************************************************************************/

/**
 * @brief   initialisation function for smbus interfaces (generic across all smbus interfaces)
 *
 * @param   cfg         pointer to the config to initialise the driver with
 *
 * @return  See FW_IF_ERRORS
 */
extern uint32_t ulFW_IF_SMBUS_Init( FW_IF_SMBUS_INIT_CFG *pxCfg );

/**
 * @brief   creates an instance of the smbus interface
 *
 * @param   fwIf        fw_if handle to the interface instance
 * @param   smbusCfg    unique data of this instance (port, address, etc)
 *
 * @return  See FW_IF_ERRORS
 */
extern uint32_t ulFW_IF_SMBUS_Create( FW_IF_CFG *pxFwIf, FW_IF_SMBUS_CFG *pxSmbusCfg );

#endif
