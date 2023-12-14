/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF QSFP Stub abstraction.
 *
 * @file fw_if_qsfp_stub.c
 *
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "fw_if_muxed_device.h"
#include "profile_fal.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define FW_IF_QSFP_NAME             "FW_IF_QSFP"

#define QSFP_UPPER_FIREWALL         ( 0xBEEFCAFE )
#define QSFP_LOWER_FIREWALL         ( 0xDEADFACE )

#define CHECK_DRIVER                if( FW_IF_FALSE == pxThis->iInitialised ) return FW_IF_ERRORS_DRIVER_NOT_INITIALISED
#define CHECK_FIREWALLS( f )        if( ( QSFP_UPPER_FIREWALL != f->upperFirewall ) &&        \
                                        ( QSFP_LOWER_FIREWALL != f->lowerFirewall ) &&        \
                                        ( QSFP_UPPER_FIREWALL != pxThis->ulUpperFirewall ) && \
                                        ( QSFP_LOWER_FIREWALL != pxThis->ulLowerFirewall ) ) return FW_IF_ERRORS_INVALID_HANDLE

#define CHECK_HDL( f )              if( NULL == f ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_CFG( f )              if( NULL == ( f )->cfg  ) return FW_IF_ERRORS_INVALID_CFG
#define CHECK_PROFILE( f )          if( NULL == ( f )->pvProfile ) return

/* Stat & Error definitions */
#define FW_IF_QSFP_STATS( DO )   \
    DO( FW_IF_QSFP_STATS_INIT_OVERALL_COMPLETE )         \
    DO( FW_IF_QSFP_STATS_INSTANCE_CREATE )               \
    DO( FW_IF_QSFP_STATS_MAX )

#define FW_IF_QSFP_ERRORS( DO )    \
    DO( FW_IF_QSFP_STATS_INSTANCE_CREATE_FAILED )        \
    DO( FW_IF_QSFP_ERRORS_VALIDATION_FAILED )            \
    DO( FW_IF_QSFP_ERRORS_MAX )

#define PRINT_STAT_COUNTER( x )             PLL_INF( FW_IF_QSFP_NAME, "%50s . . . . %d\r\n",          \
                                                     FW_IF_QSFP_STATS_STR[ x ],      \
                                                     pxThis->ulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )            PLL_INF( FW_IF_QSFP_NAME, "%50s . . . . %d\r\n",          \
                                                     FW_IF_QSFP_ERRORS_STR[ x ],     \
                                                     pxThis->ulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )               { if( x < FW_IF_QSFP_STATS_MAX )pxThis->ulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )              { if( x < FW_IF_QSFP_ERRORS_MAX )pxThis->ulErrorCounters[ x ]++; }

/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    FW_IF_QSFP_STATS
 * @brief   Enumeration of stats counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( FW_IF_QSFP_STATS, FW_IF_QSFP_STATS, FW_IF_QSFP_STATS_STR )

/**
 * @enum    FW_IF_QSFP_ERRORS
 * @brief   Enumeration of stats errors for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( FW_IF_QSFP_ERRORS, FW_IF_QSFP_ERRORS, FW_IF_QSFP_ERRORS_STR )


/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/

/**
 * @struct  FW_IF_QSFP_PRIVATE_DATA
 * @brief   Structure to hold this FAL's private data
 */
typedef struct FW_IF_QSFP_PRIVATE_DATA
{
    uint32_t                        ulUpperFirewall;

    FW_IF_MUXED_DEVICE_INIT_CFG     xLocalCfg;
    int                             iInitialised;
    uint32_t                        ulStatCounters[ FW_IF_QSFP_STATS_MAX ];
    uint32_t                        ulErrorCounters[ FW_IF_QSFP_ERRORS_MAX ];
    int                             iDebugPrint;

    uint32_t                        ulLowerFirewall;

} FW_IF_QSFP_PRIVATE_DATA;


/*****************************************************************************/
/* local variables                                                           */
/*****************************************************************************/

static FW_IF_QSFP_PRIVATE_DATA xLocalData =
{
    QSFP_UPPER_FIREWALL,    /* ulUpperFirewall */
    { 0 },                  /* xLocalCfg       */
    FW_IF_FALSE,            /* iInitialised    */
    { 0 },                  /* ulStatCounters  */
    { 0 },                  /* ulErrorCounters */
    FW_IF_FALSE,            /* iDebugPrint     */
    QSFP_LOWER_FIREWALL     /* ulLowerFirewall */
};
static FW_IF_QSFP_PRIVATE_DATA *pxThis = &xLocalData;


/*****************************************************************************/
/* local functions                                                           */
/*****************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t ulQsfpOpen( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_MUXED_DEVICE_CFG *pxCfg = ( FW_IF_MUXED_DEVICE_CFG* )pxThisIf->cfg;

    /*
     * This is where the power IO expander would be written to, to enable power
     * for the QSFP specified in the FwIF handle.
     */
    PLL_DBG( FW_IF_QSFP_NAME, "QSFP FW_IF_open %s device (i2c address 0x%02X) \r\n",
               ( ( FW_IF_DEVICE_QSFP == pxCfg->xDevice )? "QSFP" : "DIMM" ),
               ( unsigned int )pxCfg->ucDeviceI2cAddr );

    return ulStatus;
}


/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t ulQsfpClose( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_MUXED_DEVICE_CFG *pxCfg = ( FW_IF_MUXED_DEVICE_CFG* )pxThisIf->cfg;

    /*
     * This is where the power IO expander would be written to, to disable power
     * for the QSFP specified in the FwIF handle.
     */
    PLL_DBG( FW_IF_QSFP_NAME, "QSFP FW_IF_close %s device (i2c address 0x%02X) \r\n",
                ( ( FW_IF_DEVICE_QSFP == pxCfg->xDevice )? "QSFP" : "DIMM" ),
                ( unsigned int )pxCfg->ucDeviceI2cAddr );

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t ulQsfpWrite( void *pvFwIf, uint64_t ullDstPort, uint8_t *pucData, uint32_t ulSize, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( ( NULL != pucData ) &&
        ( FW_IF_QSFP_MAX_DATA >= ulSize ) )
    {
        /*
         * This is where data will be written to either QSFP IO expander,
         * or QSFP memory map register, depending on hw level set.
         */
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_read
 */
static uint32_t ulQsfpRead( void *pvFwIf, uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulSize, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( ( NULL != pucData ) &&
        ( NULL != pulSize ) &&
        ( FW_IF_QSFP_MAX_DATA >= *pulSize ) )
    {
        /*
         * This is where data will be read from either QSFP IO expander,
         * or QSFP memory map register, depending on hw level set.
         */
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_ioctrl
 */
static uint32_t ulQsfpIoctrl( void *pvFwIf, uint32_t ulOption, void *pvValue )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_MUXED_DEVICE_CFG *pxCfg = ( FW_IF_MUXED_DEVICE_CFG* )pxThisIf->cfg;

    switch( ulOption )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            /*
             * Handle common IOCTL's.
             */
            break;

        case FW_IF_COMMON_IOCTRL_ENABLE_DEBUG_PRINT:
        {
            pxThis->iDebugPrint = FW_IF_TRUE;
            break;
        }

        case FW_IF_COMMON_IOCTRL_DISABLE_DEBUG_PRINT:
        {
            pxThis->iDebugPrint = FW_IF_FALSE;
            break;
        }

        case FW_IF_MUXED_DEVICE_IOCTL_SET_IO_EXPANDER:
        {
            pxCfg->xHwLevel = FW_IF_MUXED_DEVICE_HW_LEVEL_IO_EXPANDER;
            break;
        }

        case FW_IF_MUXED_DEVICE_IOCTL_SET_MEMORY_MAP:
        {
            pxCfg->xHwLevel = FW_IF_MUXED_DEVICE_HW_LEVEL_MEMORY_MAP;
            break;
        }

        default:
        {
            ulStatus = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            PLL_DBG( FW_IF_QSFP_NAME, "QSFP IOCTL - Unrecognised option\r\n" );
            break;
        }
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t ulQsfpBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( NULL != pxNewFunc )
    {
        FW_IF_MUXED_DEVICE_CFG *pxCfg = ( FW_IF_MUXED_DEVICE_CFG* )pxThisIf->cfg;

        /*
         * Binds in callback provided to the FW_IF.
         * Callback will be invoked when driver event occurs.
         */
        pxThisIf->raiseEvent = pxNewFunc;
        PLL_DBG( FW_IF_QSFP_NAME, "QSFP FW_IF_bindCallback %s device (i2c address 0x%02X) \r\n",
                ( ( FW_IF_DEVICE_QSFP == pxCfg->xDevice )? "QSFP" : "DIMM" ),
                ( unsigned int )pxCfg->ucDeviceI2cAddr );
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }

    return ulStatus;
}

/*****************************************************************************/
/* public functions                                                          */
/*****************************************************************************/

/**
 * @brief   initialisation function for QSFP interfaces (generic across all QSFP interfaces)
 */
uint32_t ulFW_IF_MUXED_DEVICE_Init( FW_IF_MUXED_DEVICE_INIT_CFG *pxInitCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != pxThis->iInitialised )
    {
        ulStatus = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL == pxInitCfg )
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }
    else
    {
        /*
         * Initilise config data shared between all QSFPs.
         */
        memcpy( &pxThis->xLocalCfg, pxInitCfg, sizeof( FW_IF_MUXED_DEVICE_INIT_CFG ) );
        pxThis->iInitialised = FW_IF_TRUE;
        INC_STAT_COUNTER( FW_IF_QSFP_STATS_INIT_OVERALL_COMPLETE )
    }

    return ulStatus;
}

/**
 * @brief   creates an instance of the QSFP interface
 */
uint32_t ulFW_IF_MUXED_DEVICE_Create( FW_IF_CFG *pxFwIf, FW_IF_MUXED_DEVICE_CFG *pxQsfpCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( NULL != pxFwIf ) && ( NULL != pxQsfpCfg ) )
    {
        FW_IF_CFG xLocalIf =
        {
            .upperFirewall  = QSFP_UPPER_FIREWALL,
            .open           = &ulQsfpOpen,
            .close          = &ulQsfpClose,
            .write          = &ulQsfpWrite,
            .read           = &ulQsfpRead,
            .ioctrl         = &ulQsfpIoctrl,
            .bindCallback   = &ulQsfpBindCallback,
            .cfg            = ( void* )pxQsfpCfg,
            .lowerFirewall  = QSFP_LOWER_FIREWALL
        };

        memcpy( pxFwIf, &xLocalIf, sizeof( FW_IF_CFG ) );
        INC_STAT_COUNTER( FW_IF_QSFP_STATS_INSTANCE_CREATE );
        PLL_DBG( FW_IF_QSFP_NAME, "ulFW_IF_MUXED_DEVICE_Create\r\n" );
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_QSFP_STATS_INSTANCE_CREATE_FAILED )
        ulStatus = FW_IF_ERRORS_PARAMS;
    }

    return ulStatus;
}

/**
 * @brief   Print all the stats gathered by the application
 */
uint32_t ulFW_IF_MUXED_DEVICE_PrintStatistics( void )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    int i = 0;
    PLL_INF( FW_IF_QSFP_NAME, "============================================================\n\r" );
    PLL_INF( FW_IF_QSFP_NAME, "FWIF QSFP Statistics:\n\r" );
    for( i = 0; i < FW_IF_QSFP_STATS_MAX; i++ )
    {
        PRINT_STAT_COUNTER( i );
    }
    PLL_INF( FW_IF_QSFP_NAME, "------------------------------------------------------------\n\r" );
    PLL_INF( FW_IF_QSFP_NAME, "FWIF QSFP Errors:\n\r" );
    for( i = 0; i < FW_IF_QSFP_ERRORS_MAX; i++ )
    {
        PRINT_ERROR_COUNTER( i );
    }
    PLL_INF( FW_IF_QSFP_NAME, "============================================================\n\r" );

    return ulStatus;
}

/**
 * @brief   Clear all the stats in the application
 */
uint32_t ulFW_IF_MUXED_DEVICE_ClearStatistics( void )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    if( FW_IF_TRUE == pxThis->iInitialised )
    {
        pvOSAL_MemSet( pxThis->ulStatCounters, 0, sizeof( pxThis->ulStatCounters ) );
        pvOSAL_MemSet( pxThis->ulErrorCounters, 0, sizeof( pxThis->ulErrorCounters ) );
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_QSFP_ERRORS_VALIDATION_FAILED )
        ulStatus = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
    }

    return ulStatus;
}

