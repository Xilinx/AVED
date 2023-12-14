/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the FW IF OSPI Stub abstraction.
 *
 * @file fw_if_ospi_stub.c
 *
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "fw_if_ospi.h"
#include "profile_fal.h"


/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define FW_IF_OSPI_NAME                 "FW_IF_OSPI"
#define OSPI_UPPER_FIREWALL             ( 0xBEEFCAFE )
#define OSPI_LOWER_FIREWALL             ( 0xDEADFACE )

#define CHECK_DRIVER                    ( FW_IF_FALSE == iInitialised )
#define CHECK_FIREWALLS( f )            ( ( f->upperFirewall != OSPI_UPPER_FIREWALL ) && \
                                          ( f->lowerFirewall != OSPI_LOWER_FIREWALL ) )

#define CHECK_HDL( f )                  ( NULL == f )
#define CHECK_CFG( f )                  ( NULL == ( f )->cfg )

#define FW_IF_OSPI_STATE_ENTRY( _s )    [ FW_IF_OSPI_STATE_ ## _s ] = #_s


/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

static FW_IF_OSPI_INIT_CFG xLocalCfg = { 0 };

static int iInitialised = FW_IF_FALSE;

static const char* const pcOspiStateModeStr[ ] = { FW_IF_OSPI_STATE_ENTRY( INIT ),
                                                   FW_IF_OSPI_STATE_ENTRY( OPENED ),
                                                   FW_IF_OSPI_STATE_ENTRY( CLOSED ),
                                                   FW_IF_OSPI_STATE_ENTRY( ERROR ) };


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
static uint32_t ulOspiOpen( void *pvFwIf );

/**
 * @brief   Local implementation of FW_IF_close
 *
 * @param   pvFwIf          Pointer to this fw_if
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulOspiClose( void *pvFwIf );

/**
 * @brief   Writes data from an instance of the specific fw_if
 *
 * @param   pvFwIf          Local implementation of FW_IF_write
 * @param   ullAddrOffset   The address offset from the start address specified in the create
 * @param   pucData         Data buffer to write
 * @param   ulLength        Number of bytes in data buffer
 * @param   ulTimeoutMs     Time (in ms) to wait for write to complete
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulOspiWrite( void *pvFwIf,
                              uint64_t ullAddrOffset,
                              uint8_t *pucData,
                              uint32_t ulLength,
                              uint32_t ulTimeoutMs );

/**
 * @brief   Local implementation of FW_IF_read
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ullAddrOffset   The address offset from the start address specified in the create
 * @param   pucData         Data buffer to read
 * @param   pulLength       Pointer to maximum number of bytes allowed in data buffer
 *                          This value is updated to the actual number of bytes read
 * @param   ulTimeoutMs     Time (in ms) to wait for read to complete
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulOspiRead( void *pvFwIf,
                             uint64_t ullAddrOffset,
                             uint8_t *pucData,
                             uint32_t *pulLength,
                             uint32_t ulTimeoutMs );

/**
 * @brief   Local implementation of FW_IF_ioctrl
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ulOption        Unique IO Ctrl option to set/get
 * @param   pvValue         Pointer to value to set/get
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulOspiIoCtrl( void *pvFwIf, uint32_t ulOption, void *pvValue );

/**
 * @brief   Local implementation of FW_IF_bindCallback
 *
 * @param   vFwIf           Pointer to this fw_if
 * @param   pxNewFunc       Function pointer to call
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulOspiBindCallback( void *vFwIf, FW_IF_callback *pxNewFunc );

/**
 * @brief   Validate the read/write access address is in range
 *
 * @param   pxCfg           The interface config options
 * @param   ulAddrOffset    The address offset from the base address specified in the create
 * @param   ulLength        Number of bytes in data buffer
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulValidateAddressRange( FW_IF_OSPI_CFG *pxCfg, uint32_t ulAddrOffset, uint32_t ulLength );


/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/**
 * @brief   initialisation function for OSPI interfaces (generic across all OSPI interfaces)
 */
uint32_t ulFW_IF_OSPI_Init( FW_IF_OSPI_INIT_CFG *pxInitCfg )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != iInitialised )
    {
        xRet = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL != pxInitCfg )
    {
        /*
         * Initilise config data shared between all instances of OSPI.
         */
        memcpy( &xLocalCfg, pxInitCfg, sizeof( FW_IF_OSPI_INIT_CFG ) );

        /*
         * Initialise the driver based on the device id supplied in the xparameters.h
         * and the page size 'ospi_flash_init()', The default page size in versal is 256.
         */
        PLL_DBG( FW_IF_OSPI_NAME, "Device Id:%d\r\n", xLocalCfg.ucOspiDeviceId );
        PLL_DBG( FW_IF_OSPI_NAME, "Page Size:%d\r\n", xLocalCfg.usPageSize );
        iInitialised = FW_IF_TRUE;
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}

/**
 * @brief   opens an instance of the OSPI interface
 */
uint32_t ulFW_IF_OSPI_Create( FW_IF_CFG *pxFwIf, FW_IF_OSPI_CFG *pxOspiCfg )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_ERRORS_NONE;

    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    if( ( NULL != pxFwIf ) &&
        ( NULL != pxOspiCfg ) )
    {

        FW_IF_CFG myLocalIf =
        {
            .upperFirewall  = OSPI_UPPER_FIREWALL,
            .open           = &ulOspiOpen,
            .close          = &ulOspiClose,
            .write          = &ulOspiWrite,
            .read           = &ulOspiRead,
            .ioctrl         = &ulOspiIoCtrl,
            .bindCallback   = &ulOspiBindCallback,
            .cfg            = ( void* )pxOspiCfg,
            .lowerFirewall  = OSPI_LOWER_FIREWALL
        };

        memcpy( pxFwIf, &myLocalIf, sizeof( FW_IF_CFG ) );

        FW_IF_OSPI_CFG *pxCfg = ( FW_IF_OSPI_CFG* )pxFwIf->cfg;

        /*
         * Configuration options, start address will the offset to the RPU/APU
         * or SC address within the flash device
         */
        PLL_DBG( FW_IF_OSPI_NAME, "Start Address: 0x%x\r\n", pxCfg->ulBaseAddress );
        PLL_DBG( FW_IF_OSPI_NAME, "Length: 0x%x\r\n", pxCfg->ulLength );
        PLL_DBG( FW_IF_OSPI_NAME, "Flag Erase Before Write: %d\r\n", pxCfg->ucEraseBeforeWriteFlag );
        pxCfg->xState = FW_IF_OSPI_STATE_INIT;
        xRet = FW_IF_ERRORS_NONE;
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}


/******************************************************************************/
/* Local Function Implementations                                             */
/******************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t ulOspiOpen( void *pvFwIf )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_OSPI_CFG *pxCfg = ( FW_IF_OSPI_CFG* )pxThisIf->cfg;
    if( FW_IF_OSPI_STATE_INIT == pxCfg->xState )
    {
        pxCfg->xState = FW_IF_OSPI_STATE_OPENED;
        PLL_DBG( FW_IF_OSPI_NAME, "OSPI FW_IF_open\r\n" );
    }
    else
    {
        xRet = FW_IF_OSPI_ERRORS_INVALID_STATE;
        PLL_ERR( FW_IF_OSPI_NAME, "Error: open() should only be called from init state [%s]\r\n",
                 pcOspiStateModeStr[ pxCfg->xState ] );
    }

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t ulOspiClose( void *pvFwIf )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_OSPI_CFG *pxCfg = ( FW_IF_OSPI_CFG* )pxThisIf->cfg;
    if( FW_IF_OSPI_STATE_OPENED == pxCfg->xState )
    {
        pxCfg->xState = FW_IF_OSPI_STATE_CLOSED;
        PLL_DBG( FW_IF_OSPI_NAME, "OSPI FW_IF_close\r\n" );
    }
    else
    {
        xRet = FW_IF_OSPI_ERRORS_INVALID_STATE;
        PLL_DBG( FW_IF_OSPI_NAME, "Error: close() should only be called from opened state [%s]\r\n",
                 pcOspiStateModeStr[ pxCfg->xState ] );
    }

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t ulOspiWrite( void *pvFwIf,
                             uint64_t ullAddrOffset,
                             uint8_t *pucData,
                             uint32_t ulLength,
                             uint32_t ulTimeoutMs )
{
    FW_IF_ERRORS xRet = FW_IF_ERRORS_NONE;
    uint32_t ulAddrOffset = ( uint32_t )ullAddrOffset;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_OSPI_CFG *pxCfg = ( FW_IF_OSPI_CFG* )pxThisIf->cfg;
    if( ( NULL != pucData ) )
    {
        if( FW_IF_OSPI_STATE_OPENED == pxCfg->xState )
        {
            PLL_DBG( FW_IF_OSPI_NAME, "OSPI FW_IF_write Offset:0x%x Addr:0x%x\r\n",
                     ulAddrOffset,
                     pxCfg->ulBaseAddress + ulAddrOffset);
   
            xRet = ulValidateAddressRange( pxCfg, ulAddrOffset, ulLength );
            if( FW_IF_ERRORS_NONE == xRet )
            {
                /* Check flag to see if erase if required before write */
                if( FW_IF_TRUE == pxCfg->ucEraseBeforeWriteFlag )
                {
                    /* Call ospi function: ospi_flash_erase() */
                    PLL_DBG( FW_IF_OSPI_NAME, "OSPI FW_IF_write: erase before write\r\n" );
                }

                /* Call ospi function: ospi_flash_write() */
            }
        }
        else
        {
            xRet = FW_IF_OSPI_ERRORS_INVALID_STATE;
            PLL_ERR( FW_IF_OSPI_NAME, "Error: write() should only be called from opened state [%s]\r\n",
                     pcOspiStateModeStr[ pxCfg->xState ] );
        }
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
static uint32_t ulOspiRead( void *pvFwIf,
                            uint64_t ullAddrOffset,
                            uint8_t *pucData,
                            uint32_t *pulLength,
                            uint32_t ulTimeoutMs )
{
    FW_IF_ERRORS xRet = FW_IF_ERRORS_NONE;
    uint32_t ulAddrOffset = ( uint32_t )ullAddrOffset;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    FW_IF_OSPI_CFG *pxCfg = ( FW_IF_OSPI_CFG* )pxThisIf->cfg;

    if( ( NULL != pucData ) &&
        ( NULL != pulLength ) )
    {
        if( FW_IF_OSPI_STATE_OPENED == pxCfg->xState )
        {
            PLL_DBG( FW_IF_OSPI_NAME, "OSPI FW_IF_read Offset:0x%x Addr:0x%x\r\n",
                     ulAddrOffset,
                     pxCfg->ulBaseAddress + ulAddrOffset);

            xRet = ulValidateAddressRange( pxCfg, ulAddrOffset, *pulLength );
            if( FW_IF_ERRORS_NONE == xRet )
            {
                /*
                 * Call ospi function: ospi_flash_read() and return the number
                 * of bytes read using ulSize
                 */
            }
        }
        else
        {
            xRet = FW_IF_OSPI_ERRORS_INVALID_STATE;
            PLL_DBG( FW_IF_OSPI_NAME, "Error: read() should only be called from opened state [%s]\r\n",
                     pcOspiStateModeStr[ pxCfg->xState ] );
        }
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
static uint32_t ulOspiIoCtrl( void *pvFwIf, uint32_t ulOption, void *pvValue )
{
    FW_IF_ERRORS xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

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

    case FW_IF_OSPI_IOCTL_GET_PROGRESS:
        /*
         * Will call ospi_flash_progress( ) to get the progress of the operation
         */
        break;

    default:
        xRet = FW_IF_ERRORS_UNRECOGNISED_OPTION;
        PLL_ERR( FW_IF_OSPI_NAME, "OSPI IOCTL - Unrecognised option\r\n" );
        break;
    }

    if( FW_IF_ERRORS_NONE == xRet )
    {
        PLL_DBG( FW_IF_OSPI_NAME, "OSPI FW_IF_ioctl\r\n" );
    }

    return xRet;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t ulOspiBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    if( CHECK_HDL( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_CFG( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_CFG; }
    if( CHECK_FIREWALLS( pxThisIf ) ) { return FW_IF_ERRORS_INVALID_HANDLE; }
    if( CHECK_DRIVER ) { return FW_IF_ERRORS_DRIVER_NOT_INITIALISED; }

    if( NULL != pxNewFunc )
    {
        /*
         * Binds in callback provided to the FW_IF.
         * Callback will be invoked when by the driver when event occurs.
         */
        pxThisIf->raiseEvent = pxNewFunc;
        PLL_DBG( FW_IF_OSPI_NAME, "OSPI FW_IF_bindCallback called\r\n" );
    }
    else
    {
        xRet = FW_IF_ERRORS_PARAMS;
    }

    return xRet;
}

/**
 * @brief   Validate the read/write access address is in range
 */
static uint32_t ulValidateAddressRange( FW_IF_OSPI_CFG *pxCfg, uint32_t ulAddrOffset, uint32_t ulLength )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_OSPI_ERRORS_DRIVER_INVALID_ADDRESS;

    if( NULL != pxCfg )
    {
        /*
         * Test the offset plus the length is less than the total length
         */
        if( ( ulAddrOffset + ulLength ) <= pxCfg->ulLength )
        {
            xRet = FW_IF_ERRORS_NONE;
        }
    }

    return xRet;
}

/**
 * @brief    Print all the stats gathered by the driver
 */
int iFW_IF_OSPI_PrintStatistics( void )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
    
    /* TODO implement print stats */

    return xRet;
}

/**
 * @brief    Clears all the stats gathered by the driver
 */
int iFW_IF_OSPI_ClearStatistics( void )
{
    FW_IF_OSPI_ERRORS xRet = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;

    /* TODO implement clear stats */

    return xRet;
}

