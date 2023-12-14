/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF GCQ abstraction.
 *
 * @file fw_if_gcq_linux.c
 *
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#ifdef __KERNEL__
#include <asm/io.h>
#else
#include <string.h>
#include <assert.h>
#endif

#include "fw_if_gcq.h"
#include "gcq.h"
#include "profile_fal.h"


/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define FW_IF_GCQ_NAME                  "FW_IF_GCQ"

#define GCQ_UPPER_FIREWALL              ( 0xBEEFCAFE )
#define GCQ_LOWER_FIREWALL              ( 0xDEADFACE )

#define CHECK_DRIVER                    ( FW_IF_FALSE == iInitialised )
#define CHECK_FIREWALLS( f )            ( ( f->upperFirewall != GCQ_UPPER_FIREWALL ) &&\
                                          ( f->lowerFirewall != GCQ_LOWER_FIREWALL ) )
#define CHECK_NULL( f )                 ( NULL == f )
#define CHECK_NOT_NULL( f )             ( NULL != f )
#define CHECK_CFG( f )                  ( NULL == ( f )->cfg )

#define CHECK_PROFILE( f )              ( NULL == ( f )->pvProfile )

#define CHECK_INVALID_STATE( f )        ( ( FW_IF_GCQ_STATE_OPENED != ( f )->xState ) &&\
                                          ( FW_IF_GCQ_STATE_ATTACHED != ( f )->xState ) )


/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * @struct  FW_IF_GCQ_STATE
 * @brief   The internal GCQ IF state
 */
typedef enum FW_IF_GCQ_STATE
{
    FW_IF_GCQ_STATE_CLOSED = 0,
    FW_IF_GCQ_STATE_INIT,
    FW_IF_GCQ_STATE_OPENED,
    FW_IF_GCQ_STATE_ATTACHED,

    FW_IF_GCQ_STATE_MAX

} FW_IF_GCQ_STATE;


/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * @struct  FW_IF_GCQ_PROFILE_TYPE
 * @brief   The definition of a profile, used to store internal
 *          state & ptr to driver instance
 */
typedef struct FW_IF_GCQ_PROFILE_TYPE
{
    uint32_t ulIOHandle;
    FW_IF_GCQ_STATE xState;
    struct GCQ_INSTANCE_TYPE *pxGCQInstance;

} FW_IF_GCQ_PROFILE_TYPE;


/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

static FW_IF_GCQ_INIT_CFG xLocalCfg = { 0 };

static GCQ_IO_ACCESS_TYPE xGCQIOAccess = { 0 };

static int iInitialised = FW_IF_FALSE;

static FW_IF_GCQ_PROFILE_TYPE xGCQProfile[ GCQ_MAX_INSTANCES ] = { { 0 } };     /**< Uses the same compile time value as the GCQ driver  */

static uint32_t ulProfilesAllocated = 0;


/*****************************************************************************/
/* Local Functions                                                           */
/*****************************************************************************/

/**
 *
 * @brief   Handle memory & register writes in Linux Host/Kernel
 *
 * @param   ullDestAddr is the destination address
 * @param   ulValue is the 32-bit value to write
 *
 * @return  N/A
 *
 */
static inline void prvvWriteMemReg32( uint64_t ullDestAddr, uint32_t ulValue )
{ 
#ifdef __KERNEL__
    iowrite32( ulValue, ( void __iomem * ) ullDestAddr );
#else
    *( ( volatile uint32_t * )ullDestAddr ) = ulValue;
#endif
}

/**
 *
 * @brief   Handle memory & register reads in Linux Host/Kernel
 *
 * @param   ullSrcAddr is the source address
 *
 * @return  the 32-bit value read
 *
 */
static inline uint32_t prvulReadMemReg32( uint64_t ullSrcAddr )
{
#ifdef __KERNEL__
    return ( ioread32( ( void __iomem * ) ullSrcAddr ) );
#else
    uint32_t ulValue = *( ( volatile uint64_t * )ullSrcAddr );
    return ( ulValue );
#endif
}

/**
 *
 * @brief   Map interface error return code
 *
 * @param   error is the return code returned by the gcq driver
 *
 * @return  the mapped interface return code
 *
 */
static FW_IF_GCQ_ERRORS_TYPE prvxMapIFDriverReturnCode( GCQ_ERRORS_TYPE xError )
{
    FW_IF_GCQ_ERRORS_TYPE xMappedErr = MAX_FW_IF_GCQ_ERROR;

    switch( xError )
    {
        case GCQ_ERRORS_NONE:                       xMappedErr = FW_IF_ERRORS_NONE; break;
        case GCQ_ERRORS_DRIVER_NOT_INITIALISED:     xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_NOT_INITIALISED; break;
        case GCQ_ERRORS_NO_FREE_INSTANCES:          xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_NO_FREE_INSTANCES; break;
        case GCQ_ERRORS_INVALID_INSTANCE:           xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_INVALID_INSTANCE; break;
        case GCQ_ERRORS_INVALID_ARG:                xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_INVALID_ARG; break;
        case GCQ_ERRORS_INVALID_SLOT_SIZE:          xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_INVALID_SLOT_SIZE; break;
        case GCQ_ERRORS_INVALID_VERSION:            xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_INVALID_VERSION; break;
        case GCQ_ERRORS_INVALID_NUM_SLOTS:          xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_INVALID_NUM_SLOTS; break;
        case GCQ_ERRORS_CONSUMER_NOT_ATTACHED:      xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_CONSUMER_NOT_ATTACHED; break;
        case GCQ_ERRORS_CONSUMER_NOT_AVAILABLE:     xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_CONSUMER_NOT_AVAILABLE; break;
        case GCQ_ERRORS_CONSUMER_NO_DATA_RECEIVED:  xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_CONSUMER_NO_DATA_RECEIVED; break;
        case GCQ_ERRORS_PRODUCER_NO_FREE_SLOTS:     xMappedErr = FW_IF_GCQ_ERRORS_DRIVER_PRODUCER_NO_FREE_SLOTS; break;
        case MAX_GCQ_ERRORS_TYPE:
        default:
            gcq_assert( 0 );
            break;
    }

    return ( xMappedErr );
}

/**
 *
 * @brief   Map interrupt mode
 *
 * @param   xInterruptMode is the interface interrupt mode
 *
 * @return  the mapped gcq driver interrupt mode
 *
 */
static GCQ_INTERRUPT_MODE_TYPE prvxMapInterruptMode( FW_IF_GCQ_INTERRUPT_MODE_TYPE xInterruptMode )
{
    GCQ_INTERRUPT_MODE_TYPE  xMappedMode = MAX_GCQ_INTERRUPT_MODE;

    switch( xInterruptMode )
    {
        case FW_IF_GCQ_INTERRUPT_MODE_NONE:                 xMappedMode = GCQ_INTERRUPT_MODE_POLLING; break;
        case FW_IF_GCQ_INTERRUPT_MODE_TAIL_POINTER_TRIGGER: xMappedMode = GCQ_INTERRUPT_MODE_TAIL_POINTER; break;
        case FW_IF_GCQ_INTERRUPT_MODE_MANUAL_TRIGGER:       xMappedMode = GCQ_INTERRUPT_MODE_INTERRUPT_REG; break;
        case MAX_FW_IF_GCQ_INTERRUPT_MODE:
        default:
            gcq_assert( 0 );
            break;
    }

    return ( xMappedMode );
}

/**
 *
 * @brief   Map consumer/producer mode
 *
 * @param   xMode is the interface mode (producer/consumer)
 *
 * @return  the mapped gcq driver interface mode
 *
 */
static GCQ_MODE_TYPE prvxMapMode( FW_IF_GCQ_MODE_TYPE xMode )
{
    GCQ_MODE_TYPE xMappedMode = MAX_GCQ_MODE_TYPE;

    switch( xMode )
    {
        case FW_IF_GCQ_MODE_PRODUCER:   xMappedMode = GCQ_MODE_TYPE_PRODUCER_MODE; break;
        case FW_IF_GCQ_MODE_CONSUMER:   xMappedMode = GCQ_MODE_TYPE_CONSUMER_MODE; break;
        case MAX_FW_IF_GCQ_MODE:
        default:
            gcq_assert( 0 );
            break;
    }

    return ( xMappedMode );
}

/**
 *
 * @brief   Attempt to find an unused GCQ profile
 *
 * @param   ppxGCQProfile is a variable to store the free GCQ profile
 *
 * @return  FW_IF_ERRORS_NONE if profile found, error otherwise
 *
 */
static uint32_t prvFindNextFreeProfile( FW_IF_GCQ_PROFILE_TYPE **ppxGCQProfile )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_GCQ_ERRORS_NO_FREE_PROFILES;

    if( CHECK_NULL( ppxGCQProfile ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_NOT_NULL( *ppxGCQProfile ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( ulProfilesAllocated > ( GCQ_MAX_INSTANCES - 1 ) ) { return FW_IF_GCQ_ERRORS_NO_FREE_PROFILES; }

    {
        int iIndex = 0;
        for( iIndex = 0; iIndex < GCQ_MAX_INSTANCES; iIndex++ )
        {
            if( FW_IF_GCQ_STATE_CLOSED == xGCQProfile[ iIndex ].xState )
            {
                *ppxGCQProfile = &xGCQProfile[ iIndex ];
                xRet = FW_IF_ERRORS_NONE;
                break;
            }
        }
    }

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t prvGCQOpen( void *pvFWIf )
{
    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_NULL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    {
        FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
        GCQ_INTERRUPT_MODE_TYPE xIntMode = MAX_GCQ_INTERRUPT_MODE;
        GCQ_MODE_TYPE xMode = MAX_GCQ_MODE_TYPE;
        GCQ_FLAGS_TYPE xFlags = 0;
        FW_IF_GCQ_CFG *pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
        FW_IF_GCQ_PROFILE_TYPE *pxGCQProfile = NULL;

        if( FW_IF_ERRORS_NONE != prvFindNextFreeProfile( &pxGCQProfile ) ) { return FW_IF_GCQ_ERRORS_NO_FREE_PROFILES; }
        if( CHECK_NULL( pxGCQProfile ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

        pxGCQProfile->ulIOHandle = 0;
        pxGCQProfile->xState = FW_IF_GCQ_STATE_INIT;
        pxGCQProfile->pxGCQInstance = NULL;
        pxCfg->pvProfile = pxGCQProfile;
        ulProfilesAllocated++;

        /* in user space build we use in memory pointers */
#ifndef __KERNEL__
        xFlags = GCQ_FLAGS_TYPE_IN_MEM_PTR_ENABLE;
#endif
        xIntMode = prvxMapInterruptMode( pxCfg->xInterruptMode );
        xMode = prvxMapMode( pxCfg->xMode );

        /* Initially only interrupt polling mode supported */
        if( xIntMode == GCQ_INTERRUPT_MODE_POLLING )
        {
            FW_IF_GCQ_PROFILE_TYPE *pxProfile = ( FW_IF_GCQ_PROFILE_TYPE* )pxCfg->pvProfile;
            GCQ_ERRORS_TYPE xStatus = MAX_GCQ_ERRORS_TYPE;
            pxProfile->pxGCQInstance = NULL;

            xStatus = xGCQInit( &pxProfile->pxGCQInstance,
                                xLocalCfg.pvIOAccess,
                                xMode,
                                xIntMode,
                                xFlags,
                                pxCfg->ullBaseAddress,
                                pxCfg->ullRingAddress,
                                pxCfg->ulRingLength,
                                pxCfg->ulSubmissionQueueSlotSize,
                                pxCfg->ulCompletionQueueSlotSize );

            /* Update state & attach if in consumer mode */
            if( GCQ_ERRORS_NONE == xStatus )
            {
                pxProfile->xState = FW_IF_GCQ_STATE_OPENED;
                PLL_DBG( FW_IF_GCQ_NAME, "FW_IF_GCQ_open (%s)\r\n", ( FW_IF_GCQ_MODE_PRODUCER == pxCfg->xMode )
                                ? ( "Producer" ):( "Consumer" ));

                if( FW_IF_GCQ_MODE_CONSUMER == pxCfg->xMode )
                {
                    xStatus = xGCQAttachConsumer( pxProfile->pxGCQInstance );
                    if( GCQ_ERRORS_NONE == xStatus )
                    {
                        pxProfile->xState = FW_IF_GCQ_STATE_ATTACHED;
                        PLL_DBG( FW_IF_GCQ_NAME, "Attached ok!\r\n" );
                    }
                }
            }

            /* Map return code */
            xRet = prvxMapIFDriverReturnCode( xStatus );
        }
        else
        {
            xRet = FW_IF_GCQ_ERRORS_NOT_SUPPORTED;
        }

        return xRet;
    }
}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t prvGCQClose( void *pvFWIf )
{
    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_NULL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    {
        FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
        FW_IF_GCQ_PROFILE_TYPE *pxProfile = NULL;
        FW_IF_GCQ_CFG *pxCfg = NULL;
        GCQ_ERRORS_TYPE xStatus = GCQ_ERRORS_NONE;

        pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
        if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

        pxProfile = ( FW_IF_GCQ_PROFILE_TYPE* )pxCfg->pvProfile;
        if( CHECK_INVALID_STATE ( pxProfile ) ) { return FW_IF_GCQ_ERRORS_NOT_SUPPORTED; }

        xStatus = xGCQDeinit( pxProfile->pxGCQInstance );
        if( GCQ_ERRORS_NONE == xStatus )
        {
            pxProfile->xState = FW_IF_GCQ_STATE_CLOSED;
            ulProfilesAllocated--;
        }
        else
        {
            xRet = prvxMapIFDriverReturnCode( xStatus );
        }

        return xRet;
    }
}

/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t prvGCQWrite( void *pvFWIf, uint64_t ullDstPort, uint8_t *pucData, uint32_t ulSize, uint32_t ulTimeoutMs )
{

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_NULL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }
    if( CHECK_NULL( pucData ) ) { return FW_IF_ERRORS_PARAMS; };

    {
        FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
        FW_IF_GCQ_PROFILE_TYPE *pxProfile = NULL;
        FW_IF_GCQ_CFG *pxCfg = NULL;
        GCQ_ERRORS_TYPE xStatus = GCQ_ERRORS_NONE;

        pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
        if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

        pxProfile = ( FW_IF_GCQ_PROFILE_TYPE* )pxCfg->pvProfile;
        if( CHECK_INVALID_STATE ( pxProfile ) ) { return FW_IF_GCQ_ERRORS_NOT_SUPPORTED; }

        xStatus = xGCQProduceData( pxProfile->pxGCQInstance, pucData, ulSize );

        /* Map error return code */
        xRet = prvxMapIFDriverReturnCode( xStatus );

        return xRet;
    }
}

/**
 * @brief   Local implementation of FW_IF_read
 */
static uint32_t prvGCQRead( void *pvFWIf, uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulSize, uint32_t ulTimeoutMs )
{
    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_NULL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }
    if( CHECK_NULL( pulSize ) ) { return FW_IF_ERRORS_PARAMS; };
    if( CHECK_NULL( pucData ) ) { return FW_IF_ERRORS_PARAMS; };

    {
        FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
        FW_IF_GCQ_CFG *pxCfg = NULL;
        FW_IF_GCQ_PROFILE_TYPE *pxProfile = NULL;
        GCQ_ERRORS_TYPE xStatus = GCQ_ERRORS_NONE;

        pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
        if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

        pxProfile = ( FW_IF_GCQ_PROFILE_TYPE* )pxCfg->pvProfile;
        if( CHECK_INVALID_STATE ( pxProfile ) ) { return FW_IF_GCQ_ERRORS_NOT_SUPPORTED; }

        xStatus = xGCQConsumeData( pxProfile->pxGCQInstance, pucData, *pulSize );

        xRet = prvxMapIFDriverReturnCode( xStatus );

        return xRet;
    }
}

/**
 * @brief   Local implementation of FW_IF_ioctrl
 */
static uint32_t prvGCQIOCtrl( void *pvFWIf, uint32_t ulOption, void *pvValue )
{
    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_NULL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    {
        FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
        FW_IF_GCQ_CFG *pxCfg = NULL;
        FW_IF_GCQ_PROFILE_TYPE *pxProfile = NULL;

        pxCfg = ( FW_IF_GCQ_CFG* )pxThisIf->cfg;
        if( CHECK_PROFILE( pxCfg ) ) { return FW_IF_GCQ_ERRORS_INVALID_PROFILE; }

        pxProfile = ( FW_IF_GCQ_PROFILE_TYPE* )pxCfg->pvProfile;

        /*
        * Specialization options supported outside definition of standard API.
        */
        switch( ulOption )
        {
            case FW_IF_COMMON_IOCTRL_FLUSH_TX:
            case FW_IF_COMMON_IOCTRL_FLUSH_RX:
                /*
                * Handle common IOCTL's.
                */
                break;

            case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
                if( FW_IF_GCQ_INTERRUPT_MODE_NONE == pxCfg->xInterruptMode )
                {
                    *( uint32_t* )pvValue = FW_IF_RX_MODE_POLLING;
                }
                else
                {
                    *( uint32_t* )pvValue = FW_IF_RX_MODE_EVENT;
                }
                break;

            case FW_IF_GCQ_IOCTRL_SET_OPAQUE_HANDLE:
                /*
                * Set the opaque handle used by calling API to store a reference.
                */
                pxProfile->ulIOHandle = *( uint32_t* )pvValue;
                PLL_DBG( FW_IF_GCQ_NAME, "GCQ IOCTL - set opaque handle 0x%x\r\n", pxProfile->ulIOHandle );
                break;

            case FW_IF_GCQ_IOCTRL_GET_OPAQUE_HANDLE:
                /*
                *Get the opaque handle used by calling API to retreive a reference.
                */
                PLL_DBG( FW_IF_GCQ_NAME, "GCQ IOCTL - get opaque handle 0x%x\r\n", pxProfile->ulIOHandle );
                *( uint32_t* )pvValue = pxProfile->ulIOHandle;
                break;

            default:
                xRet = FW_IF_ERRORS_UNRECOGNISED_OPTION;
                PLL_DBG( FW_IF_GCQ_NAME, "Error:  GCQ IOCTL unrecognised option\r\n" );
                break;
        }

        return xRet;
    }
}


/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t prvGCQBindCallback( void *pvFWIf, FW_IF_callback *pxNewFunc )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFWIf;
    if( CHECK_NULL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }
    if( CHECK_NULL( pxNewFunc ) ) { return FW_IF_ERRORS_PARAMS; };

    /*
     * Binds in callback provided to the FW_IF.
     * Callback will be invoked when by the driver when event occurs.
     */
    pxThisIf->raiseEvent = pxNewFunc;
    PLL_DBG( FW_IF_GCQ_NAME, "FW_IF_bindCallback called\r\n" );

    return xRet;
}

/*****************************************************************************/
/* public functions                                                          */
/*****************************************************************************/

/**
 * @brief   initialisation function for GCQ interfaces (generic across all GCQ interfaces)
 */
uint32_t ulFW_IF_GCQ_Init( FW_IF_GCQ_INIT_CFG *pxCfg )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
    if( CHECK_NULL( pxCfg ) ) { return FW_IF_ERRORS_PARAMS; };
    if( CHECK_NOT_NULL( pxCfg->pvIOAccess ) ) { return FW_IF_ERRORS_PARAMS; };

    if( FW_IF_FALSE != iInitialised )
    {
        xRet = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else
    {
        /* 
         * Bind in register and memory R/W function pointers
         * and assign to the local profile to be used by all
         * GCQ instances
         */
        xGCQIOAccess.xGCQReadMem32 = prvulReadMemReg32;
        xGCQIOAccess.xGCQWriteMem32 = prvvWriteMemReg32;
        xGCQIOAccess.xGCQReadReg32 = prvulReadMemReg32;
        xGCQIOAccess.xGCQWriteReg32 = prvvWriteMemReg32;
        xLocalCfg.pvIOAccess = &xGCQIOAccess;
        iInitialised = FW_IF_TRUE;

        PLL_DBG( FW_IF_GCQ_NAME, "FW_IF_GCQ_init\r\n" );
    }

    return xRet;
}

/**
 * @brief   opens an instance of the GCQ interface
 */
uint32_t ulFW_IF_GCQ_Create( FW_IF_CFG *xFWIf, FW_IF_GCQ_CFG *xGCQCfg )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_NONE;
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }
    if( CHECK_NULL( xFWIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_NULL( xGCQCfg ) ) { return FW_IF_ERRORS_INVALID_CFG; }

    /* Validate the configuration provided */
    if( ( MAX_FW_IF_GCQ_MODE > xGCQCfg->xMode ) &&
        ( MAX_FW_IF_GCQ_INTERRUPT_MODE > xGCQCfg->xInterruptMode ) )
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
            .cfg            = ( void* )xGCQCfg,
            .lowerFirewall  = GCQ_LOWER_FIREWALL    
        };

        memcpy( xFWIf, &myLocalIf, sizeof( FW_IF_CFG ) );
        PLL_DBG( FW_IF_GCQ_NAME, "FW_IF_GCQ_create\r\n" );
    }
    else
    {
        xRet = FW_IF_ERRORS_INVALID_CFG;
    } 

    return xRet;
}

/**
 * @brief    Print all the stats gathered by the driver
 */
int iFW_IF_GCQ_PrintStatistics( void )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;

    /* TODO implement print stats */

    return xRet;
}

/**
 * @brief    Clears all the stats gathered by the driver
 */
int iFW_IF_GCQ_ClearStatistics( void )
{
    FW_IF_GCQ_ERRORS_TYPE xRet = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;

    /* TODO implement clear stats */

    return xRet;
}
