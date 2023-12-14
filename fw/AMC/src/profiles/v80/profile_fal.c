/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the fal profile for the Linux platform
 *
 * @file profile_fal.c
 *
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* core libs */
#include "pll.h"

/* fal */
#include "fw_if_muxed_device.h"
#include "fw_if_gcq.h"
#include "fw_if_ospi.h"
#include "fw_if_test.h"

/* proxy drivers*/
#include "apc_proxy_driver.h"
#include "ami_proxy_driver.h"

/* hardware definitions */
#include "profile_fal.h"
#include "profile_hal.h"
#include "profile_muxed_device.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FAL_PROFILE_NAME                        "PROFILE_FAL"
#define OSPI_PAGE_SIZE                      ( 256 )
#define OSPI_RPU_BASE_ADDRESS               ( 0x0 )
#define OSPI_RPU_LENGTH                     ( 0x10000000 )   /* 2Gb (256MB) */


/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/* FAL objects */
FW_IF_CFG xGcqIf    = { 0 };
FW_IF_CFG xOspiIf   = { 0 };
FW_IF_CFG xQsfpIf1  = { 0 };
FW_IF_CFG xQsfpIf2  = { 0 };
FW_IF_CFG xQsfpIf3  = { 0 };
FW_IF_CFG xQsfpIf4  = { 0 };
FW_IF_CFG xDimmIf   = { 0 };


/*****************************************************************************/
/* Local variables                                                           */
/*****************************************************************************/

static FW_IF_MUXED_DEVICE_CFG xQsfpCfg1 = { FW_IF_DEVICE_QSFP,
                                            QSFP_POWER_IO_EXPANDER_I2C_ADDR,
                                            QSFP_1_REG_BIT_OFFSET,
                                            MUXED_DEVICE_MUX_1_ADDR,
                                            { MUXED_DEVICE_MUX_2_ADDR, MUXED_DEVICE_MUX_3_ADDR },
                                            MUXED_DEVICE_MUX_REG_BIT_1,
                                            MUXED_DEVICE_MUX_REG_BIT_2,
                                            QSFP_IO_EXPANDER_I2C_ADDR,
                                            QSFP_I2C_ADDR,
                                            FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP }; /* QSFP 1 */
static FW_IF_MUXED_DEVICE_CFG xQsfpCfg2 = { FW_IF_DEVICE_QSFP,
                                            QSFP_POWER_IO_EXPANDER_I2C_ADDR,
                                            QSFP_2_REG_BIT_OFFSET,
                                            MUXED_DEVICE_MUX_1_ADDR,
                                            { MUXED_DEVICE_MUX_2_ADDR, MUXED_DEVICE_MUX_3_ADDR },
                                            MUXED_DEVICE_MUX_REG_BIT_3,
                                            MUXED_DEVICE_MUX_REG_BIT_4,
                                            QSFP_IO_EXPANDER_I2C_ADDR,
                                            QSFP_I2C_ADDR,
                                            FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP }; /* QSFP 2 */
static FW_IF_MUXED_DEVICE_CFG xQsfpCfg3 = { FW_IF_DEVICE_QSFP,
                                            QSFP_POWER_IO_EXPANDER_I2C_ADDR,
                                            QSFP_3_REG_BIT_OFFSET,
                                            MUXED_DEVICE_MUX_2_ADDR,
                                            { MUXED_DEVICE_MUX_1_ADDR, MUXED_DEVICE_MUX_3_ADDR },
                                            MUXED_DEVICE_MUX_REG_BIT_1,
                                            MUXED_DEVICE_MUX_REG_BIT_2,
                                            QSFP_IO_EXPANDER_I2C_ADDR,
                                            QSFP_I2C_ADDR,
                                            FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP }; /* QSFP 3 */
static FW_IF_MUXED_DEVICE_CFG xQsfpCfg4 = { FW_IF_DEVICE_QSFP,
                                            QSFP_POWER_IO_EXPANDER_I2C_ADDR,
                                            QSFP_4_REG_BIT_OFFSET,
                                            MUXED_DEVICE_MUX_2_ADDR,
                                            { MUXED_DEVICE_MUX_1_ADDR, MUXED_DEVICE_MUX_3_ADDR },
                                            MUXED_DEVICE_MUX_REG_BIT_3,
                                            MUXED_DEVICE_MUX_REG_BIT_4,
                                            QSFP_IO_EXPANDER_I2C_ADDR,
                                            QSFP_I2C_ADDR,
                                            FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP }; /* QSFP 4 */
static FW_IF_MUXED_DEVICE_CFG xDimmCfg  = { FW_IF_DEVICE_DIMM,
                                            0,
                                            0,
                                            MUXED_DEVICE_MUX_3_ADDR,
                                            { MUXED_DEVICE_MUX_1_ADDR, MUXED_DEVICE_MUX_2_ADDR },
                                            MUXED_DEVICE_MUX_REG_BIT_4,
                                            0,
                                            0,
                                            DIMM_I2C_ADDR,
                                            FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP }; /* DIMM */
static FW_IF_GCQ_CFG xGcqCfg    = { ( uint64_t )HAL_BASE_LOGIC_GCQ_M2R_S01_AXI_BASEADDR,
                                    FW_IF_GCQ_MODE_PRODUCER,
                                    FW_IF_GCQ_INTERRUPT_MODE_NONE,
                                    ( uint64_t )HAL_RPU_RING_BUFFER_BASE,
                                    HAL_RPU_RING_BUFFER_LEN,
                                    AMI_PROXY_RESPONSE_SIZE,
                                    AMI_PROXY_REQUEST_SIZE,
                                    "" };

static FW_IF_GCQ_INIT_CFG myGcqIf      = { NULL };

static FW_IF_OSPI_CFG xOspiCfg  = { OSPI_RPU_BASE_ADDRESS,
                                    OSPI_RPU_LENGTH,
                                    TRUE,   /* Enable erase before write */
                                    FW_IF_OSPI_STATE_INIT };

static FW_IF_OSPI_INIT_CFG myOspiIf    = { HAL_OSPI_0_DEVICE_ID,
                                           OSPI_PAGE_SIZE };

static FW_IF_MUXED_DEVICE_INIT_CFG myQsfpIf    = { "QsfpIfCfg",
                                                    HAL_I2C_BUS_1_DEVICE_ID };


/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise FAL layer
 */
int iFAL_Initialise( void )
{
    int iStatus = OK;

    /* Init the Muxed Device FAL */
    if( 0 != MAX_NUM_EXTERNAL_DEVICES_AVAILABLE )
    {
        if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Init( &myQsfpIf ) )
        {
            PLL_DBG( FAL_PROFILE_NAME, "Muxed Device FAL initialised OK\r\n" );
        }
        else
        {
            PLL_ERR( FAL_PROFILE_NAME, "Error initialising Muxed Device FAL\r\n" );
            iStatus = ERROR;
        }
    }

    /* Init the GCQ FAL */
    if( FW_IF_ERRORS_NONE == ulFW_IF_GCQ_Init( &myGcqIf ) )
    {
        PLL_DBG( FAL_PROFILE_NAME, "GCQ FAL initialised OK\r\n" );
    }
    else
    {
        PLL_ERR( FAL_PROFILE_NAME, "Error initialising GCQ FAL\r\n" );
        iStatus = ERROR;
    }

    /* Init the OPSI FAL */
    if( FW_IF_ERRORS_NONE == ulFW_IF_OSPI_Init( &myOspiIf ) )
    {
        PLL_DBG( FAL_PROFILE_NAME, "OSPI FAL initialised OK\r\n" );
    }
    else
    {
        PLL_ERR( FAL_PROFILE_NAME, "Error initialising OSPI FAL\r\n" );
        iStatus = ERROR;
    }

    if( OK == iStatus )
    {
        if( 0 != MAX_NUM_EXTERNAL_DEVICES_AVAILABLE )
        {
            if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xQsfpIf1, &xQsfpCfg1 ) )
            {
                PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error creating QSFP FAL handle\r\n" );
                iStatus = ERROR;
            }

            if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xQsfpIf2, &xQsfpCfg2 ) )
            {
                PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error creating QSFP FAL handle\r\n" );
                iStatus = ERROR;
            }

            if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xQsfpIf3, &xQsfpCfg3 ) )
            {
                PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error creating QSFP FAL handle\r\n" );
                iStatus = ERROR;
            }

            if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xQsfpIf4, &xQsfpCfg4 ) )
            {
                PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error creating QSFP FAL handle\r\n" );
                iStatus = ERROR;
            }

            if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xDimmIf, &xDimmCfg ) )
            {
                PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error creating DIMM FAL handle\r\n" );
                iStatus = ERROR;
            }
        }

        /* Create instance of the GCQ based on the global configuration */
        if( FW_IF_ERRORS_NONE == ulFW_IF_GCQ_Create( &xGcqIf, &xGcqCfg ) )
        {
            PLL_DBG( FAL_PROFILE_NAME, "GCQ created OK\r\n" );
        }
        else
        {
            PLL_ERR( FAL_PROFILE_NAME, "Error creating GCQ\r\n" );
            iStatus = ERROR;
        }

        /* Create instance of the OSPI based on the global configuration */
        if( FW_IF_ERRORS_NONE == ulFW_IF_OSPI_Create( &xOspiIf, &xOspiCfg ) )
        {
            PLL_DBG( FAL_PROFILE_NAME, "OSPI created OK\r\n" );
        }
        else
        {
            PLL_ERR( FAL_PROFILE_NAME, "Error creating OSPI\r\n" );
            iStatus = ERROR;
        }
    }

    return iStatus;
}
