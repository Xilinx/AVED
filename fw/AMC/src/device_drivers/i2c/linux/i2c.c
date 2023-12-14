/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the user API definitions for the I2C driver (stubbed out Linux version).
 *
 * @file i2c.c
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "osal.h"
#include "pll.h"

#include "i2c.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define UPPER_FIREWALL                  ( 0xBABECAFE )
#define LOWER_FIREWALL                  ( 0xDEADFACE )

#define I2C_NAME                        "I2C"

/* Stat & Error definitions */
#define I2C_STATS( DO )                                     \
    DO( I2C_STATS_INIT_COMPLETED )                          \
    DO( I2C_STATS_REINIT_COMPLETED )                        \
    DO( I2C_STATS_SEND_COMPLETED )                          \
    DO( I2C_STATS_RECEIVE_COMPLETED )                       \
    DO( I2C_STATS_MAX )

#define I2C_ERRORS( DO )                                    \
    DO( I2C_ERRORS_VALIDAION_FAILED)                        \
    DO( I2C_ERRORS_XIIC_PS_CONFIG_FAILED )                  \
    DO( I2C_ERRORS_XIIC_PS_SET_CLK_FAILED )                 \
    DO( I2C_ERRORS_XIIC_PS_MASTER_SEND_POLLED_FAILED )      \
    DO( I2C_ERRORS_XIIC_PS_MASTER_RECEIVE_POLLED_FAILED )   \
    DO( I2C_ERRORS_XIIC_PS_SET_OPTIONS_FAILED )             \
    DO( I2C_ERRORS_XIIC_PS_CLEAR_OPTIONS_FAILED )           \
    DO( I2C_ERRORS_MAX )

#define PRINT_STAT_COUNTER( x )         PLL_INF( I2C_NAME, "%50s . . . . %d\r\n",          \
                                                 I2C_STATS_STR[ x ],                       \
                                                 pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )        PLL_INF( I2C_NAME, "%50s . . . . %d\r\n",          \
                                                 I2C_ERRORS_STR[ x ],                      \
                                                 pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )           { if( x < I2C_STATS_MAX )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )          { if( x < I2C_ERRORS_MAX )pxThis->pulErrorCounters[ x ]++; }


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    I2C_STATS
 * @brief   Enumeration of stats counters for this driver
 */
UTIL_MAKE_ENUM_AND_STRINGS( I2C_STATS, I2C_STATS, I2C_STATS_STR )

/**
 * @enum    I2C_ERRORS
 * @brief   Enumeration of stats errors for this driver
 */
UTIL_MAKE_ENUM_AND_STRINGS( I2C_ERRORS, I2C_ERRORS, I2C_ERRORS_STR )


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct  I2C_PRIVATE_DATA
 * @brief   Structure to hold ths driver's private data
 */
typedef struct I2C_PRIVATE_DATA
{
    uint32_t        ulUpperFirewall;

    int             iInitialised;

    uint32_t        pulStatCounters[ I2C_STATS_MAX ];
    uint32_t        pulErrorCounters[ I2C_ERRORS_MAX ];

    uint32_t        ulLowerFirewall;

} I2C_PRIVATE_DATA;


/******************************************************************************/
/* Local Variables                                                            */
/******************************************************************************/

static I2C_PRIVATE_DATA xLocalData =
{
    UPPER_FIREWALL,
    FALSE,
    { 0 },
    { 0 },
    LOWER_FIREWALL
};
static I2C_PRIVATE_DATA *pxThis = &xLocalData;


/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/**
 * @brief   Re-initialise the I2C driver.
 */
int iI2C_ReInit( uint8_t ucDeviceId )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( I2C_NUM_INSTANCES > ucDeviceId ) )
    {

        PLL_LOG( I2C_NAME, "Device %d ReInitialised: \r\n", ucDeviceId);
        iStatus = OK;
        INC_STAT_COUNTER( I2C_STATS_REINIT_COMPLETED )
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Initializes the I2C driver.
 */
int iI2C_Init( I2C_CFG_TYPE *pxI2cCfg, uint16_t usBusIdleWaitMs )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FALSE == pxThis->iInitialised ) &&
        ( NULL != pxI2cCfg ) )
    {
        int i = 0;

        for( i = 0; i < I2C_NUM_INSTANCES; i++ )
        {
            PLL_LOG( I2C_NAME, "Device Configured: %d 0x%llx %d\r\n",
                     pxI2cCfg[ i ].ucDeviceId,
                     pxI2cCfg[ i ].ullBaseAddress,
                     pxI2cCfg[ i ].ulInputClockHz );
        }

        pxThis->iInitialised = TRUE;
        iStatus = OK;
        INC_STAT_COUNTER( I2C_STATS_INIT_COMPLETED )
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   This function sends data from the I2C device into a specified buffer.
 */
int iI2C_Send( uint8_t ucDeviceId,
               uint8_t ucAddr,
               uint8_t *pucDataBuff,
               uint32_t ulLength )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucDataBuff ) &&
        ( ucDeviceId < I2C_NUM_INSTANCES ) )
    {
        INC_STAT_COUNTER( I2C_STATS_SEND_COMPLETED )
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   This function reads data from the I2C device into a specified buffer.
 */
int iI2C_Recv( uint8_t ucDeviceId,
               uint8_t ucAddr,
               uint8_t *pucDataBuff,
               uint32_t ulLength )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucDataBuff ) &&
        ( ucDeviceId < I2C_NUM_INSTANCES ) )
    {
        INC_STAT_COUNTER( I2C_STATS_RECEIVE_COMPLETED )
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   This function sends data from the I2C device and waits for a response coming back.
 */
int iI2C_SendRecv( uint8_t ucDeviceId,
                   uint8_t ucWriteReadAddr,
                   uint8_t *pucWriteDataBuff,
                   uint32_t ulWriteLength,
                   uint8_t *pucReadDataBuff,
                   uint32_t ulReadLength )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucWriteDataBuff ) &&
        ( NULL != pucReadDataBuff ) &&
        ( ucDeviceId < I2C_NUM_INSTANCES ) )
    {
        INC_STAT_COUNTER( I2C_STATS_SEND_COMPLETED )
        INC_STAT_COUNTER( I2C_STATS_RECEIVE_COMPLETED )
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Print all the stats gathered by the driver
 */
int iI2C_PrintStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        int i = 0;
        PLL_INF( I2C_NAME, "============================================================\n\r" );
        PLL_INF( I2C_NAME, "I2C Statistics:\n\r" );
        for( i = 0; i < I2C_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( I2C_NAME, "------------------------------------------------------------\n\r" );
        PLL_INF( I2C_NAME, "I2C Errors:\n\r" );
        for( i = 0; i < I2C_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( I2C_NAME, "============================================================\n\r" );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Clear all the stats in the driver
 */
int iI2C_ClearStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        pvOSAL_MemSet( pxThis->pulStatCounters, 0, sizeof( pxThis->pulStatCounters ) );
        pvOSAL_MemSet( pxThis->pulErrorCounters, 0, sizeof( pxThis->pulErrorCounters ) );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Print the circular log
 */
int iI2C_PrintLog( int iDevice )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( I2C_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}
