/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the fal profile for the V80
 *
 * @file profile_fal.h
 *
 */

#ifndef _PROFILE_FAL_H_
#define _PROFILE_FAL_H_

#include "util.h"
#include "pll.h"
#include "fw_if.h"

/* QSFP */
#define FW_IF_QSFP_PROCESS_TIME_MS                      ( 2 )
#define FW_IF_QSFP_PROCESS_TIME_TICKS                   ( 10 )
#define FW_IF_QSFP_MAX_DATA                             ( 256 )
#define FW_IF_QSFP_READ_DEFAULT_SIZE                    ( 1 )
#define FW_IF_QSFP_WRITE_DEFAULT_SIZE                   ( 2 )
#define FW_IF_QSFP_MODPRES_L_BIT_MASK                   ( 1 << 3 )
#define FW_IF_QSFP_MODSEL_L_BIT_MASK                    ( 1 << 0 )
#define FW_IF_QSFP_MODSELL_L_SET_LOW                    ( 0xFE )
#define FW_IF_QSFP_MODSELL_L_SET_HIGH                   ( 0x01 )
#define FW_IF_QSFP_POWER_IO_EXPANDER_NUM_INPUTS         ( 4 )
#define FW_IF_QSFP_ALL_OUTPUTS_HIGH                     ( 0xFF )
/* defines for IO expander registers */
#define FW_IF_QSFP_IO_EXPANDER_INPUT_PORT_REG           ( 0 )
#define FW_IF_QSFP_IO_EXPANDER_OUTPUT_PORT_REG          ( 1 )
#define FW_IF_QSFP_IO_EXPANDER_POLARITY_INVERSION_REG   ( 2 )
#define FW_IF_QSFP_IO_EXPANDER_CONFIGURATION_REG        ( 3 )
#define FW_IF_QSFP_MUX_IO_EXPANDER_DESELECTED           ( 0 )

/* OSPI */
#define FW_IF_OSPI_STATE_ENTRY( _s )    [ FW_IF_OSPI_STATE_ ## _s ] = #_s

/* FAL objects */
extern FW_IF_CFG xGcqIf;  
extern FW_IF_CFG xOspiIf; 
extern FW_IF_CFG xQsfpIf1;
extern FW_IF_CFG xQsfpIf2;
extern FW_IF_CFG xQsfpIf3;
extern FW_IF_CFG xQsfpIf4;
extern FW_IF_CFG xDimmIf;


/**
 * @brief   Initialise FAL layer
 *
 * @return  OK if all FAL objects initialised and created successfully
 *          ERROR if any or all FAL objects not initialised or created
 *
 */
int iFAL_Initialise( void );

#endif
