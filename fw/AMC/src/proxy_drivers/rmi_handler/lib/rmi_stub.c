/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>

#include "xil_printf.h"
#include "xstatus.h"
#include "rmi.h"
#include "FreeRTOS.h"
#include "task.h"

#include "fw_if.h"
#include "fw_if_smbus.h"

flags_t _x_rmi_flags = 
{
    .rmi_is_configured = false, 
    .rmi_sensors_is_configured = false,
    .rmi_board_info_is_ready = false,
    .reserved = 0
};

rmi_board_info_t x_rmi_board_info;
sensors_ds_t *px_rmi_sensors = NULL;
uint32_t ul_sensors_count = 0;
rmi_config_t x_rmi_cfg = { 0 };

FW_IF_CFG alveo_if_cfg;
FW_IF_SMBUS_INIT_CFG smbus_init_cfg = { 
    .baseAddr = XPAR_SMBUS_0_BASEADDR,
    .baudRate = SMBUS_FREQ_1MHZ
};

FW_IF_SMBUS_CFG alveo_smbus_cfg = {
    .port = SMBUS_ADDRESS,
    .role = FW_IF_SMBUS_ROLE_TARGET,
    .arpCapability = FW_IF_SMBUS_ARP_FIXED_DISCOVERABLE,
    .udid = {  commonUDID_DEVICE_CAPABILITIES , commonUDID_VERSION_OR_REVISION,
                commonSHIFT_RIGHT( commonUDID_VENDOR_ID, 8 ), commonSHIFT_RIGHT( commonUDID_VENDOR_ID, 0 ),
                commonSHIFT_RIGHT( commonUDID_DEVICE_ID, 8 ), commonSHIFT_RIGHT( commonUDID_DEVICE_ID, 0 ),
                commonSHIFT_RIGHT( commonUDID_INTERFACE, 8 ), commonSHIFT_RIGHT( commonUDID_INTERFACE, 0 ),
                commonSHIFT_RIGHT( commonUDID_SUBSYS_VENDOR_ID, 8 ), commonSHIFT_RIGHT( commonUDID_SUBSYS_VENDOR_ID, 0 ),
                commonSHIFT_RIGHT( commonUDID_SUBSYS_DEVICE_ID, 8 ), commonSHIFT_RIGHT( commonUDID_SUBSYS_DEVICE_ID, 0 ),
                commonSHIFT_RIGHT( commonUDID_VENDOR_SPECIFIC_ID, 24 ), commonSHIFT_RIGHT( commonUDID_VENDOR_SPECIFIC_ID, 16 ),
                commonSHIFT_RIGHT( commonUDID_VENDOR_SPECIFIC_ID, 8 ), commonSHIFT_RIGHT( commonUDID_VENDOR_SPECIFIC_ID, 0 ) }
};

static int32_t prvlFW_Interface_Init( void )
{
    int32_t err = eRMI_SUCCESS;

    err = ulFW_IF_SMBUS_Init( &smbus_init_cfg );

    if( FW_IF_ERRORS_NONE != err )
    {
        err = eRMI_ERROR_FW_IF;
        xil_printf( "fwif err init %d\r\n", err );
    }
    else
    {
        err = ulFW_IF_SMBUS_Create( &alveo_if_cfg, &alveo_smbus_cfg );

        if( FW_IF_ERRORS_NONE != err )
        {
            err = eRMI_ERROR_FW_IF;
            xil_printf( "fwif err create %d\r\n", err );
        }
        else
        {
            err = alveo_if_cfg.open( &alveo_if_cfg );

            if( FW_IF_ERRORS_NONE != err )
            {
                err = eRMI_ERROR_FW_IF;
                xil_printf( "fwif open err %d\r\n", err );
            }
            else
            {
                err = alveo_if_cfg.ioctrl( &alveo_if_cfg, FW_IF_SMBUS_IOCTRL_SET_TARGET, NULL );

                if( FW_IF_ERRORS_NONE != err )
                {
                    err = eRMI_ERROR_FW_IF;
                    xil_printf( "error changing smbus role %d\r\n", err );
                }
            }
        }
    }

    return err;
}

bool bCheck_Rmi_Config_Ok( rmi_config_t _x_rmi_cfg )
{
    bool b_Config_Ok = false;

    if( NULL != _x_rmi_cfg.rmi_malloc_fptr && NULL != _x_rmi_cfg.rmi_request_fptr && NULL != _x_rmi_cfg.rmi_free_fptr &&
        NULL != _x_rmi_cfg.rmi_memcpy_fptr && NULL != _x_rmi_cfg.rmi_memset_fptr && NULL != _x_rmi_cfg.rmi_memmove_fptr &&
        NULL != _x_rmi_cfg.rmi_memcmp_fptr && NULL != _x_rmi_cfg.rmi_strncpy_fptr && NULL != _x_rmi_cfg.rmi_strncmp_fptr )
    {
        b_Config_Ok = true;
    }

    return b_Config_Ok;
}

int32_t lRmi_Init( rmi_config_t _x_rmi_config )
{
    int32_t err = eRMI_SUCCESS;

    err = prvlFW_Interface_Init();

    if( eRMI_SUCCESS != err )
    {
        err = eRMI_ERROR_GENERIC;
    }
    else
    {
        x_rmi_cfg = _x_rmi_config;

        if( prvbCheck_Rmi_Config_Ok( x_rmi_cfg ) )
        {
            g_rmi_flags.rmi_is_configured = true;
        }
        else
        {
            err = eRMI_ERROR_GENERIC;
        }
    }

    return err;
}

int32_t lRmi_Configure_Sensors( sensors_ds_t* _px_sensors, uint32_t _ul_num_of_sensors )
{
    if ( NULL == _px_sensors || 0 == _ul_num_of_sensors )
    {
        return ( int32_t )eRMI_ERROR_GENERIC;
    }

    px_rmi_sensors = _px_sensors;
    ul_sensors_count = _ul_num_of_sensors;

    g_rmi_flags.rmi_sensors_is_configured = true;

    return ( int32_t )eRMI_SUCCESS;
}

int32_t lRmi_Write_Board_Info( rmi_board_info_t* _px_board_info, uint32_t _ul_size )
{
    if( NULL == _px_board_info || 0 == _ul_size )
    {
        return ( int32_t )eRMI_ERROR_GENERIC;
    }

    if( _ul_size != sizeof( x_rmi_board_info ) )
    {
        return ( int32_t )eRMI_ERROR_INVALID_SIZE;
    }

	memmove( ( void * )&x_rmi_board_info, ( const void * )_px_board_info, _ul_size );

	g_rmi_flags.x_rmi_board_info_is_ready = true;

    return ( int32_t )eRMI_SUCCESS;
}

static int32_t prvlSend_Sensor_Values( void )
{
    uint8_t tar_buf[SMBUS_DATA_SIZE_MAX] = { 0 };
    uint32_t err = ul_FW_IF_ERRORS_NONE;

    for( int i = 0; i < ul_sensors_count; ++i ){

        // Command code is first byte in buffer
        tar_buf[0] = *px_rmi_sensors[i].id;
        //TODO replace memcpy with Cl_SecureMemcpy
        memcpy( tar_buf + 1, px_rmi_sensors[i].value, *px_rmi_sensors[i].size );
        err = alveo_if_cfg.write( &alveo_if_cfg, 0x55, tar_buf, *px_rmi_sensors[i].size, 0 );

        if( ul_FW_IF_ERRORS_NONE != err )
        {
            xil_printf( "error writing smbus %d\r\n", err );
        }

        err = ul_FW_IF_ERRORS_NONE;
    }

    return err;
}

void vRmi_Task_Func( void )
{

    uint8_t tar_buf[SMBUS_DATA_SIZE_MAX] = { 0 };
    uint32_t ul_tar_size = 0;

    uint32_t err = ul_FW_IF_ERRORS_NONE;

    err = alveo_if_cfg.read( &alveo_if_cfg, 0x55, tar_buf, &ul_tar_size, FW_IF_TIMEOUT_NO_WAIT );
    if( ul_FW_IF_ERRORS_NONE == err )
    {
        // TODO Do something with read data
        xil_printf( "smbus read data 0x%x 0x%x %d\r\n", tar_buf[0], tar_buf[1], ul_tar_size );
    }
    else if( FW_IF_ERRORS_TIMEOUT == err )
    {
        // Timing out is expected to be frequent since we are polling
    }
    else
    {
        xil_printf( "error reading smbus %d\r\n", err );
    }

    prvlSend_Sensor_Values();

    return;
}