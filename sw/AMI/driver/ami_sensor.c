// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_sensor.c - This file contains sensor-related functionality.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

#include "ami_top.h"
#include "ami_sensor.h"
#include "ami_utils.h"

#define DEFAULT_BDINFO_POWER (0xFF)


static struct sensor_status_name_map_t sensor_status_name_map[] = {
	{ SENSOR_NOT_PRESENT,          SENSOR_STATUS_NAME_NOT_PRESENT },
	{ SENSOR_PRESENT_AND_VALID,    SENSOR_STATUS_NAME_PRESENT     },
	{ DATA_NOT_AVAILABLE,          SENSOR_STATUS_NAME_UNAVAIL     },
	{ SENSOR_STATUS_NOT_AVAILABLE, SENSOR_STATUS_NAME_NA          },
};

char *convert_sensor_status_name_map(int status)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(sensor_status_name_map); i++) {
		if (sensor_status_name_map[i].status == status)
			return sensor_status_name_map[i].name;
	}
	return "";
}

/**
 * sdr_repo_type_to_str() - Get the human readable name of an SDR type.
 * @sdr: The SDR repo type.
 * 
 * Return: Human readable string (empty if type unknown).
 */
const char *sdr_repo_type_to_str(enum gcq_sdr_repo_type sdr)
{
	switch (sdr) {
	case SDR_TYPE_FPT:
		return "fpt";

	case SDR_TYPE_BDINFO:
		return "board info";
	
	case SDR_TYPE_TEMP:
		return "temperature";
	
	case SDR_TYPE_VOLTAGE:
		return "voltage";
	
	case SDR_TYPE_CURRENT:
		return "current";
	
	case SDR_TYPE_POWER:
		return "power";

	case SDR_TYPE_TOTAL_POWER:
		return "total power";

	default:
		break;
	}

	return "";
}

/**
 * get_flags_for_repo() - Get the appropriate command flags for a repo type.
 * @repo_type: The repo type (SID).
 * 
 * This function is to be used when a GCQ/AMC command can take various
 * forms depending on the repo type. This means we can have a single
 * command (e.g. GET_SDR) and set the appropriate flag for the repo type,
 * rather than having many commands (GET_SDR_TEMP, GET_SDR_POWER, etc.).
 * 
 * Return: The relevant command flag or 0.
 */
enum gcq_submit_cmd_req get_flags_for_repo(enum gcq_sdr_repo_type repo_type)
{
	enum gcq_submit_cmd_req ret = GCQ_CMD_FLAG_NONE;

	switch (repo_type) {
	case SDR_TYPE_BDINFO:
		ret = GCQ_CMD_FLAG_REPO_TYPE_BD_INFO;
		break;
	
	case SDR_TYPE_TEMP:
		ret = GCQ_CMD_FLAG_REPO_TYPE_TEMP;
		break;
	
	case SDR_TYPE_VOLTAGE:
		ret = GCQ_CMD_FLAG_REPO_TYPE_VOLTAGE;
		break;
	
	case SDR_TYPE_CURRENT:
		ret = GCQ_CMD_FLAG_REPO_TYPE_CURRENT;
		break;
	
	case SDR_TYPE_POWER:
		ret = GCQ_CMD_FLAG_REPO_TYPE_POWER;
		break;

	case SDR_TYPE_TOTAL_POWER:
		ret = GCQ_CMD_FLAG_REPO_TYPE_TOTAL_POWER;
		break;

        case SDR_TYPE_FPT:
		ret = GCQ_CMD_FLAG_REPO_TYPE_FPT;
		break;
	
	default:
		break;
	}

	return ret;
}

/**
 * parse_sdr() - Parse an SDR from the raw byte buffer.
 * @amc_ctrl_ctxt: Pointer to top level AMC struct.
 * @sdr_buf: Raw data buffer, including header.
 * @repo: Pointer to sdr_repo struct to be populated.
 * 
 * Return: 0 or negative error code.
 */
int parse_sdr(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t *sdr_buf,
		struct sdr_repo *repo)  /* TODO: Test with board info SDR */
{
	int i = 0;
	int ret = 0;
	int buf_index = 0;
	struct sdr_record *rec = NULL;

	if (!amc_ctrl_ctxt || !sdr_buf || !repo)
		return -EINVAL;

	/* Construct SDR header. */
	repo->repo_type   = sdr_buf[buf_index++];
	repo->repo_ver    = sdr_buf[buf_index++];
	repo->num_records = sdr_buf[buf_index++];
	repo->size        = sdr_buf[buf_index++];

	if (!repo->size)
		return -EINVAL;

	/* AMC may send empty SDR data with just the header and EoR. */
	if (repo->num_records > 0) {
		switch(repo->repo_type) {
		case SDR_TYPE_FPT:
			repo->fpt.hdr.version = sdr_buf[buf_index++];
			repo->fpt.hdr.header_size = sdr_buf[buf_index++];
			repo->fpt.hdr.entry_size = sdr_buf[buf_index++];
			repo->fpt.hdr.num_entries = sdr_buf[buf_index++];
			repo->fpt.partition = devm_kzalloc(&(amc_ctrl_ctxt->pcie_dev->dev), repo->fpt.hdr.num_entries *
				sizeof(struct fpt_partition), GFP_KERNEL);

			for(i = 0; i < repo->fpt.hdr.num_entries; i++)
			{
				/* type */
				memcpy(&repo->fpt.partition[i].type, &sdr_buf[buf_index],
				sizeof(repo->fpt.partition[i].type));
				buf_index += sizeof(repo->fpt.partition[i].type);
				/* base address */
				memcpy(&repo->fpt.partition[i].base_addr, &sdr_buf[buf_index],
				sizeof(repo->fpt.partition[i].base_addr));
				buf_index += sizeof(repo->fpt.partition[i].base_addr);
				/* partition size */
				memcpy(&repo->fpt.partition[i].partition_size, &sdr_buf[buf_index],
				sizeof(repo->fpt.partition[i].partition_size));
				buf_index += sizeof(repo->fpt.partition[i].partition_size);
			}
			break;

		case SDR_TYPE_BDINFO:
		{
			repo->bd_info = devm_kzalloc(&(amc_ctrl_ctxt->pcie_dev->dev), repo->num_records *
				sizeof(struct bd_info_record), GFP_KERNEL);

			if (!repo->bd_info)
				return -ENOMEM;

			/* eeprom version */
			repo->bd_info->eeprom_version.type = sdr_buf[buf_index++];
			repo->bd_info->eeprom_version.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->eeprom_version.bytes, &sdr_buf[buf_index], repo->bd_info->eeprom_version.len);
			buf_index += repo->bd_info->eeprom_version.len;

			/* product name */
			repo->bd_info->product_name.type = sdr_buf[buf_index++];
			repo->bd_info->product_name.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->product_name.bytes, &sdr_buf[buf_index], repo->bd_info->product_name.len);
			buf_index += repo->bd_info->product_name.len;

			/* board revsion */
			repo->bd_info->board_rev.type = sdr_buf[buf_index++];
			repo->bd_info->board_rev.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->board_rev.bytes, &sdr_buf[buf_index], repo->bd_info->board_rev.len);
			buf_index += repo->bd_info->board_rev.len;

			/* board serial */
			repo->bd_info->board_serial.type = sdr_buf[buf_index++];
			repo->bd_info->board_serial.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->board_serial.bytes, &sdr_buf[buf_index], repo->bd_info->board_serial.len);
			buf_index += repo->bd_info->board_serial.len;

			/* mac address count */
			repo->bd_info->mac_addr_count.type = sdr_buf[buf_index++];
			repo->bd_info->mac_addr_count.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->mac_addr_count.bytes, &sdr_buf[buf_index], repo->bd_info->mac_addr_count.len);
			buf_index += repo->bd_info->mac_addr_count.len;

			/* first mac address */
			repo->bd_info->first_mac_addr.type = sdr_buf[buf_index++];
			repo->bd_info->first_mac_addr.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->first_mac_addr.bytes, &sdr_buf[buf_index], repo->bd_info->first_mac_addr.len);
			buf_index += repo->bd_info->first_mac_addr.len;

			/* active state */
			repo->bd_info->active_state.type = sdr_buf[buf_index++];
			repo->bd_info->active_state.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->active_state.bytes, &sdr_buf[buf_index], repo->bd_info->active_state.len);
			buf_index += repo->bd_info->active_state.len;

			/* config mode */
			repo->bd_info->config_mode.type = sdr_buf[buf_index++];
			repo->bd_info->config_mode.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->config_mode.bytes, &sdr_buf[buf_index], repo->bd_info->config_mode.len);
			buf_index += repo->bd_info->config_mode.len;

			/* manufacturing date */
			repo->bd_info->manufacturing_date.type = sdr_buf[buf_index++];
			repo->bd_info->manufacturing_date.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->manufacturing_date.bytes, &sdr_buf[buf_index], repo->bd_info->manufacturing_date.len);
			buf_index += repo->bd_info->manufacturing_date.len;

			/* part number */
			repo->bd_info->part_number.type = sdr_buf[buf_index++];
			repo->bd_info->part_number.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->part_number.bytes, &sdr_buf[buf_index], repo->bd_info->part_number.len);
			buf_index += repo->bd_info->part_number.len;

			/* uuid */
			repo->bd_info->uuid.type = sdr_buf[buf_index++];
			repo->bd_info->uuid.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->uuid.bytes, &sdr_buf[buf_index], repo->bd_info->uuid.len);
			buf_index += repo->bd_info->uuid.len;

			/* pcie_id */
			repo->bd_info->pcie_id.type = sdr_buf[buf_index++];
			repo->bd_info->pcie_id.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->pcie_id.bytes, &sdr_buf[buf_index], repo->bd_info->pcie_id.len);
			buf_index += repo->bd_info->pcie_id.len;

			/* power mode */
			repo->bd_info->power_mode.type = sdr_buf[buf_index++];
			repo->bd_info->power_mode.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->power_mode.bytes, &sdr_buf[buf_index], repo->bd_info->power_mode.len);
			/*
			* default power value of zero (75W) overridden when length of data
			* is zero so that app can display 'N/A'
			*/
			if(repo->bd_info->power_mode.len == 0)
			{
				repo->bd_info->power_mode.bytes[ 0 ] = DEFAULT_BDINFO_POWER;
			}
			buf_index += repo->bd_info->power_mode.len;

			/* memory size */
			repo->bd_info->memory_size.type = sdr_buf[buf_index++];
			repo->bd_info->memory_size.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->memory_size.bytes, &sdr_buf[buf_index], repo->bd_info->memory_size.len);
			buf_index += repo->bd_info->memory_size.len;

			/* oem id */
			repo->bd_info->oem_id.type = sdr_buf[buf_index++];
			repo->bd_info->oem_id.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->oem_id.bytes, &sdr_buf[buf_index], repo->bd_info->oem_id.len);
			buf_index += repo->bd_info->oem_id.len;

			/* capability */
			repo->bd_info->capability.type = sdr_buf[buf_index++];
			repo->bd_info->capability.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->capability.bytes, &sdr_buf[buf_index], repo->bd_info->capability.len);
			buf_index += repo->bd_info->capability.len;

			/* mfg part number */
			repo->bd_info->mfg_part_number.type = sdr_buf[buf_index++];
			repo->bd_info->mfg_part_number.len = sdr_buf[buf_index++];
			memcpy(&repo->bd_info->mfg_part_number.bytes, &sdr_buf[buf_index], repo->bd_info->mfg_part_number.len);
			buf_index += repo->bd_info->mfg_part_number.len;
			break;
		}

		case SDR_TYPE_TEMP:
		case SDR_TYPE_VOLTAGE:
		case SDR_TYPE_CURRENT:
		case SDR_TYPE_POWER:
		case SDR_TYPE_TOTAL_POWER:
			/* Parse records. */
			repo->records = devm_kzalloc(&(amc_ctrl_ctxt->pcie_dev->dev), repo->num_records *
				sizeof(struct sdr_record), GFP_KERNEL);

			if (!repo->records)
				return -ENOMEM;

			for (i = 0; i < repo->num_records; i++) {
				rec = repo->records + i;

				rec->id = sdr_buf[buf_index++];

				/* Name */
				rec->name_type = sdr_buf[buf_index] >> SDR_TYPE_POS;
				rec->name_len = sdr_buf[buf_index] & SDR_LENGTH_MASK;
				buf_index++;
				memcpy(rec->name, &sdr_buf[buf_index], rec->name_len);
				buf_index += rec->name_len;

				/* Value */
				rec->value_type = sdr_buf[buf_index] >> SDR_TYPE_POS;
				rec->value_len = sdr_buf[buf_index] & SDR_LENGTH_MASK;
				buf_index++;
				memcpy(rec->value, &sdr_buf[buf_index], rec->value_len);
				buf_index += rec->value_len;

				/* Unit */
				rec->unit_type = sdr_buf[buf_index] >> SDR_TYPE_POS;
				rec->unit_len = sdr_buf[buf_index] & SDR_LENGTH_MASK;
				buf_index++;
				memcpy(rec->unit, &sdr_buf[buf_index], rec->unit_len);
				buf_index += rec->unit_len;

				rec->unit_mod = sdr_buf[buf_index++];
				rec->threshold_support = sdr_buf[buf_index++];

				/* Lower fatal */
				if (rec->threshold_support & THRESHOLD_LOWER_FATAL_MASK) {
					memcpy(rec->lower_fatal_limit, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				/* Lower critical */
				if (rec->threshold_support & THRESHOLD_LOWER_CRITICAL_MASK) {
					memcpy(rec->lower_crit_limit, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				/* Lower warning */
				if (rec->threshold_support & THRESHOLD_LOWER_WARNING_MASK) {
					memcpy(rec->lower_warn_limit, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				/* Upper fatal */
				if (rec->threshold_support & THRESHOLD_UPPER_FATAL_MASK) {
					memcpy(rec->upper_fatal_limit, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				/* Upper critical */
				if (rec->threshold_support & THRESHOLD_UPPER_CRITICAL_MASK) {
					memcpy(rec->upper_crit_limit, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				/* Upper warning */
				if (rec->threshold_support & THRESHOLD_UPPER_WARNING_MASK) {
					memcpy(rec->upper_warn_limit, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				rec->sensor_status = sdr_buf[buf_index++];

				if (rec->threshold_support & THRESHOLD_SENSOR_AVG_MASK) {
					memcpy(rec->avg, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				if (rec->threshold_support & THRESHOLD_SENSOR_MAX_MASK) {
					memcpy(rec->max, &sdr_buf[buf_index], rec->value_len);
					buf_index += rec->value_len;
				}

				/* Min is not supported. */
			}
			break;

		default:
			return -EINVAL;
			break;
		}
	} else {
		ret = -ENODATA;
		AMI_WARN(amc_ctrl_ctxt,
			"Received empty data for %s (SDR type %d)",
			sdr_repo_type_to_str(repo->repo_type),
			repo->repo_type
		);
	}

	/* Verify End of Repo (EoR) */
	if (((sdr_buf[buf_index] << 16) | (sdr_buf[buf_index + 1] << 8) |
			(sdr_buf[buf_index + 2])) != SDR_EOR)
		return -EILSEQ;  /* Illegal byte sequence */

	return ret;
}

/*
 * TODO: Currently, the GET_SDR_SIZE API is useless because AMC
 * checks the size against a hardcoded value (512).
 */
#define SDR_RESP_LEN 4096

/**
 * get_sdr_size() - Perform the GET_SDR_SIZE API call.
 * @amc_ctrl_ctxt: The top level AMC struct instance.
 * @repo_type: The repo type.
 * @sdr_size: Pointer to a variable which will hold the returned size.
 * 
 * Return: 0 or negative error code.
 */
int get_sdr_size(struct amc_control_ctxt *amc_ctrl_ctxt, enum gcq_sdr_repo_type repo_type,
		uint16_t *sdr_size)
{
	int ret = 0;
	char *sdr_raw_buf = NULL;
	int buf_index = 0;

	int rid = 0;
	enum gcq_sdr_completion_code completion_code = SDR_CODE_NOT_AVAILABLE;

	if (!amc_ctrl_ctxt || !sdr_size)
		return -EINVAL;

	sdr_raw_buf = vzalloc(sizeof(char) * SDR_RESP_LEN);
	if (!sdr_raw_buf) {
		AMI_ERR(amc_ctrl_ctxt, "Failed to allocate memory buffer for sdr_raw_buf");
		ret = -ENOMEM;
		goto done;
	}

	ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_GET_SDR_SIZE,
		get_flags_for_repo(repo_type), sdr_raw_buf, SDR_RESP_LEN);

	if (ret) {
		AMI_ERR(amc_ctrl_ctxt, "Submit command failed");
		ret = -EIO;
		goto done;
	}

	completion_code = sdr_raw_buf[buf_index];
	if (completion_code != SDR_CODE_OP_SUCCESS) {
		AMI_ERR(amc_ctrl_ctxt, "Completion code : %d", completion_code);
		ret = -EINVAL;
		goto done;
	}
	buf_index++;

	rid = (uint8_t)sdr_raw_buf[buf_index];
	if (rid != repo_type) {
		AMI_ERR(amc_ctrl_ctxt, "Sensor response ID %d does not match sensor request ID %d",
			rid, AMC_PROXY_CMD_SENSOR_REQUEST_GET_SIZE);
		ret = -EINVAL;
		goto done;
	}
	buf_index++;

	*sdr_size = (uint16_t)(sdr_raw_buf[buf_index + 1] << 8);
	*sdr_size |= (uint16_t)(sdr_raw_buf[buf_index]);

done:
	if (sdr_raw_buf)
		vfree(sdr_raw_buf);
	
	if (ret == SUCCESS)
		AMI_VDBG(amc_ctrl_ctxt, "Successfully fetched SDR size");
	else
		AMI_ERR(amc_ctrl_ctxt, "Failed to fetch SDR size");
	
	return ret;
}

/**
 * get_sdr() - Perform the GET_SDR API call.
 * @amc_ctrl_ctxt: The top level AMC struct instance.
 * @repo_type: The repo type.
 * @repo: Pointer to sdr_repo struct which will hold the SDR records.
 * 
 * Return: 0 or negative error code.
 */
int get_sdr(struct amc_control_ctxt *amc_ctrl_ctxt, enum gcq_sdr_repo_type repo_type,
		struct sdr_repo *repo)
{
	int ret = 0;
	int buf_index = 0;
	char *sdr_raw_buf = NULL;

	int rid = 0;
	enum gcq_sdr_completion_code completion_code = SDR_CODE_NOT_AVAILABLE;

	if (!amc_ctrl_ctxt || !repo)
		return -EINVAL;

	/* TODO: Get the SDR size. */

	sdr_raw_buf = vzalloc(sizeof(char) * SDR_RESP_LEN);
	if (!sdr_raw_buf) {
		AMI_ERR(amc_ctrl_ctxt, "Failed to allocate memory buffer for sdr_raw_buf");
		ret = -ENOMEM;
		goto done;
	}

	ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_GET_SDR,
			get_flags_for_repo(repo_type), sdr_raw_buf, SDR_RESP_LEN);
	if (ret) {
		AMI_ERR(amc_ctrl_ctxt, "Submit command failed");
		ret = -EIO;
		goto done;
	}

	/* Parse response */
	completion_code = sdr_raw_buf[buf_index];
	if (completion_code != SDR_CODE_OP_SUCCESS) {
		AMI_ERR(amc_ctrl_ctxt, "Completion code : %d", completion_code);
		ret = -EINVAL;
		goto done;
	}
	buf_index++;

	rid = (uint8_t)sdr_raw_buf[buf_index];
	if (rid != repo_type) {
		AMI_ERR(amc_ctrl_ctxt, "Sensor response ID %d does not match sensor request ID %d",
			rid, AMC_PROXY_CMD_SENSOR_REQUEST_GET_SIZE);
		ret = -EINVAL;
		goto done;
	}
	buf_index++;

	/* Parse records */
	ret = parse_sdr(amc_ctrl_ctxt, sdr_raw_buf + 1, repo);  /* +1 to omit completion code */

done:
	if (sdr_raw_buf)
		vfree(sdr_raw_buf);
	
	if (ret == SUCCESS)
		AMI_VDBG(amc_ctrl_ctxt, "Successfully fetched SDR");
	else
		AMI_ERR(amc_ctrl_ctxt, "Failed to fetch SDR");
	
	return ret;
}

#define SDR_VALUE_SIZE_BYTE  1
#define SDR_VALUE_SIZE_WORD  2
#define SDR_VALUE_SIZE_DWORD 4

/**
 * make_val() - Return a signed number representation of an SDR record value.
 * @type: The type of the record value (binary).
 * @size: The size of the requested value in bytes.
 * @data: Pointer to the value data buffer.
 *
 * Return: The converted value (0 on fail).
 */
long make_val(enum sdr_value_type type, uint8_t size, uint8_t *data)
{
	if ((type != SDR_VALUE_TYPE_BINARY) || !data)
		return 0;

	switch (size) {
	case SDR_VALUE_SIZE_BYTE:
		return (long)(*(uint8_t *)data);

	case SDR_VALUE_SIZE_WORD:
		return (long)(*(uint16_t *)data);

	case SDR_VALUE_SIZE_DWORD:
		return (long)(*(uint32_t *)data);

	default:
		break;
	}

	return 0;
}

/*
 * Note that board info is static throughout the lifetime of a device.
 * It only needs to be read once, at the sensor discover stage.
 */
static enum gcq_sdr_repo_type discovery_repos[NUM_SENSOR_REPOS] = {
	SDR_TYPE_TEMP,
	SDR_TYPE_VOLTAGE,
	SDR_TYPE_CURRENT,
	SDR_TYPE_POWER,
	SDR_TYPE_TOTAL_POWER,
	SDR_TYPE_BDINFO,
        SDR_TYPE_FPT,
};

/**
 * discover_sensors() - Get the SDR for all repo types.
 * @pf_dev: Pointer to PCI device data.
 * @empty_sdr_count: Pointer to empty SDR counter.
 * 
 * This function should be called once for every PCI device. It will
 * allocate the necessary memory to store the fetched sensor data
 * and populate it with sensor data for each type of supported sensor.
 * This uses managed allocation and does not need to be freed by the caller.
 * 
 * This function updates the `last_update` member of each SDR repo.
 * 
 * Return: 0 or negative error code.
 */
int discover_sensors(struct pf_dev_struct *pf_dev, int *empty_sdr_count)
{
	int i = 0;
	int ret = 0;

	if (!pf_dev  || !empty_sdr_count)
		return -EINVAL;
	
	pf_dev->num_sensor_repos = NUM_SENSOR_REPOS;
	pf_dev->sensor_repos = devm_kzalloc(&(pf_dev->pci->dev),
		NUM_SENSOR_REPOS * sizeof(struct sdr_repo), GFP_KERNEL);
	
	if (!pf_dev->sensor_repos)
		return -ENOMEM;

	for (i = 0; i < NUM_SENSOR_REPOS; i++) {
		ret = get_sdr(pf_dev->amc_ctrl_ctxt, discovery_repos[i], &(pf_dev->sensor_repos[i]));

		if(ret == -ENODATA) {
			*empty_sdr_count = *empty_sdr_count + 1;
			ret = SUCCESS;
		}
		else if (ret) {
			break;
		}
		
		pf_dev->sensor_repos[i].last_update = jiffies;
	}

	/* Free sensors if we failed. */
	if (ret)
		delete_sensors(pf_dev);

	return ret;
}

/**
 * delete_repo_records() - Delete all records from a sensor repo.
 * @pf_dev: The device this repo belongs to.
 * @repo: Pointer to repo.
 *
 * Note that the repo itself is not deleted.
 *
 * Return: None
 */
static void delete_repo_records(struct pf_dev_struct *pf_dev, struct sdr_repo *repo)
{
	if (!pf_dev || !repo)
		return;

	switch (repo->repo_type) {
	case SDR_TYPE_FPT:
		if (repo->fpt.partition) {
			devm_kfree(&(pf_dev->pci->dev), repo->fpt.partition);
			repo->fpt.partition = NULL;
			repo->num_records = 0;
		}
		break;
	
	case SDR_TYPE_BDINFO:
		if (repo->bd_info) {
			devm_kfree(&(pf_dev->pci->dev), repo->bd_info);
			repo->bd_info = NULL;
			repo->num_records = 0;
		}
		break;
	
	case SDR_TYPE_TEMP:
	case SDR_TYPE_VOLTAGE:
	case SDR_TYPE_CURRENT:
	case SDR_TYPE_POWER:
	case SDR_TYPE_TOTAL_POWER:
		if (repo->records) {
			devm_kfree(&(pf_dev->pci->dev), repo->records);
			repo->records = NULL;
			repo->num_records = 0;
		}
		break;
	
	default:
		break;
	}
}

/**
 * update_sdr() - Update a specific SDR.
 * @pf_dev: The device this SDR belongs to.
 * @repo_type: Repo type.
 *
 * The existing repo only gets updated on success; if the function fails
 * the old data is not altered in any way.
 *
 * Return: 0 or negative error code.
 */
int update_sdr(struct pf_dev_struct *pf_dev, enum gcq_sdr_repo_type repo_type)
{
	int ret = 0;
	struct sdr_repo *repo = NULL;
	struct sdr_repo *new_repo = NULL;

	if (!pf_dev)
		return -EINVAL;

	repo = find_sdr_repo(pf_dev->sensor_repos,
		pf_dev->num_sensor_repos, repo_type);

	if (!repo)
		return -ENODATA;

	new_repo = kzalloc(sizeof(struct sdr_repo), GFP_KERNEL);

	if (!new_repo)
		return -ENOMEM;

	ret = get_sdr(pf_dev->amc_ctrl_ctxt, repo_type, new_repo);

	if (!ret) {
		delete_repo_records(pf_dev, repo);
		memcpy(repo, new_repo, sizeof(struct sdr_repo));
	}

	kfree(new_repo);
	return ret;
}

/**
 * delete_sensors() - Delete ASDM sensor data.
 * @pf_dev: Pointer to PCI device data.
 * 
 * Note that this function should normally not be called.
 * 
 * Return: None.
 */
void delete_sensors(struct pf_dev_struct *pf_dev)
{
	int i = 0;

	if (!pf_dev || !pf_dev->sensor_repos || (pf_dev->num_sensor_repos == 0))
		return;

	for (i = 0; i < NUM_SENSOR_REPOS; i++) 
		delete_repo_records(pf_dev, &(pf_dev->sensor_repos[i]));

	devm_kfree(&(pf_dev->pci->dev), pf_dev->sensor_repos);
}

/**
 * find_sdr_record() - Find the SDR record for a pamiicular sensor.
 * @sensor_repos: List of all collected ASDM sensor repos.
 * @num_sensor_repos: Total number of ASDM sensor repos.
 * @type: Sensor type. Same as the SDR repo type.
 * @sid: Sensor ID (index). Same as the hwmon channel it belongs to.
 *
 * Return: The matched SDR record or NULL.
 */
struct sdr_record *find_sdr_record(struct sdr_repo *sensor_repos,
		uint8_t num_sensor_repos, enum gcq_sdr_repo_type type, int sid)
{
	int i = 0, j = 0;

	if (!sensor_repos) 
		return NULL;
	
	for (i = 0; i < num_sensor_repos; i++) {
		if (sensor_repos[i].repo_type != type)
			continue;
		
		for (j = 0; j < sensor_repos[i].num_records; j++) {
			/* channel = id - 1 */
			if (sensor_repos[i].records[j].id - 1 == sid) {
				return &sensor_repos[i].records[j];
			}
		}
	}

	return NULL;
}

/**
 * find_sdr_repo() - Find the SDR repo from a list of repos.
 * @sensor_repos: List of all collected ASDM sensor repos.
 * @num_sensor_repos: Total number of ASDM sensor repos.
 * @type: Repo type.
 *
 * NOTE: This function may return an empty repo with no records!
 * 
 * Return: The matched SDR repo or NULL.
 */
struct sdr_repo *find_sdr_repo(struct sdr_repo *sensor_repos,
	uint8_t num_sensor_repos, enum gcq_sdr_repo_type type)
{
	int i = 0;

	if (!sensor_repos)
		return NULL;
	
	for (i = 0; i < num_sensor_repos; i++) {
		if (sensor_repos[i].repo_type == type)
			return &sensor_repos[i];
	}

	return NULL;
}

/**
 * get_all_sensors() - Perform the ASDM GET_ALL_SENSOR_DATA API call.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @gcq_cmd: The CMD code to submit; used to populate payload fields.
 * @sensor_repo: Pointer to parent repo. Repo type must be appropriate for cmd.
 *
 * Note that this function does not allocate any memory and does not discover
 * any new sensors. It simply fetches all available sensor data and updates
 * the values in the, already existing, sensor repos which are passed as an
 * argument.
 * 
 * This function updates the `last_update` member of each SDR repo.
 *
 * Return: 0 on success or negative error code.
 */
static int get_all_sensors(struct amc_control_ctxt *amc_ctrl_ctxt, enum gcq_submit_cmd_req gcq_cmd,
		struct sdr_repo *sensor_repo)
{
	int ret = SUCCESS;
	char *sdr_raw_buf = NULL;
	enum gcq_sdr_completion_code completion_code = SDR_CODE_NOT_AVAILABLE;

	int rid = 0, sid = 0, i = 0;
	uint8_t size = 0;
	uint8_t val_len = 0;
	int num_sensor = 0;
	int buf_index = 0, rec_start_buf_index = 0;
	struct sdr_record *rec = NULL;

	if (!amc_ctrl_ctxt || !sensor_repo)
		return -EINVAL;

	sdr_raw_buf = vzalloc(sizeof(char) * SENSOR_RSP_LEN);
	if (!sdr_raw_buf) {
		AMI_ERR(amc_ctrl_ctxt, "Failed to allocate memory buffer for sdr_raw_buf");
		ret = -ENOMEM;
		goto done;
	}

	ret = submit_gcq_command(amc_ctrl_ctxt, gcq_cmd, GCQ_CMD_FLAG_NONE,
			sdr_raw_buf, SENSOR_RSP_LEN);
	if (ret) {
		AMI_ERR(amc_ctrl_ctxt, "Submit command failed");
		ret = -EIO;
		goto done;
	}

	completion_code = sdr_raw_buf[buf_index++];
	if (completion_code != SDR_CODE_OP_SUCCESS) {
		AMI_ERR(amc_ctrl_ctxt, "Completion code : %d", completion_code);
		ret = -EINVAL;
		goto done;
	}

	sid = get_sid(gcq_cmd, GCQ_CMD_FLAG_NONE);
	rid = (uint8_t)sdr_raw_buf[buf_index++];
	if (get_rid(rid) != sid) {
		AMI_ERR(amc_ctrl_ctxt,
			"Sensor response ID %d does not match sensor request ID %d",
			rid, sid);
		ret = -EINVAL;
		goto done;
	}

	size = sdr_raw_buf[buf_index++];

	/* Parse sensor values */
	num_sensor = 0;
	rec_start_buf_index = buf_index;

	while (buf_index < rec_start_buf_index + size) { 
		val_len = sdr_raw_buf[buf_index++];
		rec = find_sdr_record(sensor_repo, 1, rid, num_sensor);

		if (!rec) {
			/*
			 * Each record contains len (1), value (len), max (len),
			 * average (len), status (1).
			 */
			buf_index += (SDR_PARSE_BUF_STATUS_INDEX * val_len) + 1;
			num_sensor++;
			continue;
		}

		rec->value_len = val_len;

		for (i = SDR_PARSE_BUF_INST_INDEX; i < SDR_PARSE_BUF_STATUS_INDEX; i++) {
			switch (i)
			{
			case SDR_PARSE_BUF_INST_INDEX:
				memset(rec->value, 0x00, SDR_VALUE_MAX_LEN);
				memcpy(rec->value, &sdr_raw_buf[buf_index], val_len);
				break;
			
			case SDR_PARSE_BUF_MAX_INDEX:
				memset(rec->max, 0x00, SDR_THRESHOLD_MAX_LEN);
				memcpy(rec->max, &sdr_raw_buf[buf_index], val_len);
				break;
			
			case SDR_PARSE_BUF_AVG_INDEX:
				memset(rec->avg, 0x00, SDR_THRESHOLD_MAX_LEN);
				memcpy(rec->avg, &sdr_raw_buf[buf_index], val_len);
				break;
			
			default:
				break;
			}

			buf_index += val_len;
		}

		rec->sensor_status = sdr_raw_buf[buf_index++];
		num_sensor++;
	}

done:
	if (sdr_raw_buf)
		vfree(sdr_raw_buf);

	if (ret == SUCCESS) {
		AMI_VDBG(amc_ctrl_ctxt, "Successfully fetched sensors");
		sensor_repo->last_update = jiffies;
	} else {
		AMI_ERR(amc_ctrl_ctxt, "Failed to fetch sensors");
	}

	return ret;
}

/* CG TODO: Add mutex */

/**
 * read_sensors() - Wrapper function around `get_all_sensors` which has the
 *                   added option of not reading sensors unless they are "stale".
 * @pf_dev: Pointer to top level PCI data struct.
 * @gcq_cmd: The CMD code to submit; used to populate payload fields.
 * @fresh: boolean indicating if the value came from the cache or over GCQ
 * 
 * Return: 0 or negative error code.
 */
static int read_sensors(struct pf_dev_struct *pf_dev,
	enum gcq_submit_cmd_req gcq_cmd, bool *fresh)
{
	unsigned long stamp = 0;
	unsigned long delta = 0;
	struct sdr_repo *repo = NULL;
	enum gcq_sdr_repo_type repo_type = SDR_TYPE_MAX;

	/* `fresh` may be NULL */

	if (!pf_dev)
		return -EINVAL;

	switch (gcq_cmd) {
	case GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR:
		repo_type = SDR_TYPE_TEMP;
		break;
	
	case GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR:
		repo_type = SDR_TYPE_VOLTAGE;
		break;
	
	case GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR:
		repo_type = SDR_TYPE_CURRENT;
		break;
	
	case GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR:
		repo_type = SDR_POWER_TYPE;
		break;
	
	default:
		break;
	}

	if (repo_type == SDR_TYPE_MAX)
		return -EINVAL;

	repo = find_sdr_repo(
		pf_dev->sensor_repos,
		pf_dev->num_sensor_repos,
		repo_type
	);

	if (!repo)
		return -EINVAL;

	stamp = jiffies;
	delta = (long)stamp - (long)repo->last_update;

	if ((pf_dev->sensor_refresh == 0 ) || ((delta * 1000 / HZ) > pf_dev->sensor_refresh)) {
		if (fresh)
			*fresh = true;

		return get_all_sensors(
			pf_dev->amc_ctrl_ctxt,
			gcq_cmd,
			repo
		);
	}
	
	if (fresh)
		*fresh = false;

	return 0;
}

/**
 * read_thermal_sensors() - Retrieve all temperature sensor readings.
 * @pf_dev: Pointer to top level PCI data struct.
 * @fresh: boolean indicating if the value came from the cache or over GCQ
 *
 * Like the `get_all_sensors` function, this simply populates data for
 * previously discovered sensor repos. It does not fetch any new sensors.
 *
 * Return: 0 on success or negative error code.
 */
int read_thermal_sensors(struct pf_dev_struct *pf_dev, bool *fresh)
{
	return read_sensors(
		pf_dev,
		GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR,
		fresh
	);
}

/**
 * read_voltage_sensors() - Retrieve all voltage sensor readings.
 * @pf_dev: Pointer to top level PCI data struct.
 * @fresh: boolean indicating if the value came from the cache or over GCQ
 *
 * Like the `get_all_sensors` function, this simply populates data for
 * previously discovered sensor repos. It does not fetch any new sensors.
 *
 * Return: 0 on success or negative error code.
 */
int read_voltage_sensors(struct pf_dev_struct *pf_dev, bool *fresh)
{
	return read_sensors(
		pf_dev,
		GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR,
		fresh
	);

}

/**
 * read_current_sensors() - Retrieve all current sensor readings.
 * @pf_dev: Pointer to top level PCI data struct.
 * @fresh: boolean indicating if the value came from the cache or over GCQ
 *
 * Like the `get_all_sensors` function, this simply populates data for
 * previously discovered sensor repos. It does not fetch any new sensors.
 *
 * Return: 0 on success or negative error code.
 */
int read_current_sensors(struct pf_dev_struct *pf_dev, bool *fresh)
{
	return read_sensors(
		pf_dev,
		GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR,
		fresh
	);

}

/**
 * read_power_sensors() - Retrieve all power sensor readings.
 * @pf_dev: Pointer to top level PCI data struct.
 * @fresh: boolean indicating if the value came from the cache or over GCQ
 *
 * Like the `get_all_sensors` function, this simply populates data for
 * previously discovered sensor repos. It does not fetch any new sensors.
 *
 * Return: 0 on success or negative error code.
 */
int read_power_sensors(struct pf_dev_struct *pf_dev, bool *fresh)
{
	return read_sensors(
		pf_dev,
		GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR,
		fresh
	);
}

/*
 * read_fpt_hdr() - Retrieve the FPT header.
 * @pf_dev: Pointer to top level PCI data struct.
 * @hdr: Pointer to populated with the header.
 *
 * Returns the stored FTP header
 *
 * Return: 0 on success or negative error code.
 */
int read_fpt_hdr(struct pf_dev_struct *pf_dev, struct fpt_header *hdr)
{
        struct sdr_repo *repo = NULL;

        if (!pf_dev || !hdr)
                return -EINVAL;

        repo = find_sdr_repo(
		pf_dev->sensor_repos,
		pf_dev->num_sensor_repos,
		SDR_TYPE_FPT);

        if (!repo)
		return -EINVAL;

	if (!(repo->num_records))
		return -ENODATA;

        *hdr = repo->fpt.hdr;
        return 0;
}

/*
 * read_fpt_partition() - Retrieve a FPT partition.
 * @pf_dev: Pointer to top level PCI data struct.
 * @partition_id: The partition to fetch.
 * @partition: Pointer to populated with the partition information.
 *
 * Returns a stored FTP partition.
 *
 * Return: 0 on success or negative error code.
 */
int read_fpt_partition(struct pf_dev_struct *pf_dev,
		       uint32_t partition_id,
		       struct fpt_partition *partition)
{
        struct sdr_repo *repo = NULL;

        if (!pf_dev || !partition)
                return -EINVAL;

        repo = find_sdr_repo(pf_dev->sensor_repos,
		             pf_dev->num_sensor_repos,
                             SDR_TYPE_FPT);
        if (!repo)
                return -EINVAL;
	
	if (!(repo->num_records))
		return -ENODATA;

        if (partition_id > (repo->fpt.hdr.num_entries - 1))
                return -EINVAL;

        *partition = repo->fpt.partition[partition_id];

        return 0;
}

/*
 * read_board_info() - Retrieve the board information.
 * @pf_dev: Pointer to top level PCI data struct.
 * @bd_info: Pointer to populated with the board info data.
 *
 * Returns the stored board information.
 *
 * Return: 0 on success or negative error code.
 */
int read_board_info(struct pf_dev_struct *pf_dev, struct bd_info_record *bd_info)
{
        struct sdr_repo *repo = NULL;

        if (!pf_dev || !bd_info)
                return -EINVAL;

        repo = find_sdr_repo(pf_dev->sensor_repos,
		             pf_dev->num_sensor_repos,
		             SDR_TYPE_BDINFO);

        if (!repo)
		return -EINVAL;

	if (!(repo->num_records))
		return -ENODATA;

        *bd_info = *repo->bd_info;
        return 0;
}
