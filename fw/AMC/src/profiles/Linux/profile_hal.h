/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the hal profile for the Linux platform
 *
 * @file profile_hal.h
 *
 */

#ifndef _PROFILE_HAL_H_
#define _PROFILE_HAL_H_

#include "eeprom.h"

#define HAL_EMMC_FEATURE                        ( 0 )
#define HAL_EMMC_DEVICE_ID                      ( 0 )
#define HAL_EMMC_BLOCK_SIZE                     ( 512 )
#define HAL_EMMC_MAX_BLOCKS                     ( 0x7690000 )   /* 64 GBytes / 512 Bytes */

#define HAL_EEPROM_VERSION                      ( EEPROM_VERSION_4_0 )
#define HAL_EEPROM_I2C_BUS                      ( 0 )
#define HAL_EEPROM_SLAVE_ADDRESS                ( 0x52 )
#define HAL_EEPROM_ADDRESS_SIZE                 ( 1 )
#define HAL_EEPROM_PAGE_SIZE                    ( 16 )
#define HAL_EEPROM_NUM_PAGES                    ( 16 )
#define HAL_EEPROM_DEVICE_ID                    ( 0x0000 )
#define HAL_EEPROM_DEVICE_ID_ADDRESS            ( 0x00 )
#define HAL_EEPROM_DEVICE_ID_REGISTER           ( 0x00 )

#define HAL_AMC_CLOCK_CONTROL                   ( 1 )
#if ( 0 != HAL_AMC_CLOCK_CONTROL )
#ifdef XPAR_SHELL_UTILS_UCC_0_BASEADDR
#define HAL_USER_CLOCK_CONTROL_BASE_ADDRESS     ( XPAR_SHELL_UTILS_UCC_0_BASEADDR )
#else
#define HAL_USER_CLOCK_CONTROL_BASE_ADDRESS     ( 0x1 )
#endif
#endif

/* Definitions for peripheral CIPS_PSPMC_0_PSV_I2C_0 */
#define HAL_I2C_BUS_0_DEVICE_ID           ( 0 )
#define HAL_I2C_BUS_0_BASEADDR            ( 0xFF020000 )
#define HAL_I2C_BUS_0_HIGHADDR            ( 0xFF02FFFF )
#define HAL_I2C_BUS_0_I2C_CLK_FREQ_HZ     ( UTIL_100KHZ )
/* Definitions for peripheral CIPS_PSPMC_0_PSV_I2C_1 */
#define HAL_I2C_BUS_1_DEVICE_ID           ( 1 )
#define HAL_I2C_BUS_1_BASEADDR            ( 0xFF030000 )
#define HAL_I2C_BUS_1_HIGHADDR            ( 0xFF03FFFF )
#define HAL_I2C_BUS_1_I2C_CLK_FREQ_HZ     ( UTIL_100KHZ )

#define HAL_I2C_DEFAULT_SCLK_RATE         ( 33333333 )
#define HAL_I2C_RETRY_COUNT               ( 5 )

/* Definitions OSPI */
#define HAL_OSPI_0_DEVICE_ID               ( 0 )
#define HAL_PSV_PMC_GLOBAL_0_AXI_BASEADDR  ( 0xF1110000 )

/* Apps */
/* AMC */
#define HAL_PARTITION_TABLE_SIZE                ( 0x1000 )
#define HAL_PARTITION_TABLE_MAGIC_NO            ( 0x564D5230 )
#define HAL_ENABLE_AMI_COMMS                    ( 0x1 )
#define HAL_GCQ_BASE_ADDR_SIZE                  ( 0x110 )
#define HAL_RPU_RING_BUFFER_LEN                 ( 0x1000 )
#define HAL_RPU_SHARED_MEMORY_SIZE              ( 0x7FFF000 )

extern uint8_t HAL_BASE_LOGIC_GCQ_M2R_S01_AXI_BASEADDR[ HAL_GCQ_BASE_ADDR_SIZE ];
extern uint64_t HAL_USER_CLOCK_CONTROL_BASE;
extern const uint64_t HAL_USER_CLOCK_CONTROL_BASE_ADDERSS;
extern uint8_t HAL_RPU_SHARED_MEMORY_BASE_ARRAY[ HAL_RPU_SHARED_MEMORY_SIZE ];
const uint64_t HAL_RPU_SHARED_MEMORY_BASE_ADDR;

#define HAL_RPU_RING_BUFFER_BASE                ( HAL_RPU_SHARED_MEMORY_BASE_ARRAY + HAL_PARTITION_TABLE_SIZE )

#define HAL_FLUSH_CACHE_DATA( addr, size )  ( ( void ) ( 0 ) )

/* FAL */
/* GCQ */
#ifndef HAL_IO_WRITE32
#define HAL_IO_WRITE32( val, addr )    ( ( void ) ( 0 ) )
#endif

#ifndef HAL_IO_READ32
#define HAL_IO_READ32( addr )          ( 0 )
#endif

/* Proxies */
/* APC */
#define HAL_APC_PMC_BOOT_REG            ( XPAR_PSV_PMC_GLOBAL_0_S_AXI_BASEADDR + 0x00004 )
#define HAL_APC_PMC_SRST_REG            ( XPAR_PSV_PMC_GLOBAL_0_S_AXI_BASEADDR + 0x20084 )
#define HAL_APC_PDI_BIT_MASK            ( 0x14 )

/**
 * @struct  HAL_PARTITION_TABLE_RING_BUFFER
 *
 * @brief   Stores the ring buffer info - part of the partition table.
 */
typedef struct HAL_PARTITION_TABLE_RING_BUFFER
{
    uint32_t ulRingBufferOff;
    uint32_t ulRingBufferLen;

} HAL_PARTITION_TABLE_RING_BUFFER;

/**
 * @struct  HAL_PARTITION_TABLE_STATUS
 *
 * @brief   Stores the AMC status info - part of the partition table.
 */
typedef struct HAL_PARTITION_TABLE_STATUS
{
    uint32_t ulStatusOff;
    uint32_t ulStatusLen;

} HAL_PARTITION_TABLE_STATUS;

/**
 * @struct  HAL_PARTITION_TABLE_LOG_MSG
 *
 * @brief   Stores the AMC logs and info - part of the partition table.
 */
typedef struct HAL_PARTITION_TABLE_LOG_MSG
{
    uint32_t ulLogMsgIndex;
    uint32_t ulLogMsgBufferOff;
    uint32_t ulLogMsgBufferLen;

} HAL_PARTITION_TABLE_LOG_MSG;

/**
 * @struct  HAL_PARTITION_TABLE_DATA
 *
 * @brief   Stores the AMC data - part of the partition table.
 */
typedef struct HAL_PARTITION_TABLE_DATA
{
    uint32_t ulDataStart;
    uint32_t ulDataEnd;

} HAL_PARTITION_TABLE_DATA;

/**
 * @struct  HAL_PARTITION_TABLE
 *
 * @brief   Table stored at the top of the shared memory and used by
 *          AMI to read offsets & state.
 */
typedef struct HAL_PARTITION_TABLE
{
    uint32_t                        ulMagicNum;
    HAL_PARTITION_TABLE_RING_BUFFER xRingBuffer;
    HAL_PARTITION_TABLE_STATUS      xStatus;
    HAL_PARTITION_TABLE_LOG_MSG     xLogMsg;
    HAL_PARTITION_TABLE_DATA        xData;

} HAL_PARTITION_TABLE;

#endif
