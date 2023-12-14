/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the event definitions for the AMC
 *
 * @file event_id.h
 *
 */

#ifndef _EVENT_ID_H_
#define _EVENT_ID_H_


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    AMC_EVENT_UNIQUE_IDS
 * @brief   Unique ID of each proxy driver
 */
typedef enum AMC_EVENT_UNIQUE_IDS
{
    AMC_EVENT_UNIQUE_ID_ACC,
    AMC_EVENT_UNIQUE_ID_AXC,
    AMC_EVENT_UNIQUE_ID_APC,
    AMC_EVENT_UNIQUE_ID_ASC,
    AMC_EVENT_UNIQUE_ID_AMI,
    AMC_EVENT_UNIQUE_ID_RMI_HANDLER,

    MAX_AMC_EVENT_UNIQUE_ID

} AMC_EVENT_UNIQUE_IDS;


#endif /* _EVENT_ID_H_ */
