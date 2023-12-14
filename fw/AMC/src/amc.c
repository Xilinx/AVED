/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the main entry point for the Alveo Managment Controller
 *
 * @file amc.c
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

/* common includes */
#include "standard.h"
#include "util.h"
#include "event_id.h"
#include "amc_version.h"

/* osal */
#include "osal.h"

/* core_libs */
#include "pll.h"
#include "evl.h"

/* device drivers */
#include "i2c.h"
#include "eeprom.h"
#include "sys_mon.h"
#include "gcq.h"
#include "emmc.h"

/* fal */
#include "fw_if_test.h"
#include "fw_if_gcq.h"
#include "fw_if_ospi.h"
#include "fw_if_muxed_device.h"

/* proxy drivers */
#include "acc_proxy_driver.h"
#include "axc_proxy_driver.h"
#include "apc_proxy_driver.h"
#include "asc_proxy_driver.h"
#include "ami_proxy_driver.h"
#include "rmi_handler.h"

/* bim app data */
#include "profile_bim.h"

/* apps */
#include "asdm.h"
#include "in_band_telemetry.h"
#include "out_of_band_telemetry.h"
#include "bim.h"

/* sensor data */
#include "profile_sensors.h"

/* hardware definitions */
#include "profile_clocks.h"
#include "profile_hal.h"
#include "profile_fal.h"
#include "profile_muxed_device.h"

/* debug monitoring */
#ifdef DEBUG_BUILD
#include "osal_debug.h"
#include "dal.h"
#include "pll_debug.h"
#include "dal_debug.h"
#include "evl_debug.h"
#include "ina3221_debug.h"
#include "isl68221_debug.h"
#include "cat34ts02_debug.h"
#include "eeprom_debug.h"
#include "sys_mon_debug.h"
#include "gcq_debug.h"
#include "ospi_debug.h"
#include "i2c_debug.h"
#include "emmc_debug.h"
#include "fw_if_gcq_debug.h"
#include "fw_if_ospi_debug.h"
#include "fw_if_muxed_device_debug.h"
#include "acc_proxy_driver_debug.h"
#include "ami_proxy_driver_debug.h"
#include "apc_proxy_driver_debug.h"
#include "axc_proxy_driver_debug.h"
#include "asc_proxy_driver_debug.h"
#include "rmi_handler_debug.h"
#include "asdm_debug.h"
#include "in_band_telemetry_debug.h"
#include "bim_debug.h"
#endif


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define AMC_OUTPUT_LEVEL                    ( PLL_OUTPUT_LEVEL_DEBUG )

#define AMC_NAME                            "AMC"

#define AMC_HASH_LEN                        ( 7 )
#define AMC_DATE_LEN                        ( 8 )

#define AMC_TASK_DEFAULT_STACK              ( 0x1000 )
#define AMC_PROXY_NAME_LEN                  ( 15 )

#define AMC_TASK_SLEEP_MS                   ( 100 )


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    AMC_TASK_PRIOS
 * @brief   AMC Task priorities
 */
typedef enum AMC_TASK_PRIOS
{
    AMC_TASK_PRIO_RSVD = 5, /* TODO: get actual value from osal.h */

    AMC_TASK_PRIO_DEFAULT,

    MAX_AMC_TASK_PRIO

} AMC_TASK_PRIOS;


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/


/******************************************************************************/
/* EVL Callback Declarations                                                  */
/******************************************************************************/

/**
 * @brief   EVL Callbacks for binding to Proxy Drivers
 *
 * @param   pxSignal     Event raised
 *
 * @return  OK if no errors were raised in the callback
 *          ERROR if an error was raised in the callback
 *
 */
static int iApcCallback( EVL_SIGNAL *pxSignal );
static int iAmiCallback( EVL_SIGNAL *pxSignal );
static int iRmiHandlerCallback( EVL_SIGNAL *pxSignal );

#if ( 0 != HAL_AMC_CLOCK_CONTROL )
static int iAccCallback( EVL_SIGNAL *pxSignal );
#endif

static int iAxcCallback( EVL_SIGNAL *pxSignal );


/******************************************************************************/
/* Local Function Declarations                                                */
/******************************************************************************/

/**
 * @brief   Get project info
 *
 * @return  N/A
 *
 */
static void vGetProjectInfo( void );

/**
 * @brief   Initialise core libraries
 *
 * @return  OK if all core libraries initialised successfully
 *          ERROR if any or all core libraries not initialised
 *
 */
static int iInitCoreLibs( void );

/**
 * @brief   Initialise device drivers
 *
 * @return  OK if all device drivers initialised and created successfully
 *          ERROR if any or all device drivers not initialised
 *
 */
static int iInitDeviceDrivers( void );

/**
 * @brief   Initialise Proxy Driver layer
 *
 * @return  OK if all Proxy Drivers initialised and bound successfully
 *          ERROR if any or all proxy drivers not initialised
 *
 */
static int iInitProxies( void );

/**
 * @brief   Initialise App layer
 *
 * @return  OK if all Apps initialised and created successfully
 *          ERROR if any or all apps not initialised
 *
 */
static int iInitApp( void );

/**
 * @brief   Initialise Debug monitoring
 *
 * @return  OK if debug initialised (or if debug disabled)
 *          ERROR if debug enabled but did not initialise
 */
static int iInitDebug( void );

/**
 * @brief   The main task that init the FAL & proxy drivers
 *
 * @return  N/A
 */
static void vTaskFuncMain( void );

/**
 * @brief   Configure the partition table stored at the start of
 *          shared memory and used by the AMI to deremine the AMC state
 * @return  N/A
 */
static void vConfigurePartitionTable( void );


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

/* Note: the default I2C clock frequency isn't used */
static I2C_CFG_TYPE xI2cCfg[ I2C_NUM_INSTANCES ] = { { HAL_I2C_BUS_0_DEVICE_ID,
                                                       ( uint64_t )HAL_I2C_BUS_0_BASEADDR,
                                                       HAL_I2C_BUS_0_I2C_CLK_FREQ_HZ,
                                                       HAL_I2C_RETRY_COUNT },
                                                     { HAL_I2C_BUS_1_DEVICE_ID,
                                                       ( uint64_t )HAL_I2C_BUS_1_BASEADDR,
                                                       HAL_I2C_BUS_1_I2C_CLK_FREQ_HZ,
                                                       HAL_I2C_RETRY_COUNT }, };
static EEPROM_CFG xEepromCfg = {
                    HAL_EEPROM_I2C_BUS,
                    HAL_EEPROM_SLAVE_ADDRESS,
                    HAL_EEPROM_ADDRESS_SIZE,
                    HAL_EEPROM_PAGE_SIZE,
                    HAL_EEPROM_NUM_PAGES,
                    HAL_EEPROM_DEVICE_ID_ADDRESS,
                    HAL_EEPROM_DEVICE_ID_REGISTER,
                    HAL_EEPROM_DEVICE_ID };

/* AXC External Device configs */
AXC_PROXY_DRIVER_EXTERNAL_DEVICE_CONFIG xQsfpDevice1 = { &xQsfpIf1, 1 };
AXC_PROXY_DRIVER_EXTERNAL_DEVICE_CONFIG xQsfpDevice2 = { &xQsfpIf2, 2 };
AXC_PROXY_DRIVER_EXTERNAL_DEVICE_CONFIG xQsfpDevice3 = { &xQsfpIf3, 3 };
AXC_PROXY_DRIVER_EXTERNAL_DEVICE_CONFIG xQsfpDevice4 = { &xQsfpIf4, 4 };
AXC_PROXY_DRIVER_EXTERNAL_DEVICE_CONFIG xDimmDevice  = { &xDimmIf, 5 };


/******************************************************************************/
/* Function implementations                                                   */
/******************************************************************************/

/*
 * @brief   The main task
 */
static void vTaskFuncMain( void )
{
    int iStatus = ERROR;

    vConfigurePartitionTable();

    if( OK == iInitCoreLibs() )
    {
        PLL_DBG( AMC_NAME, "Core Libs initialised OK\r\n" );
        iStatus = OK;
    }
    else
    {
        PLL_ERR( AMC_NAME, "Core Libs initialisation ERROR\t\n" );
    }

    if( OK == iStatus )
    {
        if( OK == iInitDeviceDrivers() )
        {
            PLL_DBG( AMC_NAME, "Device drivers Initialised OK\r\n" );
            iStatus = OK;
        }
        else
        {
            PLL_ERR( AMC_NAME, "Device drivers Initialisation ERROR\r\n" );
        }
    }

    if( OK == iStatus )
    {
        if( OK == iFAL_Initialise() )
        {
            PLL_DBG( AMC_NAME, "FAL Initialised OK\r\n" );

            if( OK == iInitProxies() )
            {
                PLL_DBG( AMC_NAME, "Proxy Drivers Initialised OK\r\n" );
                iStatus = OK;
            }
            else
            {
                PLL_ERR( AMC_NAME, "Proxy Drivers Initialisation ERROR\r\n" );
            }
        }
        else
        {
            PLL_ERR( AMC_NAME, "FAL Initialisation ERROR\r\n" );
        }
    }

    if( OK == iStatus )
    {
        if( OK == iInitApp() )
        {
            PLL_DBG( AMC_NAME, "Apps Initialisated OK\r\n" );
        }
        else
        {
            PLL_ERR( AMC_NAME, "Apps Initialisation ERROR\r\n" );
            iStatus = ERROR;
        }
    }

    /* always initialise debug interface (if enabled) */
    if( OK != iInitDebug() )
    {
        PLL_ERR( AMC_NAME, "Debug access initialisation error\r\n" );
    }

    if( ERROR == iStatus )
    {
        /*
         * The final step before starting the main task is to configure the start
         * of the shared memory with the information needed by the AMI.
        */
        PLL_ERR( AMC_NAME, "Error Main Task has initialisation failures\r\n" );
    }

    FOREVER
    {
        iOSAL_Task_SleepMs( AMC_TASK_SLEEP_MS );
    }
}

/**
 * @brief   Main entry point
 */
int main( void )
{
    vGetProjectInfo();

    void *pvMainTaskHandle = NULL;
    if( OSAL_ERRORS_OS_NOT_STARTED != iOSAL_StartOS( TRUE,
                                                     &pvMainTaskHandle,
                                                     &vTaskFuncMain,
                                                     AMC_TASK_DEFAULT_STACK,
                                                     AMC_TASK_PRIO_DEFAULT ) )
    {
        PLL_ERR( AMC_NAME, "Error failed to start the OS Task\r\n" );
    }

    return -1;
}


/******************************************************************************/
/* EVL Callback Implementations                                               */
/******************************************************************************/

/**
 * @brief   ACC Proxy Driver EVL callback
 */
#if ( 0 != HAL_AMC_CLOCK_CONTROL )
static int iAccCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_ACC == pxSignal->ucModule ) )
    {
        /* PLL_DBG( AMC_NAME, "ACC Proxy Driver (0x%02X), Instance %d, ", pxSignal->ucModule, pxSignal->ucInstance ); */
        switch( pxSignal->ucEventType )
        {

        case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ENABLED:
        {
            PLL_DBG( AMC_NAME, "Event ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ENABLED (0x%02X)\r\n", pxSignal->ucEventType );
            iStatus = OK;
            break;
        }

        case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_DISABLED:
        {
            PLL_DBG( AMC_NAME, "Event ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_DISABLED (0x%02X)\r\n", pxSignal->ucEventType );
            iStatus = OK;
            break;
        }

        case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ACTIVATED:
        {
            PLL_DBG( AMC_NAME, "Event ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_ACTIVATED (0x%02X)\r\n", pxSignal->ucEventType );
            iStatus = OK;
            break;
        }

        case ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_CLOCK_REENABLED:
        {
            PLL_DBG( AMC_NAME, "Event ACC_PROXY_DRIVER_E_CLOCK_SHUTDOWN_CLOCK_REENABLED (0x%02X)\r\n", pxSignal->ucEventType );
            iStatus = OK;
            break;
        }
        default:
            break;
        }
    }

    return iStatus;
}
#endif

/**
 * @brief   AXC Proxy Driver EVL callback
 */
static int iAxcCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_AXC == pxSignal->ucModule ) )
    {
        switch( pxSignal->ucEventType )
        {
        case AXC_PROXY_DRIVER_E_QSFP_PRESENT:
        {
            iStatus = OK;
            break;
        }
        case AXC_PROXY_DRIVER_E_QSFP_NOT_PRESENT:
        {
            iStatus = OK;
            break;
        }
        default:
            break;
        }
    }

    return iStatus;
}

/**
 * @brief   APC Proxy Driver EVL callback
 */
static int iApcCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_APC == pxSignal->ucModule ) )
    {
        switch( pxSignal->ucEventType )
        {
        case APC_PROXY_DRIVER_E_DOWNLOAD_STARTED:
        {
            iStatus = OK;
            break;
        }
        case APC_PROXY_DRIVER_E_DOWNLOAD_COMPLETE:
        {
            iStatus = iAMI_SetPdiDownloadCompleteResponse( pxSignal, AMI_PROXY_RESULT_SUCCESS );
            break;
        }
        case APC_PROXY_DRIVER_E_DOWNLOAD_FAILED:
        {
            iStatus = iAMI_SetPdiDownloadCompleteResponse( pxSignal, AMI_PROXY_RESULT_FAILURE );
            break;
        }
        case APC_PROXY_DRIVER_E_DOWNLOAD_BUSY:
        {
            iStatus = iAMI_SetPdiDownloadCompleteResponse( pxSignal, AMI_PROXY_RESULT_ALREADY_IN_PROGRESS );
            break;
        }
        case APC_PROXY_DRIVER_E_COPY_STARTED:
        {
            iStatus = OK;
            break;
        }
        case APC_PROXY_DRIVER_E_COPY_COMPLETE:
        {
            iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_SUCCESS );
            break;
        }
        case APC_PROXY_DRIVER_E_COPY_FAILED:
        {
            iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_FAILURE );
            break;
        }
        case APC_PROXY_DRIVER_E_COPY_BUSY:
        {
            iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_ALREADY_IN_PROGRESS );
            break;
        }
        case APC_PROXY_DRIVER_E_PARTITION_SELECTED:
        {
            iStatus = iAMI_SetBootSelectCompleteResponse( pxSignal, AMI_PROXY_RESULT_SUCCESS );
            break;
        }
        case APC_PROXY_DRIVER_E_PARTITION_SELECTION_FAILED:
        {
            iStatus = iAMI_SetBootSelectCompleteResponse( pxSignal, AMI_PROXY_RESULT_FAILURE );
            break;
        }
        default:
            break;
        }
    }

    return iStatus;
}

/**
 * @brief   AMI Proxy Driver EVL callback
 */
static int iAmiCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_AMI == pxSignal->ucModule ) )
    {
        switch( pxSignal->ucEventType )
        {
        case AMI_PROXY_DRIVER_E_PDI_DOWNLOAD_START:
        {
            AMI_PROXY_PDI_DOWNLOAD_REQUEST xDownloadRequest = { 0 };
            PLL_DBG( AMC_NAME, "Event PDI Download Start (0x%02X)\r\n", pxSignal->ucEventType );

            if( OK == iAMI_GetPdiDownloadRequest( pxSignal, &xDownloadRequest ) )
            {
                PLL_DBG( AMC_NAME, "PDI download address : 0x%llx\r\n", xDownloadRequest.ullAddress );
                PLL_DBG( AMC_NAME, "PDI download length  : 0x%x\r\n",   xDownloadRequest.ulLength );
                PLL_DBG( AMC_NAME, "PDI partition        : 0x%x\r\n",   xDownloadRequest.ulPartitionSel );
                PLL_DBG( AMC_NAME, "PDI has FPT          : 0x%x\r\n",   xDownloadRequest.iUpdateFpt );
                PLL_DBG( AMC_NAME, "PDI last packet      : 0x%x\r\n",   xDownloadRequest.iLastPacket );
                PLL_DBG( AMC_NAME, "PDI packet number    : 0x%hx\r\n",  xDownloadRequest.usPacketNum );
                PLL_DBG( AMC_NAME, "PDI packet size (KB) : 0x%hx\r\n",  xDownloadRequest.usPacketSize );

                if( TRUE == xDownloadRequest.iUpdateFpt )
                {
                    iStatus = iAPC_UpdateFpt( pxSignal,
                                              ( uint32_t )xDownloadRequest.ullAddress + ( uint32_t )HAL_RPU_SHARED_MEMORY_BASE_ADDR,
                                              xDownloadRequest.ulLength,
                                              xDownloadRequest.usPacketNum,
                                              xDownloadRequest.usPacketSize,
                                              xDownloadRequest.iLastPacket );
                }
                else
                {
                    iStatus = iAPC_DownloadImage( pxSignal,
                                                  ( int )xDownloadRequest.ulPartitionSel,
                                                  ( uint32_t )xDownloadRequest.ullAddress + ( uint32_t )HAL_RPU_SHARED_MEMORY_BASE_ADDR,
                                                  xDownloadRequest.ulLength,
                                                  xDownloadRequest.usPacketNum,
                                                  xDownloadRequest.usPacketSize );
                }

                if( OK != iStatus )
                {
                    iStatus = iAMI_SetPdiDownloadCompleteResponse( pxSignal, AMI_PROXY_RESULT_PROCESS_REQUEST_FAILED );
                }
            }
            else
            {
                iStatus = iAMI_SetPdiDownloadCompleteResponse( pxSignal, AMI_PROXY_RESULT_GET_REQUEST_FAILED );
            }

            if( OK != iStatus)
            {
                PLL_ERR( AMC_NAME, "Error downloading to partition %d\r\n", xDownloadRequest.ulPartitionSel );
            }
            break;
        }
        case AMI_PROXY_DRIVER_E_PDI_COPY_START:
        {
            AMI_PROXY_PDI_COPY_REQUEST xCopyRequest = { 0 };
            PLL_DBG( AMC_NAME, "Event PDI Copy Start (0x%02X)\r\n", pxSignal->ucEventType );

            if( OK == iAMI_GetPdiCopyRequest( pxSignal, &xCopyRequest ) )
            {
                PLL_DBG( AMC_NAME, "PDI copy address     : 0x%llx\r\n", xCopyRequest.ullAddress );
                PLL_DBG( AMC_NAME, "PDI max copy length  : 0x%x\r\n",  xCopyRequest.ulMaxLength );
                PLL_DBG( AMC_NAME, "PDI src partition    : 0x%x\r\n",  xCopyRequest.ulSrcPartition );
                PLL_DBG( AMC_NAME, "PDI dst partition    : 0x%x\r\n",  xCopyRequest.ulDestPartition );
                if( OK == iAPC_CopyImage( pxSignal,
                                          ( int )xCopyRequest.ulSrcPartition,
                                          ( int )xCopyRequest.ulDestPartition,
                                          ( uint32_t )xCopyRequest.ullAddress + ( uint32_t )HAL_RPU_SHARED_MEMORY_BASE_ADDR,
                                          xCopyRequest.ulMaxLength ) )
                {
                    iStatus = OK;
                }
                else
                {
                    iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_PROCESS_REQUEST_FAILED );
                }
            }
            else
            {
                iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_GET_REQUEST_FAILED );
            }

            if( OK != iStatus)
            {
                PLL_ERR( AMC_NAME, "Error copying p%d to p%d\r\n",
                         xCopyRequest.ulSrcPartition,
                         xCopyRequest.ulDestPartition );
            }
            break;
        }
        case AMI_PROXY_DRIVER_E_GET_IDENTITY:
        {
            PLL_DBG( AMC_NAME, "Event Get Identity (0x%02X)\r\n", pxSignal->ucEventType );
            AMI_PROXY_RESULT xResult = AMI_PROXY_RESULT_SUCCESS;

            GCQ_VERSION_TYPE xGcqVersion = { 0 };
            if( OK != iGCQGetVersion( &xGcqVersion ) )
            {
                PLL_DBG( AMC_NAME, "Error getting GCQ version\r\n" );
                xResult = AMI_PROXY_RESULT_FAILURE;
            }

            AMI_PROXY_IDENTITY_RESPONSE xIdentityResponse =
            {
                .ucVerMajor       = ( uint8_t )GIT_TAG_VER_MAJOR,
                .ucVerMinor       = ( uint8_t )GIT_TAG_VER_MINOR,
                .ucVerPatch       = ( uint8_t )GIT_TAG_VER_PATCH,
                .ucLocalChanges   = ( uint8_t )( GIT_STATUS )?( 1 ):( 0 ),
                .usDevCommits     = ( uint16_t )GIT_TAG_VER_DEV_COMMITS,
                .ucLinkVerMajor   = xGcqVersion.ucVerMajor,
                .ucLinkVerMinor   = xGcqVersion.ucVerMinor
            };
            iStatus = iAMI_SetIdentityResponse( pxSignal, xResult, &xIdentityResponse );

            /* AMI is ready - enable hot reset */
            if( OK == iAPC_EnableHotReset( pxSignal ) )
            {
                PLL_DBG( AMC_NAME, "Hot reset enabled\r\n" );
            }

            break;
        }
        case AMI_PROXY_DRIVER_E_BOOT_SELECT:
        {
            AMI_PROXY_BOOT_SELECT_REQUEST xBootSelRequest = { 0 };
            PLL_DBG( AMC_NAME, "Event Boot Select Request (0x%02X)\r\n", pxSignal->ucEventType );

            if( OK == iAMI_GetBootSelectRequest( pxSignal, &xBootSelRequest ) )
            {
                PLL_DBG( AMC_NAME, "Selecting partition %d\r\n", xBootSelRequest.ulPartitionSel );
                if( OK == iAPC_SetNextPartition( pxSignal, ( int )xBootSelRequest.ulPartitionSel ) )
                {
                    iStatus = OK;
                }
                else
                {
                    iStatus = iAMI_SetBootSelectCompleteResponse( pxSignal, AMI_PROXY_RESULT_PROCESS_REQUEST_FAILED );
                }
            }
            else
            {
                iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_GET_REQUEST_FAILED );
            }

            if( OK != iStatus )
            {
                PLL_ERR( AMC_NAME, "Error selecting partition %d\r\n", xBootSelRequest.ulPartitionSel );
            }
            break;
        }
        default:
            iStatus = OK;
            break;
        }
    }

    return iStatus;
}

/**
 * @brief   RMI Handler EVL callback
 */
static int iRmiHandlerCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( ( NULL != pxSignal ) && ( AMC_EVENT_UNIQUE_ID_RMI_HANDLER == pxSignal->ucModule ) )
    {
        switch( pxSignal->ucEventType )
        {
        default:
            break;
        }
    }

    return iStatus;
}


/**
 * @brief   Get project info
 */
static void vGetProjectInfo( void )
{
    char pcOsName[ OSAL_OS_NAME_LEN ] = "unknown";
    uint8_t ucVerMaj = 0, ucVerMin = 0, ucVerBld = 0;

    iOSAL_GetOsVersion( pcOsName, &ucVerMaj, &ucVerMin, &ucVerBld );

    vPLL_Printf( "\r\n" );
    vPLL_Printf( "###############################################################\r\n" );
    vPLL_Printf( "#                                                             #\r\n" );
    vPLL_Printf( "#                             AMC                             #\r\n" );
    vPLL_Printf( "#                                                             #\r\n" );
    vPLL_Printf( "# Copyright (c) 2023 Advanced Micro Devices, Inc.             #\r\n" );
    vPLL_Printf( "# All rights reserved.                                        #\r\n" );
    vPLL_Printf( "#                                                             #\r\n" );
    vPLL_Printf( "# SPDX-License-Identifier: MIT                                #\r\n" );
    vPLL_Printf( "#                                                             #\r\n" );
    vPLL_Printf( "###############################################################\r\n" );
    vPLL_Printf( " AMC: %d.%d.%d-%d.%.*s.%.*s%c\r\n",
                 GIT_TAG_VER_MAJOR,
                 GIT_TAG_VER_MINOR,
                 GIT_TAG_VER_PATCH,
                 GIT_TAG_VER_DEV_COMMITS,
                 AMC_HASH_LEN, GIT_HASH,
                 AMC_DATE_LEN, GIT_DATE,
                 ( GIT_STATUS )?( '*' ):( ' ' ) );
    vPLL_Printf( " OS:  %s v%u.%u.%u\r\n",
                 pcOsName,
                 ucVerMaj,
                 ucVerMin,
                 ucVerBld );
    vPLL_Printf( "\r\n\r\n" );
}

/**
 * @brief   Initialise core libraries
 */
static int iInitCoreLibs( void )
{
    int iStatus = ERROR;

    if( OK == iPLL_Initialise( AMC_OUTPUT_LEVEL ) )
    {
        PLL_DBG( AMC_NAME, "PLL initialised OK\r\n" );
        iStatus = OK;
    }
    else
    {
        PLL_ERR( AMC_NAME, "PLL initialisation ERROR\r\n" );
    }

    if( OK == iEVL_Initialise() )
    {
        PLL_DBG( AMC_NAME, "EVL initialised OK\r\n" );
        iStatus = OK;
    }
    else
    {
        PLL_ERR( AMC_NAME, "EVL initialisation ERROR\r\n" );
    }

    return iStatus;
}

/**
 * @brief   Initialise device drivers
 */
static int iInitDeviceDrivers( void )
{
    int iStatus = OK;

    if( OK == iI2C_Init( xI2cCfg, I2C_DEFAULT_BUS_IDLE_WAIT_MS ) )
    {
        PLL_DBG( AMC_NAME, "I2C driver Initialised OK\r\n" );
    }
    else
    {
        PLL_ERR( AMC_NAME, "I2C driver Initialisation ERROR\r\n" );
        iStatus = ERROR;
    }

    if( OK == iEEPROM_Initialise( HAL_EEPROM_VERSION, &xEepromCfg ) )
    {
        PLL_DBG( AMC_NAME, "iEEPROM_Initialised OK\r\n" );

        if( ERROR == iEEPROM_DisplayEepromValues( ) )
        {
            PLL_ERR( AMC_NAME, "iEEPROM_DisplayEepromValues FAILED\r\n" );
        }
    }
    else
    {
        PLL_ERR( AMC_NAME, "iEEPROM_Initialised FAILED\r\n" );
    }

    if( OK == iSYS_MON_Initialise() )
    {
        PLL_DBG( AMC_NAME, "SysMon Driver Initialised OK\r\n" );
    }
    else
    {
        PLL_ERR( AMC_NAME, "SysMon Driver Initialisation ERROR\r\n" );
        iStatus = ERROR;
    }

#if( 0 != HAL_EMMC_FEATURE )
    if( OK == iEMMC_Initialise( HAL_EMMC_DEVICE_ID ) )
    {
        PLL_DBG( AMC_NAME, "EMMC Driver Initialised OK\r\n" );
    }
    else
    {
        PLL_ERR( AMC_NAME, "EMMC Driver Initialisation ERROR\r\n" );
        iStatus = ERROR;
    }
#endif

    return iStatus;
}

/**
 * @brief   Initialise Proxy Driver layer
 */
static int iInitProxies( void )
{
    int iStatus = OK;

    if( OK == iAPC_Initialise( AMC_EVENT_UNIQUE_ID_APC, &xOspiIf, AMC_TASK_PRIO_DEFAULT, AMC_TASK_DEFAULT_STACK ) )
    {
        if( OK == iAPC_BindCallback( &iApcCallback ) )
        {
            PLL_DBG( AMC_NAME, "APC Proxy Driver initialised and bound\r\n" );
        }
        else
        {
            PLL_ERR( AMC_NAME, "Error binding to APC Proxy Driver\r\n" );
        }
    }
    else
    {
        PLL_ERR( AMC_NAME, "Error initialising APC Proxy Driver\r\n" );
        iStatus = ERROR;
    }

    if( 0 != MAX_NUM_EXTERNAL_DEVICES_AVAILABLE )
    {
        if( OK == iAXC_Initialise( AMC_EVENT_UNIQUE_ID_AXC, AMC_TASK_PRIO_DEFAULT, AMC_TASK_DEFAULT_STACK ) )
        {
            if( OK == iAXC_BindCallback( &iAxcCallback ) )
            {
                if( ( OK == iAXC_AddExternalDevice( &xQsfpDevice1 ) ) &&
                    ( OK == iAXC_AddExternalDevice( &xQsfpDevice2 ) ) &&
                    ( OK == iAXC_AddExternalDevice( &xQsfpDevice3 ) ) &&
                    ( OK == iAXC_AddExternalDevice( &xQsfpDevice4 ) ) &&
                    ( OK == iAXC_AddExternalDevice( &xDimmDevice ) ) )
                {
                    PLL_DBG( AMC_NAME, "AXC Proxy Driver initialised and bound\r\n" );
                }
                else
                {
                    PLL_ERR( AMC_NAME, "Error adding External Device to AXC Proxy Driver\r\n" );
                }
            }
            else
            {
                PLL_ERR( AMC_NAME, "Error binding to AXC Proxy Driver\r\n" );
                iStatus = ERROR;
            }
        }
        else
        {
            PLL_ERR( AMC_NAME, "Error initialising AXC Proxy Driver\r\n" );
            iStatus = ERROR;
        }
    }
    else
    {
        PLL_DBG( AMC_NAME, "No external devices available - skipping AXC initialisation\r\n" );
    }

    if( OK == iASC_Initialise( AMC_EVENT_UNIQUE_ID_ASC, AMC_TASK_PRIO_DEFAULT, AMC_TASK_DEFAULT_STACK, PROFILE_SENSORS_SENSOR_DATA, PROFILE_SENSORS_NUM_SENSORS ) )
    {
        PLL_DBG( AMC_NAME, "ASC Proxy Driver initialised\r\n" );
    }
    else
    {
        PLL_ERR( AMC_NAME, "Error initialising ASC Proxy Driver\r\n" );
        iStatus = ERROR;
    }

#if ( 0 != HAL_AMC_CLOCK_CONTROL )
    if( OK == iACC_Initialise( AMC_EVENT_UNIQUE_ID_ACC, AMC_TASK_PRIO_DEFAULT, AMC_TASK_DEFAULT_STACK,
                                &xShutdownSensors, ( void * )HAL_USER_CLOCK_CONTROL_BASE_ADDRESS ) )
    {
        if( OK == iACC_BindCallback( &iAccCallback ) )
        {
            PLL_DBG( AMC_NAME, "ACC Proxy Driver initialised and bound\r\n" );
        }
        else
        {
            PLL_ERR( AMC_NAME, "Error binding to ACC Proxy Driver\r\n" );
        }
    }
    else
    {
        PLL_ERR( AMC_NAME, "Error initialising ACC Proxy Driver\r\n" );
        iStatus = ERROR;
    }
#endif

    if( OK == iAMI_Initialise( AMC_EVENT_UNIQUE_ID_AMI, &xGcqIf, 0, AMC_TASK_PRIO_DEFAULT, AMC_TASK_DEFAULT_STACK ) )
    {
        if( OK == iAMI_BindCallback( &iAmiCallback ) )
        {
            PLL_DBG( AMC_NAME, "AMI Proxy Driver initialised and bound\r\n" );
        }
        else
        {
            PLL_ERR( AMC_NAME, "Error binding to AMI Proxy Driver\r\n" );
        }
    }
    else
    {
        PLL_ERR( AMC_NAME, "Error initialising AMI Proxy Driver\r\n" );
        iStatus = ERROR;
    }

    if( OK == iRMI_HANDLER_Initialise( AMC_EVENT_UNIQUE_ID_RMI_HANDLER, AMC_TASK_PRIO_DEFAULT, AMC_TASK_DEFAULT_STACK ) )
    {
        if( OK == iRMI_HANDLER_BindCallback( &iRmiHandlerCallback ) )
        {
            PLL_DBG( AMC_NAME, "RMI Handler initialised and bound\r\n" );
        }
        else
        {
            PLL_ERR( AMC_NAME, "Error binding to RMI Handler\r\n" );
        }
    }
    else
    {
        PLL_ERR( AMC_NAME, "Error initialising RMI Handler\r\n" );
        iStatus = ERROR;
    }

    return iStatus;
}

/**
 * @brief   Initialise App layer
 */
static int iInitApp( void )
{
    int iStatus = OK;

    if( OK != iASDM_Initialise( PROFILE_SENSORS_NUM_SENSORS ) )
    {
        PLL_ERR( AMC_NAME, "ASDM Initialisation ERROR\r\n" );
        iStatus = ERROR;
    }

    if( OK != iIN_BAND_TELEMETRY_Initialise( HAL_RPU_SHARED_MEMORY_BASE_ADDR ) )
    {
        PLL_ERR( AMC_NAME, "In Band Telemetry Initialisation ERROR\r\n" );
        iStatus = ERROR;
    }
    
    if( OK != iOUT_OF_BAND_TELEMETRY_Initialise() )
    {
        PLL_ERR( AMC_NAME, "Out of Band Telemetry Initialisation ERROR\r\n" );
        iStatus = ERROR;
    }

    if( OK != iBIM_Initialise( PROFILE_BIM_MODULE_DATA ) )
    {
        PLL_ERR( AMC_NAME, "Built in Monitoring Initialisation ERROR\r\n" );
        iStatus = ERROR;
    }

    return iStatus;
}

/**
 * @brief   Initialise Debug monitoring
 */
static int iInitDebug( void )
{
    int iStatus = OK;

#ifdef DEBUG_BUILD
    if( OK != iDAL_Initialise( "AMC_Debug", AMC_TASK_PRIO_DEFAULT, AMC_TASK_DEFAULT_STACK ) )
    {
        PLL_ERR( AMC_NAME, "Unable to initialise debug menu\r\n" );
        iStatus = ERROR;
    }
    else
    {
        /* top level directories */
        static DAL_HDL pxDeviceDrivers   = NULL;
        static DAL_HDL pxCoreLibsTop     = NULL;
        static DAL_HDL pxFwIfTop         = NULL;
        static DAL_HDL pxProxyDriversTop = NULL;
        static DAL_HDL pxAppsTop         = NULL;

        /* osal */
        vOSAL_DebugInit();

        /* device drivers - TODO */
        pxDeviceDrivers = pxDAL_NewDirectory( "device_drivers" );

        vINA3221_DebugInit( pxDeviceDrivers );
        vISL68221_DebugInit( pxDeviceDrivers );
        vCAT34TS02_DebugInit( pxDeviceDrivers );
        vSYS_MON_DebugInit( pxDeviceDrivers );
        vEeprom_DebugInit( pxDeviceDrivers );
        vOSPI_DebugInit( pxDeviceDrivers );
        vI2C_DebugInit( pxDeviceDrivers );
#if ( 0 != HAL_EMMC_FEATURE )
        vEMMC_DebugInit( pxDeviceDrivers );
#endif
        /* core libraries - TODO */
        pxCoreLibsTop = pxDAL_NewDirectory( "core_libs" );

        vPLL_DebugInit( pxCoreLibsTop );
        vDAL_DebugInit( pxCoreLibsTop );
        vEVL_DebugInit( pxCoreLibsTop );

        /* FALs */
        pxFwIfTop = pxDAL_NewDirectory( "fw_if" );

        vFW_IF_GCQ_DebugInit( pxFwIfTop );
        vFW_IF_OSPI_DebugInit( pxFwIfTop );
        if( 0 != MAX_NUM_EXTERNAL_DEVICES_AVAILABLE )
        {
            vFW_IF_MUXED_DEVICE_DebugInit( pxFwIfTop );
        }

        /* proxy drivers */
        pxProxyDriversTop = pxDAL_NewDirectory( "proxy_drivers" );

        vAMI_DebugInit( pxProxyDriversTop );
        vAPC_DebugInit( pxProxyDriversTop );
        vASC_DebugInit( pxProxyDriversTop );
#if ( 0 != HAL_AMC_CLOCK_CONTROL )
        vACC_DebugInit( pxProxyDriversTop );
#endif
        if( 0 != MAX_NUM_EXTERNAL_DEVICES_AVAILABLE )
        {
            vAXC_DebugInit( pxProxyDriversTop );
        }
        vRMI_HANDLER_DebugInit( pxProxyDriversTop );

        /* apps */
        pxAppsTop = pxDAL_NewDirectory( "apps" );

        vASDM_DebugInit( pxAppsTop );
        vIN_BAND_TELEMETRY_DebugInit( pxAppsTop );
        vBIM_DebugInit( pxAppsTop );

        /* test application - TODO */

        PLL_DBG( AMC_NAME, "Debug initialised\r\n" );
    }
#endif

    return iStatus;
}

/**
 * @brief   Configure the partition table stored at the start of
 *          shared memory and used by the AMI to determine the AMC state
 */
static void vConfigurePartitionTable( void )
{
    HAL_PARTITION_TABLE xPartTable = { 0 };
    uint8_t *pucDestAdd = NULL;

    xPartTable.ulMagicNum = HAL_PARTITION_TABLE_MAGIC_NO;
    xPartTable.xRingBuffer.ulRingBufferOff = HAL_PARTITION_TABLE_SIZE;
    xPartTable.xRingBuffer.ulRingBufferLen = HAL_RPU_RING_BUFFER_LEN;
    xPartTable.xStatus.ulStatusOff =  HAL_PARTITION_TABLE_SIZE + HAL_RPU_RING_BUFFER_LEN;
    xPartTable.xStatus.ulStatusLen = sizeof( uint32_t );
    xPartTable.xLogMsg.ulLogMsgIndex = 0;
    xPartTable.xLogMsg.ulLogMsgBufferOff = xPartTable.xStatus.ulStatusOff + xPartTable.xStatus.ulStatusLen;
    xPartTable.xLogMsg.ulLogMsgBufferLen = PLL_LOG_BUF_LEN;
    xPartTable.xData.ulDataStart = xPartTable.xLogMsg.ulLogMsgBufferOff + xPartTable.xLogMsg.ulLogMsgBufferLen;
    xPartTable.xData.ulDataEnd = HAL_RPU_SHARED_MEMORY_SIZE;

    /* Copy the populated table into the start of shared memory */
    pucDestAdd = ( uint8_t* )( HAL_RPU_SHARED_MEMORY_BASE_ADDR );
    pvOSAL_MemCpy( pucDestAdd, ( uint8_t* )&xPartTable, sizeof( xPartTable ) );
    HAL_FLUSH_CACHE_DATA( HAL_RPU_SHARED_MEMORY_BASE_ADDR, sizeof( xPartTable ) );

    /* Flush stale logs */
    if( PLL_LOG_BUF_LEN >= xPartTable.xLogMsg.ulLogMsgBufferLen )
    {
        pvOSAL_MemSet( ( uint8_t* )( HAL_RPU_SHARED_MEMORY_BASE_ADDR + xPartTable.xLogMsg.ulLogMsgBufferOff ), 0, xPartTable.xLogMsg.ulLogMsgBufferLen );
        HAL_FLUSH_CACHE_DATA( HAL_RPU_SHARED_MEMORY_BASE_ADDR + xPartTable.xLogMsg.ulLogMsgBufferOff, xPartTable.xLogMsg.ulLogMsgBufferLen );
    }

    /*
     * AMI is waiting for the status to be set to a value of 0x1, currently we have no
     * concept of stopping/starting the AMC so once initialised this will always be valid
     */
    pucDestAdd = ( uint8_t* )( HAL_RPU_SHARED_MEMORY_BASE_ADDR + xPartTable.xStatus.ulStatusOff );
    pvOSAL_MemSet( pucDestAdd, HAL_ENABLE_AMI_COMMS, xPartTable.xStatus.ulStatusLen );
    HAL_FLUSH_CACHE_DATA( ( HAL_RPU_SHARED_MEMORY_BASE_ADDR + xPartTable.xStatus.ulStatusOff ), xPartTable.xStatus.ulStatusLen );
}
