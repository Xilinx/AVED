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
static uint32_t smbusOpen( void *fwIf )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    FW_IF_SMBUS_CFG *thisSmbusCfg = ( FW_IF_SMBUS_CFG* )thisIf->cfg;
        
    /*
     * this is where the SMBus device would be created/enabled
     */

    printf( "SMBus FW_IF_open for addr 0x%02X\r\n", thisSmbusCfg->port );

    return status;
}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t smbusClose( void *fwIf )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    FW_IF_SMBUS_CFG *thisSmbusCfg = ( FW_IF_SMBUS_CFG* )thisIf->cfg;
    
    /*
     * this is where the SMBus device would be destroyed/disabled
     */

    printf( "SMBus FW_IF_close for addr 0x%02X\r\n", thisSmbusCfg->port );

    return status;
}


/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t smbusWrite( void *fwIf, uint32_t dstPort, uint8_t * data, uint32_t size, uint32_t timeoutMs )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    if( ( NULL != data ) && ( FW_IF_SMBUS_MAX_DATA >=size ) )
    {
        FW_IF_SMBUS_CFG *thisSmbusCfg = ( FW_IF_SMBUS_CFG* )thisIf->cfg;
        
        /*
         * this is where the SMBus data would be tx'd
         *
         *  - Controller: initiate a WRITE command
         *  - Target    : load the response data
         */

        printf( "SMBus FW_IF_write (%s) from addr 0x%02X to addr 0x%02X\r\n",
                ( FW_IF_SMBUS_ROLE_CONTROLLER == thisSmbusCfg->role ) ? ( "controller" ):( "target" ),
                thisSmbusCfg->port, dstPort );
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
static uint32_t smbusRead( void *fwIf, uint32_t srcPort, uint8_t * data, uint32_t * size, uint32_t timeoutMs )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    if( ( NULL != data ) && ( NULL != size ) && ( FW_IF_SMBUS_MAX_DATA >= *size ) )
    {
        FW_IF_SMBUS_CFG *thisSmbusCfg = ( FW_IF_SMBUS_CFG* )thisIf->cfg;
        
        /*
         * this is where the SMBus data would be rx'd
         *
         *  - Controller: initiate a READ command
         *  - Target    : retrieve any unread received data
         */

        printf( "SMBus FW_IF_read (%s) at addr 0x%02X from addr 0x%02X\r\n",
                ( FW_IF_SMBUS_ROLE_CONTROLLER == thisSmbusCfg->role ) ? ( "controller" ):( "target" ),
                thisSmbusCfg->port, srcPort );
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
static uint32_t smbusIoctrl( void *fwIf, uint32_t option, void * value )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;
    
    FW_IF_SMBUS_CFG *thisSmbusCfg = ( FW_IF_SMBUS_CFG* )thisIf->cfg;
    
    printf( "SMBus FW_IF_ioctrl for addr 0x%02X (option %u)\r\n", thisSmbusCfg->port, option );
    
    switch( option )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
            break;

        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            *(uint8_t* )value = myRxMode;
            break;

        case FW_IF_SMBUS_IOCTRL_SET_CONTROLLER:
            thisSmbusCfg->role = FW_IF_SMBUS_ROLE_CONTROLLER;
            printf( "SMBus addr 0x%02X - Controller\r\n", thisSmbusCfg->port );
            break;

        case FW_IF_SMBUS_IOCTRL_SET_TARGET:
            thisSmbusCfg->role = FW_IF_SMBUS_ROLE_TARGET;
            printf( "SMBus addr 0x%02X - Target\r\n", thisSmbusCfg->port );
            break;
        
        default:
            status = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            printf( "SMBus addr 0x%02X - Unrecognised option\r\n", thisSmbusCfg->port );
            break;
    }


    return status;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t smbusBindCallback( void *fwIf, FW_IF_callback * newFunc )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    if( NULL != newFunc )
    {
        FW_IF_SMBUS_CFG *thisSmbusCfg = ( FW_IF_SMBUS_CFG* )thisIf->cfg;
        
        thisIf->raiseEvent = newFunc;
    
        printf( "SMBus FW_IF_bindCallback called for port 0x%02X\r\n", thisSmbusCfg->port );
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
uint32_t FW_IF_smbus_init( FW_IF_SMBUS_INIT_CFG * cfg )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != iInitialised )
    {
        status = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL == cfg )
    {
        status = FW_IF_ERRORS_PARAMS;
    }
    else
    {
        /*
         * This is where the SMBus driver would be initialised
         */

        memcpy( &myLocalCfg, cfg, sizeof( FW_IF_SMBUS_INIT_CFG ) );
        iInitialised = FW_IF_TRUE;

        printf( "FW_IF_smbus_init, addr: 0x%08X, baudrate: %uHz\r\n",
                myLocalCfg.baseAddr,
                myLocalCfg.baudRate );

    }

    return status;
}

/**
 * @brief   opens an instance of the smbus interface
 */
uint32_t FW_IF_smbus_create( FW_IF_CFG *fwIf, FW_IF_SMBUS_CFG *smbusCfg )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( NULL != fwIf ) && ( NULL != smbusCfg ) )
    {
        if( ( MAX_FW_IF_SMBUS_ROLE > smbusCfg->role ) && ( NULL != smbusCfg->udid ) )
        {
            FW_IF_CFG myLocalIf =
            {
                .upperFirewall  = SMBUS_UPPER_FIREWALL,
                .open           = &smbusOpen,
                .close          = &smbusClose,
                .write          = &smbusWrite,
                .read           = &smbusRead,
                .ioctrl         = &smbusIoctrl,
                .bindCallback   = &smbusBindCallback,
                .cfg            = ( void* )smbusCfg,
                .lowerFirewall  = SMBUS_LOWER_FIREWALL
            };

            memcpy( fwIf, &myLocalIf, sizeof( FW_IF_CFG ) );

            FW_IF_SMBUS_CFG *thisSmbusCfg = ( FW_IF_SMBUS_CFG* )fwIf->cfg;

            printf( "FW_IF_smbus_create for addr 0x%02X (%s)\r\n",
                    thisSmbusCfg->port,
                    ( FW_IF_SMBUS_ROLE_CONTROLLER == thisSmbusCfg->role ) ? ( "Controller" ):( "Target" ) );
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

