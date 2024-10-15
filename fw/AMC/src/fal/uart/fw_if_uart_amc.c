/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF UART AMC abstraction.
 *
 * @file fw_if_uart_amc.c
 *
 */


/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "osal.h"
#include "standard.h"
#include "xtime_l.h"

#include "fw_if_uart.h"
#include "profile_fal.h"
#include "xuartpsv.h"


/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define FW_IF_UART_NAME             "FW_IF_UART"
#define BUFFER_SIZE                 ( 64 )

#define UART_UPPER_FIREWALL         ( 0xBEEFCAFE )
#define UART_LOWER_FIREWALL         ( 0xDEADFACE )

#define CHECK_DRIVER                if( FW_IF_FALSE == pxThis->iInitialised ) { INC_ERROR_COUNTER( FW_IF_UART_ERRORS_DRIVER_NOT_INITIALISED_COUNT ); return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }
#define CHECK_FIREWALLS( f )        if( ( f->upperFirewall != UART_UPPER_FIREWALL ) && ( f->lowerFirewall != UART_LOWER_FIREWALL ) ) { INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_HANDLE_COUNT ); return FW_IF_ERRORS_INVALID_HANDLE; }
#define CHECK_HDL( f )              if( NULL == f ) { INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_HANDLE_COUNT ); return FW_IF_ERRORS_INVALID_HANDLE; }
#define CHECK_CFG( f )              if( NULL == ( f )->cfg ) { INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_CFG_COUNT ); return FW_IF_ERRORS_INVALID_CFG; }

#define WAIT_1_MS                   iOSAL_Task_SleepMs( 1 )
#define FW_IF_UART_TIMEOUT_MAX_MS   ( 10000 )
#define SCALE_MS                    ( 1000 )
#define NO_WAIT_TIME                ( 1 )

#define FW_IF_UART_STATE_ENTRY( _s )    [ FW_IF_UART_STATE_ ## _s ] = #_s

/* Stat & Error definitions */
#define FW_IF_UART_STAT_COUNTS( DO )                    \
    DO( FW_IF_UART_STATS_INIT_OVERALL_COMPLETE_COUNT )  \
    DO( FW_IF_UART_STATS_INSTANCE_CREATE_COUNT )        \
    DO( FW_IF_UART_STATS_OPEN_COUNT )                   \
    DO( FW_IF_UART_STATS_CLOSE_COUNT )                  \
    DO( FW_IF_UART_STATS_IO_CTRL_COUNT )                \
    DO( FW_IF_UART_STATS_BIND_CALLBACK_CALLED_COUNT )   \
    DO( FW_IF_UART_STATS_READ_COUNT )                   \
    DO( FW_IF_UART_STATS_WRITE_COUNT )                  \
    DO( FW_IF_UART_STATS_MAX_COUNT )

#define FW_IF_UART_ERROR_COUNTS( DO )                       \
    DO( FW_IF_UART_ERRORS_PARAMS_COUNT )                    \
    DO( FW_IF_UART_ERRORS_DRIVER_NOT_INITIALISED_COUNT )    \
    DO( FW_IF_UART_ERRORS_DRIVER_FAILURE_COUNT )            \
    DO( FW_IF_UART_ERRORS_INVALID_CFG_COUNT )               \
    DO( FW_IF_UART_ERRORS_INVALID_HANDLE_COUNT )            \
    DO( FW_IF_UART_ERRORS_VALIDATION_FAILED_COUNT )         \
    DO( FW_IF_UART_ERRORS_INVALID_STATE_COUNT )             \
    DO( FW_IF_UART_ERRORS_MAX_COUNT )

#define PRINT_STAT_COUNTER( x )             PLL_INF( FW_IF_UART_NAME, "%50s . . . . %d\r\n",    \
                                                     FW_IF_UART_STAT_COUNTS_STR[ x ],           \
                                                     pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )            PLL_INF( FW_IF_UART_NAME, "%50s . . . . %d\r\n",    \
                                                     FW_IF_UART_ERROR_COUNTS_STR[ x ],          \
                                                     pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )               { if( x < FW_IF_UART_STATS_MAX_COUNT )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )              { if( x < FW_IF_UART_ERRORS_MAX_COUNT )pxThis->pulErrorCounters[ x ]++; }


/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * @enum    FW_IF_UART_STAT_COUNTS
 * @brief   Enumeration of stats counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( FW_IF_UART_STAT_COUNTS, FW_IF_UART_STAT_COUNTS, FW_IF_UART_STAT_COUNTS_STR )

/**
 * @enum    FW_IF_UART_ERROR_COUNTS
 * @brief   Enumeration of stats errors for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( FW_IF_UART_ERROR_COUNTS, FW_IF_UART_ERROR_COUNTS, FW_IF_UART_ERROR_COUNTS_STR )


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct  FW_IF_UART_PRIVATE_DATA
 * @brief   Structure to hold this modules private data
 */
typedef struct FW_IF_UART_PRIVATE_DATA
{
    uint32_t                ulUpperFirewall;

    FW_IF_UART_INIT_CFG     xLocalCfg;
    int                     iInitialised;

    uint32_t                pulStatCounters[ FW_IF_UART_STATS_MAX_COUNT ];
    uint32_t                pulErrorCounters[ FW_IF_UART_ERRORS_MAX_COUNT ];

    uint32_t                ulLowerFirewall;

} FW_IF_UART_PRIVATE_DATA;


/*****************************************************************************/
/* Local variables                                                           */
/*****************************************************************************/

static const char* const pcUartStateModeStr[ ] = { FW_IF_UART_STATE_ENTRY( INIT ),
                                                   FW_IF_UART_STATE_ENTRY( OPENED ),
                                                   FW_IF_UART_STATE_ENTRY( CLOSED ),
                                                   FW_IF_UART_STATE_ENTRY( ERROR ) };

static FW_IF_UART_PRIVATE_DATA xLocalData =
{
    UART_UPPER_FIREWALL,    /* ulUpperFirewall */

    { 0 },                  /* xLocalCfg */
    FW_IF_FALSE,            /* iInitialised */

    { 0 },                  /* pulStatCounters */
    { 0 },                  /* pulErrorCounters */

    UART_LOWER_FIREWALL     /* ulLowerFirewall */
};
static FW_IF_UART_PRIVATE_DATA *pxThis = &xLocalData;

XUartPsv xUartInstance = { 0 };
XUartPsv_Config *pxUartCfg = { 0 };


/******************************************************************************/
/* Local function declarations                                                */
/******************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 * 
 * @param   pvFwIf          Pointer to this fw_if
 * 
 * @return  See FW_IF_ERRORS
*/
static uint32_t ulFW_IF_UART_Open( void *pvFwIf );

/**
 * @brief   Local implementation of FW_IF_close
 * 
 * @param   pvFwIf          Pointer to this fw_if
 * 
 * @return  See FW_IF_ERRORS
*/
static uint32_t ulFW_IF_UART_Close( void *pvFwIf );

/**
 * @brief   Local implementation of FW_IF_write
 * 
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ullDstPort      The port where the data will be written to
 * @param   pucData         Pointer to data buffer to write
 * @param   ulLength        Maximum number of bytes allowed in data buffer. This value is updated to the actual number of bytes written
 * @param   ulTimeoutMs     Time (in ms) to wait for write to complete
 * 
 * @return  See FW_IF_ERRORS
*/
static uint32_t ulFW_IF_UART_Write( void *pvFwIf, uint64_t ullDstPort, uint8_t *pucData, uint32_t ulLength, uint32_t ulTimeoutMs );

/**
 * @brief   Local implementation of FW_IF_read
 * 
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ullSrcPort      The port where the data will be read from
 * @param   pucData         Pointer to data buffer to read
 * @param   pulLength       Pointer to maximum number of bytes allowed in data buffer. This value is updated to the actual number of bytes read
 * @param   ulTimeoutMs     Time (in ms) to wait for read to complete
 * 
 * @return  See FW_IF_ERRORS
*/
static uint32_t ulFW_IF_UART_Read( void *pvFwIf, uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulLength, uint32_t ulTimeoutMs );

/**
 * @brief   Local implementation of FW_IF_ioctrl
 * 
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ulOption        Unique IO Ctrl option to set/get
 * @param   pvValue         Pointer to value to set/get
 * 
 * @return  See FW_IF_ERRORS
*/
static uint32_t ulFW_IF_UART_Ioctrl( void *pvFwIf, uint32_t ulOption, void *pvValue );

/**
 * @brief   Local implementation of FW_IF_bindCallback
 * 
 * @param   pvFwIf          Pointer to this fw_if
 * @param   pxNewFunc       Function pointer to call
 * 
 * @return  See FW_IF_ERRORS
*/
static uint32_t ulFW_IF_UART_BindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc );

/**
 * @brief   Write to the UART.
 * 
 * @param   ullDstPort      The port where the data will be written to
 * @param   pucData         Pointer to data buffer to write
 * @param   pulLength       Pointer to maximum number of bytes allowed in data buffer. This value is updated to the actual number of bytes written
 * 
 * @return  Number of bytes written
 */
static uint32_t ulUART_Write( uint64_t ullDstPort, uint8_t *pucData, uint32_t *pulLength );

/**
 * @brief Read from the UART
 * 
 * @param   ullSrcPort      The port where the data will be read from
 * @param   pucData         Pointer to data buffer to read
 * @param   pulLength       Pointer to maximum number of bytes allowed in data buffer. This value is updated to the actual number of bytes read
 * 
 * @return  See FW_IF_ERRORS
*/
static uint32_t ulUART_Read( uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulLength );

/**
 * @brief   Read from the UART until the timeout runs out.
 * 
 * @param   ullSrcPort      The port where the data will be read from
 * @param   pucData         Pointer to data buffer to read
 * @param   pulLength       Pointer to maximum number of bytes allowed in data buffer. This value is updated to the actual number of bytes read
 * @param   usTimeoutMs     Time (in ms) to wait for read to complete
 * 
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulUART_PollRead( uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulLength, uint16_t usTimeoutMs );



/******************************************************************************/
/* Local functions                                                            */
/******************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
*/
static uint32_t ulFW_IF_UART_Open( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxThisIf->cfg;

    switch( pxThisUartCfg->xState )
    {
        case FW_IF_UART_STATE_CREATE:
        case FW_IF_UART_STATE_CLOSED:
        case FW_IF_UART_STATE_OPENED:
            pxThisUartCfg->xState = FW_IF_UART_STATE_OPENED;
            INC_STAT_COUNTER( FW_IF_UART_STATS_OPEN_COUNT );
            PLL_DBG( FW_IF_UART_NAME, "UART FW_IF_open for addr 0x%02X\r\n", pxThisUartCfg->ulPort );
            break;

        default:
            pxThisUartCfg->xState = FW_IF_UART_STATE_ERROR;
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
            PLL_ERR( FW_IF_UART_NAME, "Error: Open failed due to invalid state [ %s ]\r\n", pcUartStateModeStr[ pxThisUartCfg->xState ] );
            break;
    }

    return ulStatus;
}

/**
 * @brief   Local implentation of FW_IF_close
*/
static uint32_t ulFW_IF_UART_Close( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxThisIf->cfg;

    switch( pxThisUartCfg->xState )
    {
        case FW_IF_UART_STATE_OPENED:
        case FW_IF_UART_STATE_ERROR:
        case FW_IF_UART_STATE_CLOSED:
            pxThisUartCfg->xState = FW_IF_UART_STATE_CLOSED;
            INC_STAT_COUNTER( FW_IF_UART_STATS_CLOSE_COUNT );
            PLL_DBG( FW_IF_UART_NAME, "UART FW_IF_close for addr 0x%02X\r\n", pxThisUartCfg->ulPort );
            break;

        default:
            pxThisUartCfg->xState = FW_IF_UART_STATE_ERROR;
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
            PLL_ERR( FW_IF_UART_NAME, "Error: Close failed due to invalid state [ %s ]\r\n", pcUartStateModeStr[ pxThisUartCfg->xState ] );
            break;
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_write
*/
static uint32_t ulFW_IF_UART_Write( void *pvFwIf, uint64_t ullDstPort, uint8_t *pucData, uint32_t ulLength, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;
    uint32_t ulBytesWritten = 0;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxThisIf->cfg;

    if( ( NULL != pucData ) )
    {
        if( FW_IF_UART_STATE_OPENED == pxThisUartCfg->xState )
        {
            /* Currently, there is no timeout implementation for the UART write */
            switch( ulTimeoutMs )
            {
                case FW_IF_TIMEOUT_NO_WAIT:
                case FW_IF_TIMEOUT_WAIT_FOREVER:
                default:
                    INC_STAT_COUNTER( FW_IF_UART_STATS_WRITE_COUNT );
                    ulBytesWritten = ulUART_Write( ullDstPort, pucData, &ulLength );
                    break;
            }
            if( 0 == ulBytesWritten )
            {
                ulStatus = FW_IF_ERRORS_WRITE;
                INC_ERROR_COUNTER( FW_IF_UART_ERRORS_DRIVER_FAILURE_COUNT );
            }
        }
        else
        {
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
        }
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_UART_ERRORS_PARAMS_COUNT );
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_read
*/
static uint32_t ulFW_IF_UART_Read( void *pvFwIf, uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulLength, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;
    uint32_t ulReadData = FW_IF_FALSE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxThisIf->cfg;
    if( ( NULL != pucData ) && ( NULL != pulLength ) )
    {
        if( FW_IF_UART_STATE_OPENED == pxThisUartCfg->xState )
        {
            switch( ulTimeoutMs )
            {
                case FW_IF_TIMEOUT_NO_WAIT:
                    INC_STAT_COUNTER( FW_IF_UART_STATS_READ_COUNT );
                    ulReadData = ulUART_PollRead( ullSrcPort, pucData, pulLength, NO_WAIT_TIME );
                    break;
                case FW_IF_TIMEOUT_WAIT_FOREVER:
                    INC_STAT_COUNTER( FW_IF_UART_STATS_READ_COUNT );
                    ulReadData = ulUART_Read( ullSrcPort, pucData, pulLength );
                    break;
                default:
                    if( ( 0 < ulTimeoutMs ) && ( FW_IF_UART_TIMEOUT_MAX_MS >= ulTimeoutMs ) )
                    {
                        INC_STAT_COUNTER( FW_IF_UART_STATS_READ_COUNT );
                        ulReadData = ulUART_PollRead( ullSrcPort, pucData, pulLength, ulTimeoutMs );
                    }
                    break;
            }
            if( FW_IF_FALSE == ulReadData )
            {
                ulStatus = FW_IF_ERRORS_READ;
                INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
            }
        }
        else
        {
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
        }
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_UART_ERRORS_PARAMS_COUNT );
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_ioctrl
*/
static uint32_t ulFW_IF_UART_Ioctrl( void *pvFwIf, uint32_t ulOption, void *pvValue )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxThisIf->cfg;

    switch( ulOption )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            /**
             * Handle common IOCTRL's.
            */
           break;
        
        default:
            ulStatus = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            PLL_ERR( FW_IF_UART_NAME, "UART IOCTL - Unrecognised option\r\n" );
            break;
    }

    if( FW_IF_ERRORS_NONE == ulStatus )
    {
        INC_STAT_COUNTER( FW_IF_UART_STATS_IO_CTRL_COUNT );
        PLL_INF( FW_IF_UART_NAME, "UART FW_IF_ioctrl for addr 0x%02X (option %u)\r\n", pxThisUartCfg->ulPort, ulOption );
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_BindCallback
 */
static uint32_t ulFW_IF_UART_BindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( NULL != pxNewFunc )
    {
        FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxThisIf->cfg;
        
        pxThisIf->raiseEvent = pxNewFunc;
    
        PLL_DBG( FW_IF_UART_NAME, "UART FW_IF_bindCallback called for port 0x%02X\r\n", pxThisUartCfg->ulPort );
        INC_STAT_COUNTER( FW_IF_UART_STATS_BIND_CALLBACK_CALLED_COUNT );
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_UART_ERRORS_PARAMS_COUNT );
    }

    return ulStatus;
}

/**
 * @brief   Write to the UART.
 */
static uint32_t ulUART_Write( uint64_t ullDstPort, uint8_t *pucData, uint32_t *pulLength )
{
    uint32_t ulBytesWritten = 0;

    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( NULL != pulLength ) )
    {
        while( ulBytesWritten < *pulLength )
        {
            ulBytesWritten += XUartPsv_Send( &xUartInstance, &pucData[ ulBytesWritten ], sizeof( pucData[ 0 ] ) );
        }
    }
    return ulBytesWritten;
}

/**
 * @brief Read to the UART
*/
static uint32_t ulUART_Read( uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulLength )
{
    int ulStatus = FW_IF_FALSE;

    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( NULL != pulLength ) )
    {
        *pucData   = XUartPsv_RecvByte( pxUartCfg->BaseAddress );
        *pulLength = sizeof( pucData );
        if( 0 < *pulLength )
        {
            ulStatus   = FW_IF_TRUE;
        }
    }

    return ulStatus;
}

/**
 * @brief   Read from the UART until the timeout runs out.
 */
static uint32_t ulUART_PollRead( uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulLength, uint16_t usTimeoutMs )
{
    int ulStatus = FW_IF_FALSE;
    uint16_t usCounter = 0;

    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( NULL != pulLength ) )
    {
        while( usCounter < usTimeoutMs )
        {
            if( FW_IF_FALSE != XUartPsv_IsReceiveData( pxUartCfg->BaseAddress ) )
            {
                *pucData = XUartPsv_RecvByte( pxUartCfg->BaseAddress );
                *pulLength = sizeof( pucData );
                if( 0 < *pulLength  )
                {
                    ulStatus = FW_IF_TRUE;
                }
                break;
            }
            WAIT_1_MS;
            usCounter++;
        }
    }
    return ulStatus;
}


/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/

/**
 * @brief   Initialisation function for UART interfaces (generic across all UART unterfaces)
 */
uint32_t ulFW_IF_UART_Init( FW_IF_UART_INIT_CFG *pxInitCfg )
{
    uint32_t ulStatus = FW_IF_UART_ERRORS_DRIVER_FAILURE;
    pxUartCfg = &xUartInstance.Config;

    if( ( UART_UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( UART_LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FW_IF_FALSE == pxThis->iInitialised ) )
    {  
        ulStatus = FW_IF_ERRORS_NONE;

        if( FW_IF_FALSE != pxThis->iInitialised )
        {
            ulStatus = FW_IF_ERRORS_DRIVER_IN_USE;
        }
        else if ( NULL != pxInitCfg )
        {
            pvOSAL_MemCpy( &pxThis->xLocalCfg, pxInitCfg, sizeof( FW_IF_UART_INIT_CFG ) );

            pxUartCfg = XUartPsv_LookupConfig( pxThis->xLocalCfg.ucUartDeviceId );

            if( NULL != pxUartCfg )
            {
                if( OK != XUartPsv_CfgInitialize( &xUartInstance, pxUartCfg, pxUartCfg->BaseAddress ) )
                {
                    ulStatus = FW_IF_UART_ERRORS_DRIVER_FAILURE;
                }
            }
            else
            {
                ulStatus = FW_IF_UART_ERRORS_DRIVER_FAILURE;
            }

            if( FW_IF_ERRORS_NONE == ulStatus )
            {
                pxThis->iInitialised = FW_IF_TRUE;
                INC_STAT_COUNTER( FW_IF_UART_STATS_INIT_OVERALL_COMPLETE_COUNT );
            }
        }
        else
        {
            ulStatus = FW_IF_ERRORS_PARAMS;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_PARAMS_COUNT );
        }
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_UART_ERRORS_DRIVER_FAILURE_COUNT );
    }

    return ulStatus;
}

/**
 * @brief   Creates an instance of the UART interface
*/
uint32_t ulFW_IF_UART_Create( FW_IF_CFG *pxFwIf, FW_IF_UART_CFG *pxUartCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;
    uint32_t ulBaudRate = 0;
    uint32_t ulDataBits = 0;
    uint32_t ulParity = 0;
    uint8_t ucStopBits = 0;

    CHECK_DRIVER;

    if( ( UART_UPPER_FIREWALL == pxThis->ulUpperFirewall ) && 
        ( UART_LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        if( ( NULL != pxFwIf ) && ( NULL != pxUartCfg ) )
        {
            FW_IF_CFG xLocalIf = 
            {
                .upperFirewall  = UART_UPPER_FIREWALL,
                .open           = &ulFW_IF_UART_Open,
                .close          = &ulFW_IF_UART_Close,
                .write          = &ulFW_IF_UART_Write,
                .read           = &ulFW_IF_UART_Read,
                .ioctrl         = &ulFW_IF_UART_Ioctrl,
                .bindCallback   = &ulFW_IF_UART_BindCallback,
                .cfg            = ( void* )pxUartCfg,
                .lowerFirewall  = UART_LOWER_FIREWALL
            };

            pvOSAL_MemCpy( pxFwIf, &xLocalIf, sizeof( FW_IF_CFG ) );

            FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxFwIf->cfg;

            if( ( pxThisUartCfg->ulBaudRate < XUARTPSV_MIN_RATE ) || ( pxThisUartCfg->ulBaudRate > XUARTPSV_MAX_RATE ) )
            {
                ulBaudRate = XUARTPSV_DFT_BAUDRATE;
                ulStatus = FW_IF_ERRORS_INVALID_CFG;
                INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_CFG_COUNT );
            }
            else
            {
                ulBaudRate = pxThisUartCfg->ulBaudRate;
            }
            switch( pxThisUartCfg->xDataBits )
            {
                case FW_IF_UART_6_BITS:
                    ulDataBits = XUARTPSV_FORMAT_6_BITS;
                    break;
                case FW_IF_UART_7_BITS:
                    ulDataBits = XUARTPSV_FORMAT_7_BITS;
                    break;
                case FW_IF_UART_8_BITS:
                default:
                    ulDataBits = XUARTPSV_FORMAT_8_BITS;
                    break;
            }
            switch( pxThisUartCfg->xParity )
            {
                case FW_IF_UART_PARITY_EVEN:
                    ulParity = XUARTPSV_FORMAT_EVEN_PARITY;
                    break;
                case FW_IF_UART_PARITY_ODD:
                    ulParity = XUARTPSV_FORMAT_ODD_PARITY;
                    break;
                case FW_IF_UART_PARITY_NONE:
                default:
                    ulParity = XUARTPSV_FORMAT_NO_PARITY;
                    break;
            }
            switch( pxThisUartCfg->xStopBits )
            {
                case FW_IF_UART_2_STOP_BITS:
                    ucStopBits = XUARTPSV_FORMAT_2_STOP_BIT;
                    break;
                case FW_IF_UART_1_STOP_BIT:
                default:
                    ucStopBits = XUARTPSV_FORMAT_1_STOP_BIT;
                    break;
            }

            XUartPsvFormat xFormat = 
            {
                ulBaudRate,
                ulDataBits,
                ulParity,
                ucStopBits
            };
            if( FW_IF_TRUE != XUartPsv_SetDataFormat( &xUartInstance, &xFormat ) )
            {
                ulStatus = FW_IF_ERRORS_INVALID_CFG;
                INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_CFG_COUNT );
            }

            pxThisUartCfg->xState = FW_IF_UART_STATE_CREATE;

            INC_STAT_COUNTER( FW_IF_UART_STATS_INSTANCE_CREATE_COUNT );
        }
        else
        {
            ulStatus = FW_IF_ERRORS_PARAMS;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_PARAMS_COUNT );
        }
    }

    return ulStatus;
}

/**
 * @brief    Print all the stats gathered by the driver
 */
int iFW_IF_UART_PrintStatistics( void )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( UART_UPPER_FIREWALL == pxThis->ulUpperFirewall ) && 
        ( UART_LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        int i = 0;
        PLL_INF( FW_IF_UART_NAME, "============================================================\r\n" );
        PLL_INF( FW_IF_UART_NAME, "FW IF UART Statistics:\n\r" );
        for( i = 0; i < FW_IF_UART_STATS_MAX_COUNT; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( FW_IF_UART_NAME, "------------------------------------------------------------\r\n" );
        PLL_INF( FW_IF_UART_NAME, "FW IF UART Errors:\n\r" );
        for( i = 0; i < FW_IF_UART_ERRORS_MAX_COUNT; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( FW_IF_UART_NAME, "============================================================\r\n" );
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_UART_ERRORS_VALIDATION_FAILED_COUNT );
        ulStatus = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
    }

    return ulStatus;
}

/**
 * @brief    Clears all the stats gathered by the driver
 */
int iFW_IF_UART_ClearStatistics( void )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( UART_UPPER_FIREWALL == pxThis->ulUpperFirewall ) && 
        ( UART_LOWER_FIREWALL == pxThis->ulLowerFirewall ) )
    {
        PLL_INF( FW_IF_UART_NAME, "Stats have been cleared.\r\n");
        pvOSAL_MemSet( pxThis->pulStatCounters, 0, sizeof( pxThis->pulStatCounters ) );
        pvOSAL_MemSet( pxThis->pulErrorCounters, 0, sizeof( pxThis->pulErrorCounters ) );
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_UART_ERRORS_VALIDATION_FAILED_COUNT );
    }

    return ulStatus;
}

