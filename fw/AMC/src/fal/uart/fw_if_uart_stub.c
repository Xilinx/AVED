/**
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF UART stubbed abstraction.
 *
 * @file fw_if_uart_stub.c
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

#include "fw_if_uart.h"
#include "profile_fal.h"


/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define FW_IF_UART_NAME         "FW_IF_UART"
#define BUFFER_SIZE             ( 64 )

#define UART_UPPER_FIREWALL     ( 0xBEEFCAFE )
#define UART_LOWER_FIREWALL     ( 0xDEADFACE )

#define CHECK_DRIVER            if( FW_IF_FALSE == pxThis->iInitialised ) { INC_ERROR_COUNTER( FW_IF_ERRORS_DRIVER_NOT_INITIALISED_COUNT ); return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }
#define CHECK_FIREWALLS( f )    if( ( f->upperFirewall != UART_UPPER_FIREWALL ) && ( f->lowerFirewall != UART_LOWER_FIREWALL ) ) { INC_ERROR_COUNTER( FW_IF_ERRORS_INVALID_HANDLE_COUNT ); return FW_IF_ERRORS_INVALID_HANDLE; }
#define CHECK_HDL( f )          if( NULL == f ) { INC_ERROR_COUNTER( FW_IF_ERRORS_INVALID_HANDLE_COUNT ); return FW_IF_ERRORS_INVALID_HANDLE; }
#define CHECK_CFG( f )          if( NULL == ( f )->cfg ) { INC_ERROR_COUNTER( FW_IF_ERRORS_INVALID_CFG_COUNT ); return FW_IF_ERRORS_INVALID_CFG; }

#define FW_IF_UART_STATE_ENTRY( _s )    [ FW_IF_UART_STATE_ ## _s ] = #_s

/* Stat & Error definitions */
#define FW_IF_UART_STAT_COUNTS( DO )                    \
    DO( FW_IF_UART_STATS_INIT_OVERALL_COMPLETE_COUNT )  \
    DO( FW_IF_UART_STATS_INSTANCE_CREATE_COUNT )        \
    DO( FW_IF_UART_STATS_OPEN_COUNT )                   \
    DO( FW_IF_UART_STATS_CLOSE_COUNT )                  \
    DO( FW_IF_UART_IO_CTRL_COUNT )                      \
    DO( FW_IF_UART_STATS_BIND_CALLBACK_CALLED_COUNT )   \
    DO( FW_IF_UART_STATS_READ_COUNT )                   \
    DO( FW_IF_UART_STATS_WRITE_COUNT )                  \
    DO( FW_IF_UART_STATS_MAX_COUNT )

#define FW_IF_UART_ERROR_COUNTS( DO )                  \
    DO( FW_IF_ERRORS_PARAMS_COUNT )                    \
    DO( FW_IF_ERRORS_DRIVER_NOT_INITIALISED_COUNT )    \
    DO( FW_IF_ERRORS_INVALID_CFG_COUNT )               \
    DO( FW_IF_ERRORS_INVALID_HANDLE_COUNT )            \
    DO( FW_IF_UART_ERRORS_INVALID_STATE_COUNT )        \
    DO( FW_IF_UART_ERRORS_MAX_COUNT )

#define PRINT_STAT_COUNTER( x )             PLL_INF( FW_IF_UART_NAME, "%50s . . . . %d\r\n",          \
                                                     FW_IF_UART_STAT_COUNTS_STR[ x ],       \
                                                     pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )            PLL_INF( FW_IF_UART_NAME, "%50s . . . . %d\r\n",          \
                                                     FW_IF_UART_ERROR_COUNTS_STR[ x ],      \
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
 * @struct FW_IF_UART_PRIVATE_DATA
 * @brief   Structure to hold this module's private data
 */
typedef struct FW_IF_UART_PRIVATE_DATA
{
    uint32_t                ulUpperFirewall;

    FW_IF_UART_INIT_CFG     xMyLocalCfg;
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
    FALSE,                  /* iInitialised */

    { 0 },                  /* pulStatCounters */
    { 0 },                  /* pulErrorCounters */

    UART_LOWER_FIREWALL     /* ulLowerFirewall */
};
static FW_IF_UART_PRIVATE_DATA *pxThis = &xLocalData;


/******************************************************************************/
/* Local Function Declarations                                                */
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
 * @param   ulTimeout       Time (in ms) to wait for write to complete
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


/******************************************************************************/
/* local functions                                                            */
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
            INC_STAT_COUNTER( FW_IF_UART_STATS_OPEN_COUNT );
            PLL_DBG( FW_IF_UART_NAME, "UART FW_IF_open for addr 0x%02X\r\n", pxThisUartCfg->ulPort );
            break;

        default:
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
            PLL_ERR( FW_IF_UART_NAME, "Error: UART is currently in an invalid state [ %s ]\r\n", pcUartStateModeStr[ pxThisUartCfg->xState ] );
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
            INC_STAT_COUNTER( FW_IF_UART_STATS_CLOSE_COUNT );
            PLL_DBG( FW_IF_UART_NAME, "UART FW_IF_close for addr 0x%02X\r\n", pxThisUartCfg->ulPort );
            break;

        default:
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
            PLL_ERR( FW_IF_UART_NAME, "Error: UART is currently in an invalid state [ %s ]\r\n", pcUartStateModeStr[ pxThisUartCfg->xState ] );
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
            switch( ulTimeoutMs )
            {
                case FW_IF_TIMEOUT_NO_WAIT:
                    PLL_DBG( FW_IF_UART_NAME, "This is a non-blocking call. UART FW_IF_write Destination:0x%x Length:0x%x\r\n", ullDstPort, ulLength );
                    INC_STAT_COUNTER( FW_IF_UART_STATS_WRITE_COUNT );
                    break;
                case FW_IF_TIMEOUT_WAIT_FOREVER:
                    PLL_DBG( FW_IF_UART_NAME, "This is a blocking call. Exiting early. UART FW_IF_write Destination:0x%x Length:0x%x\r\n", ullDstPort, ulLength );
                    INC_STAT_COUNTER( FW_IF_UART_STATS_WRITE_COUNT );
                    break;
                default:
                    PLL_DBG( FW_IF_UART_NAME, "UART FW_IF_write Destination:0x%x Length:0x%x\r\n", ullDstPort, ulLength );
                    INC_STAT_COUNTER( FW_IF_UART_STATS_WRITE_COUNT );
                    iOSAL_Task_SleepMs( ulTimeoutMs );
                    break;
            }
        }
        else
        {
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
            PLL_ERR( FW_IF_UART_NAME, "Error: write() should only be called from opened state [ %s ]\r\n", pcUartStateModeStr[ pxThisUartCfg->xState ] );
        }
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_ERRORS_PARAMS_COUNT );
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_read
*/
static uint32_t ulFW_IF_UART_Read( void *pvFwIf, uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulLength, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

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
                    PLL_DBG( FW_IF_UART_NAME, "This is a non-blocking call. UART FW_IF_read Source:0x%x Length:0x%x\r\n", ullSrcPort, *pulLength );
                    INC_STAT_COUNTER( FW_IF_UART_STATS_READ_COUNT );
                    break;
                case FW_IF_TIMEOUT_WAIT_FOREVER:
                    PLL_DBG( FW_IF_UART_NAME, "This is a blocking call. Exiting early. UART FW_IF_write Source:0x%x Length:0x%x\r\n", ullSrcPort, *pulLength );
                    INC_STAT_COUNTER( FW_IF_UART_STATS_READ_COUNT );
                    break;
                default:
                    PLL_DBG( FW_IF_UART_NAME, "UART FW_IF_write Source:0x%x Length:0x%x\r\n", ullSrcPort, *pulLength );
                    INC_STAT_COUNTER( FW_IF_UART_STATS_READ_COUNT );
                    iOSAL_Task_SleepMs( ulTimeoutMs );
                    break;
            }
        }
        else
        {
            ulStatus = FW_IF_UART_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_UART_ERRORS_INVALID_STATE_COUNT );
            PLL_ERR( FW_IF_UART_NAME, "Error: read() should only be called from opened state [ %s ]\r\n", pcUartStateModeStr[ pxThisUartCfg->xState ] );
        }
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_ERRORS_PARAMS_COUNT );
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

    PLL_INF( FW_IF_UART_NAME, "UART FW_IF_ioctrl for addr 0x%02X (option %u)\r\n", pxThisUartCfg->ulPort, ulOption );
    INC_STAT_COUNTER( FW_IF_UART_IO_CTRL_COUNT );

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
        INC_ERROR_COUNTER( FW_IF_ERRORS_PARAMS_COUNT );
    }

    return ulStatus;
}


/******************************************************************************/
/* public functions                                                           */
/******************************************************************************/
/**
 * @brief   Initialisation function for UART interfaces (generic across all UART unterfaces)
 */
uint32_t ulFW_IF_UART_Init( FW_IF_UART_INIT_CFG *pxInitCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != pxThis->iInitialised )
    {
        ulStatus = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL != pxInitCfg )
    {
        /*
         * Initialise config data shared between all instances of UART.
         */
        pvOSAL_MemCpy( &pxThis->xMyLocalCfg, pxInitCfg, sizeof( FW_IF_UART_INIT_CFG ) );
        pxThis->iInitialised = FW_IF_TRUE;

        INC_STAT_COUNTER( FW_IF_UART_STATS_INIT_OVERALL_COMPLETE_COUNT );

        PLL_DBG( FW_IF_UART_NAME, "ulFW_IF_UART_Init, Device ID: 0x%08X, Base: %uHz\r\n", pxThis->xMyLocalCfg.ucUartDeviceId );
    }
    else 
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_ERRORS_PARAMS_COUNT );
    }

    return ulStatus;
}

/**
 * @brief   Creates an instance of the UART interface
*/
uint32_t ulFW_IF_UART_Create( FW_IF_CFG *pxFwIf, FW_IF_UART_CFG *pxUartCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;
    
    if( ( NULL != pxFwIf ) && ( NULL != pxUartCfg ) )
    {
        FW_IF_CFG myLocalIf = 
        {
            .upperFirewall  = UART_UPPER_FIREWALL,
            .open           = &ulFW_IF_UART_Open,
            .close          = &ulFW_IF_UART_Close,
            .write          = &ulFW_IF_UART_Write,
            .read           = &ulFW_IF_UART_Read,
            .ioctrl         = &ulFW_IF_UART_Ioctrl,
            .bindCallback   = &ulFW_IF_UART_BindCallback,
            .cfg            = ( void* )pxUartCfg,
            .lowerFirewall   = UART_LOWER_FIREWALL
        };

        pvOSAL_MemCpy( pxFwIf, &myLocalIf, sizeof( FW_IF_CFG ) );

        FW_IF_UART_CFG *pxThisUartCfg = ( FW_IF_UART_CFG* )pxFwIf->cfg;

        PLL_DBG( FW_IF_UART_NAME, "ulFW_IF_UART_Create for addr 0x%02X \r\n", pxThisUartCfg->ulPort );
        pxThisUartCfg->xState = FW_IF_UART_STATE_CREATE;
        ulStatus = FW_IF_ERRORS_NONE;
        INC_STAT_COUNTER( FW_IF_UART_STATS_INSTANCE_CREATE_COUNT );
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_ERRORS_PARAMS_COUNT );
    }

    return ulStatus;
}

/**
 * @brief    Print all the stats gathered by the driver
 */
int iFW_IF_UART_PrintStatistics( void )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;
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

    return ulStatus;
}

/**
 * @brief    Clears all the stats gathered by the driver
 */
int iFW_IF_UART_ClearStatistics( void )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    PLL_INF( FW_IF_UART_NAME, "Stats have been cleared.");
    pvOSAL_MemSet( pxThis->pulStatCounters, 0, sizeof( pxThis->pulStatCounters ) );
    pvOSAL_MemSet( pxThis->pulErrorCounters, 0, sizeof( pxThis->pulErrorCounters ) );

    return ulStatus;
}
