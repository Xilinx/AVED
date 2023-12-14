/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the AVED Management Interface (AMI) debug implementation
 *
 * @file ami_proxy_driver_debug.c
 *
 */

#include "standard.h"
#include "util.h"
#include "pll.h"

#include "dal.h"

#include "ami_proxy_driver_debug.h"
#include "ami_proxy_driver.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define AMI_DBG_NAME      "AMI_DBG"


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

static int iIsInitialised = FALSE;

static DAL_HDL pxAmiTop = NULL;
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
 * @brief   Debug function to bind a callback to this module
 *
 * @return  N/A
 */
static void vBindCallbacks( void );

/**
 * @brief   Debug function to set the pdi download response
 *
 * @return  N/A
 */
static void vSetPdiDownloadResponse( void );

/**
 * @brief   Debug function to set the pdi copy response
 *
 * @return  N/A
 */
static void vSetPdiCopyResponse( void );

/**
 * @brief   Debug function to set the sensor response
 *
 * @return  N/A
 */
static void vSetSensorResponse( void );

/**
 * @brief   Debug function to set the identity response
 *
 * @return  N/A
 */
static void vSetIdentityResponse( void );

/**
 * @brief   Debug function to set the boot select response
 *
 * @return  N/A
 */
static void vSetBootSelectResponse( void );

/**
 * @brief   Debug function to set the eeprom read/write response
 *
 * @return  N/A
 */
static void vSetEepromRwResponse( void );

/**
 * @brief   Debug function to get the pdi download request
 *
 * @return  N/A
 */
static void vGetPdiDownloadRequest( void );

/**
 * @brief   Debug function to get the pdi copy request
 *
 * @return  N/A
 */
static void vGetPdiCopyRequest( void );

/**
 * @brief   Debug function to get the sensor request
 *
 * @return  N/A
 */
static void vGetSensorRequest( void );

/**
 * @brief   Debug function to get the boot select request
 *
 * @return  N/A
 */
static void vGetBootSelectRequest( void );

/**
 * @brief   Debug function to get the eeprom read/write request
 *
 * @return  N/A
 */
static void vGetEepromRwRequest( void );

/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 *
 * @param   pxSignal     Event raised
 *
 * @return  OK if no errors were raised in the callback
 *          ERROR if an error was raised in the callback
 *
 */
static int iTestCallback( EVL_SIGNAL *pxSignal );


/******************************************************************************/
/* Local variables                                                            */
/******************************************************************************/

const char *pcSensorReqString[ MAX_AMI_PROXY_CMD_SENSOR_REQUEST ] =
{
    "Get size", "Get SDR", "Get single SDR", "Get all SDRs" 
};

const char *pcSensorRepoString[ MAX_AMI_PROXY_CMD_SENSOR_REPO ] =
{
    "Get size", "BDF info", "Temperature", "Voltage", "Current", "Power", "QSFP", "FPT", "All"
};

const char *pcResultStrings[ MAX_AMI_PROXY_RESULT ] =
{
    "Success", "Invalid arg", "IO error", "Memory issue"
};

const char *pcEepromRwReqString[ MAX_AMI_PROXY_CMD_RW_REQUEST ] =
{
    "Read", "Write",
};

/******************************************************************************/
/* Public function implementations                                            */
/******************************************************************************/

/**
 * @brief   Initialise the AMI debug access
 */
void vAMI_DebugInit( DAL_HDL pxParentHandle )
{
    if( FALSE == iIsInitialised )
    {
        if( NULL == pxParentHandle )
        {
            pxAmiTop = pxDAL_NewDirectory( "ami" );
        }
        else
        {
            pxAmiTop = pxDAL_NewSubDirectory( "ami", pxParentHandle );
        }

        if( NULL != pxAmiTop )
        {
            pxDAL_NewDebugFunction( "print_stats",    pxAmiTop, vPrintStats );
            pxDAL_NewDebugFunction( "clear_stats",    pxAmiTop, vClearStats );
            pxDAL_NewDebugFunction( "bind_callbacks", pxAmiTop, vBindCallbacks );
            pxSetDir = pxDAL_NewSubDirectory( "sets", pxAmiTop );
            pxGetDir = pxDAL_NewSubDirectory( "gets", pxAmiTop );

            if( NULL != pxSetDir )
            {
                pxDAL_NewDebugFunction( "set_pdi_download_response", pxSetDir, vSetPdiDownloadResponse );
                pxDAL_NewDebugFunction( "set_pdi_copy_response",     pxSetDir, vSetPdiCopyResponse );
                pxDAL_NewDebugFunction( "set_sensor_response",       pxSetDir, vSetSensorResponse );
                pxDAL_NewDebugFunction( "set_identity_response",     pxSetDir, vSetIdentityResponse );
                pxDAL_NewDebugFunction( "set_boot_select_response",  pxSetDir, vSetBootSelectResponse );
                pxDAL_NewDebugFunction( "set_eeprom_rw_response",    pxSetDir, vSetEepromRwResponse );
            }
            if( NULL != pxGetDir )
            {
                pxDAL_NewDebugFunction( "get_pdi_download_request", pxGetDir, vGetPdiDownloadRequest );
                pxDAL_NewDebugFunction( "get_pdi_copy_request",     pxGetDir, vGetPdiCopyRequest );
                pxDAL_NewDebugFunction( "get_sensor_request",       pxGetDir, vGetSensorRequest );
                pxDAL_NewDebugFunction( "get_boot_select_request",  pxGetDir, vGetBootSelectRequest );
                pxDAL_NewDebugFunction( "get_eeprom_rw_request",    pxGetDir, vGetEepromRwRequest );
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
    if( OK != iAMI_PrintStatistics() )
    {
        PLL_DAL( AMI_DBG_NAME, "Error printing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to clear this module's stats
 */
static void vClearStats( void )
{
    if( OK != iAMI_ClearStatistics() )
    {
        PLL_DAL( AMI_DBG_NAME, "Error clearing statistics\r\n" );
    }
}

/**
 * @brief   Debug function to bind a callback to this module
 */
static void vBindCallbacks( void )
{
    if( OK != iAMI_BindCallback( &iTestCallback ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error binding callback\r\n" );
    }
    else
    {
        PLL_DAL( AMI_DBG_NAME, "Callback bound\r\n" );
    }
}

/**
 * @brief   Debug function to set the pdi download response
 */
static void vSetPdiDownloadResponse( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL       xEv     = { 0 };
        AMI_PROXY_RESULT xRes    = { 0 };
        int              iResult = 0;
        int              i       = 0;

        xEv.ucInstance = ( uint8_t )iInstance;

        for( i = 0; i < MAX_AMI_PROXY_RESULT; i++ )
        {
            PLL_DAL( AMI_DBG_NAME, "\t- %d - %s\r\n", i, pcResultStrings[ i ] );
        }

        if( OK != iDAL_GetIntInRange( "Select result:", &iResult, 0, MAX_AMI_PROXY_RESULT - 1 ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Invalid result\r\n" );
        }
        else
        {
            xRes = ( AMI_PROXY_RESULT )iResult;
            if( OK != iAMI_SetPdiDownloadCompleteResponse( &xEv, xRes ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting result %s to instance %d\r\n",
                                pcResultStrings[ iResult ], iInstance );
            }
            else
            {
                PLL_DAL( AMI_DBG_NAME, "Set result %s to instance %d\r\n",
                                pcResultStrings[ iResult ], iInstance );
            }
        }
    }
}

/**
 * @brief   Debug function to set the pdi copy response
 */
static void vSetPdiCopyResponse( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL       xEv     = { 0 };
        AMI_PROXY_RESULT xRes    = { 0 };
        int              iResult = 0;
        int              i       = 0;

        xEv.ucInstance = ( uint8_t )iInstance;

        for( i = 0; i < MAX_AMI_PROXY_RESULT; i++ )
        {
            PLL_DAL( AMI_DBG_NAME, "\t- %d - %s\r\n", i, pcResultStrings[ i ] );
        }

        if( OK != iDAL_GetIntInRange( "Select result:", &iResult, 0, MAX_AMI_PROXY_RESULT - 1 ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Invalid result\r\n" );
        }
        else
        {
            xRes = ( AMI_PROXY_RESULT )iResult;
            if( OK != iAMI_SetPdiCopyCompleteResponse( &xEv, xRes ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting result %s to instance %d\r\n",
                         pcResultStrings[ iResult ], iInstance );
            }
            else
            {
                PLL_DAL( AMI_DBG_NAME, "Set result %s to instance %d\r\n",
                         pcResultStrings[ iResult ], iInstance );
            }
        }
    }
}

/**
 * @brief   Debug function to set the sensor response
 */
static void vSetSensorResponse( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL       xEv     = { 0 };
        AMI_PROXY_RESULT xRes    = { 0 };
        int              iResult = 0;
        int              i       = 0;

        xEv.ucInstance = ( uint8_t )iInstance;

        for( i = 0; i < MAX_AMI_PROXY_RESULT; i++ )
        {
            PLL_DAL( AMI_DBG_NAME, "\t- %d - %s\r\n", i, pcResultStrings[ i ] );
        }

        if( OK != iDAL_GetIntInRange( "Select result:", &iResult, 0, MAX_AMI_PROXY_RESULT - 1 ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Invalid result\r\n" );
        }
        else
        {
            xRes = ( AMI_PROXY_RESULT )iResult;
            if( OK != iAMI_SetSensorCompleteResponse( &xEv, xRes ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting result %s to instance %d\r\n",
                         pcResultStrings[ iResult ], iInstance );
            }
            else
            {
                PLL_DAL( AMI_DBG_NAME, "Set result %s to instance %d\r\n",
                         pcResultStrings[ iResult ], iInstance );
            }
        }
    }
}

/**
 * @brief   Debug function to set the boot select response
 */
static void vSetBootSelectResponse( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL       xEv     = { 0 };
        AMI_PROXY_RESULT xRes    = { 0 };
        int              iResult = 0;
        int              i       = 0;

        xEv.ucInstance = ( uint8_t )iInstance;

        for( i = 0; i < MAX_AMI_PROXY_RESULT; i++ )
        {
            PLL_DAL( AMI_DBG_NAME, "\t- %d - %s\r\n", i, pcResultStrings[ i ] );
        }

        if( OK != iDAL_GetIntInRange( "Select result:", &iResult, 0, MAX_AMI_PROXY_RESULT - 1 ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Invalid result\r\n" );
        }
        else
        {
            xRes = ( AMI_PROXY_RESULT )iResult;
            if( OK != iAMI_SetBootSelectCompleteResponse( &xEv, xRes ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting result %s to instance %d\r\n",
                         pcResultStrings[ iResult ], iInstance );
            }
            else
            {
                PLL_DAL( AMI_DBG_NAME, "Set result %s to instance %d\r\n",
                         pcResultStrings[ iResult ], iInstance );
            }
        }
    }
}

/**
 * @brief   Debug function to set the identity response
 */
static void vSetIdentityResponse( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL       xEv     = { 0 };
        AMI_PROXY_RESULT xRes    = { 0 };
        int              iResult = 0;
        int              i       = 0;

        xEv.ucInstance = ( uint8_t )iInstance;

        for( i = 0; i < MAX_AMI_PROXY_RESULT; i++ )
        {
            PLL_DAL( AMI_DBG_NAME, "\t- %d - %s\r\n", i, pcResultStrings[ i ] );
        }

        if( OK != iDAL_GetIntInRange( "Select result:", &iResult, 0, MAX_AMI_PROXY_RESULT - 1 ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Invalid result\r\n" );
        }
        else
        {
            int iMaj     = 0;
            int iMin     = 0;
            int iPatch   = 0;
            int iChanges = 0;
            int iCommits = 0;
            int iLinkMaj = 0;
            int iLinkMin = 0;

            xRes = ( AMI_PROXY_RESULT )iResult;

            if( OK != iDAL_GetIntInRange( "Set major version:", &iMaj, 0, UTIL_MAX_UINT8 ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting major version\r\n" );
            }
            else if( OK != iDAL_GetIntInRange( "Set minor version:", &iMin, 0, UTIL_MAX_UINT8 ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting minor version\r\n" );
            }
            else if( OK != iDAL_GetIntInRange( "Set patch:", &iPatch, 0, UTIL_MAX_UINT8 ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting patch\r\n" );
            }
            else if( OK != iDAL_GetIntInRange( "Set dev commits:", &iCommits, 0, UTIL_MAX_UINT16 ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting dev commits\r\n" );
            }
            else if( OK != iDAL_GetIntInRange( "Local changes? (yes:1, no:0):", &iChanges, 0, 1 ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting local changes\r\n" );
            }
            else
            {
                AMI_PROXY_IDENTITY_RESPONSE xId  = { 0 };
                xId.ucVerMajor     = ( uint8_t )iMaj;
                xId.ucVerMinor     = ( uint8_t )iMin;
                xId.ucVerPatch     = ( uint8_t )iPatch;
                xId.ucLocalChanges = ( uint8_t )iChanges;
                xId.usDevCommits   = ( uint16_t )iCommits;
                xId.ucLinkVerMajor = ( uint8_t )iLinkMaj;
                xId.ucLinkVerMinor = ( uint8_t )iLinkMin;
        
                if( OK != iAMI_SetIdentityResponse( &xEv, xRes, &xId ) )
                {
                    PLL_DAL( AMI_DBG_NAME, "Error setting result %s to instance %d\r\n",
                             pcResultStrings[ iResult ], iInstance );
                }
                else
                {
                    PLL_DAL( AMI_DBG_NAME, "Set result %s to instance %d\r\n",
                             pcResultStrings[ iResult ], iInstance );
                }
            }
        }
    }
}

/**
 * @brief   Debug function to set the eeprom read/write response
 */
static void vSetEepromRwResponse( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL       xEv     = { 0 };
        AMI_PROXY_RESULT xRes    = { 0 };
        int              iResult = 0;
        int              i       = 0;

        xEv.ucInstance = ( uint8_t )iInstance;

        for( i = 0; i < MAX_AMI_PROXY_RESULT; i++ )
        {
            PLL_DAL( AMI_DBG_NAME, "\t- %d - %s\r\n", i, pcResultStrings[ i ] );
        }

        if( OK != iDAL_GetIntInRange( "Select result:", &iResult, 0, MAX_AMI_PROXY_RESULT - 1 ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Invalid result\r\n" );
        }
        else
        {
            xRes = ( AMI_PROXY_RESULT )iResult;
            if( OK != iAMI_SetSensorCompleteResponse( &xEv, xRes ) )
            {
                PLL_DAL( AMI_DBG_NAME, "Error setting result %s to instance %d\r\n",
                                pcResultStrings[ iResult ], iInstance );
            }
            else
            {
                PLL_DAL( AMI_DBG_NAME, "Set result %s to instance %d\r\n",
                                pcResultStrings[ iResult ], iInstance );
            }
        }
    }
}

/**
 * @brief   Debug function to get the pdi download request
 */
static void vGetPdiDownloadRequest( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL                     xEv  = { 0 };
        AMI_PROXY_PDI_DOWNLOAD_REQUEST xReq = { 0 };
        
        xEv.ucInstance = ( uint8_t )iInstance;

        if( OK != iAMI_GetPdiDownloadRequest( &xEv, &xReq ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Error getting download request\r\n" );
        }
        else
        {
            PLL_DAL( AMI_DBG_NAME, "PDI download request (instance: %d) retrieved\r\n", iInstance );
            PLL_DAL( AMI_DBG_NAME, "\tAddress . . . . . 0x%16llX\r\n", xReq.ullAddress );
            PLL_DAL( AMI_DBG_NAME, "\tLength. . . . . . 0x%08X\r\n", xReq.ulLength );
            PLL_DAL( AMI_DBG_NAME, "\tPartition . . . . 0x%08X\r\n", xReq.ulPartitionSel );
        }
    }
}

/**
 * @brief   Debug function to get the pdi copy request
 */
static void vGetPdiCopyRequest( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL                 xEv  = { 0 };
        AMI_PROXY_PDI_COPY_REQUEST xReq = { 0 };
        
        xEv.ucInstance = ( uint8_t )iInstance;

        if( OK != iAMI_GetPdiCopyRequest( &xEv, &xReq ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Error getting copy request\r\n" );
        }
        else
        {
            PLL_DAL( AMI_DBG_NAME, "PDI copy request (instance: %d) retrieved\r\n", iInstance );
            PLL_DAL( AMI_DBG_NAME, "\tSrc partition . . 0x%08X\r\n", xReq.ulSrcPartition );
            PLL_DAL( AMI_DBG_NAME, "\tDst partition . . 0x%08X\r\n", xReq.ulDestPartition );
        }
    }
}

/**
 * @brief   Debug function to get the sensor request
 */
static void vGetSensorRequest( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL               xEv  = { 0 };
        AMI_PROXY_SENSOR_REQUEST xReq = { 0 };

        xEv.ucInstance = ( uint8_t )iInstance;
        
        if( OK != iAMI_GetSensorRequest( &xEv, &xReq ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Error getting sensor request\r\n" );
        }
        else
        {
            PLL_DAL( AMI_DBG_NAME, "PDI sensor request (instance: %d) retrieved\r\n", iInstance );
            PLL_DAL( AMI_DBG_NAME, "\tSensor req. . . . %s\r\n", pcSensorReqString[ xReq.ulSensorId ] );
            PLL_DAL( AMI_DBG_NAME, "\tSensor repo . . . %s\r\n", pcSensorRepoString[ xReq.ulSensorId ] );
            PLL_DAL( AMI_DBG_NAME, "\tSensor ID . . . . 0x%08X\r\n", xReq.ulSensorId );
            PLL_DAL( AMI_DBG_NAME, "\tLength. . . . . . 0x%08X\r\n", xReq.ulLength );
            PLL_DAL( AMI_DBG_NAME, "\tAddress . . . . . 0x%16llX\r\n", xReq.ullAddress );
        }
    }
}

/**
 * @brief   Debug function to get the boot select request
 */
static void vGetBootSelectRequest( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL                    xEv  = { 0 };
        AMI_PROXY_BOOT_SELECT_REQUEST xReq = { 0 };
        
        xEv.ucInstance = ( uint8_t )iInstance;

        if( OK != iAMI_GetBootSelectRequest( &xEv, &xReq ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Error getting boot select request\r\n" );
        }
        else
        {
            PLL_DAL( AMI_DBG_NAME, "Boot select request (instance: %d) retrieved\r\n", iInstance );
            PLL_DAL( AMI_DBG_NAME, "\tPartition . . . . 0x%08X\r\n", xReq.ulPartitionSel );
        }
    }
}

/**
 * @brief   Debug function to get the eeprom read/write request
 */
static void vGetEepromRwRequest( void )
{
    int iInstance = 0;
    if( OK != iDAL_GetIntInRange( "Enter this request instance:", &iInstance, 0, UTIL_MAX_UINT8 ) )
    {
        PLL_DAL( AMI_DBG_NAME, "Error setting instance\r\n" );
    }
    else
    {
        EVL_SIGNAL                  xEv  = { 0 };
        AMI_PROXY_EEPROM_RW_REQUEST xReq = { 0 };

        xEv.ucInstance = ( uint8_t )iInstance;
        if( OK != iAMI_GetEepromReadWriteRequest( &xEv, &xReq ) )
        {
            PLL_DAL( AMI_DBG_NAME, "Error EEPROM read/write request\r\n" );
        }
        else
        {
            PLL_DAL( AMI_DBG_NAME, "EEPROM read/write request (instance: %d) retrieved\r\n", iInstance );
            PLL_DAL( AMI_DBG_NAME, "\tReq . . . . . . . %s\r\n", pcEepromRwReqString[ xReq.xRequest ] );
            PLL_DAL( AMI_DBG_NAME, "\tAddress . . . . . 0x%16llX\r\n", xReq.ullAddress );
            PLL_DAL( AMI_DBG_NAME, "\tLength. . . . . . 0x%08X\r\n", xReq.ulLength );
            PLL_DAL( AMI_DBG_NAME, "\tOffset. . . . . . 0x%08X\r\n", xReq.ulOffset );
        }
    }
}

/***** Helper functions *****/

/**
 * @brief   EVL Callback for binding test prints
 */
static int iTestCallback( EVL_SIGNAL *pxSignal )
{
    int iStatus = ERROR;

    if( NULL != pxSignal )
    {
        PLL_DAL( AMI_DBG_NAME, "AMI Signal raised: Module[%d], Type[%d], Instance[%d]\r\n",
                 pxSignal->ucModule, pxSignal->ucEventType, pxSignal->ucInstance );
        iStatus = OK;
    }

    return iStatus;
}

