/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the EEPROM debug implementation
 *
 * @file eeprom_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "eeprom_debug.h"
#include "eeprom.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define EEPROM_DBG_NAME             "EEPROM_DBG"

#define EEPROM_DBG_MAX_FIELD_SIZE   ( 30 )
#define EEPROM_BUFFER_WIDTH         ( 16 )


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxEepromTop = NULL;
static DAL_HDL pxSetDir = NULL;
static DAL_HDL pxGetDir = NULL;


/******************************************************************************/
/* Private function declarations                                              */
/******************************************************************************/

/**
 * @brief   Debug function to print this module's stats
 *
 * @return  N/A
 */
static void vPrintStats( void );

/**
 * @brief   Debug function to clear this module's stats
 *
 * @return  N/A
 */
static void vClearStats( void );

/**
 * @brief   Debug function to write a raw value to EEPROM
 *
 * @return  N/A
 */
static void vWriteEepromData( void );

/**
 * @brief   Debug function to read a raw value from EEPROM
 *
 * @return  N/A
 */
static void vReadEepromData( void );

/**
 * @brief   Debug function to retrieve the EEPROM version
 *
 * @return  N/A
 */
static void vGetEepromVersion( void );

/**
 * @brief   Debug function to retrieve the Product Name
 *
 * @return  N/A
 */
static void vGetEepromProductName( void );

/**
 * @brief   Debug function to retrieve the Product Revision
 *
 * @return  N/A
 */
static void vGetEepromProductRevision( void );

/**
 * @brief   Debug function to retrieve the Board Serial Number
 *
 * @return  N/A
 */
static void vGetEepromSerialNumber( void );

/**
 * @brief   Debug function to retrieve the number of MAC addresses
 *
 * @return  N/A
 */
static void vGetEepromMacAddressCount( void );

/**
 * @brief   Debug function to retrieve the first Mac address
 *
 * @return  N/A
 */
static void vGetEepromFirstMacAddress( void );

/**
 * @brief   Debug function to retrieve the Active or Passive state
 *
 * @return  N/A
 */
static void vGetEepromActiveState( void );

/**
 * @brief   Debug function to retrieve the Config Mode
 *
 * @return  N/A
 */
static void vGetEepromConfigMode( void );

/**
 * @brief   Debug function to retrieve the board maufacturing date
 *
 * @return  N/A
 */
static void vGetEepromMfgDate( void );

/**
 * @brief   Debug function to retrieve the board part number
 *
 * @return  N/A
 */
static void vGetEepromPartNumber( void );

/**
 * @brief   Debug function to retrieve the UUID
 *
 * @return  N/A
 */
static void vGetEepromUuid( void );

/**
 * @brief   Debug function to retrieve the PCIe Id
 *
 * @return  N/A
 */
static void vGetEepromPcieId( void );

/**
 * @brief   Debug function to retrieve the Max Power Mode
 *
 * @return  N/A
 */
static void vGetEepromBoardMaxPowerMode( void );

/**
 * @brief   Debug function to retrieve the DIMM memory size
 *
 * @return  N/A
 */
static void vGetEepromMemorySize( void );

/**
 * @brief   Debug function to retrieve the OEM Id
 *
 * @return  N/A
 */
static void vGetEepromOemId( void );

/**
 * @brief   Debug function to retrieve the board capability
 *
 * @return  N/A
 */
static void vGetEepromCapability( void );

/**
 * @brief   Debug function to retrieve the mfg part number
 *  *
 * @return  N/A
 */
static void vGetEepromMfgPartNumber( void );

/**
 * @brief   Debug function to retrieve all EEPROM values
 *
 * @return  N/A
 */
static void vGetAllEepromValues( void );


/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the EEPROM debug access
 */
void vEeprom_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxEepromTop = pxDAL_NewDirectory( "eeprom" );
        }
        else
        {
            pxEepromTop = pxDAL_NewSubDirectory( "eeprom", pxParentHandle );
        }

        if( NULL != pxEepromTop )
        {
            pxDAL_NewDebugFunction( "print_stats",                      pxEepromTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",                      pxEepromTop, vClearStats );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxEepromTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxEepromTop );
            
            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "write_eeprom_data",                pxSetDir, vWriteEepromData );
            }

            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "read_eeprom_data",                 pxGetDir, vReadEepromData );
                pxDAL_NewDebugFunction( "get_eeprom_version",               pxGetDir, vGetEepromVersion );
                pxDAL_NewDebugFunction( "get_eeprom_product_name",          pxGetDir, vGetEepromProductName );
                pxDAL_NewDebugFunction( "get_eeprom_board_rev",             pxGetDir, vGetEepromProductRevision );
                pxDAL_NewDebugFunction( "get_eeprom_board_serial",          pxGetDir, vGetEepromSerialNumber );
                pxDAL_NewDebugFunction( "get_eeprom_num_macs",              pxGetDir, vGetEepromMacAddressCount );
                pxDAL_NewDebugFunction( "get_eeprom_first_mac",             pxGetDir, vGetEepromFirstMacAddress );
                pxDAL_NewDebugFunction( "get_eeprom_active_passive",        pxGetDir, vGetEepromActiveState );
                pxDAL_NewDebugFunction( "get_eeprom_config_mode",           pxGetDir, vGetEepromConfigMode );
                pxDAL_NewDebugFunction( "get_eeprom_mfg_date",              pxGetDir, vGetEepromMfgDate );
                pxDAL_NewDebugFunction( "get_eeprom_board_part_num",        pxGetDir, vGetEepromPartNumber );
                pxDAL_NewDebugFunction( "get_eeprom_uuid",                  pxGetDir, vGetEepromUuid );
                pxDAL_NewDebugFunction( "get_eeprom_pcie",                  pxGetDir, vGetEepromPcieId );
                pxDAL_NewDebugFunction( "get_eeprom_board_max_power_mode",  pxGetDir, vGetEepromBoardMaxPowerMode );
                pxDAL_NewDebugFunction( "get_eeprom_memory_size",           pxGetDir, vGetEepromMemorySize );
                pxDAL_NewDebugFunction( "get_eeprom_oem_id",                pxGetDir, vGetEepromOemId );
                pxDAL_NewDebugFunction( "get_eeprom_capability",            pxGetDir, vGetEepromCapability );
                pxDAL_NewDebugFunction( "get_eeprom_mfg_part_num",          pxGetDir, vGetEepromMfgPartNumber );
                pxDAL_NewDebugFunction( "get_all_eeprom_fields",            pxGetDir, vGetAllEepromValues );
            }
        }

        iIsInitialised = TRUE;
    }
}

/**
 * @brief   Debug function to print this module's stats
 */
static void vPrintStats( void )
{
    if( OK != iEEPROM_PrintStatistics() )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iEEPROM_ClearStatistics() )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to write raw EEPROM data
 */
static void vWriteEepromData( void )
{
    uint8_t pucData[ EEPROM_MAX_DATA_SIZE ] = { 0 };
    int iSize = 0;
    uint32_t ulAddr = 0;

    if( OK != iDAL_GetIntInRange( "Enter data size", &iSize, 1, EEPROM_MAX_DATA_SIZE ) )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Error retrieving data size\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "Enter EEPROM address", &ulAddr, 0x00, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Error retrieving EEPROM address\r\n" );
    }
    else
    {
        int i = 0;
        for( i = 0; i < iSize; i++ )
        {
            if( OK != iDAL_GetHexInRange( "Enter next byte", ( uint32_t* )( &pucData[ i ] ), 0, UTIL_MAX_UINT8 ) )
            {
                PLL_DAL( EEPROM_DBG_NAME, "Error retrieving data[%d]\r\n", i );
            }
        }
        
        if( OK != iEEPROM_WriteRawValue( pucData, ( uint8_t )iSize, ( uint8_t )ulAddr ) )
        {
            PLL_DAL( EEPROM_DBG_NAME, "Error writing data to 0x%02X\r\n", ulAddr );
        }
        else
        {
            PLL_DAL( EEPROM_DBG_NAME, "Wrote %d bytes to EEPROM address 0x%02X\r\n", iSize, ulAddr );
        }
    }
}

/**
 * @brief   Debug function to read raw EEPROM data
 */
static void vReadEepromData( void )
{
    uint8_t pucData[ EEPROM_MAX_DATA_SIZE ] = { 0 };
    int iSize = 0;
    uint32_t ulAddr = 0;

    if( OK != iDAL_GetIntInRange( "Enter data size", &iSize, 1, EEPROM_MAX_DATA_SIZE ) )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Error retrieving data size\r\n" );
    }
    else if( OK != iDAL_GetHexInRange( "Enter EEPROM address", &ulAddr, 0x00, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Error retrieving EEPROM address\r\n" );
    }
    else
    {
        if( OK != iEEPROM_ReadRawValue( pucData, ( uint8_t )iSize, ( uint8_t )ulAddr ) )
        {
            PLL_DAL( EEPROM_DBG_NAME, "Error reading data from 0x%02X\r\n", ulAddr );
        }
        else
        {
            int i = 0;

            PLL_DAL( EEPROM_DBG_NAME, "%d bytes from EEPROM address 0x%02X\r\n", iSize, ulAddr );
            for( i = 0; i < iSize; i++ )
            {
                if( 0 == ( i % EEPROM_BUFFER_WIDTH ) )
                {
                    if( 0 != i )
                    {
                        vPLL_Printf( "\r\n" );
                    }
                    PLL_DAL( EEPROM_DBG_NAME, "\t[%3d]", i );
                }
                vPLL_Printf( " %02X", pucData[ i ] );
            }

        }
    }
}

/**
 * @brief   Debug function to retrieve the EEPROM version
 */
static void vGetEepromVersion( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetEepromVersion( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "EEPROM version         : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "EEPROM version         : %s\n\r", pucField );
    }
}

/**
 * @brief   Debug function to retrieve the Product Name
 */
static void vGetEepromProductName( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetProductName( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Product name          : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Product name          : %s\n\r", pucField );
    }
}

/**
 * @brief   Debug function to retrieve the Product Revision
 */
static void vGetEepromProductRevision( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetProductRevision( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Revision        : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Revision        : %s\n\r", pucField );
    }
}

/**
 * @brief   Debug function to retrieve the Board Serial Number
 */
static void vGetEepromSerialNumber( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetSerialNumber( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Serial          : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Serial          : %s\n\r", pucField );
    }
}

/**
 * @brief   Debug function to retrieve the number of MAC addresses
 */
static void vGetEepromMacAddressCount( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetMacAddressCount( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "# MACs                : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "# MACs                : %d\n\r", pucField[ 0 ] );
    }
}

/**
 * @brief   Debug function to retrieve the first MAC address
 */
static void vGetEepromFirstMacAddress( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetFirstMacAddress( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board MAC             : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board MAC             : %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                 pucField[0],
                 pucField[1],
                 pucField[2],
                 pucField[3],
                 pucField[4],
                 pucField[5] );
    }
}

/**
 * @brief   Debug function to retrieve the Active or Passive state
 */
static void vGetEepromActiveState( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetActiveState( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board A/P             : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board A/P             : %s\n\r",
                 pucField);
    }
}

/**
 * @brief   Debug function to retrieve the Config Mode
 */
static void vGetEepromConfigMode( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetConfigMode( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Config Mode     : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Config Mode     : %02x\n\r",          pucField[0]   );
    }
}

/**
 * @brief   Debug function to retrieve the board maufacturing date
 */
static void vGetEepromMfgDate( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetManufacturingDate( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Mfg Date              : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Mfg Date              : %x%x%x\n\r",   pucField[0],
                 pucField[1],
                 pucField[2] );
    }
}

/**
 * @brief   Debug function to retrieve the board part number
 */
static void vGetEepromPartNumber( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetPartNumber( pucField, &ucSizeBytes);
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Part Num        : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Part Num        : %s\n\r",            pucField       );
    }
}

/**
 * @brief   Debug function to retrieve the board part number
 */
static void vGetEepromMfgPartNumber( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;

    iStatus = iEEPROM_GetMfgPartNumber( pucField, &ucSizeBytes);
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Mfg Part Num        : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Mfg Part Num        : %s\n\r",            pucField       );
    }
}

/**
 * @brief   Debug function to retrieve the UUID
 */
static void vGetEepromUuid( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetUuid( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "UUID                  : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "UUID                  : %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n\r", 
                 pucField[0],
                 pucField[1],
                 pucField[2],
                 pucField[3],
                 pucField[4],
                 pucField[5],
                 pucField[6],
                 pucField[7],
                 pucField[8],
                 pucField[9],
                 pucField[10],
                 pucField[11],
                 pucField[12],
                 pucField[13],
                 pucField[14],
                 pucField[15] );
    }
}

/**
 * @brief   Debug function to retrieve the PCIE Id
 */
static void vGetEepromPcieId( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetPcieId( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "PCIe Info             : FAILED" );
    }
    else
    {
    PLL_DAL( EEPROM_DBG_NAME, "PCIe Info             : %02x%02x, %02x%02x, %02x%02x, %02x%02x\n\r",
             pucField[0],
             pucField[1],
             pucField[2],
             pucField[3],
             pucField[4],
             pucField[5],
             pucField[6],
             pucField[7] );
    }
}

/**
 * @brief   Debug function to retrieve the Max Power Mode
 */
static void vGetEepromBoardMaxPowerMode( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetMaxPowerMode( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Max Power Mode  : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Board Max Power Mode  : %s\n\r", pucField );
    }
}

/**
 * @brief   Debug function to retrieve the DIMM memory size
 */
static void vGetEepromMemorySize( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetMemorySize( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Memory Size           : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Memory Size           : %s\n\r", pucField );
    }
}

/**
 * @brief   Debug function to retrieve the OEM Id
 */
static void vGetEepromOemId( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetOemId( pucField, &ucSizeBytes );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "OEM ID                : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "OEM ID                : %02x%02x%02x%02x\n\r",  
                 pucField[3],
                 pucField[2],
                 pucField[1],
                 pucField[0] );
    }
}

/**
 * @brief   Debug function to retrieve the capability
 */
static void vGetEepromCapability( void )
{
    uint8_t pucField[ EEPROM_DBG_MAX_FIELD_SIZE ] = { 0 };
    uint8_t ucSizeBytes = 0;
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_GetCapability( pucField, &ucSizeBytes);
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "Capability            : FAILED" );
    }
    else
    {
        PLL_DAL( EEPROM_DBG_NAME, "Capability            : %02x%02x\n\r",          
                 pucField[1],
                 pucField[0] );
    }
}

/**
 * @brief   Debug function to retrieve all EEPROM values
 */
static void vGetAllEepromValues( void )
{
    int iStatus = ERROR;
                                                                     
    iStatus = iEEPROM_DisplayEepromValues( );
    if( ERROR == iStatus )
    {
        PLL_DAL( EEPROM_DBG_NAME, "DisplayEepromValues            : FAILED" );
    }
}
