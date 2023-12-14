/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the user API definitions for the OSPI device driver.
 *
 * @file ospi.c
 *
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "standard.h"

#include "ospi.h"
#include "util.h"
#include "pll.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define UPPER_FIREWALL                      ( 0xBABECAFE )
#define LOWER_FIREWALL                      ( 0xDEADFACE )

#define OSPI_NAME                           "OSPI"

/* Stat & Error definitions */
#define OSPI_STATS( DO )                            \
    DO( OSPI_STATS_INIT_COMPLETED )                 \
    DO( OSPI_STATS_ERASE_SUCCESS  )                 \
    DO( OSPI_STATS_READ_SUCCESS  )                  \
    DO( OSPI_STATS_WRITE_SUCCESS  )                 \
    DO( OSPI_STATS_CREATE_TIMER )                   \
    DO( OSPI_STATS_CREATE_MUTEX )                   \
    DO( OSPI_STATS_TAKE_MUTEX )                     \
    DO( OSPI_STATS_RELEASE_MUTEX )                  \
    DO( OSPI_STATS_MAX )

#define OSPI_ERRORS( DO )                           \
    DO( OSPI_ERRORS_VALIDAION_FAILED )              \
    DO( OSPI_ERRORS_DEVICE_RESET )                  \
    DO( OSPI_ERRORS_LOOKUP_CONFIG )                 \
    DO( OSPI_ERRORS_CONFIG_INIT )                   \
    DO( OSPI_ERRORS_SET_OPTIONS )                   \
    DO( OSPI_ERRORS_SET_CLK_PRESCALER )             \
    DO( OSPI_ERRORS_SELECT_FLASH )                  \
    DO( OSPI_ERRORS_FLASH_READ_ID )                 \
    DO( OSPI_ERRORS_SET_DDR_MODE_INDEX )            \
    DO( OSPI_ERRORS_4B_ADDRESS_MODE )               \
    DO( OSPI_ERRORS_SET_SDR_DDR_MODE )              \
    DO( OSPI_ERRORS_PAGE_SIZE )                     \
    DO( OSPI_ERRORS_FLASH_ERASE_FAILED )            \
    DO( OSPI_ERRORS_FLASH_READ_FAILED )             \
    DO( OSPI_ERRORS_FLASH_LINEAR_WRITE_FAILED )     \
    DO( OSPI_ERRORS_FLASH_WRITE_FAILED )            \
    DO( OSPI_ERRORS_FLASH_READ_MISMATCH )           \
    DO( OSPI_ERRORS_FLASH_POLL_RETRY_FAILED )       \
    DO( OSPI_ERRORS_SET_EDGE_MODE )                 \
    DO( OSPI_ERRORS_SET_4B_ADDR_MODE )              \
    DO( OSPI_ERRORS_TIMER_CREATE_FAILED )           \
    DO( OSPI_ERRORS_TIMER_START_FAILED )            \
    DO( OSPI_ERRORS_TIMER_STOP_FAILED )             \
    DO( OSPI_ERRORS_MUTEX_CREATE_FAILED )           \
    DO( OSPI_ERRORS_MUTEX_RELEASE_FAILED )          \
    DO( OSPI_ERRORS_MUTEX_TAKE_FAILED )             \
    DO( OSPI_ERRORS_FLASH_ID_READ )                 \
    DO( OSPI_ERRORS_MAX )

#define PRINT_STAT_COUNTER( x )         PLL_INF( OSPI_NAME, "%50s . . . . %d\r\n",             \
                                                 OSPI_STATS_STR[ x ],                          \
                                                 pxThis->pulStatCounters[ x ] )
#define PRINT_ERROR_COUNTER( x )        PLL_INF( OSPI_NAME, "%50s . . . . %d\r\n",             \
                                                 OSPI_ERRORS_STR[ x ],                         \
                                                 pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )           { if( x < OSPI_STATS_MAX )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )          { if( x < OSPI_ERRORS_MAX )pxThis->pulErrorCounters[ x ]++; }


/* Flash command Id's */
#define WRITE_STATUS_CMD                ( 0x01 )
#define WRITE_DISABLE_CMD               ( 0x04 )
#define WRITE_ENABLE_CMD                ( 0x06 )
#define BULK_ERASE_CMD                  ( 0xC7 )
#define DIE_ERASE_CMD                   ( 0xC4 )
#define READ_ID                         ( 0x9F )
#define READ_CONFIG_CMD                 ( 0x35 )
#define WRITE_CONFIG_CMD                ( 0x01 )
#define READ_FLAG_STATUS_CMD            ( 0x70 )
#define WRITE_CMD_4B                    ( 0x12 )
#define SEC_ERASE_CMD_4B                ( 0xDC )
#define READ_CMD_OCTAL_IO_4B            ( 0xCC )
#define READ_CMD_OCTAL_DDR              ( 0x9D )
#define WRITE_CMD_OCTAL_4B              ( 0x84 )
#define ENTER_4B_ADDR_MODE              ( 0xB7 )
#define EXIT_4B_ADDR_MODE               ( 0xE9 )
#define WRITE_CONFIG_REG                ( 0x81 )
#define READ_CONFIG_REG                 ( 0x85 )

#define SIXTEENMB                       ( 0x1000000 )

#define FLASH_PAGE_SIZE_DEFAULT         ( 256 )
#define FLASH_SECTOR_SIZE_64KB          ( 0x10000 )
#define FLASH_SECTOR_SIZE_128KB         ( 0x20000 )
#define FLASH_DEVICE_SIZE_256M          ( 0x2000000 )
#define FLASH_DEVICE_SIZE_512M          ( 0x4000000 )
#define FLASH_DEVICE_SIZE_1G            ( 0x8000000 )
#define FLASH_DEVICE_SIZE_2G            ( 0x10000000 )

#define MICRON_OCTAL_ID_BYTE0           ( 0x2c )
#define GIGADEVICE_OCTAL_ID_BYTE0       ( 0xc8 )
#define ISSI_OCTAL_ID_BYTE0             ( 0x9d )

#define OSPI_READ_BUFFER_SIZE           ( 8 )
#define OSPI_READ_BUFFER_ALIGNMENT      ( 4 )
#define OSPI_DATA_ALIGNMENT             ( 8 )
#define OSPI_CMD_BUFFER_SIZE            ( 8 )
#define OSPI_STATUS_BUFFER_SIZE         ( 2 )

#define OSPI_POLL_OVERALL_TIMEOUT_MS    ( 1000 )
#define OSPI_POLL_INTERVAL_TIMEOUT_MS   ( 100 )

#define BITSHIFT_1B                     ( 8 )
#define BITSHIFT_2B                     ( 16 )
#define BITSHIFT_3B                     ( 24 )

#define MISMATCH_CHECK_COUNT            ( 16 )
#define FLASH_ID_STR_BUFFER             ( 100 )
#define FLASH_ID_SPECIFIER_SIZE         ( 6 )
#define FLASH_ID_READ_SIZE              ( 8 )
#define FLASH_WRITE_BYTE_SIZE           ( 8 )

#define XFLASH_CMD_ADDRSIZE_3           ( 3 )
#define XFLASH_CMD_ADDRSIZE_4           ( 4 )
#define XFLASH_BYTE_COUNT_1             ( 1 )
#define XFLASH_BYTE_COUNT_2             ( 2 )
#define XFLASH_OPCODE_DUMMY_CYCLES      ( 8 )
#define XFLASH_STATUS_BYTE              ( 0x80 )


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    OSPI_STATS
 * @brief   Enumeration of stats counters for this driver
 */
UTIL_MAKE_ENUM_AND_STRINGS( OSPI_STATS, OSPI_STATS, OSPI_STATS_STR )

/**
 * @enum    OSPI_ERRORS
 * @brief   Enumeration of stats errors for this driver
 */
UTIL_MAKE_ENUM_AND_STRINGS( OSPI_ERRORS, OSPI_ERRORS, OSPI_ERRORS_STR )


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct  OSPI_FLASH_INFO
 * @brief   Structure describing a flash device
 */
typedef struct OSPI_FLASH_INFO
{
    uint32_t        ulJedecId;         /* JEDEC ID */
    uint32_t        ulSectSize;        /* Individual sector size or
                                        * combined sector size in case of parallel config*/
    uint32_t        ululNumSect;       /* Total no. of sectors in one/two flash devices */
    uint32_t        ulPageSize;        /* Individual page size or
                                        * combined page size in case of parallel config*/
    uint32_t        ulNumPage;         /* Total no. of pages in one/two flash devices */
    uint32_t        ulFlashDeviceSize; /* This is the size of one flash device */
    uint32_t        ulSectMask;        /* Mask to get sector start address */
    uint8_t         ucNumDie;          /* No. of die forming a single flash */
    uint32_t        ulReadCmd;         /* Read command used to read data from flash */
    uint32_t        ulWriteCmd;        /* Write command used to write data to flash */
    uint32_t        ulEraseCmd;        /* Erase Command */
    uint8_t         ucStatusCmd;       /* Status Command */
    uint8_t         ucDummyCycles;     /* Number of Dummy cycles for Read operation */

} OSPI_FLASH_INFO;

/**
 * @struct  OSPI_PRIVATE_DATA
 * @brief   Structure to hold ths driver's private data
 */
typedef struct OSPI_PRIVATE_DATA
{
    uint32_t        ulUpperFirewall;

    int             iInitialised;
    uint32_t        ulFlashMake;
    uint32_t        ulOspiSectorSize;
    uint8_t         ucFctIndex;
    uint32_t        ulPageSize;
    uint8_t         ucOspiFlashPercentage;
    void            *pvOsalMutexHdl;
    void            *pvTimerHandle;
    int             iAbortPollWait;
    uint8_t         ucReadBfrPtr[ OSPI_READ_BUFFER_SIZE ]__attribute__ ( ( aligned( OSPI_DATA_ALIGNMENT ) ) );

    uint32_t        pulStatCounters[ OSPI_STATS_MAX ];
    uint32_t        pulErrorCounters[ OSPI_ERRORS_MAX ];

    uint32_t        ulLowerFirewall;

} OSPI_PRIVATE_DATA;


/******************************************************************************/
/* Local Variables                                                            */
/******************************************************************************/

/* Note: there are a number of hardcoded values here used to identify and config the flash device */
static OSPI_FLASH_INFO pxFlashConfigTable[ ] = {
    /* Micron */
    { 0x2c5b1a, FLASH_SECTOR_SIZE_128KB, 0x200, FLASH_PAGE_SIZE_DEFAULT, 0x40000,
      FLASH_DEVICE_SIZE_512M,0xFFFE0000, 1,
      READ_CMD_OCTAL_IO_4B, ( WRITE_CMD_OCTAL_4B << BITSHIFT_1B ) | WRITE_CMD_4B,
      ( DIE_ERASE_CMD << BITSHIFT_2B ) | ( BULK_ERASE_CMD << BITSHIFT_1B ) | SEC_ERASE_CMD_4B,
      READ_FLAG_STATUS_CMD, 16 },
    { 0x2c5b1b, FLASH_SECTOR_SIZE_128KB, 0x400, FLASH_PAGE_SIZE_DEFAULT, 0x80000,
      FLASH_DEVICE_SIZE_1G,0xFFFE0000, 1,
      READ_CMD_OCTAL_IO_4B, ( WRITE_CMD_OCTAL_4B << BITSHIFT_1B ) | WRITE_CMD_4B,
      ( DIE_ERASE_CMD << BITSHIFT_2B ) | ( BULK_ERASE_CMD << BITSHIFT_1B ) | SEC_ERASE_CMD_4B,
      READ_FLAG_STATUS_CMD, 16 },
    { 0x2c5b1c, FLASH_SECTOR_SIZE_128KB, 0x800, FLASH_PAGE_SIZE_DEFAULT, 0x100000,
      FLASH_DEVICE_SIZE_2G,0xFFFE0000, 1,
      READ_CMD_OCTAL_IO_4B, ( WRITE_CMD_OCTAL_4B << BITSHIFT_1B ) | WRITE_CMD_4B,
      ( DIE_ERASE_CMD << BITSHIFT_2B ) | ( BULK_ERASE_CMD << BITSHIFT_1B ) | SEC_ERASE_CMD_4B,
      READ_FLAG_STATUS_CMD, 16 },
    /* GIGADEVICE */
    { 0xc86819, FLASH_SECTOR_SIZE_64KB, 0x200, FLASH_PAGE_SIZE_DEFAULT, 0x20000,
      FLASH_DEVICE_SIZE_256M, 0xFFFF0000, 1,
      READ_CMD_OCTAL_IO_4B, ( WRITE_CMD_OCTAL_4B << BITSHIFT_1B ) | WRITE_CMD_4B,
      ( DIE_ERASE_CMD << BITSHIFT_2B ) | ( BULK_ERASE_CMD << BITSHIFT_1B ) | SEC_ERASE_CMD_4B,
      READ_FLAG_STATUS_CMD, 16 },
    /* ISSI */
    { 0x9d5b19, FLASH_SECTOR_SIZE_128KB, 0x100, FLASH_PAGE_SIZE_DEFAULT, 0x20000,
      FLASH_DEVICE_SIZE_256M, 0xFFFE0000, 1,
      READ_CMD_OCTAL_IO_4B, ( WRITE_CMD_OCTAL_4B << BITSHIFT_1B ) | WRITE_CMD_4B,
      ( DIE_ERASE_CMD << BITSHIFT_2B ) | ( BULK_ERASE_CMD << BITSHIFT_1B ) | SEC_ERASE_CMD_4B,
      READ_FLAG_STATUS_CMD, 16 },
};


static OSPI_PRIVATE_DATA xLocalData =
{
    UPPER_FIREWALL,     /* ulUpperFirewall */
    FALSE,              /* iInitialised */
    0,                  /* ulFlashMake */
    0,                  /* ulOspiSectorSize */
    0,                  /* ucFctIndex */
    0,                  /* ulPageSize */
    0,                  /* ucOspiFlashPercentage */
    NULL,               /* pvOsalMutexHdl */
    NULL,               /* pvTimerHandle */
    FALSE,              /* iAbortPollWait */
    { 0 },              /* ucReadBfrPtr */
    { 0 },              /* pulStatCounters */
    { 0 },              /* pulErrorCounters */
    LOWER_FIREWALL      /* ulLowerFirewall */
};
static OSPI_PRIVATE_DATA *pxThis = &xLocalData;


/******************************************************************************/
/* Private Function declarations                                              */
/******************************************************************************/

/**
 * @brief   Callback invoked whenever polling interval has been exceeded
 *
 * @param   pvTimerHandle       Pointer to OS Timer Handle.
 */
static void vTimerPollTimeoutCb( void *pvTimerHandle );


/******************************************************************************/
/* Public Function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initializes the OSPI driver.
 */
int iOSPI_FlashInit( OSPI_CFG_TYPE *pxOspiCfg )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FALSE == pxThis->iInitialised ) &&
        ( NULL != pxOspiCfg ) )
    {
        pxThis->ulPageSize = pxOspiCfg->usPageSize;

        /* Timer created first as needed by the poll transfer function */
        if( OSAL_ERRORS_NONE != iOSAL_Timer_Create( &pxThis->pvTimerHandle,
                                                    OSAL_TIMER_CONFIG_ONE_SHOT,
                                                    vTimerPollTimeoutCb,
                                                    "ospi_poll_timeout" ) )
        {
            PLL_ERR( OSPI_NAME, "Error: iOSAL_Timer_Create failed\r\n" );
            INC_ERROR_COUNTER( OSPI_ERRORS_TIMER_CREATE_FAILED )
            iStatus = ERROR;
        }
        else
        {
            INC_STAT_COUNTER( OSPI_STATS_CREATE_TIMER )
            iStatus = OK;
        }

        if( OK == iStatus )
        {
            if( OSAL_ERRORS_NONE != iOSAL_Mutex_Create( &pxThis->pvOsalMutexHdl,
                                                        "ami_proxy mutex" ) )
            {
                PLL_ERR( OSPI_NAME, "Error: initialising mutex\r\n" );
                INC_ERROR_COUNTER( OSPI_ERRORS_MUTEX_CREATE_FAILED )
                iStatus = ERROR;
            }
            else
            {
                INC_STAT_COUNTER( OSPI_STATS_CREATE_MUTEX )
            }
        }

        /* Page Size */
        if( OK == iStatus )
        {
            if( pxFlashConfigTable[ pxThis->ucFctIndex ].ulPageSize != pxThis->ulPageSize )
            {
                PLL_ERR( OSPI_NAME, "Error: page size is: %d, expected: %d\r\n",
                         pxFlashConfigTable[ pxThis->ucFctIndex ].ulPageSize,
                         pxThis->ulPageSize );
                INC_ERROR_COUNTER( OSPI_ERRORS_PAGE_SIZE )
                iStatus = ERROR;
            }
        }

        /* Sector Size */
        if( OK == iStatus )
        {
            pxThis->ulOspiSectorSize = pxFlashConfigTable[ pxThis->ucFctIndex ].ulSectSize;
            PLL_DBG( OSPI_NAME, "FCT Index:%d, page size:%d, sector size:%d\r\n",
                     pxThis->ucFctIndex,
                     pxThis->ulPageSize,
                     pxThis->ulOspiSectorSize );
            pxThis->iInitialised = TRUE;
            INC_STAT_COUNTER( OSPI_STATS_INIT_COMPLETED )
        }
    }
    else
    {
        INC_ERROR_COUNTER( OSPI_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Function to erase the flash device.
 */
int iOSPI_FlashErase( uint32_t ulAddr, uint32_t ulLength )
{
    int iStatus = ERROR;

    /* TODO: need to add support for flash percentage into erase */
    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvOsalMutexHdl,
                                                  OSAL_TIMEOUT_WAIT_FOREVER ) )
        {
            INC_STAT_COUNTER( OSPI_STATS_TAKE_MUTEX )

            if( ulAddr & pxThis->ulPageSize )
            {
                PLL_WRN( OSPI_NAME, "Warning: base address is not %d aligned\r\n", ulAddr );
            }

            if( OSAL_ERRORS_NONE != iOSAL_Mutex_Release( pxThis->pvOsalMutexHdl ) )
            {
                INC_ERROR_COUNTER( OSPI_ERRORS_MUTEX_RELEASE_FAILED )
                iStatus = ERROR;
            }
            else
            {
                INC_STAT_COUNTER( OSPI_STATS_RELEASE_MUTEX )
            }
        }
        else
        {
            INC_ERROR_COUNTER( OSPI_ERRORS_MUTEX_TAKE_FAILED )
        }
    }
    else
    {
        INC_ERROR_COUNTER( OSPI_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Function to read a number of bytes from the flash device.
 */
int iOSPI_FlashRead( uint32_t ulAddr, uint8_t *pucReadBuffer, uint32_t *pulLength )
{
    int iStatus = ERROR;

    /* TODO: need to add support for flash percentage into read */
    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucReadBuffer ) &&
        ( NULL != pulLength ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvOsalMutexHdl,
                                                  OSAL_TIMEOUT_WAIT_FOREVER ) )
        {
            INC_STAT_COUNTER( OSPI_STATS_TAKE_MUTEX )

            if( OSAL_ERRORS_NONE != iOSAL_Mutex_Release( pxThis->pvOsalMutexHdl ) )
            {
                INC_ERROR_COUNTER( OSPI_ERRORS_MUTEX_RELEASE_FAILED )
                iStatus = ERROR;
            }
            else
            {
                INC_STAT_COUNTER( OSPI_STATS_RELEASE_MUTEX )
            }
        }
        else
        {
            INC_ERROR_COUNTER( OSPI_ERRORS_MUTEX_TAKE_FAILED )
        }

    }
    else
    {
        INC_ERROR_COUNTER( OSPI_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Function to write a number of bytes from the flash device.
 */
int iOSPI_FlashWrite( uint32_t ulAddr, uint8_t *pucWriteBuffer, uint32_t ulLength )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucWriteBuffer ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Mutex_Take( pxThis->pvOsalMutexHdl,
                                                  OSAL_TIMEOUT_WAIT_FOREVER ) )
        {
            uint32_t ucPageCount = 0;
            uint32_t ucPageSize = pxThis->ulPageSize;
            pxThis->ucOspiFlashPercentage = 0;

            INC_STAT_COUNTER( OSPI_STATS_TAKE_MUTEX )

            /* Validation */
            ucPageCount = ( ulLength / ucPageSize );
            if( ulLength % ucPageSize )
            {
                ucPageCount++;
                PLL_WRN( OSPI_NAME, "Warning: len:%d is not page:%d aligned\r\n", ulLength, ucPageSize );
            }

            /* Note this used to be the offset as opposed to the address */
            if( ulAddr % ucPageSize )
            {
                PLL_WRN( OSPI_NAME, "Warning: address:%d is not page:%d aligned\r\n", ulAddr, ucPageSize );
            }

            if( OK == iStatus )
            {
                INC_STAT_COUNTER( OSPI_STATS_WRITE_SUCCESS )
                pxThis->ucOspiFlashPercentage = 100;
                PLL_DBG( OSPI_NAME, "Flash write complete\r\n" );
            }

            if( OSAL_ERRORS_NONE != iOSAL_Mutex_Release( pxThis->pvOsalMutexHdl ) )
            {
                INC_ERROR_COUNTER( OSPI_ERRORS_MUTEX_RELEASE_FAILED )
                iStatus = ERROR;
            }
            else
            {
                INC_STAT_COUNTER( OSPI_STATS_RELEASE_MUTEX )
            }
        }
        else
        {
            INC_ERROR_COUNTER( OSPI_ERRORS_MUTEX_TAKE_FAILED )
        }
    }
    else
    {
        INC_ERROR_COUNTER( OSPI_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Return the progress of the current operation
 */
int iOSPI_GetOperationProgress( uint8_t *pucPercentage )
{
    int iStatus = ERROR;

    /* Note: not mutex protected as want to return the percentage while operation is in progress */
    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pucPercentage ) )
    {
        iStatus = OK;
        *pucPercentage = pxThis->ucOspiFlashPercentage;
    }

    return iStatus;
}

/**
 * @brief   Print all the stats gathered by the driver
 */
int iOSPI_PrintStatistics( void )
{
    int iStatus = ERROR;

    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) )
    {
        int i = 0;
        PLL_INF( OSPI_NAME, "============================================================\r\n" );
        PLL_INF( OSPI_NAME, "OSPI Statistics:\n\r" );
        for( i = 0; i < OSPI_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( OSPI_NAME, "------------------------------------------------------------\r\n" );
        PLL_INF( OSPI_NAME, "OSPI Errors:\n\r" );
        for( i = 0; i < OSPI_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( OSPI_NAME, "============================================================\r\n" );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( OSPI_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}

/**
 * @brief   Clear all the stats in the driver
 */
int iOSPI_ClearStatistics( void )
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
        INC_ERROR_COUNTER( OSPI_ERRORS_VALIDAION_FAILED )
    }

    return iStatus;
}


/******************************************************************************/
/* Private Function implementation                                            */
/******************************************************************************/

/**
 * @brief   Callback invoked whenever polling interval has been exceeded
 */
static void vTimerPollTimeoutCb( void *pvTimerHandle )
{
    if( ( UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( TRUE == pxThis->iInitialised ) &&
        ( NULL != pvTimerHandle ) )
    {
        if( pxThis->pvTimerHandle == pvTimerHandle )
        {
            pxThis->iAbortPollWait = TRUE;
        }
    }
    else
    {
        INC_ERROR_COUNTER( OSPI_ERRORS_VALIDAION_FAILED )
    }
}
