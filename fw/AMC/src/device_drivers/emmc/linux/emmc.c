/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the Linux implementation for the EMMC driver.
 *
 * @file emmc.c
 *
 */


/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "pll.h"
#include "osal.h"
#include "emmc.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define UPPER_FIREWALL                  ( 0xBABECAFE )
#define LOWER_FIREWALL                  ( 0xDEADFACE )

#define EMMC_NAME                        "EMMC"

/* Stat & Error definitions */
#define EMMC_STATS( DO )                                     \
    DO( EMMC_STATS_INIT_COMPLETED )                          \
    DO( EMMC_STATS_CREATE_MUTEX )                            \
    DO( EMMC_STATS_TAKE_MUTEX )                              \
    DO( EMMC_STATS_RELEASE_MUTEX )                           \
    DO( EMMC_STATS_MAX )

#define EMMC_ERRORS( DO )                                    \
    DO( EMMC_ERRORS_VALIDATION_FAILED )                      \
    DO( EMMC_ERRORS_XSDPS_CFGINITIALIZE_FAILED )             \
    DO( EMMC_ERRORS_XSDPS_CARDINIIALIZE_FAILED )             \
    DO( EMMC_ERRORS_MUTEX_CREATE_FAILED )                    \
    DO( EMMC_ERRORS_MUTEX_RELEASE_FAILED )                   \
    DO( EMMC_ERRORS_MUTEX_TAKE_FAILED )                      \
    DO( EMMC_ERRORS_MAX )

#define PRINT_STAT_COUNTER( x )         PLL_INF( EMMC_NAME, "%50s . . . . %d\r\n",          \
                                                 EMMC_STATS_STR[ x ],                       \
                                                 pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )        PLL_INF( EMMC_NAME, "%50s . . . . %d\r\n",          \
                                                 EMMC_ERRORS_STR[ x ],                      \
                                                 pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )           { if( x < EMMC_STATS_MAX )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )          { if( x < EMMC_ERRORS_MAX )pxThis->pulErrorCounters[ x ]++; }


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    EMMC_STATS
 * @brief   Enumeration of stats counters for this driver
 */
UTIL_MAKE_ENUM_AND_STRINGS( EMMC_STATS, EMMC_STATS, EMMC_STATS_STR )

/**
 * @enum    EMMC_ERRORS
 * @brief   Enumeration of stats errors for this driver
 */
UTIL_MAKE_ENUM_AND_STRINGS( EMMC_ERRORS, EMMC_ERRORS, EMMC_ERRORS_STR )


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct  EMMC_PRIVATE_DATA
 * @brief   Structure to hold ths driver's private data
 */
typedef struct EMMC_PRIVATE_DATA
{
    uint32_t        ulUpperFirewall;

    uint16_t        ucDeviceId;
    int             iInitialised;

    void            *pvTimerHandle;
    void            *pvOsalMutexHdl;

    uint32_t        pulStatCounters[ EMMC_STATS_MAX ];
    uint32_t        pulErrorCounters[ EMMC_ERRORS_MAX ];

    uint32_t        ulLowerFirewall;

} EMMC_PRIVATE_DATA;


/******************************************************************************/
/* Local Variables                                                            */
/******************************************************************************/

static EMMC_PRIVATE_DATA xLocalData =
{
    UPPER_FIREWALL,                         /* ulUpperFirewall */
    0,                                      /* ucDeviceId */
    FALSE,                                  /* iInitialised */
    NULL,                                   /* pvTimerHandle */
    NULL,                                   /* pvOsalMutexHdl */
    { 0 },                                  /* pulStatCounters */
    { 0 },                                  /* pulErrorCounters */
    LOWER_FIREWALL                          /* ulLowerFirewall */
};
static EMMC_PRIVATE_DATA *pxThis = &xLocalData;


/******************************************************************************/
/* Private Function declarations                                              */
/******************************************************************************/


/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initializes the EMMC driver.
 */
int iEMMC_Initialise( uint16_t ucDeviceId )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FALSE == pxThis->iInitialised ) )
    {
        pxThis->ucDeviceId = ucDeviceId;
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Read from the EMMC.
 */
int iEMMC_Read( uint64_t ullAddress, uint32_t ulBlockCount , uint8_t *pucReadBuff )
{
    int iStatus = ERROR;

    if( ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucReadBuff ) )
    {
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Write to the EMMC.
 */
int iEMMC_Write( uint64_t ullAddress, uint32_t ulBlockCount , const uint8_t *pucWriteBuff )
{
    int iStatus = ERROR;

    if( ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucWriteBuff ) )
    {
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}


/**
 * @brief   Erase blocks in the EMMC.
 */
int iEMMC_Erase( uint64_t ullStartBlockAddress, uint64_t ullEndBlockAddress )
{
    int iStatus = ERROR;

    if( ( TRUE == pxThis->iInitialised ) &&
        ( ullEndBlockAddress > ullStartBlockAddress ) )
    {
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Erase blocks in the EMMC.
 */
int iEMMC_EraseAll( void )
{
    int iStatus = ERROR;

    if( TRUE == pxThis->iInitialised )
    {
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Print the EMMC detected device details.
 */
int iEMMC_PrintInstanceDetails( void )
{
    int iStatus = ERROR;

    if( TRUE == pxThis->iInitialised )
    {
        PLL_LOG( EMMC_NAME, "DeviceId:              0x%x\n\r", pxThis->ucDeviceId );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Print all the stats gathered by the driver
 */
int iEMMC_PrintStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        int i = 0;
        PLL_INF( EMMC_NAME, "============================================================\n\r" );
        PLL_INF( EMMC_NAME, "EMMC Statistics:\n\r" );
        for( i = 0; i < EMMC_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( EMMC_NAME, "------------------------------------------------------------\n\r" );
        PLL_INF( EMMC_NAME, "EMMC Errors:\n\r" );
        for( i = 0; i < EMMC_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( EMMC_NAME, "============================================================\n\r" );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Clear all the stats in the driver
 */
int iEMMC_ClearStatistics( void )
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
        INC_ERROR_COUNTER( EMMC_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}

/******************************************************************************/
/* Private Function implementation                                            */
/******************************************************************************/
