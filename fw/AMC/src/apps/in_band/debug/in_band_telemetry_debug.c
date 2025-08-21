/**
 * Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the in band telemetry debug implementation
 *
 * @file in_band_telemetry_debug.c
 *
 */

/* common includes */
#include "standard.h"
#include "util.h"
#include "pll.h"
#include "amc_cfg.h"

/* profile */
#include "profile_hal.h"

/* core_libs */
#include "evl.h"
#include "dal.h"

/* proxy drivers */
#include "ami_proxy_driver.h"

/* apps */
#include "asdm.h"
#include "in_band_telemetry_debug.h"
#include "in_band_telemetry.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define AMC_IN_BAND_DBG_NAME          "AMC_IN_BAND_DBG"
#define SENSOR_RESP_BUFFER_SIZE       ( 512 )
#define MOCKED_HDR_SIZE               ( 5 )
#define ASDM_RECORD_FIELD_TYPE_POS    ( 0x06 )
#define EOR_SIZE                      ( 3 )
#define MOCKET_FPT_HDR_SIZE           ( 37 )
#define SENSOR_NAME_SIZE              ( 15 )
#define SDR_NAME_SIZE                 ( 40 )
#define TEST_TEMP_SENSOR_ID           ( 1 )
#define TEST_VOLTAGE_SENSOR_ID        ( 1 )
#define TEST_CURRENT_SENSOR_ID        ( 1 )
#define TEST_POWER_SENSOR_ID          ( 1 )
#define TEST_TOTAL_POWER_SENSOR_ID    ( 1 )
#define MOCKED_BOARD_INFO_LENGTH      ( 269 )
#define MOCKED_SENSOR_SDR_DATA_LENGTH ( 61 )
#define MOCKED_SENSOR_DATA_LENGTH     ( 17 )


/******************************************************************************/
/* Structs                                                                     */
/******************************************************************************/

/**
 * @struct  IN_BAND_DEBUG_PRIVATE_DATA
 * @brief   Structure to hold ths in band telemetry debug private data
 */
typedef struct IN_BAND_DEBUG_PRIVATE_DATA
{
    int      iIsInitialised;
    uint64_t ullSharedMemBaseAddr;
    int      iInBandTestMode;
    int      iAmiTestCallbackBounded;

} IN_BAND_DEBUG_PRIVATE_DATA;


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static IN_BAND_DEBUG_PRIVATE_DATA xLocalData =
{
    FALSE,  /* iIsInitialised */
    0,      /* ullSharedMemBaseAddr*/
    FALSE,  /* iInBandTestMode */
    FALSE   /* iAmiTestCallbackBounded*/

};
static IN_BAND_DEBUG_PRIVATE_DATA *pxThis = &xLocalData;

static DAL_HDL pxInBandTop = NULL;


/******************************************************************************/
/* Private function declarations                                              */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats
 *
 * @return  N/A
 */
static void vPrintStats( void );

/**
 * @brief   Debug function to clear this module's stats
 *
 * @return  N/A
 */
static void vClearStats( void );

/**
 * @brief   Debug function to activate/deactivate test mode
 *
 * @return  N/A
 */
static void vInBandTelemetryTestMode( void );

/**
 * @brief   EVL Callbacks for AMI testing
 *
 * @param   pxSignal     Event raised
 *
 * @return  OK if no errors were raised in the callback
 *          ERROR if an error was raised in the callback
 */
static int iAmiCallbackTestMode( EVL_SIGNAL *pxSignal );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the in band telemetry application debug access
 */
void vIN_BAND_TELEMETRY_DebugInit( DAL_HDL pxParentHandle, uint64_t ullSharedMemBaseAddr )
{
    if( FALSE == pxThis->iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxInBandTop = pxDAL_NewDirectory( "in_band_telemetry" );
        }
        else
        {
            pxInBandTop = pxDAL_NewSubDirectory( "in_band_telemetry", pxParentHandle );
        }

        if( NULL != pxInBandTop )
        {
            pxDAL_NewDebugFunction( "print_stats", pxInBandTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats", pxInBandTop, vClearStats );
            pxDAL_NewDebugFunction( "test_mode", pxInBandTop, vInBandTelemetryTestMode );
        }

        pxThis->ullSharedMemBaseAddr = ullSharedMemBaseAddr;
        pxThis->iIsInitialised       = TRUE;
    }
}

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void )
{
    if( OK != iIN_BAND_TELEMETRY_PrintStatistics() )
    {
        PLL_DAL( AMC_IN_BAND_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iIN_BAND_TELEMETRY_ClearStatistics() )
    {
        PLL_DAL( AMC_IN_BAND_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to change AMI Callback test mode
 */
static void vInBandTelemetryTestMode( void )
{
    pxThis->iInBandTestMode = !pxThis->iInBandTestMode;

    if( OK == iIN_BAND_TELEMETRY_TestMode( pxThis->iInBandTestMode ) )
    {
        PLL_DAL( AMC_IN_BAND_DBG_NAME, "Test mode is %s\r\n", pxThis->iInBandTestMode ? "activated" : "deactivated" );
        if( FALSE == pxThis->iAmiTestCallbackBounded )
        {
            if( OK == iAMI_BindCallback( &iAmiCallbackTestMode ) )
            {
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "AMI Test Callback bound\r\n" );
                pxThis->iAmiTestCallbackBounded = TRUE;
            }
            else
            {
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "Error bounding AMI Test Callback\r\n" );
            }
        }
    }
    else
    {
        pxThis->iInBandTestMode = !pxThis->iInBandTestMode;
        PLL_DAL( AMC_IN_BAND_DBG_NAME, "Error changing AMI Test Mode\r\n" );
    }
}

/**
 * @brief   AMI Proxy Driver Test Mode EVL callback
 */
static int iAmiCallbackTestMode( EVL_SIGNAL *pxSignal )
{
    int iStatus = TRUE;

    /**
     * All the mocked datas are in a form
     * For the sdr data |Operation Success (1Byte)|Header (5 Byte)| Payload (Varying length) | EOR (3 Byte)|
     * For the sensor data |Operation Success (1Byte)|Header (5 Byte)| Payload (Varying length)
     */
    static uint8_t pucMockedFptData[ MOCKET_FPT_HDR_SIZE ] =
    {
        0x01, 0xF0, 0x01, 0x01, 0x0C, 0x00, 0x1, 0x80, 0x80, 0x2, 0x0, 0xE, 0x0, 0x0, 0x0, 0x80, 0x0, 0x0, 0x0, 0x0,
        0xE0, 0x7, 0x0, 0xE, 0x0, 0x0, 0x0, 0x80, 0xE0, 0x7, 0x0, 0x0, 0xE0, 0x7, 0x45, 0x4E, 0x44
    };
    static const uint8_t pucMockedBoardInfo[ MOCKED_BOARD_INFO_LENGTH ] =
    {
        0x01, 0xC0, 0x01, 0x01, 0x0C, 0x00, 0x03, 0x14, 0x54, 0x65, 0x73, 0x74, 0x20, 0x45, 0x65, 0x70, 0x72, 0x6F,
        0x6D, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x54, 0x65, 0x73, 0x74, 0x20, 0x50, 0x72, 0x6F, 0x64, 0x75, 0x63,
        0x74, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x14, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x00, 0x74, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x30, 0x31, 0x3A, 0x32, 0x33, 0x3A, 0x34, 0x35, 0x3A, 0x36,
        0x37, 0x3A, 0x38, 0x39, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x30, 0x31, 0x3A, 0x32, 0x33, 0x3A, 0x34, 0x35, 0x3A, 0x36, 0x37,
        0x3A, 0x38, 0x39, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x03, 0x14, 0x30, 0x31, 0x2F, 0x30, 0x31, 0x2F,
        0x30, 0x31, 0x00, 0x36, 0x37,
        0x3A, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
        0x37, 0x38, 0x39, 0x00, 0x3A,
        0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x00, 0x3A, 0x38,
        0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x00, 0x3A, 0x38, 0x39,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x03, 0x14, 0x30, 0x31, 0x32, 0x33,
        0x34, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x00, 0x3A, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x14, 0x30, 0x31,
        0x32, 0x33, 0x34, 0x35, 0x36,
        0x37, 0x38, 0x39, 0x00, 0x3A, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x4E, 0x44
    };
    static const uint8_t pucMockedTempSensorSdrData[ MOCKED_SENSOR_SDR_DATA_LENGTH ] =
    {
        0x01, 0xC1, 0x01, 0x01, 0x0C, 0x00, 0x01, 0xE8, 0x54, 0x65, 0x73, 0x74, 0x2D, 0x54, 0x65, 0x6D, 0x70, 0x65,
        0x72, 0x61, 0x74, 0x75,
        0x72, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x01, 0x45, 0x4E, 0x44
    };
    static const uint8_t pucMockedTempSensorData[ MOCKED_SENSOR_DATA_LENGTH ] =
    {
        0x01, 0xC1, 0x00, 0x04, 0x0A, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x04, 0x0E, 0x00, 0xFF, 0x01, 0x04, 0x01
    };
    static const uint8_t pucMockedVoltageSensorSdrData[ MOCKED_SENSOR_SDR_DATA_LENGTH ] =
    {
        0x01, 0xC2, 0x01, 0x01, 0x0C, 0x00, 0x01, 0xE8, 0x54, 0x65, 0x73, 0x74, 0x2D, 0x56, 0x6F, 0x6C, 0x74, 0x61,
        0x67, 0x65, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x01, 0x45, 0x4E, 0x44
    };
    static const uint8_t pucMockedVoltageSensorData[ MOCKED_SENSOR_DATA_LENGTH ] =
    {
        0x01, 0xC2, 0x00, 0x04, 0x1E, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x04, 0x0E, 0x00, 0xFF, 0x01, 0x04, 0x01
    };
    static const uint8_t pucMockedCurrentSensorSdrData[ MOCKED_SENSOR_SDR_DATA_LENGTH ] =
    {
        0x01, 0xC3, 0x01, 0x01, 0x0C, 0x00, 0x01, 0xE8, 0x54, 0x65, 0x73, 0x74, 0x2D, 0x43, 0x75, 0x72, 0x72, 0x65,
        0x6E, 0x74, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x01, 0x45, 0x4E, 0x44
    };
    static const uint8_t pucMockedCurrentSensorData[ MOCKED_SENSOR_DATA_LENGTH ] =
    {
        0x01, 0xC3, 0x00, 0x04, 0x14, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x04, 0x0E, 0x00, 0xFF, 0x01, 0x04, 0x01
    };
    static const uint8_t pucMockedTotalPowerSensorSdrData[ MOCKED_SENSOR_SDR_DATA_LENGTH ] =
    {
        0x01, 0xC6, 0x01, 0x01, 0x0C, 0x00, 0x01, 0xE8, 0x54, 0x65, 0x73, 0x74, 0x2D, 0x54, 0x6F, 0x74, 0x61, 0x6C,
        0x5F, 0x50, 0x6F, 0x77,
        0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x01, 0x45, 0x4E, 0x44
    };
    static const uint8_t pucMockedTotalPowerSensorData[ MOCKED_SENSOR_DATA_LENGTH ] =
    {
        0x01, 0xC6, 0x00, 0x04, 0x28, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x04, 0x0E, 0x00, 0xFF, 0x01, 0x04, 0x01
    };
    static const uint8_t pucMockedPowerSensorSdrData[ MOCKED_SENSOR_SDR_DATA_LENGTH ] =
    {
        0x01, 0xC4, 0x01, 0x01, 0x0C, 0x00, 0x01, 0xE8, 0x54, 0x65, 0x73, 0x74, 0x2D, 0x50, 0x6F, 0x77, 0x65, 0x72,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x04, 0x0A, 0x00, 0x00, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x01, 0x45, 0x4E, 0x44
    };
    static const uint8_t pucMockedPowerSensorData[ MOCKED_SENSOR_DATA_LENGTH ] =
    {
        0x01, 0xC4, 0x00, 0x04, 0x28, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x04, 0x0E, 0x00, 0xFF, 0x01, 0x04, 0x01
    };

    if( ( NULL != pxSignal ) && ( AMC_CFG_UNIQUE_ID_AMI == pxSignal->ucModule ) &&
        ( TRUE == pxThis->iInBandTestMode ) )
    {
        switch( pxSignal->ucEventType )
        {
        case AMI_PROXY_DRIVER_E_SENSOR_READ:
        {
            AMI_PROXY_SENSOR_REQUEST xSensorRequest =
            {
                0
            };

            if( OK == iAMI_GetSensorRequest( pxSignal, &xSensorRequest ) )
            {
                AMI_PROXY_RESULT xResult        = AMI_PROXY_RESULT_INVALID_VALUE;
                uint16_t         usResponseSize = 0;
                uint8_t          pucRespBuffer[ SENSOR_RESP_BUFFER_SIZE ] =
                {
                    0
                };
                uintptr_t ullDestAddr = ( pxThis->ullSharedMemBaseAddr + xSensorRequest.ullAddress );
                uint8_t   *pucDestAdd = ( uint8_t* )( ullDestAddr );

                /* Reset iStatus */
                iStatus = ERROR;

                if( SENSOR_RESP_BUFFER_SIZE > xSensorRequest.ulLength )
                {
                    PLL_DBG( AMC_IN_BAND_DBG_NAME,
                             "Response size 0x%x exceeding request size 0x%x",
                             SENSOR_RESP_BUFFER_SIZE,
                             xSensorRequest.ulLength );
                }
                else
                {
                    if( ( AMI_PROXY_CMD_SENSOR_REQUEST_GET_SINGLE_SDR == xSensorRequest.xRequest ) ||
                        ( AMI_PROXY_CMD_SENSOR_REQUEST_ALL_SDR == xSensorRequest.xRequest ) )
                    {
                        switch( xSensorRequest.xRepo )
                        {
                        case AMI_PROXY_CMD_SENSOR_REPO_TEMP:
                        {
                            pvOSAL_MemCpy( pucRespBuffer, pucMockedTempSensorData, sizeof( pucMockedTempSensorData ) );
                            usResponseSize = sizeof( pucMockedTempSensorData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_VOLTAGE:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedVoltageSensorData,
                                           sizeof( pucMockedVoltageSensorData ) );
                            usResponseSize = sizeof( pucMockedVoltageSensorData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_CURRENT:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedCurrentSensorData,
                                           sizeof( pucMockedCurrentSensorData ) );
                            usResponseSize = sizeof( pucMockedCurrentSensorData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_POWER:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedPowerSensorData,
                                           sizeof( pucMockedPowerSensorData ) );
                            usResponseSize = sizeof( pucMockedPowerSensorData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_TOTAL_POWER:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedTotalPowerSensorData,
                                           sizeof( pucMockedTotalPowerSensorData ) );
                            usResponseSize = sizeof( pucMockedTotalPowerSensorData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_BDINFO:
                        case AMI_PROXY_CMD_SENSOR_REPO_FPT:
                        default:
                        {
                            break;
                        }
                        }

                        iStatus = OK;
                    }
                    else if( AMI_PROXY_CMD_SENSOR_REQUEST_GET_SDR == xSensorRequest.xRequest )
                    {
                        switch( xSensorRequest.xRepo )
                        {
                        case AMI_PROXY_CMD_SENSOR_REPO_BDINFO:
                        {
                            pvOSAL_MemCpy( pucRespBuffer, pucMockedBoardInfo, sizeof( pucMockedBoardInfo ) );
                            usResponseSize = sizeof( pucMockedBoardInfo );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_TEMP:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedTempSensorSdrData,
                                           sizeof( pucMockedTempSensorSdrData ) );
                            usResponseSize = sizeof( pucMockedTempSensorSdrData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_VOLTAGE:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedVoltageSensorSdrData,
                                           sizeof( pucMockedVoltageSensorSdrData ) );
                            usResponseSize = sizeof( pucMockedVoltageSensorSdrData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_CURRENT:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedCurrentSensorSdrData,
                                           sizeof( pucMockedCurrentSensorSdrData ) );
                            usResponseSize = sizeof( pucMockedCurrentSensorSdrData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_POWER:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedPowerSensorSdrData,
                                           sizeof( pucMockedPowerSensorSdrData ) );
                            usResponseSize = sizeof( pucMockedPowerSensorSdrData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_TOTAL_POWER:
                        {
                            pvOSAL_MemCpy( pucRespBuffer,
                                           pucMockedTotalPowerSensorSdrData,
                                           sizeof( pucMockedTotalPowerSensorSdrData ) );
                            usResponseSize = sizeof( pucMockedTotalPowerSensorSdrData );
                            break;
                        }

                        case AMI_PROXY_CMD_SENSOR_REPO_FPT:
                        {
                            pvOSAL_MemCpy( pucRespBuffer, &pucMockedFptData, sizeof( pucMockedFptData ) );
                            usResponseSize = sizeof( pucMockedFptData );
                            break;
                        }

                        default:
                        {
                            break;
                        }
                        }

                        iStatus = OK;
                    }
                    else
                    {
                        PLL_DBG( AMC_IN_BAND_DBG_NAME, "Error in request type\r\n" );
                        iStatus = ERROR;
                    }
                }

                /* Check the response is not greater than the shared memory supplied */
                if( ( OK == iStatus ) && ( usResponseSize <= xSensorRequest.ulLength ) )
                {
                    /* Copy the data into the shared memory */
                    pvOSAL_MemCpy( pucDestAdd, pucRespBuffer, usResponseSize );
                    /* This is required on the target to flush the data */
                    HAL_FLUSH_CACHE_DATA( ullDestAddr, usResponseSize );
                    xResult = AMI_PROXY_RESULT_SUCCESS;
                }
                else
                {
                    /* Single byte response for failure */
                    pucDestAdd[ ASDM_SDR_RESP_BYTE_CC ]   = ASDM_SDR_COMPLETION_CODE_OPERATION_FAILED;
                    pucDestAdd[ ASDM_SDR_RESP_BYTE_SIZE ] = 1;
                }

                iStatus = iAMI_SetSensorCompleteResponse( pxSignal, xResult );
            }
            break;
        }

        case AMI_PROXY_DRIVER_E_EEPROM_READ_WRITE:
        {
            AMI_PROXY_EEPROM_RW_REQUEST xEepromReadWriteRequest =
            {
                0
            };

            if( OK == iAMI_GetEepromReadWriteRequest( pxSignal, &xEepromReadWriteRequest ) )
            {
                AMI_PROXY_RESULT xResult = AMI_PROXY_RESULT_SUCCESS;
                PLL_DBG( AMC_IN_BAND_DBG_NAME,
                         "Eeprom Read/Write Request Addr: 0x%X\r\n",
                         xEepromReadWriteRequest.ulOffset );
                iStatus = iAMI_SetEepromReadWriteCompleteResponse( pxSignal, xResult );
            }
            break;
        }

        case AMI_PROXY_DRIVER_E_MODULE_READ_WRITE:
        {
            AMI_PROXY_MODULE_RW_REQUEST xModuleReadWriteRequest =
            {
                0
            };

            if( OK == iAMI_GetModuleReadWriteRequest( pxSignal, &xModuleReadWriteRequest ) )
            {
                AMI_PROXY_RESULT xResult = AMI_PROXY_RESULT_SUCCESS;
                PLL_DBG( AMC_IN_BAND_DBG_NAME,
                         "Module Read Write/Request Id : %d Page: %d Offset: 0x%X\r\n",
                         xModuleReadWriteRequest.ucExDeviceId,
                         xModuleReadWriteRequest.ucPage,
                         xModuleReadWriteRequest.ucByteOffset );
                iStatus = iAMI_SetModuleReadWriteCompleteResponse( pxSignal, xResult );
            }
            break;
        }

        case AMI_PROXY_DRIVER_E_PDI_DOWNLOAD_START:
        {
            AMI_PROXY_PDI_DOWNLOAD_REQUEST xDownloadRequest =
            {
                0
            };
            PLL_DBG( AMC_IN_BAND_DBG_NAME, "Event PDI Download Start (0x%02X)\r\n", pxSignal->ucEventType );

            if( OK == iAMI_GetPdiDownloadRequest( pxSignal, &xDownloadRequest ) )
            {
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI download address : 0x%llx\r\n", xDownloadRequest.ullAddress );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI download length  : 0x%x\r\n",   xDownloadRequest.ulLength );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI partition        : 0x%x\r\n",   xDownloadRequest.ulPartitionSel );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI has FPT          : 0x%x\r\n",   xDownloadRequest.iUpdateFpt );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI last packet      : 0x%x\r\n",   xDownloadRequest.iLastPacket );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI packet number    : 0x%hx\r\n",  xDownloadRequest.usPacketNum );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI packet size (KB) : 0x%hx\r\n",  xDownloadRequest.usPacketSize );
                iStatus = iAMI_SetPdiDownloadCompleteResponse( pxSignal, AMI_PROXY_RESULT_SUCCESS );

            }
            else
            {
                iStatus = iAMI_SetPdiDownloadCompleteResponse( pxSignal, AMI_PROXY_RESULT_GET_REQUEST_FAILED );
            }

            if( OK != iStatus )
            {
                PLL_ERR( AMC_IN_BAND_DBG_NAME,
                         "Error downloading to partition %d\r\n",
                         xDownloadRequest.ulPartitionSel );
            }
            break;
        }

        case AMI_PROXY_DRIVER_E_PDI_COPY_START:
        {
            AMI_PROXY_PDI_COPY_REQUEST xCopyRequest =
            {
                0
            };
            PLL_DBG( AMC_IN_BAND_DBG_NAME, "Event PDI Copy Start (0x%02X)\r\n", pxSignal->ucEventType );

            if( OK == iAMI_GetPdiCopyRequest( pxSignal, &xCopyRequest ) )
            {
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI copy address     : 0x%llx\r\n", xCopyRequest.ullAddress );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI max copy length  : 0x%x\r\n",  xCopyRequest.ulMaxLength );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI src partition    : 0x%x\r\n",  xCopyRequest.ulSrcPartition );
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "PDI dst partition    : 0x%x\r\n",  xCopyRequest.ulDestPartition );
                iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_SUCCESS );
            }
            else
            {
                iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_GET_REQUEST_FAILED );
            }

            if( OK != iStatus )
            {
                PLL_ERR( AMC_IN_BAND_DBG_NAME,
                         "Error copying p%d to p%d\r\n",
                         xCopyRequest.ulSrcPartition,
                         xCopyRequest.ulDestPartition );
            }
            break;
        }

        case AMI_PROXY_DRIVER_E_BOOT_SELECT:
        {
            AMI_PROXY_BOOT_SELECT_REQUEST xBootSelRequest =
            {
                0
            };
            PLL_DBG( AMC_IN_BAND_DBG_NAME, "Event Boot Select Request (0x%02X)\r\n", pxSignal->ucEventType );

            if( OK == iAMI_GetBootSelectRequest( pxSignal, &xBootSelRequest ) )
            {
                PLL_DBG( AMC_IN_BAND_DBG_NAME, "Selecting partition %d\r\n", xBootSelRequest.ulPartitionSel );
                iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_SUCCESS );
            }
            else
            {
                iStatus = iAMI_SetPdiCopyCompleteResponse( pxSignal, AMI_PROXY_RESULT_GET_REQUEST_FAILED );
            }

            if( OK != iStatus )
            {
                PLL_ERR( AMC_IN_BAND_DBG_NAME, "Error selecting partition %d\r\n", xBootSelRequest.ulPartitionSel );
            }
            break;
        }

        case AMI_PROXY_DRIVER_E_GET_IDENTITY:
        case AMI_PROXY_DRIVER_E_HEARTBEAT:
        default:
        {
            iStatus = OK;
            break;
        }
        }
    }

    return iStatus;
}
