// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_sysfs.c - This file contains sysfs-related logic for the AMI driver.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/device.h>
#include <linux/kernel.h>

#include "ami.h"
#include "ami_top.h"
#include "ami_sysfs.h"
#include "ami_pci_dbg.h"
#include "ami_sensor.h"

#define FPGA_CONFIG_MODE_QSPIX4         (7)
#define FPGA_CONFIG_MODE_OSPI           (8)
#define MAX_POWER_MODE_75W              (0)
#define MAX_POWER_MODE_150W             (1)
#define MAX_POWER_MODE_225W             (2)
#define MAX_POWER_MODE_350W             (3)

/**
 * struct dev_state_map - Map of device state to human readable string.
 * @state: Device state.
 * @name: String representation of device state.
 */
struct dev_state_map {
	enum pf_dev_state  state;
	const char        *name;
};

static const struct dev_state_map state_names[] = {
	{ PF_DEV_STATE_INIT,         STATE_NAME_INIT         },
	{ PF_DEV_STATE_READY,        STATE_NAME_READY        },
	{ PF_DEV_STATE_MISSING_INFO, STATE_NAME_MISSING_INFO },
	{ PF_DEV_STATE_NO_AMC,       STATE_NAME_NO_AMC       },
	{ PF_DEV_STATE_INIT_ERROR,   STATE_NAME_INIT_ERROR   },
	{ PF_DEV_STATE_SHUTDOWN,     STATE_NAME_SHUTDOWN     },
	{ PF_DEV_STATE_COMPAT,       STATE_NAME_COMPAT       },
};

/**
 * get_state_name() - Get the string representation of a device state.
 * @state: Device state to lookup.
 * 
 * Return: Human readable string (empty string when state not found).
 */
static const char *get_state_name(enum pf_dev_state state)
{
	int i = 0;
	for (i = 0; i < ARRAY_SIZE(state_names); i++) {
		if (state_names[i].state == state)
			return state_names[i].name;
	}
	return "";
}

/**
 * logic_uuid_show() - Sysfs read callback.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t logic_uuid_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;

	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		ret = sprintf(
			buf,
			"%s\n",
			pf_dev->endpoints->logic_uuid_str
		);
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}
static DEVICE_ATTR_RO(logic_uuid);

/*
 * NOTE: We create the below sysfs attributes because on some systems (RHEL),
 * the pre-existing sysfs nodes (current_link_speed, current_link_width, etc...)
 * return invalid values (e.g. "Unknown speed").
 */

/**
 * link_speed_max_show() - Sysfs read callback.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t link_speed_max_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;
	
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		ret = sprintf(
			buf,
			"%hhd\n",
			pf_dev->pcie_config->cap->expected_pcie_link_speed
		);
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}
static DEVICE_ATTR_RO(link_speed_max);

/**
 * link_speed_current_show() - Sysfs read callback.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t link_speed_current_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;
	
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		ret = sprintf(
			buf,
			"%hhd\n",
			pf_dev->pcie_config->cap->current_pcie_link_speed
		);
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}
static DEVICE_ATTR_RO(link_speed_current);

/**
 * link_width_max_show() - Sysfs read callback.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t link_width_max_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;
	
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		ret = sprintf(
			buf,
			"%hhd\n",
			pf_dev->pcie_config->cap->expected_pcie_link_width
		);
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}
static DEVICE_ATTR_RO(link_width_max);

/**
 * link_width_current_show() - Sysfs read callback.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t link_width_current_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;
	
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		ret = sprintf(
			buf,
			"%hhd\n",
			pf_dev->pcie_config->cap->current_pcie_link_width
		);
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}
static DEVICE_ATTR_RO(link_width_current);

/**
 * dev_state_show() - Sysfs read callback for 'dev_state' attribute.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t dev_state_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;
	
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		ret = sprintf(
			buf,
			"%s\n",
			get_state_name(pf_dev->state)
		);
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}
static DEVICE_ATTR_RO(dev_state);

/**
 * dev_name_show() - Sysfs read callback for 'dev_name' attribute.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t dev_name_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;
	struct bd_info_record *bd_info_record = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;

	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (!pf_dev)
		return -ENODEV;

	/* Must use dynamically allocated memory here due to a large struct. */
	bd_info_record = vzalloc(sizeof(struct bd_info_record));

	if (!bd_info_record) {
		put_pf_dev_entry(pf_dev);
		return -ENOMEM;
	}

	ret = read_board_info(pf_dev, bd_info_record);

	if (!ret) {
		ret = sprintf(
			buf,
			"%s",
			bd_info_record->product_name.bytes
		);
	} else {
		/* Try to use the device ID to map to a device name. */
		ret = sprintf(
			buf,
			"%s\n",
			pcie_device_id_to_str(pf_dev->pcie_config->header->device_id)
		);
	}

	put_pf_dev_entry(pf_dev);
	vfree(bd_info_record);
	return ret;
}
static DEVICE_ATTR_RO(dev_name);

/**
 * amc_version_show() - Sysfs read callback for 'amc_Version' attribute.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t amc_version_show(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;
	
	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (pf_dev) {
		if (pf_dev->amc_ctrl_ctxt) {
			/* Format is MAJOR.MINOR.PATCH +COMMITS *CHANGES */
			ret = sprintf(
				buf,
				"%hhd.%hhd.%hhd +%hd *%hhd\n",
				pf_dev->amc_ctrl_ctxt->version.ver_major,
				pf_dev->amc_ctrl_ctxt->version.ver_minor,
				pf_dev->amc_ctrl_ctxt->version.ver_patch,
				pf_dev->amc_ctrl_ctxt->version.dev_commits,
				pf_dev->amc_ctrl_ctxt->version.local_changes
			);
		}
		put_pf_dev_entry(pf_dev);
	} else {
		ret = -ENODEV;
	}
	
	return ret;
}
static DEVICE_ATTR_RO(amc_version);

/**
 * enum sysfs_mfg_field - List of exposed EEPROM fields.
 * @SYSFS_MFG_EEPROM_VERSION: The eeprom version.
 * @SYSFS_MFG_PRODUCT_NAME: the product name.
 * @SYSFS_MFG_BOARD_REV: the board revision.
 * @SYSFS_MFG_BOARD_SERIAL: the board serial number.
 * @SYSFS_MFG_MEMORY_SIZE: Max memory in GB.
 * @SYSFS_MFG_PART_NUM: Board part number.
 * @SYSFS_MFG_M_PART_NUM: Manufacturer part number.
 * @SYSFS_MFG_MAC_ADDR_C: Number of MAC IDs.
 * @SYSFS_MFG_MAC_ADDR: Primary MAC ID.
 * @SYSFS_MFG_MAC_ADDR_N:  Number of MAC IDs.
 * @SYSFS_MFG_ACTIVE_STATE: ‘A’ = Active, ‘P’ = Passive.
 * @SYSFS_MFG_CONFIG_MODE: 07: Master SPI x4 (QSPIx4), 08: OSPI.
 * @SYSFS_MFG_M_DATE: Manufacturing Date.
 * @SYSFS_MFG_UUID: Used to uniquely ID the product.
 * @SYSFS_MFG_PCIE_ID: Vendor ID, Device ID, SubVendor ID, SubDevice ID.
 * @SYSFS_MFG_POWER_MODE: Max power mode.
 * @SYSFS_MFG_OEM_ID: OEM ID.
 * @SYSFS_MFG_CAPABILITY: Capability word.
 */
enum sysfs_mfg_field {
	SYSFS_MFG_EEPROM_VERSION,
	SYSFS_MFG_PRODUCT_NAME,
	SYSFS_MFG_BOARD_REV,
	SYSFS_MFG_BOARD_SERIAL,
	SYSFS_MFG_MEMORY_SIZE,
	SYSFS_MFG_PART_NUM,
	SYSFS_MFG_M_PART_NUM,
	SYSFS_MFG_MAC_ADDR_C,
	SYSFS_MFG_MAC_ADDR,
	SYSFS_MFG_MAC_ADDR_N,
	SYSFS_MFG_ACTIVE_STATE,
	SYSFS_MFG_CONFIG_MODE,
	SYSFS_MFG_M_DATE,
	SYSFS_MFG_UUID,
	SYSFS_MFG_PCIE_ID,
	SYSFS_MFG_POWER_MODE,
	SYSFS_MFG_OEM_ID,
	SYSFS_MFG_CAPABILITY,
};

/**
 * struct mfg_attribute - sysfs attribute for EEPROM fields
 * @attr: Low level attribute struct.
 * @var: EEPROM field identifier.
 */
struct mfg_attribute {
	struct device_attribute attr;
	enum sysfs_mfg_field var;
};

/*
 * Wrapper around sysfs attribute so we can store additional context.
 * This allows reusing a single 'show' function.
 */
#define DEVICE_MFG_ATTR(_name, _show, _var) \
	struct mfg_attribute dev_attr_##_name = \
		{ __ATTR(_name, 0444, _show, NULL), _var }

/**
 * read_mfg_field() - Sysfs read callback for EEPROM attributes.
 * @dev: Device this attribute belongs to.
 * @da: Pointer to device attribute struct.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t read_mfg_field(struct device *dev, struct device_attribute *da,
		char *buf)
{
	int ret = 0;
	struct sdr_repo *repo = NULL;
	struct pf_dev_struct *pf_dev = NULL;
	struct mfg_attribute *attr = NULL;

	if (!dev || !da || !buf)
		return -EINVAL;

	pf_dev = get_pf_dev_entry(dev, PF_DEV_CACHE_DEV);

	if (!pf_dev)
		return -ENODEV;
	
	repo = find_sdr_repo(
		pf_dev->sensor_repos,
		pf_dev->num_sensor_repos,
		SDR_TYPE_BDINFO
	);

	if (!repo || !(repo->num_records)) {
		put_pf_dev_entry(pf_dev);
		return -EINVAL;
	}

	attr = container_of(da, struct mfg_attribute, attr);

	switch (attr->var) {
	case SYSFS_MFG_EEPROM_VERSION:
		ret = snprintf(
			buf,
			repo->bd_info->eeprom_version.len + 1,
			"%s\n",
			repo->bd_info->eeprom_version.bytes
		);
		break;
	
	case SYSFS_MFG_PRODUCT_NAME:
		ret = snprintf(
			buf,
			repo->bd_info->product_name.len + 1,
			"%s\n",
			repo->bd_info->product_name.bytes
		);
		break;
	
	case SYSFS_MFG_BOARD_REV:
		ret = snprintf(
			buf,
			repo->bd_info->board_rev.len + 1,
			"%s\n",
			repo->bd_info->board_rev.bytes
		);
		break;
	
	case SYSFS_MFG_BOARD_SERIAL:
		ret = snprintf(
			buf,
			repo->bd_info->board_serial.len + 1,
			"%s\n",
			repo->bd_info->board_serial.bytes
		);
		break;
	
	case SYSFS_MFG_MEMORY_SIZE:
		if (repo->bd_info->memory_size.len)
			ret = snprintf(
				buf,
				repo->bd_info->memory_size.len + 1,
				"%s\n",
				repo->bd_info->memory_size.bytes
			);
		else
			ret = sprintf(buf, "%s\n", "N/A");
		break;
	
	case SYSFS_MFG_PART_NUM:
		ret = snprintf(
			buf,
			repo->bd_info->part_number.len + 1,
			"%s\n",
			repo->bd_info->part_number.bytes
		);
		break;
	
	case SYSFS_MFG_M_PART_NUM:
		if (repo->bd_info->mfg_part_number.len)
			ret = snprintf(
				buf,
				repo->bd_info->mfg_part_number.len + 1,
				"%s\n",
				repo->bd_info->mfg_part_number.bytes
			);
		else
			ret = sprintf(buf, "%s\n", "N/A");
		break;
	
	case SYSFS_MFG_MAC_ADDR_C:
		ret = sprintf(
			buf,
			"%d\n",
			repo->bd_info->mac_addr_count.bytes[0]
		);
		break;
	
	case SYSFS_MFG_MAC_ADDR:
		ret = sprintf(
			buf,
			"%02x:%02x:%02x:%02x:%02x:%02x\n",
			repo->bd_info->first_mac_addr.bytes[0],
			repo->bd_info->first_mac_addr.bytes[1],
			repo->bd_info->first_mac_addr.bytes[2],
			repo->bd_info->first_mac_addr.bytes[3],
			repo->bd_info->first_mac_addr.bytes[4],
			repo->bd_info->first_mac_addr.bytes[5]
		);
		break;
	
	case SYSFS_MFG_MAC_ADDR_N:
		ret = sprintf(
			buf,
			"%02x:%02x:%02x:%02x:%02x:%02x\n",
			repo->bd_info->first_mac_addr.bytes[0],
			repo->bd_info->first_mac_addr.bytes[1],
			repo->bd_info->first_mac_addr.bytes[2],
			repo->bd_info->first_mac_addr.bytes[3],
			repo->bd_info->first_mac_addr.bytes[4],
			repo->bd_info->first_mac_addr.bytes[5] + \
				repo->bd_info->mac_addr_count.bytes[0]
		);
		break;
	
	case SYSFS_MFG_ACTIVE_STATE:
		switch (repo->bd_info->active_state.bytes[0]) {
		case 'A':
			ret = sprintf(buf, "%s\n", "Active");
			break;
		
		case 'P':
			ret = sprintf(buf, "%s\n", "Passive");
			break;
		
		default:
			ret = sprintf(buf, "%s\n", "N/A");
			break;
		}
		break;
	
	case SYSFS_MFG_CONFIG_MODE:
		switch (repo->bd_info->config_mode.bytes[0]) {
		case FPGA_CONFIG_MODE_QSPIX4:
			ret = sprintf(buf, "%s\n", "QSPIx4");
			break;

		case FPGA_CONFIG_MODE_OSPI:
			ret = sprintf(buf, "%s\n", "OSPI");
			break;

		default:
			ret = sprintf(buf, "%s\n", "N/A");
			break;
		}
		break;
	
	case SYSFS_MFG_M_DATE:
	{
		/* Number of minutes from 0:00 hrs 1/1/96 in GMT LSbyte first*/
		uint32_t timestamp = (uint32_t)(repo->bd_info->manufacturing_date.bytes[2] << 16) |
			(uint32_t)(repo->bd_info->manufacturing_date.bytes[1] << 8) |
			(uint32_t)(repo->bd_info->manufacturing_date.bytes[0]);

		ret = sprintf(buf, "%d\n", timestamp);
		break;
	}

	case SYSFS_MFG_UUID:
		ret = sprintf(
			buf,
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
			repo->bd_info->uuid.bytes[0],
			repo->bd_info->uuid.bytes[1],
			repo->bd_info->uuid.bytes[2],
			repo->bd_info->uuid.bytes[3],
			repo->bd_info->uuid.bytes[4],
			repo->bd_info->uuid.bytes[5],
			repo->bd_info->uuid.bytes[6],
			repo->bd_info->uuid.bytes[7],
			repo->bd_info->uuid.bytes[8],
			repo->bd_info->uuid.bytes[9],
			repo->bd_info->uuid.bytes[10],
			repo->bd_info->uuid.bytes[11],
			repo->bd_info->uuid.bytes[12],
			repo->bd_info->uuid.bytes[13],
			repo->bd_info->uuid.bytes[14],
			repo->bd_info->uuid.bytes[15]
		);
		break;
	
	case SYSFS_MFG_PCIE_ID:
		if (repo->bd_info->pcie_id.len)
			/* Vendor Id, Device Id, Subsystem Vendor Id, Subsystem Device Id */
			ret = sprintf(
				buf,
				"%02x%02x, %02x%02x, %02x%02x, %02x%02x\n",
				repo->bd_info->pcie_id.bytes[0],
				repo->bd_info->pcie_id.bytes[1],
				repo->bd_info->pcie_id.bytes[2],
				repo->bd_info->pcie_id.bytes[3],
				repo->bd_info->pcie_id.bytes[4],
				repo->bd_info->pcie_id.bytes[5],
				repo->bd_info->pcie_id.bytes[6],
				repo->bd_info->pcie_id.bytes[7]
			);
		else
			ret = sprintf(buf, "%s\n", "N/A");
		break;
	
	case SYSFS_MFG_POWER_MODE:
		switch (repo->bd_info->power_mode.bytes[0]) {
		case MAX_POWER_MODE_75W:
			ret = sprintf(buf, "%s\n", "75W");
			break;

		case MAX_POWER_MODE_150W:
			ret = sprintf(buf, "%s\n", "150W");
			break;

		case MAX_POWER_MODE_225W:
			ret = sprintf(buf, "%s\n", "225W");
			break;

		case MAX_POWER_MODE_350W:
			ret = sprintf(buf, "%s\n", "300W");
			break;

		default:
			ret = sprintf(buf, "%s\n", "N/A");
			break;
		}
		break;
	
	case SYSFS_MFG_OEM_ID:
		if (repo->bd_info->capability.len)
			ret = sprintf(
				buf,
				"0x%02x%02x\n",
				repo->bd_info->oem_id.bytes[1],
				repo->bd_info->oem_id.bytes[0]
			);
		else
			ret = sprintf(buf, "%s\n", "N/A");
		break;
	
	case SYSFS_MFG_CAPABILITY:
		if (repo->bd_info->capability.len)
			/* Vendor Id, Device Id, Subsystem Vendor Id, Subsystem Device Id */
			ret = sprintf(
				buf,
				"%02x%02x\n",
				repo->bd_info->capability.bytes[1],
				repo->bd_info->capability.bytes[0]
			);
		else
			ret = sprintf(buf, "%s\n", "N/A");
		break;

	default:
		ret = -EINVAL;
		break;
	}

	put_pf_dev_entry(pf_dev);
	return ret;
}
static DEVICE_MFG_ATTR(eeprom_version,   read_mfg_field, SYSFS_MFG_EEPROM_VERSION);
static DEVICE_MFG_ATTR(product_name,     read_mfg_field, SYSFS_MFG_PRODUCT_NAME);
static DEVICE_MFG_ATTR(board_rev,        read_mfg_field, SYSFS_MFG_BOARD_REV);
static DEVICE_MFG_ATTR(board_serial,     read_mfg_field, SYSFS_MFG_BOARD_SERIAL);
static DEVICE_MFG_ATTR(memory_size,      read_mfg_field, SYSFS_MFG_MEMORY_SIZE);
static DEVICE_MFG_ATTR(part_num,         read_mfg_field, SYSFS_MFG_PART_NUM);
static DEVICE_MFG_ATTR(mfg_part_num,     read_mfg_field, SYSFS_MFG_M_PART_NUM);
static DEVICE_MFG_ATTR(mac_addr_count,   read_mfg_field, SYSFS_MFG_MAC_ADDR_C);
static DEVICE_MFG_ATTR(mac_addr,         read_mfg_field, SYSFS_MFG_MAC_ADDR);
static DEVICE_MFG_ATTR(mac_addr_n,       read_mfg_field, SYSFS_MFG_MAC_ADDR_N);
static DEVICE_MFG_ATTR(active_state,     read_mfg_field, SYSFS_MFG_ACTIVE_STATE);
static DEVICE_MFG_ATTR(fpga_config_mode, read_mfg_field, SYSFS_MFG_CONFIG_MODE);
static DEVICE_MFG_ATTR(mfg_date,         read_mfg_field, SYSFS_MFG_M_DATE);
static DEVICE_MFG_ATTR(mfg_uuid,         read_mfg_field, SYSFS_MFG_UUID);
static DEVICE_MFG_ATTR(pcie_id,          read_mfg_field, SYSFS_MFG_PCIE_ID);
static DEVICE_MFG_ATTR(power_mode,       read_mfg_field, SYSFS_MFG_POWER_MODE);
static DEVICE_MFG_ATTR(oem_id,           read_mfg_field, SYSFS_MFG_OEM_ID);
static DEVICE_MFG_ATTR(mfg_capability,   read_mfg_field, SYSFS_MFG_CAPABILITY);

/*
 * PF0 attributes.
 * The last element MUST be NULL and no other elements may be NULL.
 */
static const struct device_attribute *mgmt_attrs[] = {
	&dev_attr_logic_uuid,
	&dev_attr_link_speed_max,
	&dev_attr_link_speed_current,
	&dev_attr_link_width_max,
	&dev_attr_link_width_current,
	&dev_attr_dev_state,
	&dev_attr_dev_name,
	&dev_attr_amc_version,

	/* mfg data */
	&dev_attr_eeprom_version.attr,
	&dev_attr_product_name.attr,
	&dev_attr_board_rev.attr,
	&dev_attr_board_serial.attr,
	&dev_attr_memory_size.attr,
	&dev_attr_part_num.attr,
	&dev_attr_mfg_part_num.attr,
	&dev_attr_mac_addr_count.attr,
	&dev_attr_mac_addr.attr,
	&dev_attr_mac_addr_n.attr,
	&dev_attr_active_state.attr,
	&dev_attr_fpga_config_mode.attr,
	&dev_attr_mfg_date.attr,
	&dev_attr_mfg_uuid.attr,
	&dev_attr_pcie_id.attr,
	&dev_attr_power_mode.attr,
	&dev_attr_oem_id.attr,
	&dev_attr_mfg_capability.attr,

	NULL
};

/*
 * TODO: For now, the way this is done is fine but, eventually, when the list
 * of needed sysfs attributes grows, we will need a more robust mechanism
 * of creating them and associating them with a device struct (or pf_dev struct).
 */

/*
 * Register all supported sysfs attributes for a device.
 */
int register_sysfs(struct device *dev)
{
	int i = 0, ret = 0;

	if (!dev)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(mgmt_attrs); i++) {
		if (!mgmt_attrs[i])
			break;
		
		ret = device_create_file(dev, mgmt_attrs[i]);

		if (ret)
			break;
	}
	
	if (ret)
		remove_sysfs(dev);
	
	return ret;
}

/*
 * Remove all sysfs attributes for a device.
 */
void remove_sysfs(struct device *dev)
{
	int i = 0;

	if (!dev)
		return;

	for (i = 0; i < ARRAY_SIZE(mgmt_attrs); i++) {
		if (!mgmt_attrs[i])
			break;
		
		device_remove_file(dev, mgmt_attrs[i]);
	}
}
