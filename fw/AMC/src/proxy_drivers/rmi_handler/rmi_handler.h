/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the API for the RMI Handler
 *
 * @file rmi_handler.h
 *
 */

#ifndef _RMI_HANDLER_H_
#define _RMI_HANDLER_H_

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include "standard.h"
#include "evl.h"

/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    RMI_HANDLER_EVENTS
 * @brief   Events raised by this proxy driver
 */
typedef enum RMI_HANDLER_EVENTS
{
    RMI_HANDLER_E_SENSOR_REQUEST,
    RMI_HANDLER_E_CONFIG_REQUEST,
    RMI_HANDLER_E_SEND_EVENTS_REQUEST,

    MAX_RMI_HANDLER_EVENTS

} RMI_HANDLER_EVENTS;

/**
 * @enum    RMI_HANDLER_REQUEST_TYPE
 * @brief   The request type
 */
typedef enum RMI_HANDLER_REQUEST_TYPE
{
    RMI_HANDLER_REQUEST_TYPE_GET_SDR,
    RMI_HANDLER_REQUEST_TYPE_GET_ALL_SENSOR_DATA,
    RMI_HANDLER_REQUEST_TYPE_CONFIG_WRITES,
    RMI_HANDLER_REQUEST_TYPE_SEND_EVENTS,

    MAX_RMI_HANDLER_REQUEST_TYPE

} RMI_HANDLER_REQUEST_TYPE;

/**
 * @enum    RMI_HANDLER_REPO_TYPE
 * @brief   The repo type
 */
typedef enum RMI_HANDLER_REPO_TYPE
{
    RMI_HANDLER_REPO_TYPE_TEMPERATURE,
    RMI_HANDLER_REPO_TYPE_VOLTAGE,
    RMI_HANDLER_REPO_TYPE_CURRENT,
    RMI_HANDLER_REPO_TYPE_POWER,

    /* TODO: other supported repo's */

    MAX_RMI_HANDLER_REPO_TYPE

} RMI_HANDLER_REPO_TYPE;

/**
 * @enum    RMI_HANDLER_RESULT
 * @brief   The response result
 */
typedef enum RMI_HANDLER_RESULT
{
    RMI_HANDLER_RESULT_SUCCESS = 0,
    RMI_HANDLER_RESULT_FAILURE,

    /* TODO: other possible return codes */

    MAX_RMI_HANDLER_RESULT

} RMI_HANDLER_RESULT;


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct  RMI_HANDLER_SENSOR_REQUEST
 * @brief   The structure containing the RMI handler sensor request
 */
typedef struct RMI_HANDLER_SENSOR_REQUEST
{
    RMI_HANDLER_REQUEST_TYPE xReq;
    RMI_HANDLER_REPO_TYPE xType;
    uint16_t usPayloadSize;
    uintptr_t xPayloadAddress;

} RMI_HANDLER_SENSOR_REQUEST;

/**
 * @struct  RMI_HANDLER_CONFIG_REQUEST
 * @brief   The structure containing the RMI handler config request
 */
typedef struct RMI_HANDLER_CONFIG_REQUEST
{
    uint8_t ucConfigType;
    uint8_t ucConfigData;
    /* TODO: more details need specified to understand the request */

} RMI_HANDLER_CONFIG_REQUEST;

/**
 * @struct  RMI_HANDLER_SEND_EVENTS_REQUEST
 * @brief   The structure containing the RMI handler send events request
 */
typedef struct RMI_HANDLER_SEND_EVENTS_REQUEST
{
    uint8_t ucEventType;
    /* TODO: more details need specified to understand the request */

} RMI_HANDLER_SEND_EVENTS_REQUEST;


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/

/* Init functions *************************************************************/

/**
 * @brief   Main initialisation point for the RMI Handler
 *
 * @param   ucProxyId               Unique ID for the RMI Handler
 * @param   ulTaskPrio              Priority of the RMI Handler task (if RR disabled)
 * @param   ulTaskStack             Stack size of the RMI Handler task
 *
 * @return  OK          RMI Handler initialised correctly
 *          ERROR       RMI Handler not initialised, or was already initialised
 */
int iRMI_HANDLER_Initialise( uint8_t ucProxyId, uint32_t ulTaskPrio, uint32_t ulTaskStack );

/**
 * @brief   Bind into this proxy driver
 *
 * @param   pxCallback  Callback to bind into the proxy driver
 *
 * @return  OK          Callback successfully bound
 *          ERROR       Callback not bound
 * 
 */
int iRMI_HANDLER_BindCallback( EVL_CALLBACK *pxCallback );

/* Set functions **************************************************************/

 /**
 * @brief   Set the response after sensor request has completed
 *
 * @param   pxSignal        Current event occurance (used for tracking)
 * @param   xResult         The result of the request
 * @param   usResponseSize  The size of the populated response
 *
 * @return  OK          Data passed to proxy driver successfully
 *          ERROR       Data not passed successfully
 */
int iRMI_HANDLER_SetSensorResponse( EVL_SIGNAL *pxSignal,
                                    RMI_HANDLER_RESULT xResult,
                                    uint16_t usResponseSize );

/* TODO : add API for config & events response */

/* Get functions **************************************************************/

/**
 * @brief   Get the RMI handler request
 *
 * @param   pxSignal        Current event occurance (used for tracking)
 * @param   pxRequest       Pointer to request structure
 *
 * @return  OK              Data retrieved from proxy driver successfully
 *          ERROR           Data not retrieved successfully
 */
int iRMI_HANDLER_GetSensorRequest( EVL_SIGNAL *pxSignal,
                                   RMI_HANDLER_SENSOR_REQUEST *pxRequest );


/* TODO : add API for config & events requests */

/**
 * @brief   Print all the stats gathered by the proxy driver
 *
 * @return  OK              Stats retrieved from proxy driver successfully
 *          ERROR           Stats not retrieved successfully
 * 
 */
int iRMI_HANDLER_PrintStatistics( void );

/**
 * @brief   Clear all the stats in the proxy driver
 *
 * @return  OK              Stats cleared successfully
 *          ERROR           Stats not cleared successfully
 * 
 */
int iRMI_HANDLER_ClearStatistics( void );

/**
 * @brief   Gets the current state of the handler
 *
 * @param   pxState         Pointer to the state
 *
 * @return  OK              If successful
 *          ERROR           If not successful
 */
int iRMI_HANDLER_GetState( MODULE_STATE *pxState );

#endif
