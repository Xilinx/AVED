/**
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*
* This file contains the FW IF SMBus interface implementation stub.
*
* @file fw_if_smbus_stub.c
*
*/

/*****************************************************************************/
/* includes                                                                  */
/*****************************************************************************/

#include "fw_if_smbus.h"

#include <stdio.h>
#include <string.h>


/*****************************************************************************/
/* defines                                                                   */
/*****************************************************************************/

#define SMBUS_UPPER_FIREWALL    ( 0xBEEFCAFE )
#define SMBUS_LOWER_FIREWALL    ( 0xDEADFACE )

#define CHECK_DRIVER            if( FW_IF_FALSE == iInitialised ) return FW_IF_ERRORS_DRIVER_NOT_INITIALISED
#define CHECK_FIREWALLS( f )    if( ( f->upperFirewall != SMBUS_UPPER_FIREWALL ) &&\
                                    ( f->lowerFirewall != SMBUS_LOWER_FIREWALL ) ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_HDL( f )          if( NULL == f ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_CFG( f )          if( NULL == ( f )->cfg  ) return FW_IF_ERRORS_INVALID_CFG


/*****************************************************************************/
/* local variables                                                           */
/*****************************************************************************/

static const uint8_t myRxMode = FW_IF_RX_MODE_POLLING | FW_IF_RX_MODE_EVENT;

static FW_IF_SMBUS_INIT_CFG myLocalCfg = { 0 };

static int iInitialised = FW_IF_FALSE;


/*****************************************************************************/
/* local functions                                                           */
/*****************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t ulSmbusOpen( void *pvFwIf )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_SMBUS_CFG *pxThisSmbusCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;
        
    /*
     * this is where the SMBus device would be created/enabled
     */

    printf( "SMBus FW_IF_open for addr 0x%02X\r\n", pxThisSmbusCfg->port );

    return status;
}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t ulSmbusClose( void *pvFwIf )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_SMBUS_CFG *pxThisSmbusCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;
    
    /*
     * this is where the SMBus device would be destroyed/disabled
     */

    printf( "SMBus FW_IF_close for addr 0x%02X\r\n", pxThisSmbusCfg->port );

    return status;
}


/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t ulSmbusWrite( void *pvFwIf, uint32_t ulDstPort, uint8_t *pucData, uint32_t ulSize, uint32_t ulTimeoutMs )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( FW_IF_SMBUS_MAX_DATA >=ulSize ) )
    {
        FW_IF_SMBUS_CFG *pxThisSmbusCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;
        
        /*
         * this is where the SMBus data would be tx'd
         *
         *  - Controller: initiate a WRITE command
         *  - Target    : load the response data
         */

        printf( "SMBus FW_IF_write (%s) from addr 0x%02X to addr 0x%02X\r\n",
                ( FW_IF_SMBUS_ROLE_CONTROLLER == pxThisSmbusCfg->role ) ? ( "controller" ):( "target" ),
                pxThisSmbusCfg->port, ulDstPort );
    }
    else
    {
        status == FW_IF_ERRORS_PARAMS;
    }

    return status;
}

/**
 * @brief   Local implementation of FW_IF_read
 */
static uint32_t ulSmbusRead( void *pvFwIf, uint32_t ulSrcPort, uint8_t *pucData, uint32_t *pulSize, uint32_t ulTimeoutMs )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( NULL != pulSize ) && ( FW_IF_SMBUS_MAX_DATA >= *pulSize ) )
    {
        FW_IF_SMBUS_CFG *pxThisSmbusCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;
        
        /*
         * this is where the SMBus data would be rx'd
         *
         *  - Controller: initiate a READ command
         *  - Target    : retrieve any unread received data
         */

        printf( "SMBus FW_IF_read (%s) at addr 0x%02X from addr 0x%02X\r\n",
                ( FW_IF_SMBUS_ROLE_CONTROLLER == pxThisSmbusCfg->role ) ? ( "controller" ):( "target" ),
                pxThisSmbusCfg->port, ulSrcPort );
    }
    else
    {
        status = FW_IF_ERRORS_PARAMS;
    }

    return status;
}

/**
 * @brief   Local implementation of FW_IF_ioctrl
 */
static uint32_t ulSmbusIoctrl( void *pvFwIf, uint32_t ulOption, void *pvValue )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;
    
    FW_IF_SMBUS_CFG *pxThisSmbusCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;
    
    printf( "SMBus FW_IF_ioctrl for addr 0x%02X (option %u)\r\n", pxThisSmbusCfg->port, ulOption );
    
    switch( ulOption )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
            break;

        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            *(uint8_t* )pvValue = myRxMode;
            break;

        case FW_IF_SMBUS_IOCTRL_SET_CONTROLLER:
            pxThisSmbusCfg->role = FW_IF_SMBUS_ROLE_CONTROLLER;
            printf( "SMBus addr 0x%02X - Controller\r\n", pxThisSmbusCfg->port );
            break;

        case FW_IF_SMBUS_IOCTRL_SET_TARGET:
            pxThisSmbusCfg->role = FW_IF_SMBUS_ROLE_TARGET;
            printf( "SMBus addr 0x%02X - Target\r\n", pxThisSmbusCfg->port );
            break;
        
        default:
            status = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            printf( "SMBus addr 0x%02X - Unrecognised option\r\n", pxThisSmbusCfg->port );
            break;
    }


    return status;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t ulSmbusBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( NULL != pxNewFunc )
    {
        FW_IF_SMBUS_CFG *pxThisSmbusCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;
        
        pxThisIf->raiseEvent = pxNewFunc;
    
        printf( "SMBus FW_IF_bindCallback called for port 0x%02X\r\n", pxThisSmbusCfg->port );
    }
    else
    {
        status = FW_IF_ERRORS_PARAMS;
    }


    return status;
}


/*****************************************************************************/
/* public functions                                                          */
/*****************************************************************************/

/**
 * @brief   initialisation function for smbus interfaces (generic across all smbus interfaces)
 */
uint32_t ulFW_IF_SMBUS_Init( FW_IF_SMBUS_INIT_CFG *pxCfg )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != iInitialised )
    {
        status = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL == pxCfg )
    {
        status = FW_IF_ERRORS_PARAMS;
    }
    else
    {
        /*
         * This is where the SMBus driver would be initialised
         */

        memcpy( &myLocalCfg, pxCfg, sizeof( FW_IF_SMBUS_INIT_CFG ) );
        iInitialised = FW_IF_TRUE;

        printf( "ulFW_IF_SMBUS_Init, addr: 0x%08X, baudrate: %uHz\r\n",
                myLocalCfg.baseAddr,
                myLocalCfg.baudRate );

    }

    return status;
}

/**
 * @brief   opens an instance of the smbus interface
 */
uint32_t ulFW_IF_SMBUS_Create( FW_IF_CFG *pvFwIf, FW_IF_SMBUS_CFG *pxSmbusCfg )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( NULL != pvFwIf ) && ( NULL != pxSmbusCfg ) )
    {
        if( ( MAX_FW_IF_SMBUS_ROLE > pxSmbusCfg->role ) && ( NULL != pxSmbusCfg->udid ) )
        {
            FW_IF_CFG myLocalIf =
            {
                .upperFirewall  = SMBUS_UPPER_FIREWALL,
                .open           = &ulSmbusOpen,
                .close          = &ulSmbusClose,
                .write          = &ulSmbusWrite,
                .read           = &ulSmbusRead,
                .ioctrl         = &ulSmbusIoctrl,
                .bindCallback   = &ulSmbusBindCallback,
                .cfg            = ( void* )pxSmbusCfg,
                .lowerFirewall  = SMBUS_LOWER_FIREWALL
            };

            memcpy( pvFwIf, &myLocalIf, sizeof( FW_IF_CFG ) );

            FW_IF_SMBUS_CFG *pxThisSmbusCfg = ( FW_IF_SMBUS_CFG* )pvFwIf->cfg;

            printf( "ulFW_IF_SMBUS_Create for addr 0x%02X (%s)\r\n",
                    pxThisSmbusCfg->port,
                    ( FW_IF_SMBUS_ROLE_CONTROLLER == pxThisSmbusCfg->role ) ? ( "Controller" ):( "Target" ) );
        }
        else
        {
            status = FW_IF_ERRORS_INVALID_CFG;
        }
    }
    else
    {
        FW_IF_ERRORS_PARAMS;
    }
        
    return status;
}

