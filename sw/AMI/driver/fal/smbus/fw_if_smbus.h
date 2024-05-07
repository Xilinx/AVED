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

#define FW_IF_SMBUS_MAX_DATA                    ( 256 )
#define FW_IF_SMBUS_UDID_LEN                    ( 16 )
#define FAL_SMBUS_BLOCK_IO_DATA_SIZE_INDEX      ( 0 )
#define FAL_SMBUS_BLOCK_IO_PAYLOAD_INDEX        ( 1 )

/*****************************************************************************/
/* enums                                                                     */
/*****************************************************************************/

/**
 * @enum FW_IF_SMBUS_COMMAND_PROTOCOLS
 * @brief List of supported SMBus command protocols.
 */
typedef enum FW_IF_SMBUS_COMMAND_PROTOCOLS
{
    FW_IF_SMBUS_COMMAND_PROTOCOL_QUICK_COMMAND_LO = 0,
    FW_IF_SMBUS_COMMAND_PROTOCOL_QUICK_COMMAND_HI,
    FW_IF_SMBUS_COMMAND_PROTOCOL_SEND_BYTE,
    FW_IF_SMBUS_COMMAND_PROTOCOL_RECEIVE_BYTE,
    FW_IF_SMBUS_COMMAND_PROTOCOL_WRITE_BYTE,
    FW_IF_SMBUS_COMMAND_PROTOCOL_WRITE_WORD,
    FW_IF_SMBUS_COMMAND_PROTOCOL_READ_BYTE,
    FW_IF_SMBUS_COMMAND_PROTOCOL_READ_WORD,
    FW_IF_SMBUS_COMMAND_PROTOCOL_PROCESS_CALL,
    FW_IF_SMBUS_COMMAND_PROTOCOL_BLOCK_WRITE,
    FW_IF_SMBUS_COMMAND_PROTOCOL_BLOCK_READ,
    FW_IF_SMBUS_COMMAND_PROTOCOL_BLOCK_WRITE_BLOCK_READ_PROCESS_CALL,
    FW_IF_SMBUS_COMMAND_PROTOCOL_HOST_NOTIFY,
    FW_IF_SMBUS_COMMAND_PROTOCOL_WRITE_32,
    FW_IF_SMBUS_COMMAND_PROTOCOL_READ_32,
    FW_IF_SMBUS_COMMAND_PROTOCOL_WRITE_64,
    FW_IF_SMBUS_COMMAND_PROTOCOL_READ_64,
    FW_IF_SMBUS_COMMAND_PROTOCOL_PREPARE_TO_ARP,
    FW_IF_SMBUS_COMMAND_PROTOCOL_RESET_DEVICE,
    FW_IF_SMBUS_COMMAND_PROTOCOL_GET_UDID,
    FW_IF_SMBUS_COMMAND_PROTOCOL_ASSIGN_ADDRESS,
    FW_IF_SMBUS_COMMAND_PROTOCOL_GET_UDID_DIRECTED,
    FW_IF_SMBUS_COMMAND_PROTOCOL_RESET_DEVICE_DIRECTED,
    FW_IF_SMBUS_COMMAND_PROTOCOL_NONE, 

    MAX_FW_IF_SMBUS_COMMAND_PROTOCOL

} FW_IF_SMBUS_COMMAND_PROTOCOLS;

/**
 * @enum FW_IF_SMBUS_ERRORS
 * @brief Enumeration of SMBUS return values
 */
typedef enum FW_IF_SMBUS_ERRORS
{
    FW_IF_SMBUS_ERRORS_INVALID_STATE = MAX_FW_IF_ERROR,
    FW_IF_SMBUS_ERRORS_DRIVER_FAILURE,
    FW_IF_SMBUS_ERRORS_DRIVER_INVALID_ADDRESS,

    MAX_FW_IF_SMBUS_ERRORS

} FW_IF_SMBUS_ERRORS;

/**
 * @enum FW_IF_SMBUS_STATE
 * @brief SMBUS interface states (generic across all SMBUS interfaces)
 */
typedef enum FW_IF_SMBUS_STATE
{
    FW_IF_SMBUS_STATE_CREATED,
    FW_IF_SMBUS_STATE_OPENED,
    FW_IF_SMBUS_STATE_CLOSED,
    FW_IF_SMBUS_STATE_ERROR,

    MAX_FW_IF_SMBUS_STATE

} FW_IF_SMBUS_STATE;

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

/**
 * @enum    FW_IF_SMBUS_PEC
 * @brief   PEC Capability
 */
typedef enum _FW_IF_SMBUS_PEC
{
    FW_IF_SMBUS_PEC_ENABLED = 0,
    FW_IF_SMBUS_PEC_DISABLED,

    MAX_FW_IF_SMBUS_PEC

} FW_IF_SMBUS_PEC;

/*****************************************************************************/
/* structs                                                                   */
/*****************************************************************************/

/**
 * @struct  FW_IF_SMBUS_INIT_CFG
 * @brief   config options for smbus initialisation (generic across all smbus interfaces)
 */
typedef struct _FW_IF_SMBUS_INIT_CFG
{
    uint32_t            ulBaseAddr;
    uint32_t            ulBaudRate;
    uint8_t             pucCommandProtocols[ MAX_FW_IF_SMBUS_COMMAND_PROTOCOL ];

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
    uint32_t                ulPort;
    FW_IF_SMBUS_ROLE        xRole;
    FW_IF_SMBUS_ARP         xArpCapability;
    FW_IF_SMBUS_PROTOCOL    xProtocol;
    uint8_t                 pucUdid[ FW_IF_SMBUS_UDID_LEN ];
    FW_IF_SMBUS_STATE       xState;
    uint8_t                 ucInstance;
    FW_IF_SMBUS_PEC         xPecCapability;

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

/**
 *
 * @brief    Print all the stats gathered by the interface
 *
 * @return   OK                  Stats retrieved from interface successfully
 *           ERROR               Stats not retrieved successfully
 */
extern int iFW_IF_SMBUS_PrintStatistics( void );

/**
 *
 * @brief    Clears all the stats gathered by the interface
 *
 * @return   OK                  Stats cleared successfully
 *           ERROR               Stats not cleared successfully
 */
extern int iFW_IF_SMBUS_ClearStatistics( void );

#endif
