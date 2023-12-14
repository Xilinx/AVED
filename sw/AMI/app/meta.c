// SPDX-License-Identifier: GPL-2.0-only
/*
 * meta.c - This file contains utilities for printing AMI info/metadata
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/* API includes */
#include "ami.h"
#include "json.h"
#include "ami_version.h"
#include "ami_program.h"
#include "ami_mfg_info.h"

/* App includes */
#include "meta.h"
#include "printer.h"
#include "apputils.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define META_MAX_STR_LEN		(64)

/* Version table information. */
#define NUM_VERSION_ROWS		(5)
#define NUM_VERSION_COLS		(2)
#define VERSION_ROW_VER			(0)
#define VERSION_ROW_BRANCH		(1)
#define VERSION_ROW_HASH		(2)
#define VERSION_ROW_DATE		(3)
#define VERSION_ROW_DRV			(4)
#define VERSION_HEADER_AMI		(0)

/* Device overview table information */
#define NUM_OVERVIEW_COLS		(5)
#define NUM_OVERVIEW_COLS_V		(7)
/* Default fields */
#define OVERVIEW_COL_BDF		(0)
#define OVERVIEW_COL_NAME		(1)
#define OVERVIEW_COL_UUID		(2)
#define OVERVIEW_COL_AMC		(3)
#define OVERVIEW_COL_STATE		(4)
/* Verbose only fields */
#define OVERVIEW_COL_HWMON		(5)
#define OVERVIEW_COL_CDEV		(6)

/* PCI info */
#define NUM_PCIEINFO_ROWS		(6)
#define NUM_PCIEINFO_COLS		(2)
#define PCIEINFO_HEADER_INFO		(0)
#define PCIEINFO_ROW_VENDOR		(0)
#define PCIEINFO_ROW_DEVICE		(1)
#define PCIEINFO_ROW_LINK_SPEED		(2)
#define PCIEINFO_ROW_LINK_WIDTH		(3)
#define PCIEINFO_ROW_NUMA_NODE		(4)
#define PCIEINFO_ROW_CPULIST		(5)

/* FPT header information */
#define PARTITION_ID_STR_LEN		(20)
#define NUM_FPT_HEADER_ROWS		(4)
#define NUM_FPT_HEADER_COLS		(2)
#define FPT_HEADER_INFO			(0)
#define FPT_HEADER_ROW_VERSION		(0)
#define FPT_HEADER_ROW_SIZE		(1)
#define FPT_HEADER_ROW_ENTRY_SIZE	(2)
#define FPT_HEADER_ROW_NUM_ENTRIES	(3)

/* FPT partition information */
#define NUM_PARTITION_COLS		(4)
#define PARTITION_COL_ID		(0)
#define PARTITION_COL_TYPE		(1)
#define PARTITION_COL_ADDR		(2)
#define PARTITION_COL_SIZE		(3)

/* Manufacturing info */
#define NUM_MFG_INFO_ROWS		(17)
#define NUM_MFG_INFO_COLS		(2)
#define MFG_INFO_HEADER_INFO		(0)
#define MFG_INFO_EEPROM_VERSION		(0)
#define MFG_INFO_PRODUCT_NAME		(1)
#define MFG_INFO_BOARD_REVISION		(2)
#define MFG_INFO_BOARD_SERIAL		(3)
#define MFG_INFO_MAC_ADDRESS_1		(4)
#define MFG_INFO_MAC_ADDRESS_N		(5)
#define MFG_INFO_MEMORY_SIZE		(6)
#define MFG_INFO_CONFIG_MODE		(7)
#define MFG_INFO_MAX_POWER_MODE		(8)
#define MFG_INFO_ACTIVE_STATE		(9)
#define MFG_INFO_MANUFACTURING_DATE     (10)
#define MFG_INFO_UUID		        (11)
#define MFG_INFO_PCIE_ID		(12)
#define MFG_INFO_OEM_ID		        (13)
#define MFG_INFO_CAPABILITY		(14)
#define MFG_INFO_PART_NUM               (15)
#define MFG_INFO_MFG_PART_NUM		(16)

#define MFG_OEM_ID_BASE			(16)
#define MFG_TIMESTAMP_BASE		(10)
#define MFG_DATE_TM_YEAR		(1996 - 1900)
#define FPGA_CONFIG_MODE_QSPIX4         (7)
#define FPGA_CONFIG_MODE_OSPI           (8)
#define MAX_POWER_MODE_75W              (0)
#define MAX_POWER_MODE_150W             (1)
#define MAX_POWER_MODE_225W             (2)
#define MAX_POWER_MODE_350W             (3)

#define NOT_APPLICABLE_FIELD		"N/A"

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/**
 * fpt_partition_type_to_str() - Convert an FPT partition type into a human
 *     readable string.
 * @type: The partition type.
 *
 * Return: Human readable string ("unknown" if not recognised)
 */
static char* fpt_partition_type_to_str(enum ami_fpt_type type)
{
	switch (type) {
	case AMI_FPT_TYPE_FPT:
		return "FPT";
	
	case AMI_FPT_TYPE_RECOVERY_FPT:
		return "RECOVERY_FPT";

	case AMI_FPT_TYPE_EXTENSION_FPT:
		return "EXTENSION_FPT";

	case AMI_FPT_TYPE_PDI_BOOT:
		return "PDI_BOOT";

	case AMI_FPT_TYPE_PDI_BOOT_BACKUP:
		return "PDI_BOOT_BACKUP";

	case AMI_FPT_TYPE_PDI_XSABIN_META:
		return "PDI_XSABIN_META";

	case AMI_FPT_TYPE_PDI_GOLDEN:
		return "PDI_GOLDEN";

	case AMI_FPT_TYPE_PDI_SYS_DTB:
		return "PDI_SYS_DTB";

	case AMI_FPT_TYPE_PDI_META:
		return "PDI_META";

	case AMI_FPT_TYPE_PDI_META_BACKUP:
		return "PDI_META_BACKUP";

	case AMI_FPT_TYPE_SC_FW:
		return "SC_FW";

	default:
		break;
	}

	return "Unknown";
}

/**
 * populate_fpt_header() - Fill in the header of an FPT header table.
 * @dev: AMI device handle.
 * @header: Pointer to pre-allocated table header.
 * @num_fields: Number of elements in table header.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_fpt_header(ami_device *dev, char **header,
	int num_fields, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;
	
	if (!header)
		return EXIT_FAILURE;
	
	/* dev and data may be NULL */
	
	for (i = 0; (i < num_fields) && (i < NUM_FPT_HEADER_COLS); i++) {
		if (!header[i]) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (i) {
		case FPT_HEADER_INFO:
			sprintf(header[i], "%s", "FPT");
			break;

		default:
			break;
		}
	}

	return ret;
}

/**
 * populate_fpt_values() - Populate an arbitrary data structure with FPT header
 *   information for printing.
 * @dev: Device handle.
 * @values: Pointer to data structure.
 * @n_rows: Pointer to number of rows (records) in data structure.
 * @n_fields: Pointer to number of elements in each row.
 * @fmt: Format of data structure. Used to determine type of `values`.
 * @data: Pointer to fpt header struct.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_fpt_values(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data)
{
	int i = 0;
	struct ami_fpt_header *hdr = NULL;

	if (!dev || !values || !n_rows || !n_fields || !data)
		return EXIT_FAILURE;
	
	hdr = (struct ami_fpt_header*)data;
	
	for (i = 0; i < NUM_FPT_HEADER_ROWS; i++) {
		int col = 0;

		switch (i) {
		case FPT_HEADER_ROW_VERSION:
		{
			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Version");
				sprintf(((char***)values)[i][col++], "%d", hdr->version);
				break;
			
			case APP_OUT_FORMAT_JSON:
				json_append_member(
					(JsonNode*)values,
					"version",
					json_mknumber(hdr->version)
				);
				break;
			
			default:
				break;
			}

			break;
		}

		case FPT_HEADER_ROW_SIZE:
		{
			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Header Size");
				sprintf(((char***)values)[i][col++], "%d", hdr->hdr_size);
				break;
			
			case APP_OUT_FORMAT_JSON:
				json_append_member(
					(JsonNode*)values,
					"header_size",
					json_mknumber(hdr->hdr_size)
				);
				break;
			
			default:
				break;
			}

			break;
		}

		case FPT_HEADER_ROW_ENTRY_SIZE:
		{
			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Entry Size");
				sprintf(((char***)values)[i][col++], "%d", hdr->entry_size);
				break;
			
			case APP_OUT_FORMAT_JSON:
				json_append_member(
					(JsonNode*)values,
					"entry_size",
					json_mknumber(hdr->entry_size)
				);
				break;
			
			default:
				break;
			}

			break;
		}

		case FPT_HEADER_ROW_NUM_ENTRIES:
		{
			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Entries");
				sprintf(((char***)values)[i][col++], "%d", hdr->num_entries);
				break;
			
			case APP_OUT_FORMAT_JSON:
				json_append_member(
					(JsonNode*)values,
					"entries",
					json_mknumber(hdr->num_entries)
				);
				break;
			
			default:
				break;
			}

			break;
		}

		default:
			break;
		}
	}

	return EXIT_SUCCESS;
}

/**
 * populate_version_header() - Fill in the fields of a version table header.
 * @dev: AMI device handle. Not used.
 * @header: Pointer to pre-allocated header.
 * @num_fields: Number of elements in header.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_version_header(ami_device *dev, char **header,
	int num_fields, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;
	
	if (!header)
		return EXIT_FAILURE;
	
	/* dev and data may be NULL */
	
	for (i = 0; i < num_fields; i++) {
		if (!header[i]) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (i) {
		case VERSION_HEADER_AMI:
			sprintf(header[i], "%s", "AMI");
			break;
		
		default:
			break;
		}
	}

	return ret;
}

/**
 * populate_version_values() - Populate an arbitrary data structure with version
 *                             data for printing.
 * @dev: Device handle. Not used.
 * @values: Pointer to data structure
 * @n_rows:  Pointer to number of rows (records) in data structure.
 * @n_fields:  Pointer to number of elements in each row.
 * @fmt: Format of data structure. Used to determine type of `values`.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_version_values(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data)
{
	struct ami_version driver_ver = { 0 };

	if (!values || !n_rows || !n_fields)
		return EXIT_FAILURE;
	
	/* dev and data may be NULL */

	/* Get the driver version. */
	if (ami_get_driver_version(&driver_ver) != AMI_STATUS_OK)
		return EXIT_FAILURE;

	switch (fmt) {
	case APP_OUT_FORMAT_TABLE:
	{
		int i = 0;
		char ***rows = (char***)values;

		for (i = 0; i < NUM_VERSION_ROWS; i++) {
			int col = 0;

			switch (i) {
			case VERSION_ROW_VER:
				sprintf(rows[i][col++], "%s", "Version");
				sprintf(
					rows[i][col++],
					"%d.%d.%d%c (%d)",
					GIT_TAG_VER_MAJOR,
					GIT_TAG_VER_MINOR,
					GIT_TAG_VER_PATCH,
					(GIT_STATUS == 0) ? (' ') : ('*'),
					GIT_TAG_VER_DEV_COMMITS
				);
				break;
			
			case VERSION_ROW_BRANCH:
				sprintf(rows[i][col++], "%s", "Branch");
				sprintf(rows[i][col++], "%s", GIT_BRANCH);
				break;
			
			case VERSION_ROW_HASH:
				sprintf(rows[i][col++], "%s", "Hash");
				sprintf(rows[i][col++], "%s", GIT_HASH);
				break;
			
			case VERSION_ROW_DATE:
				sprintf(rows[i][col++], "%s", "Hash Date");
				sprintf(rows[i][col++], "%s", GIT_DATE);
				break;
			
			case VERSION_ROW_DRV:
				sprintf(rows[i][col++], "%s", "Driver Version");
				sprintf(
					rows[i][col++],
					"%d.%d.%d%c (%d)",
					driver_ver.major,
					driver_ver.minor,
					driver_ver.patch,
					(driver_ver.status == 0) ? (' ') : ('*'),
					driver_ver.dev_commits
				);
				break;

			default:
				break;
			}
		}
		break;
	}

	case APP_OUT_FORMAT_JSON:
	{
		JsonNode *parent = (JsonNode*)values;
		JsonNode *api_ver = json_mkobject();
		JsonNode *drv_ver = json_mkobject();

		/* Construct API version */
		json_append_member(api_ver, "major", json_mknumber(GIT_TAG_VER_MAJOR));
		json_append_member(api_ver, "minor", json_mknumber(GIT_TAG_VER_MINOR));
		json_append_member(api_ver, "patch", json_mknumber(GIT_TAG_VER_PATCH));
		json_append_member(api_ver, "commits", json_mknumber(GIT_TAG_VER_DEV_COMMITS));
		json_append_member(api_ver, "status", json_mknumber(GIT_STATUS));
		json_append_member(api_ver, "branch", json_mkstring(GIT_BRANCH));
		json_append_member(api_ver, "hash", json_mkstring(GIT_HASH));
		json_append_member(api_ver, "date", json_mkstring(GIT_DATE));

		/* Construct driver version*/
		json_append_member(drv_ver, "major", json_mknumber(driver_ver.major));
		json_append_member(drv_ver, "minor", json_mknumber(driver_ver.minor));
		json_append_member(drv_ver, "patch", json_mknumber(driver_ver.patch));
		json_append_member(drv_ver, "commits", json_mknumber(driver_ver.dev_commits));
		json_append_member(drv_ver, "status", json_mknumber(driver_ver.status));
		
		/* Add to parent */
		json_append_member(parent, "api", api_ver);
		json_append_member(parent, "driver", drv_ver);

		break;
	}

	default:
		break;
	}

	return EXIT_SUCCESS;
}

/**
 * populate_partition_header() - Fill in the header of an FPT partition table.
 * @dev: AMI device handle.
 * @header: Pointer to pre-allocated table header.
 * @num_fields: Number of elements in table header.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_partition_header(ami_device *dev, char **header,
	int num_fields, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;

	/* data is unused */
	if (!header)
		return EXIT_FAILURE;
	
	/* dev and data may be NULL */

	for (i = 0; (i < num_fields) && (i < NUM_PARTITION_COLS); i++) {
		if (!header[i]) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (i) {
		case PARTITION_COL_ID:
			sprintf(header[i], "%s", "Partition");
			break;

		case PARTITION_COL_TYPE:
			sprintf(header[i], "%s", "Type");
			break;

		case PARTITION_COL_ADDR:
			sprintf(header[i], "%s", "Address");
			break;

		case PARTITION_COL_SIZE:
			sprintf(header[i], "%s", "Size");
			break;
		
		default:
			break;
		}
	}

	return ret;
}

/**
 * construct_partition_row() - Construct a single table row with FPT partition 
 *    information.
 * @part: Partition data.
 * @part_num: Current row number (partition ID).
 * @row: Row to fill in.
 * @n_fields: Number of expected elements in the row.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int construct_partition_row(struct ami_fpt_partition *part, int part_num,
	char **row, int n_fields)
{
	int col = 0;

	if (!part || !row)
		return EXIT_FAILURE;
	
	for (col = 0; (col < n_fields) && (col < NUM_PARTITION_COLS); col++) {
		switch (col) {
		case PARTITION_COL_ID:
			sprintf(row[col], "%d", part_num);
			break;
		
		case PARTITION_COL_TYPE:
			sprintf(row[col], "%s", fpt_partition_type_to_str(part->type));
			break;
		
		case PARTITION_COL_ADDR:
			sprintf(row[col], "0x%08x", part->base_addr);
			break;
		
		case PARTITION_COL_SIZE:
			sprintf(row[col], "0x%08x", part->size);
			break;

		default:
			break;
		}
	}

	return EXIT_SUCCESS;
}

/**
 * construct_partition_node() - Construct a single Json node with partition information
 * @part: Partition data.
 * @part_num: Partition number.
 * @parent: Parent JSON object.
 * @n_fields: Number of expected elements in the JSON node.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int construct_partition_node(struct ami_fpt_partition *part, int part_num,
	JsonNode *parent, int n_fields)
{
	int col = 0;
	char id_string[PARTITION_ID_STR_LEN] = { 0 };
	JsonNode *row = NULL;

	if (!part || !parent)
		return EXIT_FAILURE;
	
	row = json_mkobject();

	for (col = 0; (col < n_fields) && (col < NUM_PARTITION_COLS); col++) {
		switch (col) {
		case PARTITION_COL_TYPE:
			json_append_member(
				row,
				"type",
				json_mkstring(fpt_partition_type_to_str(part->type))
			);
			break;
		
		case PARTITION_COL_ADDR:
			json_append_member(
				row,
				"address",
				json_mknumber(part->base_addr)
			);
			break;
		
		case PARTITION_COL_SIZE:
			json_append_member(
				row,
				"size",
				json_mknumber(part->size)
			);
			break;

		default:
			break;
		}
	}

	sprintf(id_string, "%d", part_num);
	json_append_member(parent, id_string, row);
	return EXIT_SUCCESS;
}

/**
 * populate_partition_values() - Populate an arbitrary data structure with FPT
 *   partition information for printing.
 * @dev: Device handle.
 * @values: Pointer to data structure.
 * @n_rows: Pointer to number of rows (records) in data structure.
 * @n_fields: Pointer to number of elements in each row.
 * @fmt: Format of data structure. Used to determine type of `values`.
 * @data: Pointer to fpt header struct.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_partition_values(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;
	struct ami_fpt_header *hdr = NULL;
	struct ami_fpt_partition part = { 0 };

	if (!dev || !values || !n_rows || !n_fields || !data)
		return EXIT_FAILURE;
	
	hdr = (struct ami_fpt_header*)data;

	while ((i < *n_rows) && (i < hdr->num_entries)) {
		if (ami_prog_get_fpt_partition(dev, i, &part) != AMI_STATUS_OK) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (fmt) {
		case APP_OUT_FORMAT_TABLE:
			construct_partition_row(
				&part,
				i,
				((char***)values)[i],
				*n_fields
			);
			break;

		case APP_OUT_FORMAT_JSON:
			construct_partition_node(
				&part,
				i,
				(JsonNode*)values,
				*n_fields
			);
			break;

		default:
			break;
		}

		i++;
	}

	return ret;
}

/**
 * populate_overview_header() - Fill in the fields of the device overview table header.
 * @dev: AMI device handle. Not used.
 * @header: Pointer to pre-allocated header.
 * @num_fields: Number of elements in header.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_overview_header(ami_device *dev, char **header,
	int num_fields, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;

	if (!header)
		return EXIT_FAILURE;
	
	/* dev and data may be NULL */

	for (i = 0; (i < num_fields) && (i < NUM_OVERVIEW_COLS_V); i++) {
		if (!header[i]) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (i) {
		case OVERVIEW_COL_BDF:
			sprintf(header[i], "%s", "BDF");
			break;

		case OVERVIEW_COL_UUID:
			sprintf(header[i], "%s", "UUID");
			break;

		case OVERVIEW_COL_HWMON:
			sprintf(header[i], "%s", "HWMON");
			break;

		case OVERVIEW_COL_CDEV:
			sprintf(header[i], "%s", "CDEV");
			break;
		
		case OVERVIEW_COL_STATE:
			sprintf(header[i], "%s", "State");
			break;
		
		case OVERVIEW_COL_NAME:
			sprintf(header[i], "%s", "Device");
			break;
		
		case OVERVIEW_COL_AMC:
			sprintf(header[i], "%s", "AMC");
			break;
		
		default:
			break;
		}
	}

	return ret;
}

/**
 * construct_overview_node() - Construct a single Json node with device information
 * @dev: Device handle.
 * @parent: Parent JSON object.
 * @n_fields: Number of expected elements in the JSON node.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int construct_overview_node(ami_device *dev, JsonNode *parent, int n_fields)
{
	int col = 0;
	uint16_t bdf = 0;
	char bdf_string[AMI_BDF_STR_LEN] = { 0 };
	JsonNode *row = NULL;
	JsonNode *uuid_node = NULL;
	JsonNode *hwmon_node = NULL;
	JsonNode *cdev_node = NULL;
	JsonNode *state_node = NULL;
	JsonNode *name_node = NULL;
	JsonNode *amc_node = NULL;

	if (!dev || !parent)
		return EXIT_FAILURE;
	
	row = json_mkobject();

	for (col = 0; (col < n_fields) && (col < NUM_OVERVIEW_COLS_V); col++) {
		switch (col) {
		case OVERVIEW_COL_UUID:
		{
			char uuid[AMI_LOGIC_UUID_SIZE] = { 0 };

			if (ami_dev_read_uuid(dev, uuid) == AMI_STATUS_OK)
				uuid_node = json_mkstring(uuid);
			else
				uuid_node = json_mknull();
			
			json_append_member(row, "uuid", uuid_node);
			break;
		}

		case OVERVIEW_COL_HWMON:
		{
			int hwmon = 0;

			if (ami_dev_get_hwmon_num(dev, &hwmon) == AMI_STATUS_OK)
				hwmon_node = json_mknumber(hwmon);
			else
				hwmon_node = json_mknull();
			
			json_append_member(row, "hwmon", hwmon_node);
			break;
		}

		case OVERVIEW_COL_CDEV:
		{
			int cdev = 0;

			if (ami_dev_get_cdev_num(dev, &cdev) == AMI_STATUS_OK)
				cdev_node = json_mknumber(cdev);
			else
				cdev_node = json_mknull();
			
			json_append_member(row, "cdev", cdev_node);
			break;
		}

		case OVERVIEW_COL_STATE:
		{
			char state[AMI_DEV_STATE_SIZE] = { 0 };

			if (ami_dev_get_state(dev, state) == AMI_STATUS_OK)
				state_node = json_mkstring(state);
			else
				state_node = json_mknull();
			
			json_append_member(row, "state", state_node);
			break;
		}

		case OVERVIEW_COL_NAME:
		{
			char name[AMI_DEV_NAME_SIZE] = { 0 };

			if (ami_dev_get_name(dev, name) == AMI_STATUS_OK)
				name_node = json_mkstring(name);
			else
				name_node = json_mknull();
			
			json_append_member(row, "name", name_node);
			break;
		}
	
		case OVERVIEW_COL_AMC:
		{
			struct amc_version ver = { 0 };

			if (ami_dev_get_amc_version(dev, &ver) == AMI_STATUS_OK) {
				amc_node = json_mkobject();
				json_append_member(amc_node, "major", json_mknumber(ver.major));
				json_append_member(amc_node, "minor", json_mknumber(ver.minor));
				json_append_member(amc_node, "patch", json_mknumber(ver.patch));
				json_append_member(amc_node, "commits", json_mknumber(ver.dev_commits));
				json_append_member(amc_node, "local_changes", json_mknumber(ver.local_changes));
			} else {
				amc_node = json_mknull();
			}
			
			json_append_member(row, "amc", amc_node);
			break;
		}

		default:
			break;
		}
	}

	ami_dev_get_pci_bdf(dev, &bdf);
	sprintf(
		bdf_string,
		"%02x:%02x.%01x",
		AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf)
	);
	json_append_member(parent, bdf_string, row);

	return EXIT_SUCCESS;
}

/**
 * construct_overview_row() - Construct a single table row with device information
 * @dev: Device handle.
 * @row: Row to fill in.
 * @n_fields: Number of expected elements in the row.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int construct_overview_row(ami_device *dev, char **row, int n_fields)
{
	int col = 0;

	if (!dev || !row)
		return EXIT_FAILURE;
	
	for (col = 0; (col < n_fields) && (col < NUM_OVERVIEW_COLS_V); col++) {
		switch (col) {
		case OVERVIEW_COL_BDF:
		{
			uint16_t bdf = 0;

			if (ami_dev_get_pci_bdf(dev, &bdf) == AMI_STATUS_OK)
				sprintf(
					row[col],
					"%02x:%02x.%01x",
					AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf)
				);

			break;
		}

		case OVERVIEW_COL_UUID:
		{
			char uuid[AMI_LOGIC_UUID_SIZE] = { 0 };

			if (ami_dev_read_uuid(dev, uuid) == AMI_STATUS_OK)
				sprintf(
					row[col],
					"%s",
					uuid
				);
			else
				sprintf(row[col], "%s", "N/A");

			break;
		}

		case OVERVIEW_COL_HWMON:
		{
			int hwmon = 0;

			if (ami_dev_get_hwmon_num(dev, &hwmon) == AMI_STATUS_OK)
				sprintf(row[col], "%d", hwmon);

			break;
		}

		case OVERVIEW_COL_CDEV:
		{
			int cdev = 0;

			if (ami_dev_get_cdev_num(dev, &cdev) == AMI_STATUS_OK)
				sprintf(row[col], "%d", cdev);

			break;
		}

		case OVERVIEW_COL_STATE:
		{
			char state[AMI_DEV_STATE_SIZE] = { 0 };

			if (ami_dev_get_state(dev, state) == AMI_STATUS_OK)
				sprintf(
					row[col],
					"%s",
					state
				);
			else
				sprintf(row[col], "%s", "Unknown");

			break;
		}

		case OVERVIEW_COL_NAME:
		{
			char name[AMI_DEV_NAME_SIZE] = { 0 };

			if (ami_dev_get_name(dev, name) == AMI_STATUS_OK)
				sprintf(
					row[col],
					"%s",
					name
				);
			else
				sprintf(row[col], "%s", "Unknown");

			break;
		}

		case OVERVIEW_COL_AMC:
		{
			struct amc_version ver = { 0 };

			if (ami_dev_get_amc_version(dev, &ver) == AMI_STATUS_OK)
				sprintf(
					row[col],
					"%d.%d.%d%c (%d)",
					ver.major,
					ver.minor,
					ver.patch,
					(ver.local_changes == 0) ? (' ') : ('*'),
					ver.dev_commits
				);
			else
				sprintf(row[col], "%s", "Unknown");

			break;
		}

		default:
			break;
		}
	}

	return EXIT_SUCCESS;
}

/**
 * populate_overview_values() - Populate an arbitrary data structure with device
 *   overview information for printing.
 * @dev: Device handle. Not used.
 * @values: Pointer to data structure
 * @n_rows:  Pointer to number of rows (records) in data structure.
 * @n_fields:  Pointer to number of elements in each row.
 * @fmt: Format of data structure. Used to determine type of `values`.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_overview_values(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data)
{
	int i = 0;
	ami_device *device = NULL;
	ami_device *prev = NULL;

	if (!values || !n_rows || !n_fields)
		return EXIT_FAILURE;
	
	/* dev and data may be NULL */

	while ((i < *n_rows) &&
		(ami_dev_find_next(&device, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, prev) == AMI_STATUS_OK)) {
		
		switch (fmt) {
		case APP_OUT_FORMAT_TABLE:
			construct_overview_row(
				device,
				((char***)values)[i],
				*n_fields
			);
			break;
		
		case APP_OUT_FORMAT_JSON:
			construct_overview_node(
				device,
				(JsonNode*)values,
				*n_fields
			);

		default:
			break;
		}

		/* Move to next device. */
		ami_dev_delete(&prev);
		prev = device;
		device = NULL;
		i++;
	}

	/* Check if we could iterate over the devices */
	if (i < *n_rows)
		APP_WARN("could not fetch device data");

	/* Delete final device. */
	ami_dev_delete(&prev);
	return EXIT_SUCCESS;
}

/**
 * populate_pcieinfo_header() - Fill in the fields of a PCI info table header.
 * @dev: AMI device handle. Not used.
 * @header: Pointer to pre-allocated header.
 * @num_fields: Number of elements in header.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_pcieinfo_header(ami_device *dev, char **header,
	int num_fields, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;

	if (!header || !dev)
		return EXIT_FAILURE;

	/* data may be NULL */

	for (i = 0; (i < num_fields) && (i < NUM_PCIEINFO_COLS); i++) {
		if (!header[i]) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (i) {
		case PCIEINFO_HEADER_INFO:
			sprintf(header[i], "%s", "PCIe Info");
			break;
		
		default:
			break;
		}
	}

	return ret;
}

/**
 * populate_pcieinfo_values() - Populate an arbitrary data structure with
 *   PCI info for printing.
 * @dev: Device handle.
 * @values: Pointer to data structure
 * @n_rows: Pointer to number of rows (records) in data structure.
 * @n_fields: Pointer to number of elements in each row.
 * @fmt: Format of data structure. Used to determine type of `values`.
 * @data: Implementation data. Not used.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_pcieinfo_values(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data)
{
	int i = 0;

	if (!dev || !values || !n_rows || !n_fields)
		return EXIT_FAILURE;

	/* data may be NULL */

	for (i = 0; i < NUM_PCIEINFO_ROWS; i++) {
		int col = 0;

		switch (i) {
		case PCIEINFO_ROW_VENDOR:
		{
			uint16_t vendor = 0;
			int r = ami_dev_get_pci_vendor(dev, &vendor);

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Vendor");
				sprintf(((char***)values)[i][col++], "0x%04x", vendor);
				break;
			
			case APP_OUT_FORMAT_JSON:
			{
				JsonNode *vendor_node = NULL;

				if (r == AMI_STATUS_OK)
					vendor_node = json_mknumber(vendor);
				else
					vendor_node = json_mknull();

				json_append_member((JsonNode*)values, "vendor", vendor_node);
				break;
			}

			default:
				break;
			}
			
			break;
		}

		case PCIEINFO_ROW_DEVICE:
		{
			uint16_t device = 0;
			int r = ami_dev_get_pci_device(dev, &device);

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Device");
				sprintf(((char***)values)[i][col++], "0x%04x", device);
				break;
			
			case APP_OUT_FORMAT_JSON:
			{
				JsonNode *device_node = NULL;

				if (r == AMI_STATUS_OK)
					device_node = json_mknumber(device);
				else
					device_node = json_mknull();

				json_append_member((JsonNode*)values, "device", device_node);
				break;
			}
			
			default:
				break;
			}
			
			break;
		}
		
		case PCIEINFO_ROW_LINK_SPEED:
		{
			uint8_t current = 0, max = 0;
			int r = ami_dev_get_pci_link_speed(dev, &current, &max);

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Link Speed");
				sprintf(((char***)values)[i][col++], "Gen%d (max Gen%d)", current, max);
				break;
			
			case APP_OUT_FORMAT_JSON:
			{
				JsonNode *link_node = NULL;

				if (r == AMI_STATUS_OK) {
					JsonNode *max_node = json_mknumber(max);
					JsonNode *current_node = json_mknumber(current);

					link_node = json_mkobject();
					json_append_member(link_node, "max", max_node);
					json_append_member(link_node, "current", current_node);
				} else {
					link_node = json_mknull();
				}

				json_append_member((JsonNode*)values, "link_speed", link_node);
				break;
			}
			
			default:
				break;
			}
			
			
			break;
		}
		
		case PCIEINFO_ROW_LINK_WIDTH:
		{
			uint8_t current = 0, max = 0;
			int r = ami_dev_get_pci_link_width(dev, &current, &max);

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "Link Width");
				sprintf(((char***)values)[i][col++], "x%d (max x%d)", current, max);
				break;
			
			case APP_OUT_FORMAT_JSON:
			{
				JsonNode *width_node = NULL;

				if (r == AMI_STATUS_OK) {
					JsonNode *max_node = json_mknumber(max);
					JsonNode *current_node = json_mknumber(current);

					width_node = json_mkobject();
					json_append_member(width_node, "max", max_node);
					json_append_member(width_node, "current", current_node);
				} else {
					width_node = json_mknull();
				}

				json_append_member((JsonNode*)values, "link_width", width_node);
				break;
			}
			
			default:
				break;
			}
			
			
			break;
		}

		case PCIEINFO_ROW_NUMA_NODE:
		{
			uint8_t numa = 0;
			int r = ami_dev_get_pci_numa_node(dev, &numa);

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "NUMA Node");
				sprintf(((char***)values)[i][col++], "%d", numa);
				break;
			
			case APP_OUT_FORMAT_JSON:
			{
				JsonNode *numa_node = NULL;

				if (r == AMI_STATUS_OK)
					numa_node = json_mknumber(numa);
				else
					numa_node = json_mknull();

				json_append_member((JsonNode*)values, "numa_node", numa_node);
				break;
			}
			
			default:
				break;
			}
			
			break;
		}

		case PCIEINFO_ROW_CPULIST:
		{
			char buf[AMI_PCI_CPULIST_SIZE] = { 0 };
			int r = ami_dev_get_pci_cpulist(dev, buf);

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[i][col++], "%s", "CPU Affinity");
				sprintf(
					((char***)values)[i][col++],
					"%s",
					(buf[0] == '\0') ? ("N/A") : (buf)
				);
				break;
			
			case APP_OUT_FORMAT_JSON:
			{
				JsonNode *cpu_node = NULL;

				if (r == AMI_STATUS_OK)
					cpu_node = json_mkstring(buf);
				else
					cpu_node = json_mknull();

				json_append_member((JsonNode*)values, "cpu_affinity", cpu_node);
				break;
			}
			
			default:
				break;
			}
			
			break;
		}
		
		default:
			break;
		}
	}

	return EXIT_SUCCESS;
}

/**
 * populate_mfg_info_header() - Fill in the fields of a manufacturing info table header.
 * @dev: AMI device handle. Not used.
 * @header: Pointer to pre-allocated header.
 * @num_fields: Number of elements in header.
 * @data: Implementation data. Not used.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_mfg_info_header(ami_device *dev, char **header,
	int num_fields, void *data)
{
	int i = 0;
	int ret = EXIT_SUCCESS;

	if (!header || !dev)
		return EXIT_FAILURE;

	/* data may be NULL */

	for (i = 0; (i < num_fields) && (i < NUM_MFG_INFO_COLS); i++) {
		if (!header[i]) {
			ret = EXIT_FAILURE;
			break;
		}

		switch (i) {
		case MFG_INFO_HEADER_INFO:
			sprintf(header[i], "%s", "Manufacturing Information");
			break;

		default:
			break;
		}
	}

	return ret;
}

/**
 * mfg_row_to_field_type() - Get EEPROM field type for a row in the mfg table.
 * @row_num: The row number.
 *
 * Return: The corresponding EEPROM field type or AMI_MFG_FIELD_MAX.
 */
static enum ami_mfg_field mfg_row_to_field_type(int row_num)
{
	switch (row_num) {
	case MFG_INFO_EEPROM_VERSION:     return AMI_MFG_EEPROM_VERSION;
	case MFG_INFO_PRODUCT_NAME:       return AMI_MFG_PRODUCT_NAME;
	case MFG_INFO_BOARD_REVISION:     return AMI_MFG_BOARD_REV;
	case MFG_INFO_BOARD_SERIAL:       return AMI_MFG_BOARD_SERIAL;
	case MFG_INFO_MAC_ADDRESS_1:      return AMI_MFG_MAC_ADDR;
	case MFG_INFO_MAC_ADDRESS_N:      return AMI_MFG_MAC_ADDR_N;
	case MFG_INFO_MEMORY_SIZE:        return AMI_MFG_MEMORY_SIZE;
	case MFG_INFO_CONFIG_MODE:        return AMI_MFG_CONFIG_MODE;
	case MFG_INFO_MAX_POWER_MODE:     return AMI_MFG_POWER_MODE;
	case MFG_INFO_ACTIVE_STATE:       return AMI_MFG_ACTIVE_STATE;
	case MFG_INFO_MANUFACTURING_DATE: return AMI_MFG_M_DATE;
	case MFG_INFO_UUID:               return AMI_MFG_UUID;
	case MFG_INFO_PCIE_ID:            return AMI_MFG_PCIE_ID;
	case MFG_INFO_OEM_ID:             return AMI_MFG_OEM_ID;
	case MFG_INFO_CAPABILITY:         return AMI_MFG_CAPABILITY;
	case MFG_INFO_PART_NUM:           return AMI_MFG_PART_NUM;
	case MFG_INFO_MFG_PART_NUM:       return AMI_MFG_M_PART_NUM;
	default:                          break;
	}

	return AMI_MFG_FIELD_MAX;
}

/**
 * header_name_for_mfg_row() - Get the header name of a row in the mfg table/json.
 * @row_num: The row number.
 * @fmt: The output format (table or json)
 *.
 * Return: The corresponding header name or "Unknown".
 */
static char *header_name_for_mfg_row(int row_num, enum app_out_format fmt)
{
	switch (fmt) {
	case APP_OUT_FORMAT_TABLE:
		switch (row_num) {
		case MFG_INFO_EEPROM_VERSION:     return "Eeprom Version";
		case MFG_INFO_PRODUCT_NAME:       return "Product Name";
		case MFG_INFO_BOARD_REVISION:     return "Board Revision";
		case MFG_INFO_BOARD_SERIAL:       return "Serial Number";
		case MFG_INFO_MAC_ADDRESS_1:      return "Mac Address 1";
		case MFG_INFO_MAC_ADDRESS_N:      return "Mac Address N";
		case MFG_INFO_MEMORY_SIZE:        return "Memory Size";
		case MFG_INFO_CONFIG_MODE:        return "FPGA Config Mode";
		case MFG_INFO_MAX_POWER_MODE:     return "Max Power Mode";
		case MFG_INFO_ACTIVE_STATE:       return "Active State";
		case MFG_INFO_MANUFACTURING_DATE: return "Manufacturing Date";
		case MFG_INFO_UUID:               return "UUID";
		case MFG_INFO_PCIE_ID:            return "PCIe Ids(Vendor, Device, SS Vendor, SS Device)";
		case MFG_INFO_OEM_ID:             return "OEM ID";
		case MFG_INFO_CAPABILITY:         return "Capability Word";
		case MFG_INFO_PART_NUM:           return "Board Part Num";
		case MFG_INFO_MFG_PART_NUM:       return "Mfg Part Num";
		default:                          break;
		}
		break;

	case APP_OUT_FORMAT_JSON:
		switch (row_num) {
		case MFG_INFO_EEPROM_VERSION:     return "eeprom_version";
		case MFG_INFO_PRODUCT_NAME:       return "product_name";
		case MFG_INFO_BOARD_REVISION:     return "board_rev";
		case MFG_INFO_BOARD_SERIAL:       return "serial_number";
		case MFG_INFO_MAC_ADDRESS_1:      return "mac_address_1";
		case MFG_INFO_MAC_ADDRESS_N:      return "mac_address_n";
		case MFG_INFO_MEMORY_SIZE:        return "memory_size";
		case MFG_INFO_CONFIG_MODE:        return "fpga_config_mode";
		case MFG_INFO_MAX_POWER_MODE:     return "max_power_mode";
		case MFG_INFO_ACTIVE_STATE:       return "active_state";
		case MFG_INFO_MANUFACTURING_DATE: return "manufacturing_date";
		case MFG_INFO_UUID:               return "uuid";
		case MFG_INFO_PCIE_ID:            return "pcie_ids";
		case MFG_INFO_OEM_ID:             return "oem_id";
		case MFG_INFO_CAPABILITY:         return "capability_word";
		case MFG_INFO_PART_NUM:           return "board_part_number";
		case MFG_INFO_MFG_PART_NUM:       return "mfg_part_number";
		default:                          break;
		}
		break;

	default:
		break;
	}

	return "Unknown";
}

/**
 * populate_mfg_info_values() - Populate an arbitrary data structure with
 *   manufacturing info for printing.
 * @dev: Device handle.
 * @values: Pointer to data structure
 * @n_rows: Pointer to number of rows (records) in data structure.
 * @n_fields: Pointer to number of elements in each row.
 * @fmt: Format of data structure. Used to determine type of `values`.
 * @data: Implementation data. Not used.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */
static int populate_mfg_info_values(ami_device *dev, void *values,
	int *n_rows, int *n_fields, enum app_out_format fmt, void *data)
{
	int i = 0;
	int j = 0;
	enum ami_mfg_field mfg_field = AMI_MFG_FIELD_MAX;
	char eeprom_buf[AMI_MFG_INFO_MAX_STR] = { 0 };
	char *header = NULL;

	if (!dev || !values || !n_rows || !n_fields)
		return EXIT_FAILURE;

	/* data may be NULL */
	while (i < NUM_MFG_INFO_ROWS) {
		int col = 0;

		mfg_field = mfg_row_to_field_type(i);
		header = header_name_for_mfg_row(i, fmt);

		if (mfg_field != AMI_MFG_FIELD_MAX) {
			memset(eeprom_buf, 0x00, AMI_MFG_INFO_MAX_STR);
			if (ami_mfg_get_info(dev, mfg_field, eeprom_buf) != AMI_STATUS_OK) {
				strncpy(eeprom_buf, "Unknown", AMI_MFG_INFO_MAX_STR - 1);
			} else if (!strcmp(eeprom_buf, NOT_APPLICABLE_FIELD)) {
				memset(eeprom_buf, 0x00, AMI_MFG_INFO_MAX_STR);
				*n_rows = *n_rows - 1; /* remove row */
				i++; 	  	       /* increment loop */
				continue; 	       /* skip this field */
			}
		} else {
			strncpy(eeprom_buf, "Unknown", AMI_MFG_INFO_MAX_STR - 1);
		}

		switch (i) {
		case MFG_INFO_EEPROM_VERSION:
		case MFG_INFO_PRODUCT_NAME:
		case MFG_INFO_BOARD_REVISION:
		case MFG_INFO_BOARD_SERIAL:
		case MFG_INFO_MAC_ADDRESS_1:
		case MFG_INFO_MAC_ADDRESS_N:
		case MFG_INFO_MEMORY_SIZE:
		case MFG_INFO_CONFIG_MODE:
		case MFG_INFO_MAX_POWER_MODE:
		case MFG_INFO_ACTIVE_STATE:
		case MFG_INFO_UUID:
		case MFG_INFO_PCIE_ID:
		case MFG_INFO_CAPABILITY:
		case MFG_INFO_PART_NUM:
		case MFG_INFO_MFG_PART_NUM:
		{
			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[j][col++], "%s", header);
				sprintf(((char***)values)[j][col++], "%s", eeprom_buf);
				break;

			case APP_OUT_FORMAT_JSON:
				json_append_member(
					(JsonNode*)values,
					header,
					json_mkstring(eeprom_buf)
				);
				break;

			default:
				break;
			}
			break;
		}

		case MFG_INFO_MANUFACTURING_DATE:
		{
			/* Number of minutes from 0:00 hrs 1/1/96 in GMT LSbyte first*/
			char manufacturing_date_str[META_MAX_STR_LEN] = { 0 };
			struct tm info = { 0 };
			long manufacturing_date_mins = 0;

			/* convert to long from hex */
			manufacturing_date_mins = strtol(eeprom_buf, NULL, MFG_TIMESTAMP_BASE);
			
			if (manufacturing_date_mins) {
				info.tm_year = MFG_DATE_TM_YEAR;
				info.tm_mon = 1;
				info.tm_mday = 1;
				info.tm_hour = 0;
				info.tm_min = manufacturing_date_mins;
				info.tm_sec = 0;
				info.tm_isdst = -1;

				if(mktime(&info) == AMI_LINUX_STATUS_ERROR)
					return EXIT_FAILURE;

				if(!strftime(manufacturing_date_str,
						sizeof(manufacturing_date_str),
						"%c",
						&info ))
					return EXIT_FAILURE;
			} else {
				strncpy(manufacturing_date_str, eeprom_buf, META_MAX_STR_LEN - 1);
			}

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[j][col++], "%s", header);
				sprintf(((char***)values)[j][col++], "%s", manufacturing_date_str);
				break;

			case APP_OUT_FORMAT_JSON:
				json_append_member(
					(JsonNode*)values,
					header,
					json_mkstring(manufacturing_date_str)
				);
				break;

			default:
				break;
			}
			break;
		}

		case MFG_INFO_OEM_ID:
		{
			/* LSB First Format */
			char oem_str[META_MAX_STR_LEN] = { 0 };

			/* Convert from hex string to int */
			switch((int)strtol(eeprom_buf, NULL, MFG_OEM_ID_BASE))
			{
			case AMI_MFG_INFO_OEM_ID_XILINX:
			case AMI_MFG_INFO_OEM_ID_AMD:
				sprintf(oem_str, "%s", "AMD");
				break;
			default:
				sprintf(oem_str, "%s", eeprom_buf);
				break;
			}

			switch (fmt) {
			case APP_OUT_FORMAT_TABLE:
				sprintf(((char***)values)[j][col++], "%s", header);
				sprintf(((char***)values)[j][col++], "%s", oem_str);
				break;

			case APP_OUT_FORMAT_JSON:
				json_append_member(
					(JsonNode*)values,
					header,
					json_mkstring(oem_str)
				);
				break;

			default:
				break;
			}
			break;
		}

		default:
			sprintf(((char***)values)[j][col++], "%s", "Unknown");
			sprintf(((char***)values)[j][col++], "%s", "Unknown");
			break;
		}

		i++; /* increment loop */
		j++; /* increment values offset */
	}

	return EXIT_SUCCESS;
}

/*****************************************************************************/
/* Public function definitions                                               */
/*****************************************************************************/

/*
 * Callback for the "overview" command.
 */
int print_overview(struct app_option *options)
{
	int ret = EXIT_FAILURE;
	enum app_out_format format = APP_OUT_FORMAT_TABLE;  /* default: table */
	FILE *stream = NULL;
	uint16_t num_devices = 0;
	bool verbose = false;
	
	if (parse_output_options(options, &format, &verbose, &stream,
			NULL, NULL) == EXIT_FAILURE)
		return EXIT_FAILURE;
	
	/* Print version information. */
	ret = print_table_data(
		NULL,
		NUM_VERSION_COLS,
		NUM_VERSION_ROWS,
		(format == APP_OUT_FORMAT_TABLE) ? (stream) : (NULL),
		TABLE_DIVIDER_HEADER_ONLY,
		&populate_version_values,
		&populate_version_header,
		NULL,
		NULL
	);

	if (ret == EXIT_FAILURE) {
		APP_ERROR("could not print version data");
		goto fail;
	}

	/* Print device overview. */
	ret = ami_dev_get_num_devices(&num_devices);

	if (ret == EXIT_FAILURE) {
		APP_API_ERROR("failed to fetch device data");
		goto fail;
	}

	ret = print_table_data(
		NULL,
		((verbose) ? (NUM_OVERVIEW_COLS_V) : (NUM_OVERVIEW_COLS)),
		num_devices,
		(format == APP_OUT_FORMAT_TABLE) ? (stream) : (NULL),
		TABLE_DIVIDER_HEADER_ONLY,
		&populate_overview_values,
		&populate_overview_header,
		NULL,
		NULL
	);

	if (ret)
		APP_ERROR("could not print overview data");

	/* Write to file. */
	if (stream && (ret != EXIT_FAILURE) && (format != APP_OUT_FORMAT_TABLE)) {
		switch (format) {
		case APP_OUT_FORMAT_JSON:
		{
			JsonNode *v = NULL;
			JsonNode *o = NULL;	

			ret = gen_json_data(
				NULL,
				NUM_VERSION_COLS,
				NUM_VERSION_ROWS,
				&populate_version_values,
				NULL,
				&v
			);

			if (ret == EXIT_SUCCESS) {
				ret = gen_json_data(
					NULL,
					((verbose) ? (NUM_OVERVIEW_COLS_V) : (NUM_OVERVIEW_COLS)),
					num_devices,
					&populate_overview_values,
					NULL,
					&o
				);
			
				if (ret == EXIT_SUCCESS) {
					JsonNode *parent = NULL;

					parent = json_mkobject();
					json_append_member(parent, "version", v);
					json_append_member(parent, "physical_functions", o);

					print_json_obj(parent, stream);
					json_delete(parent);
				} else {
					APP_ERROR("could not create overview json");
				}
			} else {
				APP_ERROR("could not create version json");
			}

			break;
		}

		default:
			APP_ERROR("invalid output format");
			break;
		}
	}

fail:
	if (stream)
		fclose(stream);

	return ret;
}

/*
 * Callback for the "pcieinfo" command.
 */
int print_pcieinfo(ami_device *dev, struct app_option *options)
{
	int ret = EXIT_FAILURE;
	enum app_out_format format = APP_OUT_FORMAT_TABLE;  /* default: table */
	FILE *stream = NULL;

	if (!dev || !options)
		return EXIT_FAILURE;

	if (parse_output_options(options, &format, NULL, &stream,
			NULL, NULL) == EXIT_FAILURE)
		return EXIT_FAILURE;

	/* Print PCI information. */
	ret = print_table_data(
		dev,
		NUM_PCIEINFO_COLS,
		NUM_PCIEINFO_ROWS,
		(format == APP_OUT_FORMAT_TABLE) ? (stream) : (NULL),
		TABLE_DIVIDER_HEADER_ONLY,
		&populate_pcieinfo_values,
		&populate_pcieinfo_header,
		NULL,
		NULL
	);

	if (ret)
		APP_ERROR("could not print pcieinfo data");

	/* Write to file. */
	if (stream && (ret != EXIT_FAILURE) && (format != APP_OUT_FORMAT_TABLE)) {
		switch (format) {
		case APP_OUT_FORMAT_JSON:
			ret = print_json_data(
				dev,
				NUM_PCIEINFO_COLS,
				NUM_PCIEINFO_ROWS,
				stream,
				&populate_pcieinfo_values,
				NULL
			);

			if (ret)
				APP_ERROR("could not create pcieinfo json");

			break;

		default:
			break;
		}
	}

	if (stream)
		fclose(stream);

	return ret;
}

/*
 * Callback for the "cfgmem_info" command.
 */
int print_fpt_info(ami_device *dev, struct app_option *options)
{
	int ret = EXIT_FAILURE;
	enum app_out_format format = APP_OUT_FORMAT_TABLE;  /* default: table */
	FILE *stream = NULL;
	struct ami_fpt_header hdr = { 0 };

	if (!dev || !options)
		return EXIT_FAILURE;

	if (parse_output_options(options, &format, NULL, &stream,
			NULL, NULL) == EXIT_FAILURE)
		return EXIT_FAILURE;
	
	/* Must fetch FPT header */
	if (ami_prog_get_fpt_header(dev, &hdr) != EXIT_SUCCESS) {
		APP_API_ERROR("could not fetch FPT header");
		goto fail;
	}

	/* Print FPT header information. */
	ret = print_table_data(
		dev,
		NUM_FPT_HEADER_COLS,
		NUM_FPT_HEADER_ROWS,
		(format == APP_OUT_FORMAT_TABLE) ? (stream) : (NULL),
		TABLE_DIVIDER_HEADER_ONLY,
		&populate_fpt_values,
		&populate_fpt_header,
		&hdr,
		NULL
	);

	if (ret == EXIT_FAILURE) {
		APP_ERROR("could not print FPT header");
		goto fail;
	}

	/* print partition information */
	ret = print_table_data(
		dev,
		NUM_PARTITION_COLS,
		hdr.num_entries,
		(format == APP_OUT_FORMAT_TABLE) ? (stream) : (NULL),
		TABLE_DIVIDER_HEADER_ONLY,
		&populate_partition_values,
		&populate_partition_header,
		&hdr,
		NULL
	);

	/* Write to file */
	if (stream && (ret != EXIT_FAILURE) && (format != APP_OUT_FORMAT_TABLE)) {
		switch (format) {
		case APP_OUT_FORMAT_JSON:
		{
			JsonNode *h = NULL;
			JsonNode *p = NULL;

			ret = gen_json_data(
				dev,
				NUM_FPT_HEADER_COLS,
				NUM_FPT_HEADER_ROWS,
				&populate_fpt_values,
				&hdr,
				&h
			);

			if (ret == EXIT_SUCCESS) {
				ret = gen_json_data(
					dev,
					NUM_PARTITION_COLS,
					hdr.num_entries,
					&populate_partition_values,
					&hdr,
					&p
				);

				if (ret == EXIT_SUCCESS) {
					JsonNode *parent = NULL;

					parent = json_mkobject();
					json_append_member(parent, "header", h);
					json_append_member(parent, "partitions", p);

					print_json_obj(parent, stream);
					json_delete(parent);
				} else {
					APP_ERROR("could not create partition json");
				}
			} else {
				APP_ERROR("could not create FPT header json");
			}

			break;
		}

		default:
			APP_ERROR("invalid output format");
			break;
		}
	}

fail:
	if (stream)
		fclose(stream);

	return ret;
}

/*
 * Callback for the "mfg_info" command.
 */
int print_mfg_info(ami_device *dev, struct app_option *options)
{
	int ret = EXIT_FAILURE;
	enum app_out_format format = APP_OUT_FORMAT_TABLE;  /* default: table */
	FILE *stream = NULL;

	if (!dev || !options)
		return EXIT_FAILURE;

	if (parse_output_options(options, &format, NULL, &stream,
		NULL, NULL) == EXIT_FAILURE)
		return EXIT_FAILURE;

	/* Print manufacturing information. */
	ret = print_table_data(
		dev,
		NUM_MFG_INFO_COLS,
		NUM_MFG_INFO_ROWS,
		(format == APP_OUT_FORMAT_TABLE) ? (stream) : (NULL),
		TABLE_DIVIDER_HEADER_ONLY,
		&populate_mfg_info_values,
		&populate_mfg_info_header,
		NULL,
		NULL
	);

	if (ret)
		APP_ERROR("could not print mfg_info data");

	/* Write to file. */
	if (stream && (ret != EXIT_FAILURE) && (format != APP_OUT_FORMAT_TABLE)) {
		switch (format) {
		case APP_OUT_FORMAT_JSON:
			ret = print_json_data(
				dev,
				NUM_MFG_INFO_COLS,
				NUM_MFG_INFO_ROWS,
				stream,
				&populate_mfg_info_values,
				NULL
			);

			if (ret)
				APP_ERROR("could not create mfg_info json");

			break;

		default:
			break;
		}
	}

	if (stream)
		fclose(stream);

	return ret;
}
