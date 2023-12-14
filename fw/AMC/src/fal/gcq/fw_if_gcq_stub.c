/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF GCQ Stub abstraction.
 *
 * @file fw_if_gcq_stub.c
 *
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "fw_if_gcq.h"


/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define FW_IF_GCQ_NAME                  "FW IF GCQ Stub"

#define GCQ_UPPER_FIREWALL              ( 0xBEEFCAFE )
#define GCQ_LOWER_FIREWALL              ( 0xDEADFACE )

#define CHECK_DRIVER                    ( FW_IF_FALSE == iInitialised )
#define CHECK_FIREWALLS( f )            ( ( f->upperFirewall != GCQ_UPPER_FIREWALL ) &&\
                                          ( f->lowerFirewall != GCQ_LOWER_FIREWALL ) )
#define CHECK_HDL( f )                  ( NULL == f )
#define CHECK_CFG( f )                  ( NULL == ( f )->cfg )

#define CHECK_PROFILE( f )              ( NULL == ( f )->pvProfile )

#define FW_IF_GCQ_MAX_INSTANCES         ( 4 )


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct  GCQ_PROFILE_TYPE
 * @brief   The definition of a profile, used to store internal
 *          state/structures dependant on implementation
 */
struct GCQ_PROFILE_TYPE
{
    uint32_t ulPlaceholder;
};

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

static FW_IF_GCQ_INIT_CFG xLocalCfg = { 0 };

static int iInitialised = FW_IF_FALSE;

static struct GCQ_PROFILE_TYPE xProfiles[ FW_IF_GCQ_MAX_INSTANCES ] = { { 0 } };

static uint32_t ulProfilesAllocated = 0;


/*****************************************************************************/
/* Local Functions                                                           */
/*****************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t prvGCQOpen( void *pvFWIf )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
    if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

    /*
     * This is where the instance of the GCQ device would be opened based on
     * IP HW base address provided. Interanlly the driver will set up the ring
     * buffers based on shared memory address and slot sizes.
     */
    PLL_DBG( FW_IF_GCQ_NAME, "GCQ FW_IF_open\r\n" );

    return xRet;

}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t prvGCQClose( void *pvFWIf )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
    if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

    /*
     * Free up any allocated resources.
     */
    PLL_DBG( FW_IF_GCQ_NAME, "GCQ FW_IF_close\r\n" );

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t prvGCQWrite( void *pvFWIf, uint64_t ullDstPort, uint8_t *pucData, uint32_t ulSize, uint32_t ulTimeoutMs )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
    if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

    if( ( NULL != pucData ) )
    {
        /*
         * This is where data will be written to the relevant queue
         * depending on mode set when opening GCQ instance.
         */
        PLL_DBG( FW_IF_GCQ_NAME, "GCQ FW_IF_write\r\n" );
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_read
 */
static uint32_t prvGCQRead( void *pvFWIf, uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulSize, uint32_t ulTimeoutMs )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
    if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

    if( ( NULL != pucData ) && ( NULL != pulSize ) )
    {
        /*
         * This is where data will be read back from the relevant queue
         * depending on mode set when opening GCQ instance.
         */
        PLL_DBG( FW_IF_GCQ_NAME, "GCQ FW_IF_read\r\n" );
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_ioctrl
 */
static uint32_t prvGCQIOCtrl( void *pvFWIf, uint32_t ulOption, void *pvValue )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
    if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

    /*
     * Specialization options supported outside definition of standard API.
     */
    switch( ulOption )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            /*
             * Handle common IOCTL's.
             */
            break;

        case FW_IF_GCQ_IOCTRL_SET_OPAQUE_HANDLE:
            /*
             * Set the opaque handle used by calling API to store a reference.
             */
            break;

        case FW_IF_GCQ_IOCTRL_GET_OPAQUE_HANDLE:
            /*
             *Get the opaque handle used by calling API to retreive a reference.
             */
            break;

        default:
            xRet = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            PLL_DBG( FW_IF_GCQ_NAME, "GCQ IOCTL - Unrecognised option\r\n" );
            break;
    }

    if( FW_IF_ERRORS_NONE == xRet )
    {
        PLL_DBG( FW_IF_GCQ_NAME, "GCQ FW_IF_ioctl\r\n" );
    }

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t prvGCQBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
    if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

    if( NULL != pxNewFunc )
    {
        /*
         * Binds in callback provided to the FW_IF.
         * Callback will be invoked when by the driver when event occurs.
         */
        pxThisIf->raiseEvent = pxNewFunc;
        PLL_DBG( FW_IF_GCQ_NAME, "GCQ FW_IF_bindCallback called\r\n" );
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}


/*****************************************************************************/
/* public functions                                                          */
/*****************************************************************************/

/**
 * @brief   initialisation function for GCQ interfaces (generic across all GCQ interfaces)
 */
uint32_t ulFW_IF_GCQ_Init( FW_IF_GCQ_INIT_CFG *pxInitCfg )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != iInitialised )
    {
        xRet = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL != pxInitCfg )
    {
        /*
         * Initilise config data shared between all instances of GCQ.
         */
        memcpy( &xLocalCfg, pxInitCfg, sizeof( FW_IF_GCQ_INIT_CFG ) );
        iInitialised = FW_IF_TRUE;
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}

/**
 * @brief   opens an instance of the GCQ interface
 */
uint32_t ulFW_IF_GCQ_Create( FW_IF_CFG *pxFwIf, FW_IF_GCQ_CFG *pxGCQCfg )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;

    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    if( ( NULL != pxFwIf ) && ( NULL != pxGCQCfg ) )
    {
        if( ulProfilesAllocated >= ( FW_IF_GCQ_MAX_INSTANCES - 1 ) )
        {
            xRet = FW_IF_GCQ_ERRORS_NO_FREE_PROFILES;
        }
        else if( MAX_FW_IF_GCQ_MODE > pxGCQCfg->xMode )
        {
            FW_IF_CFG myLocalIf =
            {
                .upperFirewall  = GCQ_UPPER_FIREWALL,
                .open           = &prvGCQOpen,
                .close          = &prvGCQClose,
                .write          = &prvGCQWrite,
                .read           = &prvGCQRead,
                .ioctrl         = &prvGCQIOCtrl,
                .bindCallback   = &prvGCQBindCallback,
                .cfg            = ( void* )pxGCQCfg,
                .lowerFirewall  = GCQ_LOWER_FIREWALL
            };

            memcpy( pxFwIf, &myLocalIf, sizeof( FW_IF_CFG ) );

            FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxFwIf->cfg;

            /*
             * Crude implementation of allocating the next free profile and assigning
             * it to the to FW_IF_GCQ_CFG pvInstance variable, will need updated to allow
             * dynamic adding/removing.
             */
            pxCfg->pvProfile = &xProfiles[ ulProfilesAllocated ];
            ulProfilesAllocated++;
            PLL_DBG( FW_IF_GCQ_NAME, "FW_IF_GCQ_create (%s)\r\n",
                   ( FW_IF_GCQ_MODE_PRODUCER == pxCfg->xMode ) ? ( "Producer" ):( "Consumer" ) );
        }
        else
        {
            xRet = FW_IF_ERRORS_INVALID_CFG;
        }
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}
