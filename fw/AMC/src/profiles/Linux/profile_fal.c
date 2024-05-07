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

#ifdef DEBUG_BUILD
#include "fw_if_gcq_debug.h"
#include "fw_if_ospi_debug.h"
#include "fw_if_emmc_debug.h"
#include "fw_if_muxed_device_debug.h"
#include "fw_if_smbus_debug.h"
#endif


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define FAL_PROFILE_NAME           "PROFILE_FAL"
#define OSPI_PAGE_SIZE             ( 256 )
#define OSPI_RPU_BASE_ADDRESS      ( 0x0 )
#define OSPI_RPU_LENGTH            ( 0x10000000 )                              /* 2Gb (256MB) */
#define OSPI_TEST_CONFIG_NAME      "OSPI TEST"
#define OSPI_TEST_CONFIG_NAME_SIZE ( 10 )
#define OSPI_TEST_CONFIG_ID        ( 0 )
#define OSPI_TEST_DRIVER_ID        ( 0 )

#define FPT_STUB_HEADER_OFFSET ( 0 )
#define FPT_STUB_HEADER_SIZE   ( 12 )
#define FPT_STUB_P1_OFFSET     ( 128 )
#define FPT_STUB_PART1_SIZE    ( 12 )
#define FPT_STUB_P2_OFFSET     ( 256 )
#define FPT_STUB_PART2_SIZE    ( 12 )
#define TEST_BUFFER_SIZE       ( 512 )


/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/* FAL objects */
FW_IF_CFG xGcqIf =
{
    0
};
FW_IF_CFG xOspiIf =
{
    0
};
FW_IF_CFG xQsfpIf1 =
{
    0
};
FW_IF_CFG xQsfpIf2 =
{
    0
};
FW_IF_CFG xQsfpIf3 =
{
    0
};
FW_IF_CFG xQsfpIf4 =
{
    0
};
FW_IF_CFG xDimmIf =
{
    0
};
FW_IF_CFG *pxSMBusIf = NULL;
FW_IF_CFG *pxEmmcIf  = NULL;
FW_IF_CFG *pxOspiIf  = &xOspiIf;


/*****************************************************************************/
/* Local variables                                                           */
/*****************************************************************************/

static FW_IF_TEST_CFG xOspiTestCfg =
{
    0
};
static FW_IF_TEST_INIT_CFG xTestInitCfg =
{
    0
};
static FW_IF_MUXED_DEVICE_CFG xQsfpCfg1 =
{
    FW_IF_DEVICE_QSFP,
    QSFP_POWER_IO_EXPANDER_I2C_ADDR,
    QSFP_1_REG_BIT_OFFSET,
    MUXED_DEVICE_MUX_1_ADDR,
    {
        MUXED_DEVICE_MUX_2_ADDR, MUXED_DEVICE_MUX_3_ADDR
    },
    MUXED_DEVICE_MUX_REG_BIT_1,
    MUXED_DEVICE_MUX_REG_BIT_2,
    QSFP_IO_EXPANDER_I2C_ADDR,
    QSFP_I2C_ADDR,
    FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP
};                                                                             /* QSFP 1 */
static FW_IF_MUXED_DEVICE_CFG xQsfpCfg2 =
{
    FW_IF_DEVICE_QSFP,
    QSFP_POWER_IO_EXPANDER_I2C_ADDR,
    QSFP_2_REG_BIT_OFFSET,
    MUXED_DEVICE_MUX_1_ADDR,
    {
        MUXED_DEVICE_MUX_2_ADDR, MUXED_DEVICE_MUX_3_ADDR
    },
    MUXED_DEVICE_MUX_REG_BIT_3,
    MUXED_DEVICE_MUX_REG_BIT_4,
    QSFP_IO_EXPANDER_I2C_ADDR,
    QSFP_I2C_ADDR,
    FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP
};                                                                             /* QSFP 2 */
static FW_IF_MUXED_DEVICE_CFG xQsfpCfg3 =
{
    FW_IF_DEVICE_QSFP,
    QSFP_POWER_IO_EXPANDER_I2C_ADDR,
    QSFP_3_REG_BIT_OFFSET,
    MUXED_DEVICE_MUX_2_ADDR,
    {
        MUXED_DEVICE_MUX_1_ADDR, MUXED_DEVICE_MUX_3_ADDR
    },
    MUXED_DEVICE_MUX_REG_BIT_1,
    MUXED_DEVICE_MUX_REG_BIT_2,
    QSFP_IO_EXPANDER_I2C_ADDR,
    QSFP_I2C_ADDR,
    FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP
};                                                                             /* QSFP 3 */
static FW_IF_MUXED_DEVICE_CFG xQsfpCfg4 =
{
    FW_IF_DEVICE_QSFP,
    QSFP_POWER_IO_EXPANDER_I2C_ADDR,
    QSFP_4_REG_BIT_OFFSET,
    MUXED_DEVICE_MUX_2_ADDR,
    {
        MUXED_DEVICE_MUX_1_ADDR, MUXED_DEVICE_MUX_3_ADDR
    },
    MUXED_DEVICE_MUX_REG_BIT_3,
    MUXED_DEVICE_MUX_REG_BIT_4,
    QSFP_IO_EXPANDER_I2C_ADDR,
    QSFP_I2C_ADDR,
    FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP
};                                                                             /* QSFP 4 */
static FW_IF_MUXED_DEVICE_CFG xDimmCfg =
{
    FW_IF_DEVICE_DIMM,
    0,
    0,
    MUXED_DEVICE_MUX_3_ADDR,
    {
        MUXED_DEVICE_MUX_1_ADDR, MUXED_DEVICE_MUX_2_ADDR
    },
    MUXED_DEVICE_MUX_REG_BIT_4,
    0,
    0,
    DIMM_I2C_ADDR,
    FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP
};                                                                             /* DIMM */
static FW_IF_GCQ_CFG xGcqCfg =
{
    ( uint64_t )HAL_BASE_LOGIC_GCQ_M2R_S01_AXI_BASEADDR,
    FW_IF_GCQ_MODE_PRODUCER,
    FW_IF_GCQ_INTERRUPT_MODE_NONE,
    ( uint64_t )HAL_RPU_RING_BUFFER_BASE,
    HAL_RPU_RING_BUFFER_LEN,
    AMI_PROXY_RESPONSE_SIZE,
    AMI_PROXY_REQUEST_SIZE,
    ""
};

static FW_IF_GCQ_INIT_CFG myGcqIf =
{
    NULL
};

static FW_IF_OSPI_INIT_CFG myOspiIf =
{
    HAL_OSPI_0_DEVICE_ID,
    OSPI_PAGE_SIZE
};

static FW_IF_MUXED_DEVICE_INIT_CFG myQsfpIf =
{
    "QsfpIfCfg",
    HAL_I2C_BUS_1_DEVICE_ID
};

static APC_PROXY_DRIVER_FPT_HEADER xFptHeaderStub =
{
    0x92F7A516, 0x01, 0x80, 0x80, 0x02
};
static APC_PROXY_DRIVER_FPT_PARTITION xFptPt1Stub =
{
    0x00000E00, 0x00008000, 0x07E00000
};
static APC_PROXY_DRIVER_FPT_PARTITION xFptPt2Stub =
{
    0x00000E00, 0x07E08000, 0x07E00000
};


/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise FAL layer
 */
int iFAL_Initialise( uint64_t *pullAmcInitStatus )
{
    int     iStatus            = OK;
    int     iMuxedDeviceStatus = OK;
    uint8_t *pucTestConfigName = NULL;
    uint8_t *pucTestBuffer     = NULL;

    if( NULL != pullAmcInitStatus )
    {
        /* Init the Muxed Device FAL */
        if( AMC_CFG_I2C_INITIALISED == ( *pullAmcInitStatus & AMC_CFG_I2C_INITIALISED ) )
        {
            if( 0 != MAX_NUM_EXTERNAL_DEVICES_AVAILABLE )
            {
                if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Init( &myQsfpIf ) )
                {
                    PLL_DBG( FAL_PROFILE_NAME, "Muxed Device FAL initialised OK\r\n" );
                    *pullAmcInitStatus |= AMC_CFG_MUXED_DEVICE_FAL_INITIALISED;
                }
                else
                {
                    PLL_ERR( FAL_PROFILE_NAME, "Error initialising Muxed Device FAL\r\n" );
                    iStatus = ERROR;
                }
            }
        }

        /* Init the GCQ FAL */
        PLL_LOG( FAL_PROFILE_NAME, "GCQ service: starting\r\n" );
        if( FW_IF_ERRORS_NONE == ulFW_IF_GCQ_Init( &myGcqIf ) )
        {
            PLL_DBG( FAL_PROFILE_NAME, "GCQ FAL initialised OK\r\n" );
            PLL_LOG( FAL_PROFILE_NAME, "GCQ service: ready\r\n" );
            *pullAmcInitStatus |= AMC_CFG_GCQ_FAL_INITIALISED;
        }
        else
        {
            PLL_ERR( FAL_PROFILE_NAME, "Error initialising GCQ FAL\r\n" );
            PLL_LOG( FAL_PROFILE_NAME, "GCQ service: error initialising\r\n" );
            iStatus = ERROR;
        }

        /* Init the OPSI FAL */
        PLL_LOG( FAL_PROFILE_NAME, "OSPI driver: starting\r\n" );
        if( FW_IF_ERRORS_NONE == ulFW_IF_OSPI_Init( &myOspiIf ) )
        {
            PLL_DBG( FAL_PROFILE_NAME, "OSPI FAL initialised OK\r\n" );
            PLL_LOG( FAL_PROFILE_NAME, "OSPI driver: ready\r\n" );
            *pullAmcInitStatus |= AMC_CFG_OSPI_FAL_INITIALISED;
        }
        else
        {
            PLL_ERR( FAL_PROFILE_NAME, "Error initialising OSPI FAL\r\n" );
            PLL_LOG( FAL_PROFILE_NAME, "OSPI driver: error initialising\r\n" );
            iStatus = ERROR;
        }

        if( AMC_CFG_MUXED_DEVICE_FAL_INITIALISED == ( *pullAmcInitStatus & AMC_CFG_MUXED_DEVICE_FAL_INITIALISED ) )
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
                    iStatus            = ERROR;
                    iMuxedDeviceStatus = ERROR;
                }

                if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xQsfpIf2, &xQsfpCfg2 ) )
                {
                    PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
                }
                else
                {
                    PLL_ERR( FAL_PROFILE_NAME, "Error creating QSFP FAL handle\r\n" );
                    iStatus            = ERROR;
                    iMuxedDeviceStatus = ERROR;
                }

                if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xQsfpIf3, &xQsfpCfg3 ) )
                {
                    PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
                }
                else
                {
                    PLL_ERR( FAL_PROFILE_NAME, "Error creating QSFP FAL handle\r\n" );
                    iStatus            = ERROR;
                    iMuxedDeviceStatus = ERROR;
                }

                if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xQsfpIf4, &xQsfpCfg4 ) )
                {
                    PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
                }
                else
                {
                    PLL_ERR( FAL_PROFILE_NAME, "Error creating QSFP FAL handle\r\n" );
                    iStatus            = ERROR;
                    iMuxedDeviceStatus = ERROR;
                }

                if( FW_IF_ERRORS_NONE == ulFW_IF_MUXED_DEVICE_Create( &xDimmIf, &xDimmCfg ) )
                {
                    PLL_DBG( FAL_PROFILE_NAME, "ulFW_IF_MUXED_DEVICE_Create created OK\r\n" );
                }
                else
                {
                    PLL_ERR( FAL_PROFILE_NAME, "Error creating DIMM FAL handle\r\n" );
                    iStatus            = ERROR;
                    iMuxedDeviceStatus = ERROR;
                }

                if( OK == iMuxedDeviceStatus )
                {
                    *pullAmcInitStatus |= AMC_CFG_MUXED_DEVICE_FAL_CREATED;
                }
            }
            else
            {
                *pullAmcInitStatus |= AMC_CFG_MUXED_DEVICE_FAL_CREATED;
            }
        }

        /* Create instance of the GCQ based on the global configuration */
        if( AMC_CFG_GCQ_FAL_INITIALISED == ( *pullAmcInitStatus & AMC_CFG_GCQ_FAL_INITIALISED ) )
        {
            if( FW_IF_ERRORS_NONE == ulFW_IF_GCQ_Create( &xGcqIf, &xGcqCfg ) )
            {
                PLL_DBG( FAL_PROFILE_NAME, "GCQ created OK\r\n" );
                *pullAmcInitStatus |= AMC_CFG_GCQ_FAL_CREATED;
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error creating GCQ\r\n" );
                iStatus = ERROR;
            }
        }

        /* Create instance of the OSPI based on the test configuration */
        if( AMC_CFG_OSPI_FAL_INITIALISED == ( *pullAmcInitStatus & AMC_CFG_OSPI_FAL_INITIALISED ) )
        {
            pucTestConfigName = pvOSAL_MemAlloc( OSPI_TEST_CONFIG_NAME_SIZE );
            if( NULL != pucTestConfigName )
            {
                PLL_DBG( FAL_PROFILE_NAME, "Test Config Name set OK\r\n" );
                *pullAmcInitStatus |= AMC_CFG_OSPI_FAL_CREATED;
                pvOSAL_MemSet( pucTestConfigName, 0x00, OSPI_TEST_CONFIG_NAME_SIZE );
                sprintf( ( char * )pucTestConfigName, OSPI_TEST_CONFIG_NAME );
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error memory allocation for Test Config Name\r\n" );
                iStatus = ERROR;
            }

            pucTestBuffer = pvOSAL_MemAlloc( TEST_BUFFER_SIZE );
            if( NULL != pucTestBuffer )
            {
                PLL_DBG( FAL_PROFILE_NAME, "Test Buffer created OK\r\n" );
                pvOSAL_MemCpy( pucTestBuffer + FPT_STUB_HEADER_OFFSET,
                               &xFptHeaderStub,
                               sizeof( APC_PROXY_DRIVER_FPT_HEADER ) );
                pvOSAL_MemCpy( pucTestBuffer + FPT_STUB_P1_OFFSET,
                               &xFptPt1Stub,
                               sizeof( APC_PROXY_DRIVER_FPT_PARTITION ) );
                pvOSAL_MemCpy( pucTestBuffer + FPT_STUB_P2_OFFSET,
                               &xFptPt2Stub,
                               sizeof( APC_PROXY_DRIVER_FPT_PARTITION ) );
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "Error memory allocation for Test Buffer\r\n" );
                iStatus = ERROR;
            }

            if( ERROR != iStatus )
            {
                /* Test Init Config settings*/
                xTestInitCfg.driverId   = OSPI_TEST_DRIVER_ID;
                xTestInitCfg.driverName = ( char * )pucTestConfigName;
                xTestInitCfg.debugPrint = TRUE;

                /* OSPI Test Config settings*/
                xOspiTestCfg.ifId   = OSPI_TEST_CONFIG_ID;
                xOspiTestCfg.ifName = ( char * )pucTestConfigName;

                /* OSPI Test initialisation */
                if( FW_IF_ERRORS_NONE == FW_IF_test_init( &xTestInitCfg ) )
                {
                    PLL_DBG( FAL_PROFILE_NAME, "OSPI Test initialisation OK\r\n" );
                    if( FW_IF_ERRORS_NONE == FW_IF_test_create( &xOspiIf, &xOspiTestCfg ) )
                    {
                        PLL_DBG( FAL_PROFILE_NAME, "OSPI Test created OK\r\n" );
                        xOspiIf.ioctrl( &xOspiIf, FW_IF_TEST_IOCTRL_SET_NEXT_RX_DATA, pucTestBuffer );
                    }
                    else
                    {
                        PLL_ERR( FAL_PROFILE_NAME, "Error creating OSPI Test\r\n" );
                        iStatus = ERROR;
                    }
                }
                else
                {
                    PLL_ERR( FAL_PROFILE_NAME, "Error initialisation OSPI Test\r\n" );
                    iStatus = ERROR;
                }
            }
            else
            {
                PLL_ERR( FAL_PROFILE_NAME, "OSPI Test failed\r\n" );
                iStatus = ERROR;
            }
        }
    }
    else
    {
        iStatus = ERROR;
    }

    return iStatus;

}

/**
 * @brief   Initialise FAL Debug monitoring
 */
void vFAL_DebugInitialise( void )
{
#ifdef DEBUG_BUILD
    /* FALs */
    static DAL_HDL pxFwIfTop = NULL;
    pxFwIfTop = pxDAL_NewDirectory( "fw_if" );

    vFW_IF_GCQ_DebugInit( pxFwIfTop );
    vFW_IF_OSPI_DebugInit( pxFwIfTop );
    vFW_IF_EMMC_DebugInit( pxFwIfTop );

    if( 0 != MAX_NUM_EXTERNAL_DEVICES_AVAILABLE )
    {
        vFW_IF_MUXED_DEVICE_DebugInit( pxFwIfTop );
    }

    vFW_IF_SMBUS_DebugInit( pxFwIfTop );
#endif
}
