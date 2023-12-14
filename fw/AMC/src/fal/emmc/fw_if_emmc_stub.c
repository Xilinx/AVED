/**
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*
* This file contains the FW IF EMMC interface implementation stubs.
*
* @file fw_if_emmc_stub.c
*
*/


/*****************************************************************************/
/* includes                                                                  */
/*****************************************************************************/

#include "fw_if_emmc.h"

#include <stdio.h>
#include <string.h>
#include "pll.h"


/*****************************************************************************/
/* defines                                                                   */
/*****************************************************************************/

#define FW_IF_EMMC_NAME         "FW_IF_EMMC"
#define EMMC_UPPER_FIREWALL     ( 0xBEEFCAFE )
#define EMMC_LOWER_FIREWALL     ( 0xDEADFACE )

#define CHECK_DRIVER            if( FW_IF_FALSE == pxThis->iInitialised ) return FW_IF_ERRORS_DRIVER_NOT_INITIALISED
#define CHECK_FIREWALLS( f )    if( ( f->upperFirewall != EMMC_UPPER_FIREWALL ) &&\
                                    ( f->lowerFirewall != EMMC_LOWER_FIREWALL ) ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_HDL( f )          if( NULL == f ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_CFG( f )          if( NULL == ( f )->cfg  ) return FW_IF_ERRORS_INVALID_CFG


/*****************************************************************************/
/* structures                                                                */
/*****************************************************************************/

/**
 * @struct  FW_IF_EMMC_PRIVATE_DATA
 * @brief   Structure to hold this FAL's private data
 */
typedef struct FW_IF_EMMC_PRIVATE_DATA
{
    uint32_t                ulUpperFirewall;

    FW_IF_EMMC_INIT_CFG     xLocalCfg;
    int                     iInitialised;

    uint32_t                ulLowerFirewall;

} FW_IF_EMMC_PRIVATE_DATA;


/*****************************************************************************/
/* local variables                                                           */
/*****************************************************************************/

static FW_IF_EMMC_PRIVATE_DATA xLocalData = 
{
    EMMC_UPPER_FIREWALL,    /* ulUpperFirewall */

    { 0 },                  /* xLocalCfg       */
    FW_IF_FALSE,            /* iInitialised    */

    EMMC_LOWER_FIREWALL     /* ulLowerFirewall */
};
static FW_IF_EMMC_PRIVATE_DATA *pxThis = &xLocalData;


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
static uint32_t ulEmmcOpen( void *pvFwIf );

/**
 * @brief   Local implementation of FW_IF_close
 *
 * @param   pvFwIf          Pointer to this fw_if
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulEmmcClose( void *pvFwIf );

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
static uint32_t ulEmmcWrite( void *pvFwIf,
                             uint64_t ullAddrOffset,
                             uint8_t *pucData,
                             uint32_t ulLength,
                             uint32_t ulTimeoutMs );

/**
 * @brief   Local implementation of FW_IF_read
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ullAddrOffset    The address offset from the start address specified in the create
 * @param   pucData         Data buffer to read
 * @param   pulLength       Pointer to maximum number of bytes allowed in data buffer
 *                          This value is updated to the actual number of bytes read
 * @param   ulTimeoutMs     Time (in ms) to wait for read to complete
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulEmmcRead( void *pvFwIf,
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
static uint32_t ulEmmcIoCtrl( void *pvFwIf, uint32_t ulOption, void *pvValue );

/**
 * @brief   Local implementation of FW_IF_bindCallback
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   pxNewFunc       Function pointer to call
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulEmmcBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc );


/*****************************************************************************/
/* local functions                                                           */
/*****************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t ulEmmcOpen( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );    
    CHECK_DRIVER;

    FW_IF_EMMC_CFG *pxCfg = ( FW_IF_EMMC_CFG* )pxThisIf->cfg;
        
    /*
     * This is where the EMMC driver would be initialised, with the device 
     * ID specified in the FwIf cfg.
     */
    PLL_DBG( FW_IF_EMMC_NAME, "FW_IF_open for if.%u (%s)\r\n",
             ( unsigned int )pxCfg->ulIfId,
             pxCfg->pcIfName );

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t ulEmmcClose( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );    
    CHECK_DRIVER;

    FW_IF_EMMC_CFG *pxCfg = ( FW_IF_EMMC_CFG* )pxThisIf->cfg;

    /*
     * This is where the EMMC driver would be de-initialised, with the device 
     * ID specified in the FwIf cfg.
     */
    PLL_DBG( FW_IF_EMMC_NAME, "FW_IF_close for if.%u (%s)\r\n", 
             ( unsigned int )pxCfg->ulIfId,
             pxCfg->pcIfName );

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t ulEmmcWrite( void *pvFwIf,
                             uint64_t ullAddrOffset,
                             uint8_t *pucData,
                             uint32_t ulLength,
                             uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );    
    CHECK_DRIVER;

    if( NULL != pucData )
    {   
        /*
         * This is where data will be written to the EMMC.
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
static uint32_t ulEmmcRead( void *pvFwIf,
                            uint64_t ullAddrOffset,
                            uint8_t *pucData,
                            uint32_t *pulLength,
                            uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );    
    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( NULL != pulLength ) )
    {   
        /*
         * This is where data will be read from the EMMC.
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
static uint32_t ulEmmcIoCtrl( void *pvFwIf, uint32_t ulOption, void *pvValue )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );    
    CHECK_DRIVER;
    
    FW_IF_EMMC_CFG *pxCfg = ( FW_IF_EMMC_CFG* )pxThisIf->cfg;
    
    switch( ulOption )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            /*
             * Handle common IOCTL's.
             */
            break;

        case FW_IF_EMMC_IOCTRL_PRINT_INSTANCE_DETAILS:
            /*
             * This will call iEMMC_PrintInstanceDetails( ) to print 
             * the EMMC detected device details.
             */
            break;
        
        default:
            ulStatus = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            break;
    }

    if( FW_IF_ERRORS_NONE == ulStatus )
    {
        PLL_DBG( FW_IF_EMMC_NAME, "FW_IF_ioctrl for if.%u (%s), (option %u)\r\n",
                 ( unsigned int )pxCfg->ulIfId,
                 pxCfg->pcIfName,
                 ( unsigned int )ulOption );
    }

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t ulEmmcBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );    
    CHECK_DRIVER;

    if( NULL != pxNewFunc )
    {   
        /*
         * Binds in callback provided to the FW_IF.
         * Callback will be invoked when by the driver when event occurs.
         */
        FW_IF_EMMC_CFG *pxCfg = ( FW_IF_EMMC_CFG* )pxThisIf->cfg;
        pxThisIf->raiseEvent = pxNewFunc;
    
        PLL_DBG( FW_IF_EMMC_NAME, "FW_IF_bindCallback called for if.%u (%s)\r\n",
                 ( unsigned int )pxCfg->ulIfId,
                 pxCfg->pcIfName );
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
 * @brief   initialisation function for EMMC interfaces (generic across all EMMC interfaces)
 */
uint32_t ulFW_IF_EMMC_Init( FW_IF_EMMC_INIT_CFG *pxInitCfg )
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
         * Initilise config data shared between all instances of EMMC.
         */
        pvOSAL_MemCpy( &pxThis->xLocalCfg, pxInitCfg, sizeof( pxThis->xLocalCfg ) );
        PLL_DBG( FW_IF_EMMC_NAME, "ulFW_IF_EMMC_Init for driver.%u (%s)\r\n",
                 ( unsigned int )pxThis->xLocalCfg.ulDriverId,
                 pxThis->xLocalCfg.pcDriverName );
        pxThis->iInitialised = FW_IF_TRUE;
    }

    return ulStatus;
}

/**
 * @brief   opens an instance of the EMMC interface
 */
uint32_t ulFW_IF_EMMC_Create( FW_IF_CFG *pxFwIf, FW_IF_EMMC_CFG *pxEmmcCfg )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( NULL != pxFwIf ) && ( NULL != pxEmmcCfg ) )
    {
        FW_IF_CFG myLocalIf =
        {
            .upperFirewall  = EMMC_UPPER_FIREWALL,
            .open           = &ulEmmcOpen,
            .close          = &ulEmmcClose,
            .write          = &ulEmmcWrite,
            .read           = &ulEmmcRead,
            .ioctrl         = &ulEmmcIoCtrl,
            .bindCallback   = &ulEmmcBindCallback,
            .cfg            = ( void* )pxEmmcCfg,
            .lowerFirewall  = EMMC_LOWER_FIREWALL
        };

        pvOSAL_MemCpy( pxFwIf, &myLocalIf, sizeof( *pxFwIf ) );

        FW_IF_EMMC_CFG *pxCfg = ( FW_IF_EMMC_CFG* )pxFwIf->cfg;

        PLL_DBG( FW_IF_EMMC_NAME, "ulFW_IF_EMMC_Create for if.%u (%s)\r\n",
                 ( unsigned int )pxCfg->ulIfId,
                 pxCfg->pcIfName );
    }
        
    return status;
}
