// SPDX-License-Identifier: GPL-2.0-only
/*
 * commands.h - This file contains all supported command handlers
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_APP_COMMANDS_H
#define AMI_APP_COMMANDS_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include "amiapp.h"

/*****************************************************************************/
/* Public                                                                    */
/*****************************************************************************/

/* "bar_rd" handler */
extern struct app_cmd cmd_bar_rd;

/* "bar_wr" handler */
extern struct app_cmd cmd_bar_wr;

/* "overview" handler */
extern struct app_cmd cmd_overview;

/* "pcieinfo" handler */
extern struct app_cmd cmd_pcieinfo;

/* "program" handler */
extern struct app_cmd cmd_cfgmem_program;

/* "sensors" handler */
extern struct app_cmd cmd_sensors;

/* "reload" handler */
extern struct app_cmd cmd_reload;

/* "device_boot" handler */
extern struct app_cmd cmd_device_boot;

/* "cfgmem_copy" handler */
extern struct app_cmd cmd_cfgmem_copy;

/* "cfgmem_info" handler */
extern struct app_cmd cmd_cfgmem_info;

/* "mfg_info" handler */
extern struct app_cmd cmd_mfg_info;

/* "eeprom_rd" handler */
extern struct app_cmd cmd_eeprom_rd;

/* "eeprom_wr" handler */
extern struct app_cmd cmd_eeprom_wr;

/* "cfgmem_fpt" handler */
extern struct app_cmd cmd_cfgmem_fpt;

/* "module_byte_rd" handler */
extern struct app_cmd cmd_module_byte_rd;

/* "module_byte_wr" handler */
extern struct app_cmd cmd_module_byte_wr;

#endif
