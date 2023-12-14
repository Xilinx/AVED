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

#define TEST_PRINT              if( FW_IF_FALSE == xMyLocalCfg.debugPrint) { } else vPLL_Printf


/*****************************************************************************/
/* local variables                                                           */
/*****************************************************************************/

const uint8_t myRxMode = FW_IF_RX_MODE_POLLING;

static FW_IF_TEST_INIT_CFG xMyLocalCfg = { 0 };

static int iInitialised = FW_IF_FALSE;
static int iDebugPrint  = FW_IF_FALSE;

static uint8_t  *pucTestRxData = NULL;
static uint32_t ulTestRxSize = 0;


/*****************************************************************************/
/* local functions                                                           */
/*****************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 */
static uint32_t testOpen( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_TEST_CFG *pxThisTestCfg = ( FW_IF_TEST_CFG* )pxThisIf->cfg;
        
    TEST_PRINT( "FW_IF_open for if.%u (%s)\r\n",
            ( unsigned int )pxThisTestCfg->ifId,
            pxThisTestCfg->ifName );

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_close
 */
static uint32_t testClose( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_TEST_CFG *pxThisTestCfg = ( FW_IF_TEST_CFG* )pxThisIf->cfg;
    
    TEST_PRINT( "FW_IF_close for if.%u (%s)\r\n", 
            ( unsigned int )pxThisTestCfg->ifId,
            pxThisTestCfg->ifName );

    return ulStatus;
}


/**
 * @brief   Local implementation of FW_IF_write
 */
static uint32_t testWrite( void *pvFwIf, uint64_t ullDstPort, uint8_t *pucData, uint32_t ulSize, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;
    int i = 0;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( FW_IF_TEST_MAX_DATA >=ulSize ) )
    {
        FW_IF_TEST_CFG *pxThisTestCfg = ( FW_IF_TEST_CFG* )pxThisIf->cfg;
        
        TEST_PRINT( "FW_IF_write from if.%u (%s) to port.%u\r\n",
                ( unsigned int )pxThisTestCfg->ifId,
                pxThisTestCfg->ifName,
                ( unsigned int )ullDstPort );

        TEST_PRINT( "Writing %d bytes...", ( unsigned int )ulSize );
        for( i = 0; i < ulSize; i++ )
        {
            if( 0 == i % 16 )
            {
                TEST_PRINT( "\r\n[%02x] ", i );
            }
            
            TEST_PRINT( "%02x ", pucData[ i ] );
        }
        TEST_PRINT( "\r\n" );
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
static uint32_t testRead( void *pvFwIf, uint64_t ullOffset, uint8_t *pucData, uint32_t *pulSize, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;
    int i = 0;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( NULL != pulSize ) && ( FW_IF_TEST_MAX_DATA >= *pulSize ) )
    {
        FW_IF_TEST_CFG *pxThisTestCfg = ( FW_IF_TEST_CFG* )pxThisIf->cfg;
        
        TEST_PRINT( "FW_IF_read at if.%u (%s) from address ullOffset.%u\r\n",
                ( unsigned int )pxThisTestCfg->ifId,
                pxThisTestCfg->ifName,
                ( unsigned int )ullOffset );

        memcpy( pucData, pucTestRxData+ullOffset, *pulSize );
        
        TEST_PRINT( "Reading %u bytes...", ( unsigned int )*pulSize );
        for( i = 0; i < *pulSize; i++ )
        {
            if( 0 == i % 16 )
            {
                TEST_PRINT( "\r\n[%02x] ", i );
            }
            
            TEST_PRINT( "%02x ", pucData[ i ] );
        }
        TEST_PRINT( "\r\n" );
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
static uint32_t testIoctrl( void *pvFwIf, uint32_t ulOption, void *pvValue )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;
    
    FW_IF_TEST_CFG *pxThisTestCfg = ( FW_IF_TEST_CFG* )pxThisIf->cfg;
    
    switch( ulOption )
    {
        case FW_IF_COMMON_IOCTRL_FLUSH_TX:
        case FW_IF_COMMON_IOCTRL_FLUSH_RX:
            break;

        case FW_IF_COMMON_IOCTRL_GET_RX_MODE:
            *(uint8_t* )pvValue = myRxMode;
            break;

        case FW_IF_TEST_IOCTRL_ENABLE_DEBUG_PRINT:
            iDebugPrint = FW_IF_TRUE;
            break;

        case FW_IF_TEST_IOCTRL_DISABLE_DEBUG_PRINT:
            iDebugPrint = FW_IF_FALSE;
            break;

        case FW_IF_TEST_IOCTRL_SET_NEXT_RX_DATA:
            if( NULL != pvValue )
            {
                pucTestRxData = ( uint8_t * )pvValue;
            }
            else
            {
                ulStatus = FW_IF_ERRORS_PARAMS;
            }
            break;

        case FW_IF_TEST_IOCTRL_SET_NEXT_RX_SIZE:
            if( ( NULL != pvValue ) && ( FW_IF_TEST_MAX_DATA >= *( uint32_t* )pvValue ) )
            {
                ulTestRxSize = *( uint32_t* )pvValue;
            }
            else
            {
                ulStatus = FW_IF_ERRORS_PARAMS;
            }
            break;

        case FW_IF_TEST_IOCTRL_TRIGGER_EVENT:
            if( NULL != pvValue )
            {
                if( NULL != pxThisIf->raiseEvent )
                {
                    if( ( *( uint16_t* )pvValue == FW_IF_COMMON_EVENT_NEW_RX_DATA ) && ( NULL != pucTestRxData ) )
                    {
                        pxThisIf->raiseEvent( *( uint16_t* )pvValue, pucTestRxData, ulTestRxSize );
                    }
                    else
                    {
                        pxThisIf->raiseEvent( *( uint16_t* )pvValue, NULL, 0 );
                    }
                }
            }
            else
            {
                ulStatus = FW_IF_ERRORS_PARAMS;
            }
            break;
        
        default:
            ulStatus = FW_IF_ERRORS_UNRECOGNISED_OPTION;
            break;
    }

    TEST_PRINT( "FW_IF_ioctrl for if.%u (%s), (option %u)\r\n",
            ( unsigned int )pxThisTestCfg->ifId,
            pxThisTestCfg->ifName,
            ( unsigned int )ulOption );

    return ulStatus;
}

/**
 * @brief   Local implementation of FW_IF_bindCallback
 */
static uint32_t testBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( NULL != pxNewFunc )
    {
        FW_IF_TEST_CFG *pxThisTestCfg = ( FW_IF_TEST_CFG* )pxThisIf->cfg;
        
        pxThisIf->raiseEvent = pxNewFunc;
    
        TEST_PRINT( "FW_IF_bindCallback called for if.%u (%s)\r\n",
                ( unsigned int )pxThisTestCfg->ifId,
                pxThisTestCfg->ifName );
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
 * @brief   initialisation function for test interfaces (generic across all test interfaces)
 */
uint32_t FW_IF_test_init( FW_IF_TEST_INIT_CFG *pxCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != iInitialised )
    {
        ulStatus = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL == pxCfg )
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }
    else
    {
        memcpy( &xMyLocalCfg, pxCfg, sizeof( FW_IF_TEST_INIT_CFG ) );
        iInitialised = FW_IF_TRUE;

        TEST_PRINT( "FW_IF_test_init for driver.%u (%s)\r\n",
                ( unsigned int )xMyLocalCfg.driverId,
                xMyLocalCfg.driverName );
    }

    return ulStatus;
}

/**
 * @brief   opens an instance of the test interface
 */
uint32_t FW_IF_test_create( FW_IF_CFG *pxFwIf, FW_IF_TEST_CFG *pxTestCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    CHECK_DRIVER;

    if( ( NULL != pxFwIf ) && ( NULL != pxTestCfg ) )
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
            .cfg            = ( void* )pxTestCfg,
            .lowerFirewall  = TEST_LOWER_FIREWALL
        };

        memcpy( pxFwIf, &myLocalIf, sizeof( FW_IF_CFG ) );

        FW_IF_TEST_CFG *pxThisTestCfg = ( FW_IF_TEST_CFG* )pxFwIf->cfg;

        TEST_PRINT( "FW_IF_test_create for if.%u (%s)\r\n",
                ( unsigned int )pxThisTestCfg->ifId,
                pxThisTestCfg->ifName );
    }
        
    return ulStatus;
}
