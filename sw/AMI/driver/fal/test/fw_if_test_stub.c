/**
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*
* This header file contains the FW IF test interface implementation stubs.
*
* @file fw_if_test_stub.c
*
*/

/*****************************************************************************/
/* includes                                                                  */
/*****************************************************************************/

#include "fw_if_test.h"

#include <stdio.h>
#include <string.h>
#include "pll.h"

/*****************************************************************************/
/* defines                                                                   */
/*****************************************************************************/

#define TEST_UPPER_FIREWALL     ( 0xBEEFCAFE )
#define TEST_LOWER_FIREWALL     ( 0xDEADFACE )

#define CHECK_DRIVER            if( FW_IF_FALSE == iInitialised ) return FW_IF_ERRORS_DRIVER_NOT_INITIALISED
#define CHECK_FIREWALLS( f )    if( ( f->upperFirewall != TEST_UPPER_FIREWALL ) &&\
                                    ( f->lowerFirewall != TEST_LOWER_FIREWALL ) ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_HDL( f )          if( NULL == f ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_CFG( f )          if( NULL == ( f )->cfg  ) return FW_IF_ERRORS_INVALID_CFG

#define TEST_PRINT              if( FW_IF_FALSE == myLocalCfg.debugPrint) { } else vPLL_Printf


/*****************************************************************************/
/* local variables                                                           */
/*****************************************************************************/

const uint8_t myRxMode = FW_IF_RX_MODE_POLLING;

static FW_IF_TEST_INIT_CFG myLocalCfg = { 0 };

static int iInitialised = FW_IF_FALSE;
static int iDebugPrint  = FW_IF_FALSE;

static uint8_t  *   testRxData = NULL;
static uint32_t     testRxSize = 0;


/*****************************************************************************/
/* local functions                                                           */
/*****************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t testOpen( void *fwIf )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    FW_IF_TEST_CFG *thisTestCfg = ( FW_IF_TEST_CFG* )thisIf->cfg;
        
    TEST_PRINT( "FW_IF_open for if.%u (%s)\r\n",
            ( unsigned int )thisTestCfg->ifId,
            thisTestCfg->ifName );

    return status;
}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t testClose( void *fwIf )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    FW_IF_TEST_CFG *thisTestCfg = ( FW_IF_TEST_CFG* )thisIf->cfg;
    
    TEST_PRINT( "FW_IF_close for if.%u (%s)\r\n", 
            ( unsigned int )thisTestCfg->ifId,
            thisTestCfg->ifName );

    return status;
}


/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t testWrite( void *fwIf, uint32_t dstPort, uint8_t * data, uint32_t size, uint32_t timeoutMs )
{
    uint32_t status = FW_IF_ERRORS_NONE;
    int i = 0;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    if( ( NULL != data ) && ( FW_IF_TEST_MAX_DATA >=size ) )
    {
        FW_IF_TEST_CFG *thisTestCfg = ( FW_IF_TEST_CFG* )thisIf->cfg;
        
        TEST_PRINT( "FW_IF_write from if.%u (%s) to port.%u\r\n",
                ( unsigned int )thisTestCfg->ifId,
                thisTestCfg->ifName,
                ( unsigned int )dstPort );

        TEST_PRINT( "Writing %d bytes...", ( unsigned int )size );
        for( i = 0; i < size; i++ )
        {
            if( 0 == i % 16 )
            {
                TEST_PRINT( "\r\n[%02x] ", i );
            }
            
            TEST_PRINT( "%02x ", data[ i ] );
        }
        TEST_PRINT( "\r\n" );
    }
    else
    {
        status = FW_IF_ERRORS_PARAMS;
    }

    return status;
}

/**
 * @brief   Local implementation of FW_IF_read
 */
static uint32_t testRead( void *fwIf, uint32_t srcPort, uint8_t * data, uint32_t * size, uint32_t timeoutMs )
{
    uint32_t status = FW_IF_ERRORS_NONE;
    int i = 0;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    if( ( NULL != data ) && ( NULL != size ) && ( FW_IF_TEST_MAX_DATA >= *size ) )
    {
        FW_IF_TEST_CFG *thisTestCfg = ( FW_IF_TEST_CFG* )thisIf->cfg;
        
        TEST_PRINT( "FW_IF_read at if.%u (%s) from port.%u\r\n",
                ( unsigned int )thisTestCfg->ifId,
                thisTestCfg->ifName,
                ( unsigned int )srcPort );

        *size = testRxSize;
        memcpy( data, testRxData, *size );
        
        TEST_PRINT( "Reading %u bytes...", ( unsigned int )*size );
        for( i = 0; i < *size; i++ )
        {
            if( 0 == i % 16 )
            {
                TEST_PRINT( "\r\n[%02x] ", i );
            }
            
            TEST_PRINT( "%02x ", data[ i ] );
        }
        TEST_PRINT( "\r\n" );
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
static uint32_t testIoctrl( void *fwIf, uint32_t option, void * value )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;
    
    FW_IF_TEST_CFG *thisTestCfg = ( FW_IF_TEST_CFG* )thisIf->cfg;
    
    switch( option )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
            break;

        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            *(uint8_t* )value = myRxMode;
            break;

        case FW_IF_TEST_IOCTRL_ENABLE_DEBUG_PRINT:
            iDebugPrint = FW_IF_TRUE;
            break;

        case FW_IF_TEST_IOCTRL_DISABLE_DEBUG_PRINT:
            iDebugPrint = FW_IF_FALSE;
            break;

        case FW_IF_TEST_IOCTRL_SET_NEXT_RX_DATA:
            if( NULL != value )
            {
                testRxData = ( uint8_t * )value;
            }
            else
            {
                status = FW_IF_ERRORS_PARAMS;
            }
            break;

        case FW_IF_TEST_IOCTRL_SET_NEXT_RX_SIZE:
            if( ( NULL != value ) && ( FW_IF_TEST_MAX_DATA >= *( uint32_t* )value ) )
            {
                testRxSize = *( uint32_t* )value;
            }
            else
            {
                status = FW_IF_ERRORS_PARAMS;
            }
            break;

        case FW_IF_TEST_IOCTRL_TRIGGER_EVENT:
            if( NULL != value )
            {
                if( NULL != thisIf->raiseEvent )
                {
                    if( ( *( uint16_t* )value == FW_IF_COMMON_EVENT_NEW_RX_DATA ) && ( NULL != testRxData ) )
                    {
                        thisIf->raiseEvent( *( uint16_t* )value, testRxData, testRxSize );
                    }
                    else
                    {
                        thisIf->raiseEvent( *( uint16_t* )value, NULL, 0 );
                    }
                }
            }
            else
            {
                status = FW_IF_ERRORS_PARAMS;
            }
            break;
        
        default:
            status = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            break;
    }

    TEST_PRINT( "FW_IF_ioctrl for if.%u (%s), (option %u)\r\n",
            ( unsigned int )thisTestCfg->ifId,
            thisTestCfg->ifName,
            ( unsigned int )option );

    return status;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t testBindCallback( void *fwIf, FW_IF_callback * newFunc )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    FW_IF_CFG *thisIf = ( FW_IF_CFG* )fwIf;
    CHECK_HDL( thisIf );
    CHECK_CFG( thisIf );
    CHECK_FIREWALLS( thisIf );    
    CHECK_DRIVER;

    if( NULL != newFunc )
    {
        FW_IF_TEST_CFG *thisTestCfg = ( FW_IF_TEST_CFG* )thisIf->cfg;
        
        thisIf->raiseEvent = newFunc;
    
        TEST_PRINT( "FW_IF_bindCallback called for if.%u (%s)\r\n",
                ( unsigned int )thisTestCfg->ifId,
                thisTestCfg->ifName );
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
 * @brief   initialisation function for test interfaces (generic across all test interfaces)
 */
uint32_t FW_IF_test_init( FW_IF_TEST_INIT_CFG * cfg )
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
        memcpy( &myLocalCfg, cfg, sizeof( FW_IF_TEST_INIT_CFG ) );
        iInitialised = FW_IF_TRUE;

        TEST_PRINT( "FW_IF_test_init for driver.%u (%s)\r\n",
                ( unsigned int )myLocalCfg.driverId,
                myLocalCfg.driverName );
    }

    return status;
}

/**
 * @brief   opens an instance of the test interface
 */
uint32_t FW_IF_test_create( FW_IF_CFG *fwIf, FW_IF_TEST_CFG *testCfg )
{
    uint32_t status = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( NULL != fwIf ) && ( NULL != testCfg ) )
    {
        FW_IF_CFG myLocalIf =
        {
            .upperFirewall  = TEST_UPPER_FIREWALL,
            .open           = &testOpen,
            .close          = &testClose,
            .write          = &testWrite,
            .read           = &testRead,
            .ioctrl         = &testIoctrl,
            .bindCallback   = &testBindCallback,
            .cfg            = ( void* )testCfg,
            .lowerFirewall  = TEST_LOWER_FIREWALL
        };

        memcpy( fwIf, &myLocalIf, sizeof( FW_IF_CFG ) );

        FW_IF_TEST_CFG *thisTestCfg = ( FW_IF_TEST_CFG* )fwIf->cfg;

        TEST_PRINT( "FW_IF_test_create for if.%u (%s)\r\n",
                ( unsigned int )thisTestCfg->ifId,
                thisTestCfg->ifName );
    }
        
    return status;
}
