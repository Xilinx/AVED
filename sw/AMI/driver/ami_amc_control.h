// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_amc_control.h - This file contains AMC control defintions.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_AMC_CONTROL_H
#define AMI_AMC_CONTROL_H

#include <linux/types.h>
#include <linux/pci.h>

#include "ami.h"
#include "ami_pcie.h"
#include "amc_proxy.h"


/******************************************************************************************/
/* Defines                                                                                */
/******************************************************************************************/

#define AMI_ERR(amc_ctrl_ctxt, fmt, arg...)      DEV_ERR(amc_ctrl_ctxt->pcie_dev, fmt, ##arg)
#define AMI_ERR_ONCE(amc_ctrl_ctxt, fmt, arg...) DEV_ERR_ONCE(amc_ctrl_ctxt->pcie_dev, fmt, ##arg)
#define AMI_INFO(amc_ctrl_ctxt, fmt, arg...)     DEV_INFO(amc_ctrl_ctxt->pcie_dev, fmt, ##arg)
#define AMI_WARN(amc_ctrl_ctxt, fmt, arg...)     DEV_WARN(amc_ctrl_ctxt->pcie_dev, fmt, ##arg)
#define AMI_VDBG(amc_ctrl_ctxt, fmt, arg...)     DEV_VDBG(amc_ctrl_ctxt->pcie_dev, fmt, ##arg)

#define AMC_LOG_PAGE_SIZE	                 (1024 * 1024)
#define AMC_LOG_PAGE_NUM	                 (1)
#define AMC_LOG_ADDR_OFF	                 (0)
#define AMC_DATA_ADDR_OFF	                 (AMC_LOG_PAGE_SIZE * AMC_LOG_PAGE_NUM)
 
#define SENSOR_RSP_LEN		                 (4096)

/*
 * Response format:
 * Length byte : Description
 * 1 : completion code
 * 1 : SDR Record type
 * 1 : Length = Size of (sensor value)
 * Data Payload = [value, max, average, status]
 * Note : Sensor status is always one byte
 * While parsing, it saves the sensor's value, max value, average value and status information
 */

#define SDR_NULL_BYTE	                        (0x00)
#define SDR_LENGTH_MASK	                        (0x3F)
#define SDR_TYPE_MASK	                        (0x03)
#define SDR_TYPE_POS	                        (6)
#define SDR_EOR		                        (0x454E44)  /* END */

#define SDR_THRESHOLD_SUPPORT_BITS              (8)

#define THRESHOLD_UPPER_WARNING_MASK	        (0x01 << 0)
#define THRESHOLD_UPPER_CRITICAL_MASK	        (0x01 << 1)
#define THRESHOLD_UPPER_FATAL_MASK	        (0x01 << 2)
#define THRESHOLD_LOWER_WARNING_MASK	        (0x01 << 3)
#define THRESHOLD_LOWER_CRITICAL_MASK	        (0x01 << 4)
#define THRESHOLD_LOWER_FATAL_MASK	        (0x01 << 5)
#define THRESHOLD_SENSOR_AVG_MASK	        (0x01 << 6)
#define THRESHOLD_SENSOR_MAX_MASK	        (0x01 << 7)

#define SENSOR_IDS_MAX			        (256)

#define SDR_VALUE_MAX_LEN                       (64)  /* 63 (6 bits) + NULL byte */
#define SDR_THRESHOLD_MAX_LEN                   (4)   /* max possible is uint32 */
#define UNKNOWN_SENSOR_ID                       (-EINVAL)

#define NUM_SENSOR_REPOS                        (7)  /* temp, voltage, current, power, total power, board info, fpt */

/* note: configurable POWER repos: SDR_TYPE_POWER / SDR_TYPE_TOTAL_POWER */
#define SDR_POWER_TYPE				( SDR_TYPE_TOTAL_POWER )


/******************************************************************************************/
/* Enums                                                                                  */
/******************************************************************************************/

/**
 * enum gcq_submit_cmd_flags - QCQ command-specific flags.
 * @GCQ_CMD_FLAG_NONE: No flags.
 * @GCQ_CMD_FLAG_REPO_TYPE_BD_INFO: Command relates to the board info SDR.
 * @GCQ_CMD_FLAG_REPO_TYPE_TEMP: Command relates to the temperature SDR.
 * @GCQ_CMD_FLAG_REPO_TYPE_VOLTAGE: Command relates to the voltage SDR.
 * @GCQ_CMD_FLAG_REPO_TYPE_CURRENT: Command relates to the current SDR.
 * @GCQ_CMD_FLAG_REPO_TYPE_POWER: Command relates to the power SDR.
 * @GCQ_CMD_FLAG_REPO_TYPE_TOTAL_POWER: Command relates to the total power SDR.
 * @GCQ_CMD_FLAG_REPO_TYPE_FPT: Command relates to the flash parition table SDR.
 * 
 */
enum gcq_submit_cmd_flags {
	GCQ_CMD_FLAG_NONE              	   = 0x00,
	GCQ_CMD_FLAG_REPO_TYPE_BD_INFO 	   = 0x01 << 0,
	GCQ_CMD_FLAG_REPO_TYPE_TEMP    	   = 0x01 << 1,
	GCQ_CMD_FLAG_REPO_TYPE_VOLTAGE 	   = 0x01 << 2,
	GCQ_CMD_FLAG_REPO_TYPE_CURRENT 	   = 0x01 << 3,
	GCQ_CMD_FLAG_REPO_TYPE_POWER   	   = 0x01 << 4,
	GCQ_CMD_FLAG_REPO_TYPE_TOTAL_POWER = 0x01 << 5,
	GCQ_CMD_FLAG_REPO_TYPE_FPT         = 0x01 << 6,
};

/**
 * enum gcq_submit_cmd_req - Type of GCQ command to submit
 * @GCQ_SUBMIT_CMD_RSVD: Reserved
 * @CQ_SUBMIT_CMD_GET_GCQ_VERSION: Get GCQ vesion
 * @GCQ_SUBMIT_CMD_GET_SDR_SIZE: Get SDR size
 * @GCQ_SUBMIT_CMD_GET_SDR: Get SDR
 * @GCQ_SUBMIT_CMD_DOWNLOAD_PDI: Download PDI
 * @GCQ_SUBMIT_CMD_DEVICE_BOOT: Select device boot partition
 * @GCQ_SUBMIT_CMD_COPY_PARTITION: Copy partition to another
 * @GCQ_SUBMIT_CMD_GET_INLET_TEMP_SENSOR: Get inlet temperature data
 * @GCQ_SUBMIT_CMD_GET_OUTLET_TEMP_SENSOR: Get outlet temperature data
 * @GCQ_SUBMIT_CMD_GET_BOARD_TEMP_SENSOR: Get board temp data
 * @GCQ_SUBMIT_CMD_GET_FPGA_TEMP_SENSOR: Get FPGA temp data
 * @GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR: Get all temp data
 * @GCQ_SUBMIT_CMD_GET_VCCINT_VOLTAGE: Get VCCINT voltage
 * @GCQ_SUBMIT_CMD_GET_VCC1V2_VOLTAGE: Get VCC1V2 voltage
 * @GCQ_SUBMIT_CMD_GET_12V_PEX_VOLTAGE: Get 12v_pex voltage
 * @GCQ_SUBMIT_CMD_GET_12V_AUX0_VOLTAGE: Get 12v_aux0 voltage
 * @GCQ_SUBMIT_CMD_GET_VCCINT_CURRENT: Get VCCINT current
 * @GCQ_SUBMIT_CMD_GET_VCC1V2_CURRENT: Get VCC1V2 current
 * @GCQ_SUBMIT_CMD_GET_12V_PEX_CURRENT: Get 12v_pex current
 * @GCQ_SUBMIT_CMD_GET_TOTAL_POWER: Get total power
 * @GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR: Get all voltage data
 * @GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR: Get all current data
 * @GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR: Get all power data
 * @GCQ_SUBMIT_CMD_GET_HEARTBEAT: Heartbeat response
 * @GCQ_SUBMIT_CMD_EEPROM_READ_WRITE: Read/write EEPROM
 * @GCQ_SUBMIT_CMD_MODULE_READ_WRITE: Read/write a QSFP module
 */
enum gcq_submit_cmd_req {
	GCQ_SUBMIT_CMD_RSVD                         = 0x00,
	GCQ_SUBMIT_CMD_GET_GCQ_VERSION              = 0x01,
	GCQ_SUBMIT_CMD_GET_SDR_SIZE                 = 0x02,
	GCQ_SUBMIT_CMD_GET_SDR                      = 0x03,
	GCQ_SUBMIT_CMD_DOWNLOAD_PDI                 = 0x04,
	GCQ_SUBMIT_CMD_DEVICE_BOOT                  = 0x05,
	GCQ_SUBMIT_CMD_COPY_PARTITION               = 0x06,
	GCQ_SUBMIT_CMD_GET_INLET_TEMP_SENSOR        = 0x10,
	GCQ_SUBMIT_CMD_GET_OUTLET_TEMP_SENSOR       = 0x11,
	GCQ_SUBMIT_CMD_GET_BOARD_TEMP_SENSOR        = 0x12,
	GCQ_SUBMIT_CMD_GET_FPGA_TEMP_SENSOR         = 0x13,
	GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR     = 0x20,
	GCQ_SUBMIT_CMD_GET_VCCINT_VOLTAGE           = 0x30,
	GCQ_SUBMIT_CMD_GET_VCC1V2_VOLTAGE           = 0x31,
	GCQ_SUBMIT_CMD_GET_12V_PEX_VOLTAGE          = 0x32,
	GCQ_SUBMIT_CMD_GET_12V_AUX0_VOLTAGE         = 0x33,
	GCQ_SUBMIT_CMD_GET_VCCINT_CURRENT           = 0x40,
	GCQ_SUBMIT_CMD_GET_VCC1V2_CURRENT           = 0x41,
	GCQ_SUBMIT_CMD_GET_12V_PEX_CURRENT          = 0x42,
	GCQ_SUBMIT_CMD_GET_TOTAL_POWER              = 0x50,
	GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR  = 0x60,
	GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR  = 0x61,
	GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR    = 0x62,
	GCQ_SUBMIT_CMD_GET_HEARTBEAT                = 0x70,
        GCQ_SUBMIT_CMD_EEPROM_READ_WRITE            = 0x80,
        GCQ_SUBMIT_CMD_MODULE_READ_WRITE            = 0x90,
};

/**
 * enum gcq_sdr_repo_type - ASDM repo type
 * @SDR_TYPE_GET_SIZE: SDR size
 * @SDR_TYPE_BDINFO: Board info
 * @SDR_TYPE_TEMP: Temperature
 * @SDR_TYPE_VOLTAGE: Voltage
 * @SDR_TYPE_CURRENT: Current
 * @SDR_TYPE_POWER: Power
 * @SDR_TYPE_QSFP: QSFP
 * @SDR_TYPE_VPD_PCIE: VPD PCIE
 * @SDR_TYPE_IPMIFRU: IPMI FRU
 * @SDR_TYPE_CSDR_LOGDATA: CSDR log data
 * @SDR_TYPE_VMC_LOGDATA: VMC log data 
 * @SDR_TYPE_FPT: File partition table
 * @SDR_TYPE_MAX: Max enum value
 */
enum gcq_sdr_repo_type {
	SDR_TYPE_GET_SIZE     = 0x00,
	SDR_TYPE_BDINFO       = 0xC0,
	SDR_TYPE_TEMP         = 0xC1,
	SDR_TYPE_VOLTAGE      = 0xC2,
	SDR_TYPE_CURRENT      = 0xC3,
	SDR_TYPE_POWER        = 0xC4,
	SDR_TYPE_TOTAL_POWER  = 0xC6,
	SDR_TYPE_QSFP         = 0xC5,
	SDR_TYPE_VPD_PCIE     = 0xD0,
	SDR_TYPE_IPMIFRU      = 0xD1,
	SDR_TYPE_CSDR_LOGDATA = 0xE0,
	SDR_TYPE_VMC_LOGDATA  = 0xE1,
	SDR_TYPE_FPT  	      = 0xF0,
	SDR_TYPE_MAX          = 13, //increment if new entry added in this enum
};

/**
 * enum gcq_sdr_completion_code - ASDM return code
 * @SDR_CODE_NOT_AVAILABLE: SDR N/A
 * @SDR_CODE_OP_SUCCESS: OK
 * @SDR_CODE_OP_FAILED: Error
 * @SDR_CODE_FLOW_CNTRL_RD_STALE: Read stale
 * @SDR_CODE_FLOW_CNTRL_WR_ERR: Write error
 * @SDR_CODE_INVALID_SENSOR_ID: Invalid sensor ID
 */
enum gcq_sdr_completion_code {
	SDR_CODE_NOT_AVAILABLE          = 0x00,
	SDR_CODE_OP_SUCCESS             = 0x01,
	SDR_CODE_OP_FAILED              = 0x02,
	SDR_CODE_FLOW_CNTRL_RD_STALE    = 0x03,
	SDR_CODE_FLOW_CNTRL_WR_ERR      = 0x04,
	SDR_CODE_INVALID_SENSOR_ID      = 0x05,
};


/**
 * enum sensor_status - ASDM sensor status
 * SENSOR_NOT_PRESENT: Not present
 * SENSOR_PRESENT_AND_VALID: Present and valid
 * DATA_NOT_AVAILABLE: No data
 * SENSOR_STATUS_NOT_AVAILABLE: Status not available
 */
enum sensor_status {
	SENSOR_NOT_PRESENT          = 0x00,
	SENSOR_PRESENT_AND_VALID    = 0x01,
	DATA_NOT_AVAILABLE          = 0x02,
	SENSOR_STATUS_NOT_AVAILABLE = 0x7F,
};

/**
 * enum sdr_value_type - Represents the type of an SDR record value.
 * @SDR_VALUE_TYPE_BINARY: The value is a number.
 * @SDR_VALUE_TYPE_ASCII: The value is a string.
 */
enum sdr_value_type {
	SDR_VALUE_TYPE_BINARY = 0x00,
	/* 0x01-0x02 RESERVED */
	SDR_VALUE_TYPE_ASCII  = 0x03,
};

/**
 * enum amc_cl_sensor_type - Represents the CL Sensor Type.
 * @AMC_CL_SENSOR_GET_SIZE: Get Size Type   
 * @AMC_CL_SENSOR_BOARD_INFO: Board Info Type 
 * @AMC_CL_SENSOR_TEMPERATURE: Temperature Type
 * @AMC_CL_SENSOR_VOLTAGE: Voltage Type    
 * @AMC_CL_SENSOR_CURRENT: Current Type    
 * @AMC_CL_SENSOR_POWER: Power Type    
 * @AMC_CL_SENSOR_TOTAL_POWER: Total power Type
 * @AMC_CL_SENSOR_QSFP : QSFP Type      
 * @AMC_CL_SENSOR_ALL: All Sensors
 */
enum amc_cl_sensor_type {
	AMC_CL_SENSOR_GET_SIZE       = 0x00,
	AMC_CL_SENSOR_BOARD_INFO     = 0xC0,
	AMC_CL_SENSOR_TEMPERATURE    = 0xC1,
	AMC_CL_SENSOR_VOLTAGE        = 0xC2,
	AMC_CL_SENSOR_CURRENT        = 0xC3,
	AMC_CL_SENSOR_POWER          = 0xC4,
	AMC_CL_SENSOR_QSFP           = 0xC5,
	AMC_CL_SENSOR_TOTAL_POWER    = 0xC6,
	AMC_CL_SENSOR_ALL            = 0xFF,
};

/**
 * enum asdm_temp_sensor_id - Type of temperature sensor
 * @ALL_SENSOR_ID: All sensors
 * @INLET_TEMP_SENSOR_ID: Inlet temp
 * @OUTLET_TEMP_SENSOR_ID: Outlet temp
 * @BOARD_TEMP_SENSOR_ID: Board temp
 * @FPGA_TEMP_SENSOR_ID: FPGA temp
 * @QSFP_TEMP0_SENSOR_ID: QSFP temp 0
 * @QSFP_TEMP1_SENSOR_ID: QSFP temp 1
 */
enum asdm_temp_sensor_id {
	ALL_SENSOR_ID = 0,
	INLET_TEMP_SENSOR_ID,
	OUTLET_TEMP_SENSOR_ID,
	BOARD_TEMP_SENSOR_ID,
	FPGA_TEMP_SENSOR_ID,
	QSFP_TEMP0_SENSOR_ID,
	QSFP_TEMP1_SENSOR_ID,
};

/**
 * enum asdm_volt_sensor_id - Type of voltage sensor
 * @VCC12V_PEX_VOLT_SENSOR_ID: 12v PEX
 * @VCC1V2_VOLT_SENSOR_ID: 1v2
 * @VCCINT_VOLT_SENSOR_ID: vccint
 * @VCC12V_AUX0_VOLT_SENSOR_ID: 12v aux0
 * @VCC12V_AUX1_VOLT_SENSOR_ID: 12v aux1
 */
enum asdm_volt_sensor_id {
	VCC12V_PEX_VOLT_SENSOR_ID   = 0x01,
	VCC1V2_VOLT_SENSOR_ID       = 0x02,
	VCCINT_VOLT_SENSOR_ID       = 0x03,
	VCC12V_AUX0_VOLT_SENSOR_ID  = 0x04,
	VCC12V_AUX1_VOLT_SENSOR_ID  = 0x05,
};

/**
 * enum asdm_cur_sensor_id - Type of current sensor
 * @VCCINT_CURRENT_SENSOR_ID: vccint
 * @VCC1V2_CURRENT_SENSOR_ID: 1v2
 * @VCCPEX_CURRENT_SENSOR_ID: PEX
 */
enum asdm_cur_sensor_id {
	VCCINT_CURRENT_SENSOR_ID = 0x01,
	VCC1V2_CURRENT_SENSOR_ID = 0x02,
	VCCPEX_CURRENT_SENSOR_ID = 0x03,
};

/**
 * enum asdm_power_sensor_id - Type of power sensor
 * @TOTAL_POWER_SENSOR_ID: total power
 */
enum asdm_power_sensor_id {
	TOTAL_POWER_SENSOR_ID = 0x01,
};

/**
 * enum sdr_buf_index - Fields in the GET_ALL_SENSOR_DATA response
 * @SDR_PARSE_BUF_INST_INDEX: Instantaneous value
 * @SDR_PARSE_BUF_MAX_INDEX: Max value
 * @SDR_PARSE_BUF_AVG_INDEX: Average value
 * @SDR_PARSE_BUF_STATUS_INDEX: Status
 * @SDR_PARSE_BUF_LEN: Buffer length
 */
enum sdr_buf_index {
	SDR_PARSE_BUF_INST_INDEX = 0,
	SDR_PARSE_BUF_MAX_INDEX,
	SDR_PARSE_BUF_AVG_INDEX,
	SDR_PARSE_BUF_STATUS_INDEX,
	SDR_PARSE_BUF_LEN
};

/**
 * enum amc_cmd_id - the command id
 * @AMC_CMD_ID_UNKNOW: default unknown command
 * @AMC_CMD_ID_SENSOR: sensor command
 * @AMC_CMD_ID_IDENTIFY: identity command
 * @AMC_CMD_ID_DOWNLOAD_PDI: download pdi command
 * @AMC_CMD_ID_DEVICE_BOOT: boot partition select command
 * @AMC_CMD_ID_COPY_PARTITION: copy partition command
 * @AMC_CMD_ID_HEARTBEAT: heartbeat command
 * @AMC_CMD_ID_EEPROM_READ_WRITE: eeprom read/write command
 * @AMC_CMD_ID_MODULE_READ_WRITE: module read/write command
 */
enum amc_cmd_id {
	AMC_CMD_ID_UNKNOWN = -EINVAL,
	AMC_CMD_ID_SENSOR = 0x0,
	AMC_CMD_ID_IDENTIFY,
	AMC_CMD_ID_DOWNLOAD_PDI,
	AMC_CMD_ID_DEVICE_BOOT,
	AMC_CMD_ID_COPY_PARTITION,
	AMC_CMD_ID_HEARTBEAT,
	AMC_CMD_ID_EEPROM_READ_WRITE,
	AMC_CMD_ID_MODULE_READ_WRITE,

	AMC_CMD_ID_MAX
};

/**
 * enum amc_event_id - the event id
 * @AMC_EVENT_ID_HEARTBEAT_EXPIRED: heartbeat expired
 * @AMC_EVENT_ID_HEARTBEAT_VALIDATION: heartbeat validated
 * @AMC_EVENT_ID_HEARTBEAT_FATAL: heartbeat failure count has reached threshold
 */
enum amc_event_id {
        AMC_EVENT_ID_HEARTBEAT_EXPIRED = 0,
        AMC_EVENT_ID_HEARTBEAT_VALIDATION,
	AMC_EVENT_ID_HEARTBEAT_FATAL,
        AMC_EVENT_ID_MAX,
};


/*****************************************************************************/
/* Typedefs                                                                  */
/*****************************************************************************/

/**
 * typedef amc_event_callback - the function pointer definition for the event callback
 * @id: the event id
 * @data: private callback data, optional
 */
typedef void (*amc_event_callback)(enum amc_event_id id, void *data);


/******************************************************************************************/
/* Structs                                                                                */
/******************************************************************************************/

/**
 * struct sdr_record - A single SDR sensor record.
 * @id: Sensor ID (index)
 * @name_type: Type of name field (ascii or binary)
 * @name_len: Length of name field
 * @name: Name
 * @value_type: Type of value field (ascii or binary)
 * @value_len: Length of value field
 * @value: Value
 * @unit_type: Type of unit field (ascii or binary)
 * @unit_len: Length of unit field
 * @unit: Unit
 * @unit_mod: Unit modifier
 * @threshold_support: Thresholds support byte
 * @lower_fatal_limit: Lower fatal limit
 * @lower_crit_limit: Lower critical limit
 * @lower_warn_limit: Lower warning limit
 * @upper_warn_limit: Upper warning limit
 * @upper_crit_limit: Upper critical limit
 * @upper_fatal_limit: Upper fatal limit
 * @sensor_status: Sensor status
 * @min: Min value
 * @max: Max value
 * @avg: Average value
 */
struct sdr_record {
	uint8_t   id;
	uint8_t   name_type;
	uint8_t   name_len;
	uint8_t   name[SDR_VALUE_MAX_LEN];
	uint8_t   value_type;
	uint8_t   value_len;
	uint8_t   value[SDR_VALUE_MAX_LEN];
	uint8_t   unit_type;
	uint8_t   unit_len;
	uint8_t   unit[SDR_VALUE_MAX_LEN];
	char      unit_mod;
	uint8_t   threshold_support;
	uint8_t   lower_fatal_limit[SDR_THRESHOLD_MAX_LEN];
	uint8_t   lower_crit_limit[SDR_THRESHOLD_MAX_LEN];
	uint8_t   lower_warn_limit[SDR_THRESHOLD_MAX_LEN];
	uint8_t   upper_warn_limit[SDR_THRESHOLD_MAX_LEN];
	uint8_t   upper_crit_limit[SDR_THRESHOLD_MAX_LEN];
	uint8_t   upper_fatal_limit[SDR_THRESHOLD_MAX_LEN];
	uint8_t   sensor_status;
	uint8_t   min[SDR_THRESHOLD_MAX_LEN];
	uint8_t   max[SDR_THRESHOLD_MAX_LEN];
	uint8_t   avg[SDR_THRESHOLD_MAX_LEN];
};

/**
 * struct fpt_header - The FPT header.
 * @version: FPT version
 * @header_size: Header size
 * @entry_size: Entry size
 * @num_entries: Number of entries
 */
struct fpt_header {
	uint8_t version;
	uint8_t header_size;
	uint8_t entry_size;
	uint8_t num_entries;
};

/**
 * struct fpt_partition - A single flash partition.
 * @type: Partition type
 * @base_addr: Partition base address
 * @partition_size: Partition size
 */
struct fpt_partition {
	uint32_t type;
	uint32_t base_addr;
	uint32_t partition_size;
};

/**
 * struct fpt_record - The flash partition table (FPT) record containing header and partitions.
 * @hdr: FPT header
 * @partition: List of partitions
 */
struct fpt_record {
	struct fpt_header hdr;
	struct fpt_partition *partition;
};

/**
 * struct field_info - a field in the board info
 *
 * @type: the type, either ASCII or byte array
 * @len: the length of the field
 * @bytes: the associated bytes or characters
 */
struct field_info {
	uint8_t   type;
	uint8_t   len;
	uint8_t   bytes[SDR_VALUE_MAX_LEN];
};

/**
 * struct bd_info_record - the board info record
 *
 * @eeprom_version: The eeprom version.
 * @product_name: The product name.
 * @board_rev: The board revision.
 * @board_serial: The board serial number.
 * @mac_addr_count: Number of MAC IDs.
 * @first_mac_addr: Primary MAC ID.
 * @active_state: ‘A’ = Active, ‘P’ = Passive.
 * @config_mode: 07: Master SPI x4 (QSPIx4), 08: OSPI.
 * @manufacturing_date: Manufacturing Date.
 * @part_number: Board part number.
 * @uuid: Used to uniquely ID the product.
 * @pcie_id: Vendor ID, Device ID, SubVendor ID, SubDevice ID
 * @power_mode: Max power mode.
 * @memory_size: Coveys the max memory (in GB).
 * @oem_id: OEM ID.
 * @capability: Capability word.
 * @mfg_part_number: Manufacture part number.
 */
struct bd_info_record {
        struct field_info eeprom_version;       /* string */
        struct field_info product_name;         /* string */
        struct field_info board_rev;            /* string */
        struct field_info board_serial;         /* string */
        struct field_info mac_addr_count;       /* byte */
        struct field_info first_mac_addr;       /* byte array */
        struct field_info active_state;         /* char */
        struct field_info config_mode;          /* byte */
        struct field_info manufacturing_date;   /* byte array */
        struct field_info part_number;          /* string */
        struct field_info uuid;                 /* byte array */
        struct field_info pcie_id;              /* byte array */
        struct field_info power_mode;           /* byte */
        struct field_info memory_size;          /* string */
        struct field_info oem_id;               /* byte array */
        struct field_info capability;           /* byte array */
	struct field_info mfg_part_number;      /* string */
};

/**
 * struct sdr_repo - A collection of sensor records for a particular repo type.
 * @repo_type: ASDM repo type.
 * @repo_ver: ASDM repo version.
 * @num_records: Number of records in this repo.
 * @size: Total repo size in multiples of 8
 * @last_update: Last update timestamp
 * @records: List of SDR records - only for sensor repo types
 * @fpt: FPT data - only for FPT type
 * @bd_info: Board info data - only for bdinfo type
 */
struct sdr_repo {
	uint8_t              repo_type;
	uint8_t              repo_ver;
	uint8_t              num_records;
	uint8_t              size;
	unsigned long        last_update;
	union {
                struct sdr_record       *records;
                struct fpt_record       fpt;
                struct bd_info_record   *bd_info;
	};
};

/**
 * struct amc_ring_buffer - Stores ring buffer information - part of the partition table.
 * @ring_buffer_off:    the offset of gcq ring buffer initialized by gcq server
 * @ring_buffer_len:    the length of gcq ring buffer initialized by gcq server
 */
struct amc_ring_buffer {
        uint32_t ring_buffer_off;
        uint32_t ring_buffer_len;
};

/**
 * struct amc_status - Stores AMC status information - part of the partition table.
 * @amc_status_off:     the offset of amc device status
 * @amc_status_len:     the length of amc device status
 */
struct amc_status {
        uint32_t amc_status_off;
        uint32_t amc_status_len;
};

/**
 * struct amc_log_msg - Stores AMC logs and information - part of the partition table.
 * @log_msg_index:      the current index of ring buffer log
 * @log_msg_buf_off:    the offset of dbg log
 * @log_msg_buf_len:    the length of dbg log
 */
struct amc_log_msg {
        uint32_t log_msg_index;
        uint32_t log_msg_buf_off;
        uint32_t log_msg_buf_len;
};

/**
 * struct amc_data - Stores AMC status information - part of the partition table.
 * @amc_data_start:     the offset of data buffer started
 * @amc_data_end:       the offset of data buffer ended
 */
struct amc_data {
        uint32_t amc_data_start;
        uint32_t amc_data_end;
};

/**
 * struct amc_shared_mem - GCQ memory partition table, should be positioned at shared memory offset 0,
 *     and initialized by AMC software on RPU device.
 * We use the memory partition table for sharing info between host and RPU.
 * Including:
 * @amc_magic_no:       magic number.
 * @amc_ring_buffer:    ring buffer struct.
 * @amc_status:         amc status struct.
 * @amc_log_msg:        amc log struct.
 * @amc_data:           amc data struct.
 */
struct amc_shared_mem {
        uint32_t                amc_magic_no;
        struct amc_ring_buffer  ring_buffer;
        struct amc_status       status;
        struct amc_log_msg      log_msg;
        struct amc_data         data;
};

/**
 * struct amc_version - stores AMC version information
 * @ver_major: Major version number.
 * @ver_minor: Minor version number.
 * @ver_patch: Patch version number.
 * @local_changes: 0 for no change, 1 for changes.
 * @dev_commits: Number of dev commits.
 */
struct amc_version {
	uint8_t ver_major;
	uint8_t ver_minor;
	uint8_t ver_patch;
	uint8_t local_changes;
	uint16_t dev_commits;
};

/**
 * struct amc_control_ctxt - context for the AMC.
 * @pcie_dev: the physical function
 * @gcq_payload_base_virt_addr: payload virtual base address
 * @gcq_base_virt_addr: the virtual base address
 * @amc_shared_mem: the shared memory base address
 * @gcq_ring_buf_base_virt_addr: the ring buffer virtual address
 * @fw_if_cfg: fal configuration
 * @fw_if_gcq_consumer: handle to the GCQ consumer
 * @lock: lock to protect cid creation
 * @gcq_cmd_lock: protect concurrent gcq commands
 * @gcq_halted: block/allow request messages
 * @gcq_log_page_sema: log page access semaphore
 * @gcq_data_sema: data access semaphore
 * @version: AMC version
 * @heartbeat_thread: thread that generates heartbest requests
 * @heartbeat_thread_created: flag used to determine if thread has been created
 * @event_cb: callback to be invoked when event occurs
 * @event_cb_data: private data to be passed into the event callback
 * @logging_thread: thead that handles AMC logs
 * @logging_thread_created: flag used to determine if thread has been created
 * @last_printed_msg_index: index of the last printed log message
 * @compat_mode: flag used to determine if this AMC instance is running in
 *   compatibility mode - this provides minimum functionality when an AMC
 *   version is deemed to be incompatible with the current AMI version
 */
 struct amc_control_ctxt{
	struct pci_dev          *pcie_dev;
	void __iomem            *gcq_payload_base_virt_addr;
	void __iomem            *gcq_base_virt_addr;
	struct amc_shared_mem   amc_shared_mem;
	void __iomem            *gcq_ring_buf_base_virt_addr;
	FW_IF_CFG               fw_if_cfg;
	FW_IF_GCQ_CFG           fw_if_gcq_consumer;
	struct mutex            lock;
        struct mutex            gcq_cmd_lock;
	bool                    gcq_halted;
	struct semaphore 	gcq_log_page_sema;
	struct semaphore 	gcq_data_sema;
	struct amc_version      version;
        struct task_struct      *heartbeat_thread;
        bool                    heartbeat_thread_created;
        amc_event_callback      event_cb;
	void                    *event_cb_data;
        struct task_struct      *logging_thread;
        bool                    logging_thread_created;
        int                     last_printed_msg_index;
        bool                    compat_mode;
};


/******************************************************************************************/
/* Public Functions                                                                       */
/******************************************************************************************/

/**
 * get_sid() - Get the SID (repo type).
 * @cmd_req: The command to be submitted.
 * @flags: The command specific flags.
 * 
 * Return: the SID.
 */
int get_sid(int cmd_req, int flags);

/**
 * get_rid() - Get the repo ID.
 * @repo_type: The repo type.
 * 
 * Return: the RID.
 */
int get_rid(int repo_type);

/**
 * submit_gcq_command() - Top level function to submit a request and wait on response.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @cmd_req: The CMD code to submit; used to populate payload fields.
 * @flags: The command specific flags.
 * @data_buf: Data buffer to either store payload data or response data.
 * @data_size: Data buffer size.
 * 
 * Return: 0 or negative error code.
 */
int submit_gcq_command(struct amc_control_ctxt *amc_ctrl_ctxt, enum gcq_submit_cmd_req cmd_req,
	uint32_t flags, uint8_t *data_buf, uint32_t data_size);

/**
 * stop_gcq_services() - stop the service running.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Stop service will be called from driver remove if timeout cmd is found
 *
 * Return: None.
 */
void stop_gcq_services(struct amc_control_ctxt *amc_ctrl_ctxt);

/**
 * setup_amc() - Init setup & configuration for the AMC.
 * @dev: the pci device.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @ep_gcq: The rpu endpoint info.
 * @ep_gcq_payload: The mgmt endpoint info.
 * @event_cb: Callback to be invoked when event occurs.
 * @event_cb_data: Private data to be passed into the event callback.
 * 
 * Return: 0 or negative error code.
 */
int setup_amc(struct pci_dev *dev, struct amc_control_ctxt **amc_ctrl_ctxt, endpoint_info_struct ep_gcq,
		endpoint_info_struct ep_gcq_payload, amc_event_callback event_cb,
		void *event_cb_data);

/**
 * unset_amc() - Stop the service, close proxy and tidy up PCI
 * @dev: the pci device.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * 
 * Return: 0 or negative error code.
 */
int unset_amc(struct pci_dev *dev, struct amc_control_ctxt **amc_ctrl_ctxt);

/**
 * release_amc_mem() - Free resources.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * 
 * Return: 0 or negative error code.
 */
void release_amc_mem(struct amc_control_ctxt **amc_ctrl_ctxt);

#endif /* AMI_AMC_CONTROL_H */
