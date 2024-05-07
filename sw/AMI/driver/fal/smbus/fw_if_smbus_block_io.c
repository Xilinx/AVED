/**
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*
* This file contains the FW IF SMBus Block IO interface implementation.
*
* @file fw_if_smbus_block_io.c
*
*/


/*****************************************************************************/
/* includes                                                                  */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "pll.h"
#include "util.h"
#include "osal.h"

#include "fw_if_smbus.h"
#include "profile_fal.h"
#include "smbus.h"
#include "smbus_internal.h"
#include "xil_types.h"


/*****************************************************************************/
/* defines                                                                   */
/*****************************************************************************/

#define FW_IF_SMBUS_BLOCK_IO_NAME       "FW_IF_SMBUS_BLOCK_IO"
#define SMBUS_BLOCK_IO_UPPER_FIREWALL   ( 0xBEEFCAFE )
#define SMBUS_BLOCK_IO_LOWER_FIREWALL   ( 0xDEADFACE )

#define CHECK_DRIVER            if( FW_IF_FALSE == pxThis->iInitialised ) return FW_IF_ERRORS_DRIVER_NOT_INITIALISED
#define CHECK_FIREWALLS( f )    if( ( f->upperFirewall != SMBUS_BLOCK_IO_UPPER_FIREWALL ) &&\
                                    ( f->lowerFirewall != SMBUS_BLOCK_IO_LOWER_FIREWALL ) ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_HDL( f )          if( NULL == f ) return FW_IF_ERRORS_INVALID_HANDLE
#define CHECK_CFG( f )          if( NULL == ( f )->cfg  ) return FW_IF_ERRORS_INVALID_CFG

/* Stat & Error definitions */
#define FW_IF_SMBUS_BLOCK_IO_STAT_COUNTS( DO )   \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_INIT_OVERALL_COMPLETE )         \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_INSTANCE_CREATE )               \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_OPEN )                          \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_CLOSE )                         \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_READ )                          \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_WRITE )                         \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_IO_CTRL )                       \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_BIND_CALLBACK )                 \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_GET_PROTOCOL )                  \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_READ_DATA )                     \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_WRITE_DATA )                    \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_SEMAPHORE_POST )           	   \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_SEMAPHORE_PEND )           	   \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_BLOCK_WRITE_SUCCESS )           \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_ANNOUNCE_RESULT_GENERIC )       \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_ANNOUNCE_ARP )   	           \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_SETUP_INTERRUPTS )              \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_MAX )

#define FW_IF_SMBUS_BLOCK_IO_ERROR_COUNTS( DO )    \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_INIT_OVERALL_FAILED )           \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_INSTANCE_CREATE_FAILED )       \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_OPEN_FAILED )                  \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_CLOSE_FAILED )                 \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_READ_FAILED )                  \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_WRITE_FAILED )                 \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_IO_CTRL_FAILED )               \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_BIND_CALLBACK_FAILED )         \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_GET_PROTOCOL_FAILED )          \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_READ_DATA_FAILED )             \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_WRITE_DATA_FAILED )            \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_SEMAPHORE_POST_FAILED )   	   \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_SEMAPHORE_PEND_FAILED )   	   \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_BLOCK_WRITE_FAILED )           \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_ANNOUNCE_BUS_ERROR )           \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_ANNOUNCE_BUS_WARN )            \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_VALIDATION_FAILED )            \
    DO( FW_IF_SMBUS_BLOCK_IO_STATS_SETUP_INTERRUPTS_FAILED )       \
    DO( FW_IF_SMBUS_BLOCK_IO_ERRORS_MAX )

#define PRINT_STAT_COUNTER( x )             PLL_INF( FW_IF_SMBUS_BLOCK_IO_NAME, "%50s . . . . %d\r\n",    \
                                                     FW_IF_SMBUS_BLOCK_IO_STAT_COUNTS_STR[ x ],           \
                                                     pxThis->pulStatCounters[ x ] )

#define PRINT_ERROR_COUNTER( x )            PLL_INF( FW_IF_SMBUS_BLOCK_IO_NAME, "%50s . . . . %d\r\n",    \
                                                     FW_IF_SMBUS_BLOCK_IO_ERROR_COUNTS_STR[ x ],          \
                                                     pxThis->pulErrorCounters[ x ] )

#define INC_STAT_COUNTER( x )               { if( x < FW_IF_SMBUS_BLOCK_IO_STATS_MAX )pxThis->pulStatCounters[ x ]++; }
#define INC_ERROR_COUNTER( x )              { if( x < FW_IF_SMBUS_BLOCK_IO_ERRORS_MAX )pxThis->pulErrorCounters[ x ]++; }


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    FW_IF_SMBUS_BLOCK_IO_STAT_COUNTS
 * @brief   Enumeration of stat counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( FW_IF_SMBUS_BLOCK_IO_STAT_COUNTS, FW_IF_SMBUS_BLOCK_IO_STAT_COUNTS, FW_IF_SMBUS_BLOCK_IO_STAT_COUNTS_STR )

/**
 * @enum    FW_IF_SMBUS_BLOCK_IO_ERROR_COUNTS
 * @brief   Enumeration of error counters for this application
 */
UTIL_MAKE_ENUM_AND_STRINGS( FW_IF_SMBUS_BLOCK_IO_ERROR_COUNTS, FW_IF_SMBUS_BLOCK_IO_ERROR_COUNTS, FW_IF_SMBUS_BLOCK_IO_ERROR_COUNTS_STR )


/*****************************************************************************/
/* structures                                                                */
/*****************************************************************************/

/**
 * @struct  FW_IF_SMBUS_BLOCK_IO_PRIVATE_DATA
 * @brief   Structure to hold this FAL's private data
 */
typedef struct FW_IF_SMBUS_BLOCK_IO_PRIVATE_DATA
{
    uint32_t                    ulUpperFirewall;

    FW_IF_SMBUS_INIT_CFG        xLocalCfg;
    int                         iInitialised;

    struct SMBUS_PROFILE_TYPE*  pxSMBusProfile;
    uint8_t                     pucCallBackWriteData[ FW_IF_SMBUS_MAX_DATA ];
    uint16_t                    usCallBackWriteDataSize;
    void                        *pvWriteCallbackSem;

    uint32_t                    pulStatCounters[ FW_IF_SMBUS_BLOCK_IO_STATS_MAX ];
    uint32_t                    pulErrorCounters[ FW_IF_SMBUS_BLOCK_IO_ERRORS_MAX ];

    uint32_t                    ulLowerFirewall;

} FW_IF_SMBUS_BLOCK_IO_PRIVATE_DATA;


/*****************************************************************************/
/* local variables                                                           */
/*****************************************************************************/

static FW_IF_SMBUS_BLOCK_IO_PRIVATE_DATA xLocalData =
{
    SMBUS_BLOCK_IO_UPPER_FIREWALL,  /* ulUpperFirewall */

    { 0 },                          /* xLocalCfg       */
    FW_IF_FALSE,                    /* iInitialised    */

    NULL,                           /* pxSMBusProfile */
    { 0 },                          /* pucCallBackWriteData */
    0,                              /* usCallBackWriteDataSize */
    NULL,                           /* pvWriteCallbackSem */

    { 0 },                          /* pulStatCounters */
    { 0 },                          /* pulErrorCounters */

    SMBUS_BLOCK_IO_LOWER_FIREWALL   /* ulLowerFirewall */
};
static FW_IF_SMBUS_BLOCK_IO_PRIVATE_DATA *pxThis = &xLocalData;


/******************************************************************************/
/* local Function Declarations                                                */
/******************************************************************************/

/**
 * @brief   Local implementation of FW_IF_open
 *
 * @param   pvFwIf          Pointer to this fw_if
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulSmbusBlockIoOpen( void *pvFwIf );

/**
 * @brief   Local implementation of FW_IF_close
 *
 * @param   pvFwIf          Pointer to this fw_if
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulSmbusBlockIoClose( void *pvFwIf );

/**
 * @brief   Local implementation of FW_IF_write
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ullAddrOffset   The address offset
 * @param   pucData         Data buffer to write
 * @param   ulLength        Number of bytes in data buffer
 * @param   ulTimeoutMs     Time (in ms) to wait for write to complete
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulSmbusBlockIoWrite( void *pvFwIf,
                                     uint64_t ullAddrOffset,
                                     uint8_t *pucData,
                                     uint32_t ulLength,
                                     uint32_t ulTimeoutMs );

/**
 * @brief   Local implementation of FW_IF_read
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ullAddrOffset   The address offset
 * @param   pucData         Data buffer to read
 * @param   pulLength       Pointer to maximum number of bytes allowed in data buffer
 *                          This value is updated to the actual number of bytes read
 * @param   ulTimeoutMs     Time (in ms) to wait for read to complete
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulSmbusBlockIoRead( void *pvFwIf,
                                    uint64_t ullAddrOffset,
                                    uint8_t *pucData,
                                    uint32_t *pulLength,
                                    uint32_t ulTimeoutMs );

/**
 * @brief   Local implementation of FW_IF_ioctrl
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   ulOption        Unique IO Ctrl option to set/get
 * @param   pvValue         Pointer to value to set/get
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulSmbusBlockIoIoctrl( void *pvFwIf, uint32_t ulOption, void *pvValue );

/**
 * @brief   Local implementation of FW_IF_bindCallback
 *
 * @param   pvFwIf          Pointer to this fw_if
 * @param   pxNewFunc       Function pointer to call
 *
 * @return  See FW_IF_ERRORS
 */
static uint32_t ulSmbusBlockIoBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc );

/**
 * @brief   Get protocol callback.
 *          This will set the protocol for a specific SMBus command code.
 *
 * @param   ucCommand           SMBus command code
 * @param   pxProtocol          SMBus protocol
 *
 * @return  none
 *
 * @note    SMBus Callback function
 *
 */
static void vGetProtocol( uint8_t ucCommand, SMBus_Command_Protocol_Type* pxProtocol );

/**
 * @brief   Read data callback.
 *          This will send data which can be read by the other party.
 *
 * @param   ucCommand        SMBus command code
 * @param   pucData          SMBus Data buffer
 * @param   pusDataSize      Pointer to SMBus Data size
 *
 * @return  none
 *
 * @note    SMBus Callback function
 *
 */
static void vReadData( uint8_t ucCommand, uint8_t* pucData, uint16_t* pusDataSize );

/**
 * @brief   Write data callback.
 *          This will write ( copy ) data that was sent by another party.
 *
 * @param   ucCommand        SMBus command code
 * @param   pucData          SMBus Data buffer
 * @param   usDataSize       SMBus Data size
 * @param   ulTransactionID  SMBus transaction ID
 *
 * @return  none
 *
 * @note    SMBus Callback function
 *
 */
static void vWriteData( uint8_t ucCommand, uint8_t* pucData, uint16_t usDataSize, uint32_t ulTransactionID );

/**
 * @brief   Announce result callback.
 *          This gets called when a transaction is complete.
 *
 * @param   ucCommand           SMBus command code
 * @param   ulTransactionID     SMBus transaction ID
 * @param   ulStatus            SMBus status
 *
 * @return  none
 *
 * @note    SMBus Callback function
 *
 */
static void vAnnounceResult( uint8_t ucCommand, uint32_t ulTransactionID, uint32_t ulStatus );

/**
 * @brief   ARP address change callback.
 *          This gets called when the address of the instance changes.
 *
 * @param   ucNewAddress           SMBus New ARP Address
 *
 * @return  none
 *
 * @note    SMBus Callback function
 *
 */
static void vAnnounceARP( uint8_t ucNewAddress );

/**
 * @brief   Announce bus error callback
 *          This gets called when a bus error is passed up from the IP.
 *
 * @param   ucError           SMBus Error code
 *
 * @return  none
 *
 * @note    SMBus Callback function
 *
 */
static void vAnnounceBusError( uint8_t ucError );

/**
 * @brief   Announce bus warning callback
 *          This gets called when a bus warning is passed up from the IP.
 *
 * @param   ucWarning           SMBus Warning code
 *
 * @return  none
 *
 * @note    SMBus Callback function
 *
 */
static void vAnnounceBusWarning( uint8_t ucWarning );

/**
 * @brief   Setup SMBus interrupts.
 *
 * @return  OK            Setup interrupts success
 *          ERROR         Setup interrupts failed
 *
 */
static int iSmbusSetupInterrupts( void );


/*****************************************************************************/
/* local functions                                                           */
/*****************************************************************************/

/* FW_IF implementations */

/**
 * @brief Local implementation of FW_IF_open
 */
static uint32_t ulSmbusBlockIoOpen( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;
    FW_IF_SMBUS_CFG *pxCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;

    if( ( FW_IF_SMBUS_STATE_CREATED == pxCfg->xState ) || ( FW_IF_SMBUS_STATE_CLOSED == pxCfg->xState ) )
    {
        /* Set parameters */
        uint8_t ucSMBusAddress = pxCfg->ulPort;
        uint8_t pucUDID[ SMBUS_UDID_LENGTH ] = { 0 };
        SMBus_ARP_Capability xARPCapability = SMBUS_ARP_NON_ARP_CAPABLE;
        uint8_t  ucSimpleDevice = 0;

        switch( pxCfg->xArpCapability )
        {
            case FW_IF_SMBUS_ARP_CAPABILITY:
                xARPCapability = SMBUS_ARP_CAPABLE;
                break;

            case FW_IF_SMBUS_ARP_FIXED_DISCOVERABLE:
                xARPCapability = SMBUS_ARP_FIXED_AND_DISCOVERABLE;
                break;

            case FW_IF_SMBUS_ARP_FIXED_NOT_DISCOVERABLE:
                xARPCapability = SMBUS_ARP_FIXED_NOT_DISCOVERABLE;
                break;

            case FW_IF_SMBUS_ARP_NON_ARP_CAPABLE:
                xARPCapability = SMBUS_ARP_NON_ARP_CAPABLE;
                break;

            default:
                break;
        }

        pvOSAL_MemCpy( pucUDID, pxCfg->pucUdid, SMBUS_UDID_LENGTH );

        if( FW_IF_SMBUS_PEC_ENABLED == pxCfg->xPecCapability )
        {
            /* Set device capabilities bit (enable PEC) */
            pucUDID[ SMBUS_UDID_DEVICE_CAPABILITIES_BYTE ] |= SMBUS_UDID_PEC_SUPPORTED_BIT;
        }

        /* Set Callback functions */
        SMBUS_USER_SUPPLIED_ENVIRONMENT_GET_PROTOCOL_TYPE  pFnGetProtocol      = ( SMBUS_USER_SUPPLIED_ENVIRONMENT_GET_PROTOCOL_TYPE  )&vGetProtocol;
        SMBUS_USER_SUPPLIED_ENVIRONMENT_GET_DATA_TYPE      pFnGetData          = ( SMBUS_USER_SUPPLIED_ENVIRONMENT_GET_DATA_TYPE      )&vReadData;
        SMBUS_USER_SUPPLIED_ENVIRONMENT_WRITE_DATA_TYPE    pFnWriteData        = ( SMBUS_USER_SUPPLIED_ENVIRONMENT_WRITE_DATA_TYPE    )&vWriteData;
        SMBUS_USER_SUPPLIED_ENVIRONMENT_COMMAND_COMPLETE   pFnAnnounceResult   = ( SMBUS_USER_SUPPLIED_ENVIRONMENT_COMMAND_COMPLETE   )&vAnnounceResult;
        SMBUS_USER_SUPPLIED_ENVIRONMENT_ARP_ADRRESS_CHANGE pFnArpAddressChange = ( SMBUS_USER_SUPPLIED_ENVIRONMENT_ARP_ADRRESS_CHANGE )&vAnnounceARP;
        SMBUS_USER_SUPPLIED_ENVIRONMENT_BUS_ERROR          pFnBusError         = ( SMBUS_USER_SUPPLIED_ENVIRONMENT_BUS_ERROR          )&vAnnounceBusError;
        SMBUS_USER_SUPPLIED_ENVIRONMENT_BUS_WARNING        pFnBusWarning       = ( SMBUS_USER_SUPPLIED_ENVIRONMENT_BUS_WARNING        )&vAnnounceBusWarning;

        /* create instance with smbus-driver */
        pxCfg->ucInstance = ucCreateSMBusInstance( pxThis->pxSMBusProfile, ucSMBusAddress, pucUDID, xARPCapability,
                                                   pFnGetProtocol, pFnGetData, pFnWriteData, pFnAnnounceResult,
                                                   pFnArpAddressChange, pFnBusError, pFnBusWarning, ucSimpleDevice );

        if( SMBUS_INVALID_INSTANCE != pxCfg->ucInstance )
        {
            pxCfg->xState = FW_IF_SMBUS_STATE_OPENED;
            INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_OPEN )
        }
        else
        {
            ulStatus = FW_IF_ERRORS_OPEN;
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_OPEN_FAILED )
        }
    }
    else
    {
        ulStatus = FW_IF_SMBUS_ERRORS_INVALID_STATE;
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_OPEN_FAILED )
    }

    return ulStatus;
}

/**
 * @brief Local implementation of FW_IF_close
 */
static uint32_t ulSmbusBlockIoClose( void *pvFwIf )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    FW_IF_SMBUS_CFG *pxCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;

    if( FW_IF_SMBUS_STATE_OPENED == pxCfg->xState )
    {
        /* destroy SMBus instance */
        if( SMBUS_ERROR != xDestroySMBusInstance( pxThis->pxSMBusProfile, pxCfg->ucInstance ) )
        {
            pxCfg->ucInstance = SMBUS_INVALID_INSTANCE;
            pxCfg->xState = FW_IF_SMBUS_STATE_CLOSED;
            INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_CLOSE )
        }
        else
        {
            ulStatus = FW_IF_ERRORS_CLOSE;
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_CLOSE_FAILED )
        }
    }
    else
    {
        ulStatus = FW_IF_SMBUS_ERRORS_INVALID_STATE;
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_CLOSE_FAILED )
    }

    return ulStatus;
}

/**
 * @brief Local implementation of FW_IF_write
 */
static uint32_t ulSmbusBlockIoWrite( void *pvFwIf, uint64_t dstPort, uint8_t *pucData, uint32_t ulSize, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( NULL != pucData )
    {
        FW_IF_SMBUS_CFG *pxCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;

        if( FW_IF_SMBUS_STATE_OPENED == pxCfg->xState )
        {
            uint8_t ucCommand = pxThis->xLocalCfg.pucCommandProtocols[ FW_IF_SMBUS_COMMAND_PROTOCOL_BLOCK_WRITE ]; /* specified command code for block write */
            uint32_t ulTransactionID = 0;

            /* allocate memory to send */
            uint16_t usDataToSendSize = ulSize + sizeof( uint8_t ); /* add one byte to payload for size value */
            uint8_t* pucDataToSend = ( uint8_t* )pvOSAL_MemAlloc( usDataToSendSize * sizeof( uint8_t ) );

            if( NULL != pucDataToSend )
            {
                int iPecCapability = FALSE;

                switch( pxCfg->xPecCapability )
                {
                    case FW_IF_SMBUS_PEC_ENABLED:
                        iPecCapability = TRUE;
                        break;

                    case FW_IF_SMBUS_PEC_DISABLED:
                        iPecCapability = FALSE;
                        break;

                    default:
                        break;
                }

                pucDataToSend[ FAL_SMBUS_BLOCK_IO_DATA_SIZE_INDEX ] = ulSize;
                pvOSAL_MemCpy( &pucDataToSend[ FAL_SMBUS_BLOCK_IO_PAYLOAD_INDEX ], pucData, ulSize );

                if( SMBUS_ERROR != xSMBusControllerInitiateCommand( pxThis->pxSMBusProfile, pxCfg->ucInstance, ( uint8_t )dstPort, ucCommand,
                    SMBUS_PROTOCOL_BLOCK_WRITE, usDataToSendSize, pucDataToSend, ( uint8_t )iPecCapability, &ulTransactionID ) )
                {
                    INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_WRITE )
                }
                else
                {
                    ulStatus = FW_IF_ERRORS_WRITE;
                    INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_WRITE_FAILED )
                }

                /* free memory */
                vOSAL_MemFree( ( void** )&pucDataToSend );
            }
            else
            {
                ulStatus = FW_IF_ERRORS_WRITE;
                INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_WRITE_FAILED )
            }
        }
        else
        {
            ulStatus = FW_IF_SMBUS_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_WRITE_FAILED )
        }
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }

    return ulStatus;
}


/**
 *  @brief Local implementation of FW_IF_read
 */
static uint32_t ulSmbusBlockIoRead( void *pvFwIf, uint64_t ullSrcPort, uint8_t *pucData, uint32_t *pulSize, uint32_t ulTimeoutMs )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( ( NULL != pucData ) && ( NULL != pulSize ) )
    {
        FW_IF_SMBUS_CFG *pxCfg = ( FW_IF_SMBUS_CFG* )pxThisIf->cfg;

        if( FW_IF_SMBUS_STATE_OPENED == pxCfg->xState )
        {
            /* Target Block write */
            uint32_t ulOsalTimeout = 0;
            switch( ulTimeoutMs )
            {
                case FW_IF_TIMEOUT_NO_WAIT:
                    ulOsalTimeout = OSAL_TIMEOUT_NO_WAIT;
                    break;
                case FW_IF_TIMEOUT_WAIT_FOREVER:
                    ulOsalTimeout = OSAL_TIMEOUT_WAIT_FOREVER;
                    break;
                default:
                    ulOsalTimeout = ulTimeoutMs;
                    break;
            }

            if( OSAL_ERRORS_NONE != iOSAL_Semaphore_Pend( pxThis->pvWriteCallbackSem, ulOsalTimeout ) )
            {
                ulStatus = FW_IF_ERRORS_TIMEOUT;
                INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_SEMAPHORE_PEND_FAILED )
                INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_READ_FAILED )
            }
            else
            {
                /* copy data */
                pvOSAL_MemCpy( pucData, pxThis->pucCallBackWriteData, pxThis->usCallBackWriteDataSize );
                *pulSize = pxThis->usCallBackWriteDataSize;

                /* re-set values */
                pvOSAL_MemSet( pxThis->pucCallBackWriteData, 0, pxThis->usCallBackWriteDataSize );
                pxThis->usCallBackWriteDataSize = 0;

                INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_SEMAPHORE_PEND )
                INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_READ )
            }
        }
        else
        {
            ulStatus = FW_IF_SMBUS_ERRORS_INVALID_STATE;
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_READ_FAILED )
        }
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }

    return ulStatus;
}

/**
 * @brief Local implementation of FW_IF_ioctrl
 */
static uint32_t ulSmbusBlockIoIoctrl( void *pvFwIf, uint32_t ulOption, void *pvValue )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    /* TODO: Set/get specific IO options */

    return ulStatus;
}

/**
 * @brief Local implementation of FW_IF_bindCallback
 */
static uint32_t ulSmbusBlockIoBindCallback( void *pvFwIf, FW_IF_callback *pxNewFunc )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    FW_IF_CFG *pxThisIf = ( FW_IF_CFG* )pvFwIf;
    CHECK_HDL( pxThisIf );
    CHECK_CFG( pxThisIf );
    CHECK_FIREWALLS( pxThisIf );
    CHECK_DRIVER;

    if( NULL != pxNewFunc )
    {
        /*
         * TODO: Binds in callback provided to the FW_IF.
         * Callback will be invoked when by the driver when event occurs.
         */
        INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_BIND_CALLBACK )
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_BIND_CALLBACK_FAILED )
    }

    return ulStatus;
}

/* smbus callback functions */

/**
 * @brief   Get protocol callback.
 *          This will set the protocol for a specific SMBus command code.
 */
static void vGetProtocol( uint8_t ucCommand, SMBus_Command_Protocol_Type* pxProtocol )
{
    if( NULL != pxProtocol )
    {
        INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_GET_PROTOCOL )

        /* only need to handle block writes for block io implementation */
        if( pxThis->xLocalCfg.pucCommandProtocols[ FW_IF_SMBUS_COMMAND_PROTOCOL_BLOCK_WRITE ] == ucCommand )
        {
            *pxProtocol = SMBUS_PROTOCOL_BLOCK_WRITE;
        }
        else
        {
            *pxProtocol = SMBUS_PROTOCOL_NONE;
        }
    }
    else
    {
    INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_GET_PROTOCOL_FAILED )
    }
}

/*!
 * @brief Read data callback.
 *        This will send data which can be read by the other party.
 */
static void vReadData( uint8_t ucCommand, uint8_t* pucData, uint16_t* pusDataSize )
{
    if( ( NULL != pusDataSize ) &&
        ( NULL != pucData ) )
    {
        INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_READ_DATA )

        /* not required for block io implementation */
        *pusDataSize = 0;
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_READ_DATA_FAILED )
    }
}

/*!
 * @brief Write data callback.
 *        This will copy data that was written by another party.
 */
void vWriteData( uint8_t ucCommand, uint8_t* pucData, uint16_t usDataSize, uint32_t ulTransactionID )
{
    if( ( SMBUS_DATA_SIZE_MAX >= usDataSize ) &&
        ( 0 != usDataSize ) &&
        ( NULL != pucData ) )
    {
        INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_WRITE_DATA )

        /* copy data */
        pxThis->usCallBackWriteDataSize = usDataSize;

        pvOSAL_MemCpy( pxThis->pucCallBackWriteData, pucData, usDataSize );

        /*
         * TODO: Update logic to handle more than 1 SMBus instance.
         *       Currently this logic assumes only 1 handle will be used at a time,
         *       if multiple handles are created we'll need to update the writeData
         *       callback to be able to post an individual semaphore per handle.
         */

        /* release semaphore */
        if( NULL != pxThis->pvWriteCallbackSem )
        {
            if( OSAL_ERRORS_NONE == iOSAL_Semaphore_PostFromISR( pxThis->pvWriteCallbackSem ) )
            {
                INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_SEMAPHORE_POST )
            }
            else
            {
                INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_SEMAPHORE_POST_FAILED )
            }
        }
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_WRITE_DATA_FAILED )
    }
}

/*!
 * @brief Announce result callback.
 *        This gets called when a transaction is complete.
 */
void vAnnounceResult( uint8_t ucCommand, uint32_t ulTransactionID, uint32_t ulStatus )
{
    /* check command for block write */
    if( pxThis->xLocalCfg.pucCommandProtocols[ FW_IF_SMBUS_COMMAND_PROTOCOL_BLOCK_WRITE ] == ucCommand )
    {
        if( OK == ulStatus )
        {
            INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_BLOCK_WRITE_SUCCESS )
        }
        else
        {
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_BLOCK_WRITE_FAILED )
        }
    }
    else
    {
        INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_ANNOUNCE_RESULT_GENERIC )
    }
}

/*!
 * @brief ARP address change callback.
 *        This gets called when the address of the instance changes.
 */
void vAnnounceARP( uint8_t ucNewAddress )
{
    INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_ANNOUNCE_ARP )
}

/*!
 * @brief Announce bus error callback
 *        This gets called when a bus error is passed up from the IP.
 */
void vAnnounceBusError( uint8_t ucError )
{
    INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_ANNOUNCE_BUS_ERROR )
}

/*!
 * @brief Announce bus warning callback
 *        This gets called when a bus warning is passed up from the IP.
 */
void vAnnounceBusWarning( uint8_t ucWarning )
{
    INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_ANNOUNCE_BUS_WARN )

}

/* other local functions */

/**
 * @brief   Setup SMBus interrupts.
 */
static int iSmbusSetupInterrupts( void )
{
    int iStatus = ERROR;

    /* Setup Interrupts */
    if( SMBUS_ERROR != xSMBusInterruptDisableAndClearInterrupts( pxThis->pxSMBusProfile ) )
    {
        if( OSAL_ERRORS_NONE == iOSAL_Interrupt_Setup( FAL_SMBUS_INTERRUPT, ( XInterruptHandler )vSMBusInterruptHandler, pxThis->pxSMBusProfile ) )
        {
            if( OSAL_ERRORS_NONE == iOSAL_Interrupt_Enable( FAL_SMBUS_INTERRUPT ) )
            {
                if( SMBUS_ERROR != xSMBusInterruptEnableInterrupts( pxThis->pxSMBusProfile ) )
                {
                    INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_SETUP_INTERRUPTS )
                    iStatus = OK;
                }
                else
                {
                    INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_SETUP_INTERRUPTS_FAILED )
                }
            }
            else
            {
                INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_SETUP_INTERRUPTS_FAILED )
            }
        }
        else
        {
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_SETUP_INTERRUPTS_FAILED )
        }
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_SETUP_INTERRUPTS_FAILED )
    }

    return iStatus;
}


/*****************************************************************************/
/* public functions                                                          */
/*****************************************************************************/

/**
 * @brief initialisation function for smbus interfaces (generic across all smbus interfaces)
 */
uint32_t ulFW_IF_SMBUS_Init( FW_IF_SMBUS_INIT_CFG *pxInitCfg )
{
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    if( FW_IF_FALSE != pxThis->iInitialised )
    {
        ulStatus = FW_IF_ERRORS_DRIVER_IN_USE;
    }
    else if ( NULL == pxInitCfg )
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
    }
    else
    {
        /*
         * Initilise config data shared between all instances of SMBus.
         */
        pvOSAL_MemCpy( &pxThis->xLocalCfg, pxInitCfg, sizeof( pxThis->xLocalCfg ) );

        /* create semaphore */
        if( OSAL_ERRORS_NONE == iOSAL_Semaphore_Create( &pxThis->pvWriteCallbackSem, 0,
                                                        1, "SMBus Block IO FAL Semaphore" ) )
        {
            /*
             * Initialise the smbus driver based on data supplied in the cfg
             */
            if( SMBUS_ERROR != xInitSMBus( &pxThis->pxSMBusProfile, pxThis->xLocalCfg.ulBaudRate, ( void* )pxThis->xLocalCfg.ulBaseAddr, SMBUS_LOG_LEVEL_DEBUG, NULL ) )
            {
                /* Get SMBus version */
                SMBUS_VERSION_TYPE pxSMBusVersion = { 0 };
                if( SMBUS_ERROR != xSMBusGetVersion( pxThis->pxSMBusProfile, &pxSMBusVersion ) )
                {
                    PLL_LOG( FW_IF_SMBUS_BLOCK_IO_NAME, "SMBus SW version: %d.%d.%d\r\n", pxSMBusVersion.ucSwVerMajor,
                                                                                          pxSMBusVersion.ucSwVerMinor,
                                                                                          pxSMBusVersion.ucSwVerPatch );
                    PLL_LOG( FW_IF_SMBUS_BLOCK_IO_NAME, "SMBus IP version: %d.%d\r\n", pxSMBusVersion.usIpVerMajor,
                                                                                       pxSMBusVersion.usIpVerMinor );
                    if( ERROR != iSmbusSetupInterrupts() )
                    {
                        pxThis->iInitialised = FW_IF_TRUE;
                        INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_INIT_OVERALL_COMPLETE )
                    }
                    else
                    {
                        ulStatus = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
                    }
                }
                else
                {
                    ulStatus = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
                    INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_INIT_OVERALL_FAILED )
                }
            }
            else
            {
                ulStatus = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
                INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_INIT_OVERALL_FAILED )
            }
        }
        else
        {
            ulStatus = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_INIT_OVERALL_FAILED )
        }
    }

    return ulStatus;
}

/**
 * @brief creates an instance of the smbus interface
 */
uint32_t ulFW_IF_SMBUS_Create( FW_IF_CFG *pxFwIf, FW_IF_SMBUS_CFG *pxSmbusCfg )
{
    CHECK_DRIVER;
    uint32_t ulStatus = FW_IF_ERRORS_NONE;

    if( ( NULL != pxFwIf ) && ( NULL != pxSmbusCfg ) )
    {
        if( ( MAX_FW_IF_SMBUS_ROLE > pxSmbusCfg->xRole ) &&
            ( MAX_FW_IF_SMBUS_ARP > pxSmbusCfg->xArpCapability ) &&
            ( MAX_FW_IF_SMBUS_PROTOCOL > pxSmbusCfg->xProtocol ) &&
            ( MAX_FW_IF_SMBUS_STATE > pxSmbusCfg->xState ) &&
            ( MAX_FW_IF_SMBUS_PEC > pxSmbusCfg->xPecCapability ) )
        {
            FW_IF_CFG xLocalIfCfg =
            {
                .upperFirewall  = SMBUS_BLOCK_IO_UPPER_FIREWALL,
                .open           = &ulSmbusBlockIoOpen,
                .close          = &ulSmbusBlockIoClose,
                .write          = &ulSmbusBlockIoWrite,
                .read           = &ulSmbusBlockIoRead,
                .ioctrl         = &ulSmbusBlockIoIoctrl,
                .bindCallback   = &ulSmbusBlockIoBindCallback,
                .cfg            = ( void* )pxSmbusCfg,
                .lowerFirewall  = SMBUS_BLOCK_IO_LOWER_FIREWALL
            };

            pvOSAL_MemCpy( pxFwIf, &xLocalIfCfg, sizeof( FW_IF_CFG ) );

            FW_IF_SMBUS_CFG *pxCfg = ( FW_IF_SMBUS_CFG* )pxFwIf->cfg;
            pxCfg->xState = FW_IF_SMBUS_STATE_CREATED;
            pxCfg->ucInstance = SMBUS_INVALID_INSTANCE;
            INC_STAT_COUNTER( FW_IF_SMBUS_BLOCK_IO_STATS_INSTANCE_CREATE )
        }
        else
        {
            ulStatus = FW_IF_ERRORS_INVALID_CFG;
            INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_INSTANCE_CREATE_FAILED )
        }
    }
    else
    {
        ulStatus = FW_IF_ERRORS_PARAMS;
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_INSTANCE_CREATE_FAILED )
    }

    return ulStatus;
}

/**
 * @brief   Print all the stats gathered by the interface
 */
int iFW_IF_SMBUS_PrintStatistics( void )
{
    int iStatus = FW_IF_ERRORS_NONE;

    if( ( SMBUS_BLOCK_IO_UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( SMBUS_BLOCK_IO_LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FW_IF_TRUE == pxThis->iInitialised ) )
    {
        int i = 0;
        PLL_INF( FW_IF_SMBUS_BLOCK_IO_NAME, "============================================================\n\r" );
        PLL_INF( FW_IF_SMBUS_BLOCK_IO_NAME, "FWIF SMBUS BLOCK IO Statistics:\n\r" );
        for( i = 0; i < FW_IF_SMBUS_BLOCK_IO_STATS_MAX; i++ )
        {
            PRINT_STAT_COUNTER( i );
        }
        PLL_INF( FW_IF_SMBUS_BLOCK_IO_NAME, "------------------------------------------------------------\n\r" );
        PLL_INF( FW_IF_SMBUS_BLOCK_IO_NAME, "FWIF SMBUS BLOCK IO Errors:\n\r" );
        for( i = 0; i < FW_IF_SMBUS_BLOCK_IO_ERRORS_MAX; i++ )
        {
            PRINT_ERROR_COUNTER( i );
        }
        PLL_INF( FW_IF_SMBUS_BLOCK_IO_NAME, "============================================================\n\r" );
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_VALIDATION_FAILED )
        iStatus = FW_IF_ERRORS_DRIVER_NOT_INITIALISED;
    }

    return iStatus;
}

/**
 *  @brief Clears all the stats gathered by the interface
 */
int iFW_IF_SMBUS_ClearStatistics( void )
{
    int iStatus = FW_IF_ERRORS_NONE;

    if( ( SMBUS_BLOCK_IO_UPPER_FIREWALL == pxThis->ulUpperFirewall ) &&
        ( SMBUS_BLOCK_IO_LOWER_FIREWALL == pxThis->ulLowerFirewall ) &&
        ( FW_IF_TRUE == pxThis->iInitialised ) )
    {
        pvOSAL_MemSet( pxThis->pulStatCounters, 0, sizeof( pxThis->pulStatCounters ) );
        pvOSAL_MemSet( pxThis->pulErrorCounters, 0, sizeof( pxThis->pulErrorCounters ) );
        iStatus = OK;
    }
    else
    {
        INC_ERROR_COUNTER( FW_IF_SMBUS_BLOCK_IO_ERRORS_VALIDATION_FAILED )
    }

    return iStatus;
}
