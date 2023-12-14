/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INC_RMI_H_
#define INC_RMI_H_

#include <stddef.h>
#include <stdint.h>

/* TODO make smbus header work on AMC 
#include "smbus.h" */

#define apiID_INDEX                  ( 0 )
#define apiCOMPLETION_CODE_INDEX     ( 1 )

#define apiREQUEST_REPO_TYPE_INDEX   ( 1 )
#define apiREQUEST_PAYLOAD_SIZE_LSB  ( 3 )
#define apiREQUEST_PAYLOAD_SIZE_MSB  ( 2 )
#define apiREQUEST_HEADER_SIZE       ( 4 )

#define apiRESPONSE_REPO_TYPE_INDEX  ( 2 )
#define apiRESPONSE_PAYLOAD_SIZE_LSB ( 4 )
#define apiRESPONSE_PAYLOAD_SIZE_MSB ( 3 )
#define apiRESPONSE_HEADER_SIZE      ( 5 )

#define apiMAX_PAYLOAD_SIZE ( 65536 )

#define apiCMD_GET_SDR_API_ID              ( 0x05 )
#define apiCMD_GET_ALL_SENSOR_DATA_API_ID  ( 0x06 )

#define apiLENGTH_BITMASK    ( 0x3F )
#define apiSENSOR_TYPE_ASCII ( 0x0C00 )

#define apiEEPROM_V3_1_VERSION_SIZE                    ( 3u )
#define apiEEPROM_V3_1_PAD_BYTE_SIZE                   ( 1u )
#define apiEEPROM_V3_1_DATA_LENGTH                     ( 2u )
#define apiEEPROM_V3_1_DATA_CHECKSUM                   ( 2u )
#define apiEEPROM_V3_1_PRODUCT_NAME_SIZE               ( 24u )
#define apiEEPROM_V3_1_BOARD_REV_SIZE                  ( 8u )
#define apiEEPROM_V3_1_BOARD_SERIAL_SIZE               ( 14u )
#define apiEEPROM_V3_1_BOARD_NUMBER_OF_MAC             ( 4u )
#define apiEEPROM_V3_1_BOARD_MAC_SIZE                  ( 6u )
#define apiEEPROM_V3_1_BOARD_ACT_PAS_SIZE              ( 1u )
#define apiEEPROM_V3_1_BOARD_CONFIG_MODE_SIZE          ( 1u )
#define apiEEPROM_V3_1_MFG_DATE_SIZE                   ( 3u )
#define apiEEPROM_V3_1_PART_NUM_SIZE                   ( 24u )
#define apiEEPROM_V3_1_UUID_SIZE                       ( 16u )
#define apiEEPROM_V3_1_PCIE_INFO_SIZE                  ( 8u )
#define apiEEPROM_V3_1_MAX_POWER_MODE_SIZE             ( 1u )
#define apiEEPROM_V3_1_MEM_SIZE_SIZE                   ( 4u )
#define apiEEPROM_V3_1_OEM_ID_SIZE                     ( 4u )
#define apiEEPROM_V3_1_CAPABILITY_SIZE                 ( 2u )
#define apiEEPROM_V3_1_EXTENDED_CAPABILITY_WORD_SIZE   ( 4u )
#define apiEEPROM_V3_1_EXTENDED_PRODUCT_NAME_SIZE      ( 72u )
#define apiEEPROM_V3_1_OEM_PART_NUMBER_SIZE            ( 24u )
#define apiEEPROM_V3_1_OEM_SERIAL_NUMBER_SIZE          ( 24u )
#define apiEEPROM_V3_1_RESERVED_SIZE                   ( 7943u )

#define commonSMBUS_ADDRESS 0x6A
#define commonFRU_ADDRESS 0x50

//Unique Device Identifier ( UDID )
// 8 bit device capabilities field
// Bit[6:7] -> Address type,Bit[1:5] -> Reserved,Bit[0] -> PEC
#define commonUDID_PEC_SUPPORT                         ( 0x01 )
#define commonUDID_PEC_UNSUPPORT                       ( 0x00 )
#define commonUDID_FIXED_ADDR_DEV                      ( 0x00 )
#define commonUDID_DYNAMIC_AND_PERSISTENT_ADDR_DEV     ( 0x40 )
#define commonUDID_DYNAMIC_AND_VOLATILE_ADDR_DEV       ( 0x80 )
#define commonUDID_RANDOM_NUM_DEV                      ( 0xC0 )

#define commonUDID_DEVICE_CAPABILITIES                 ( commonUDID_FIXED_ADDR_DEV ) | ( commonUDID_PEC_SUPPORT )

// Version/Revision Field
// Bit[6:7] -> Reserved, Bit[5:3] -> UDID Version, Bit[2:0] -> Silicon Revision ID
#define commonUDID_VERSION                 ( 0x08 )
#define commonUDID_SILICON_REV_ID          ( 0x00 )

#define commonUDID_VERSION_OR_REVISION      ( commonUDID_VERSION | commonUDID_SILICON_REV_ID )
#define commonUDID_VENDOR_ID                0x10EE
#define commonUDID_DEVICE_ID                0x0000

// Interface
// Bit[15:4] -> Supported Protocols, Bit[3:0] -> SMBUS Version
// Bit[15:8] -> Reserved,Bit7 -> ZONE, Bit6-> IPMI, bit5 -> ASF,bit4 -> OEM
#define commonUDID_INTERFACE           0x0027// ASF + SMBUS version 3.2

#define commonUDID_SUBSYS_VENDOR_ID    0x10EE
#define commonUDID_SUBSYS_DEVICE_ID    0x0000
#define commonUDID_VENDOR_SPECIFIC_ID  0x00202010

#define commonSHIFT_RIGHT( x, y ) ( ( uint8_t ) ( x >> y & 0xFF ) )

/* TODO add smbus to lib
typedef struct smbus_driver_instance
{
    uint8_t instance_number;
    uint8_t address;
    uint8_t UDID[SMBUS_UDID_LENGTH];
    bool pec_required;
    SMBUS_USER_SUPPLIED_ENVIRONMENT_GET_PROTOCOL_TYPE get_protocol_cb;
    SMBUS_USER_SUPPLIED_ENVIRONMENT_GET_DATA_TYPE read_data_cb;
    SMBUS_USER_SUPPLIED_ENVIRONMENT_WRITE_DATA_TYPE write_data_cb;
    SMBUS_USER_SUPPLIED_ENVIRONMENT_COMMAND_COMPLETE result_cb;
    SMBUS_USER_SUPPLIED_ENVIRONMENT_ARP_ADRRESS_CHANGE address_change_cb;
    uint32_t Transaction_ID;
} smbus_driver_instance_t;
*/

typedef enum rmi_sdr {
    eRmiSDR = 0xF0
} rmi_sdr_type_t;

typedef enum alveo_i2c_command
{
    eOoB_SMBUS_FRU_DATA_READ = 0x0,
    eOoB_SMBUS_MAX_DIMM_TEMP = 0x1,
    eOoB_SMBUS_BOARD_TEMP = 0x2,
    eOoB_SMBUS_BOARD_POWER_CONSUMPTION = 0x3,
    eOoB_SMBUS_SC_FW_VER = 0x4,
    eOoB_SMBUS_FPGA_TEMP = 0x5,
    eOoB_SMBUS_MAX_QSFP_TEMP = 0x6,
    eOoB_SMBUS_FPGA_RESET = 0x0F,
    eOoB_SMBUS_FRU_DATA_WRITE = 0x80
} alveo_i2c_command_t;

typedef enum _rmi_error_codes_t {
    eRMI_SUCCESS = 0x0000,
    eRMI_ERROR_GENERIC = 0x0001,
    eRMI_ERROR_INVALID_SIZE = 0x0020,
    eRMI_ERROR_FW_IF = 0x0003,
    eRMI_ERROR_INVALID_ARGUMENT = 0x0004,
    eRMI_ERROR_INVALID_API_ID = 0x0005,
    eRMI_ERROR_MEMORY_ALLOC = 0x0006,
} rmi_error_codes_t;

typedef struct flags_t
{
    uint32_t rmi_is_configured : 1;
    uint32_t rmi_sensors_is_configured : 1;
    uint32_t rmi_board_info_is_ready : 1;
    uint32_t reserved : 29;
} flags_t;

typedef struct rmi_board_info_t
{
    uint8_t eeprom_version[apiEEPROM_V3_1_VERSION_SIZE];
    uint8_t pad_byte[apiEEPROM_V3_1_PAD_BYTE_SIZE];
    uint8_t data_len[apiEEPROM_V3_1_DATA_LENGTH];
    uint8_t data_checksum[apiEEPROM_V3_1_DATA_CHECKSUM];
    uint8_t product_name[apiEEPROM_V3_1_PRODUCT_NAME_SIZE];
    uint8_t board_rev[apiEEPROM_V3_1_BOARD_REV_SIZE];
    uint8_t board_serial[apiEEPROM_V3_1_BOARD_SERIAL_SIZE];
    uint8_t num_mac_ids;
    uint8_t board_mac[apiEEPROM_V3_1_BOARD_MAC_SIZE];
    uint8_t board_act_pas[apiEEPROM_V3_1_BOARD_ACT_PAS_SIZE ];
    uint8_t board_config_mode[apiEEPROM_V3_1_BOARD_CONFIG_MODE_SIZE];
    uint8_t board_mfg_date[apiEEPROM_V3_1_MFG_DATE_SIZE];
    uint8_t board_part_num[apiEEPROM_V3_1_PART_NUM_SIZE];
    uint8_t board_uuid[apiEEPROM_V3_1_UUID_SIZE];
    uint8_t board_pcie_info[apiEEPROM_V3_1_PCIE_INFO_SIZE];
    uint8_t board_max_power_mode[apiEEPROM_V3_1_MAX_POWER_MODE_SIZE];
    uint8_t memory_size[apiEEPROM_V3_1_MEM_SIZE_SIZE];
    uint8_t oem_id[apiEEPROM_V3_1_OEM_ID_SIZE];
    uint8_t dimm_size[apiEEPROM_V3_1_MEM_SIZE_SIZE];
    uint8_t capability_word[apiEEPROM_V3_1_CAPABILITY_SIZE];
    uint8_t extended_capability_word[apiEEPROM_V3_1_EXTENDED_CAPABILITY_WORD_SIZE];
    uint8_t extended_product_name[apiEEPROM_V3_1_EXTENDED_PRODUCT_NAME_SIZE];
    uint8_t oem_part_num[apiEEPROM_V3_1_OEM_PART_NUMBER_SIZE];
    uint8_t oem_serial_num[apiEEPROM_V3_1_OEM_SERIAL_NUMBER_SIZE];
} rmi_board_info_t;

typedef struct __attribute__( ( packed ) ) rmi_sdr_header_t
{
    uint8_t repository_type;
    uint8_t repository_version;
    uint8_t no_of_records;
    uint16_t no_of_bytes;
} rmi_sdr_header_t;

typedef struct rmi_sdr_t
{
    uint8_t sensor_id;
    uint8_t sensor_name_type_length;
    uint8_t *sensor_name;
    uint8_t sensor_value_type_length;
    uint8_t *sensor_value;
    uint8_t sensor_base_unit_type_length;
    uint8_t *sensor_base_unit;
    int8_t  sensor_unit_modifier_byte;
    uint8_t threshold_support_byte;
    uint8_t *lower_fatal_limit;
    uint8_t *lower_critical_limit;
    uint8_t *lower_warning_limit;
    uint8_t *upper_fatal_limit;
    uint8_t *upper_critical_limit;
    uint8_t *upper_warning_limit;
    uint8_t sensor_status;
} rmi_sdr_t;

typedef enum RMI_Sdr_Threshold_Support
{
    eRMI_Upper_Warning_Threshold             = 0x01,
    eRMI_Upper_Critical_Threshold            = ( 0x01 << 1 ),
    eRMI_Upper_Fatal_Threshold               = ( 0x01 << 2 ),
    eRMI_Lower_Warning_Threshold             = ( 0x01 << 3 ),
    eRMI_Lower_Critical_Threshold            = ( 0x01 << 4 ),
    eRMI_Lower_Fatal_Threshold               = ( 0x01 << 5 ),
    eRMI_Sensor_Avg_Val_Support              = ( 0x01 << 6 ),
    eRMI_Sensor_Max_Val_Support              = ( 0x01 << 7 ),
} RMI_Sdr_Threshold_Support_t;

typedef enum rmi_completion_code
{
    eRmi_CC_Not_Available               = 0x00,
    eRmi_CC_Operation_Success           = 0x01,
    eRmi_CC_Operation_Failed            = 0x02,
    eRmi_CC_Flow_Control_Read_Stale     = 0x03,
    eRmi_CC_Flow_Control_Write_Error    = 0x04,
    eRmi_CC_Invalid_Sensor_ID           = 0x05,
} rmi_completion_code_t;

typedef rmi_error_codes_t ( *rmi_request_t )( uint8_t *pucReq, uint16_t *pusReq_size, uint8_t *pucResp, uint16_t *pusResp_size );

typedef void    *( *rmi_malloc_t )( size_t bytes );
typedef void    ( *rmi_free_t )( void *ptr );
typedef void    *( *rmi_memcpy_t )( void *dst, size_t dst_size, const void *src, size_t src_size );
typedef void    *( *rmi_memset_t )( const void *dst, int32_t val, size_t dst_size );
typedef int32_t ( *rmi_memcmp_t )( const void *dst, size_t dst_size, const void *src, size_t src_size );
typedef void    *( *rmi_memmove_t )( void *dst, size_t dst_size, const void *src, size_t src_size );
typedef void    *( *rmi_strncpy_t )( void *dst, size_t dst_size, const void *src, size_t src_size );
typedef int32_t ( *rmi_strncmp_t )( const void *dst, size_t dst_size, const void *src, size_t src_size );

typedef struct rmi_config_t
{
    rmi_malloc_t    rmi_malloc_fptr;
    rmi_request_t   rmi_request_fptr;
    rmi_free_t      rmi_free_fptr;
    rmi_memcpy_t    rmi_memcpy_fptr;
    rmi_memset_t    rmi_memset_fptr;
    rmi_memcmp_t    rmi_memcmp_fptr;
    rmi_memmove_t   rmi_memmove_fptr;
    rmi_strncpy_t   rmi_strncpy_fptr;
    rmi_strncmp_t   rmi_strncmp_fptr;
    //TODO better delay/timer ( non-blocking ) function
    void ( *task_delay )( uint32_t a );
} rmi_config_t;

/* Code from rmi_sensors.h as it was included in this header file */
typedef struct _sensors_ds_t
{
    uint8_t  id[1];             // sensor ID - 1 byte
    uint8_t  size[1];           // sensor size - 1 byte
    uint8_t  value[4];          // sensor value; parse only 'size' num of bytes
    uint8_t  status[1];         // sensor reading status
    uint8_t  tag[2];            // sensor type tag ( sensor_tag_t )
    uint8_t  rsvd[3];           // reserved bytes to make the struct word aligned
} sensors_ds_t;

int32_t lRmi_Init( rmi_config_t _x_rmi_config );
void vRmi_Task_Func( void );
int32_t lRmi_Configure_Sensors( sensors_ds_t* _px_sensors, uint32_t _ul_num_of_sensors );
int32_t lRmi_Write_Board_Info( rmi_board_info_t* _px_board_info, uint32_t _ul_size );

#endif /* INC_RMI_RMI_API_H_ */
