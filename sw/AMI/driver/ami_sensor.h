// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_sensor.h - This file contains sensor-related definitions.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_SENSOR_H
#define AMI_SENSOR_H

#include <linux/types.h>
#include <linux/pci.h>

#include "ami_amc_control.h"


/*
 * Number of milliseconds before a sensor reading is considered "stale".
 * This is a default value only - the actual value may be configured via hwmon.
 */
#define SENSOR_REFRESH_TIMEOUT_MS 1000

/**
 * struct sensor_status_name_map_t - map of status to human readable representation.
 * @status: Numeric status
 * @name: Human readable string for this status
 */
struct sensor_status_name_map_t{
	int     status;
	char    *name;
};

/* Forward declaration of pf_dev_struct */
struct pf_dev_struct;

/* enum sensor_status defined in ami_amc_control.h */

#define SENSOR_STATUS_NAME_NOT_PRESENT "Sensor Not Present"
#define SENSOR_STATUS_NAME_PRESENT     "Sensor Present and Valid"
#define SENSOR_STATUS_NAME_UNAVAIL     "Data Not Available"
#define SENSOR_STATUS_NAME_NA          "Not Applicable or Default Value"

int discover_sensors(struct pf_dev_struct *pf_dev, int *empty_sdr_count);
void delete_sensors(struct pf_dev_struct *pf_dev);
int update_sdr(struct pf_dev_struct *pf_dev, enum gcq_sdr_repo_type repo_type);

struct sdr_repo *find_sdr_repo(struct sdr_repo *sensor_repos,
	uint8_t num_sensor_repos, enum gcq_sdr_repo_type type);
struct sdr_record *find_sdr_record(struct sdr_repo *sensor_repos,
	uint8_t num_sensor_repos, enum gcq_sdr_repo_type type, int sid);

long make_val(enum sdr_value_type type, uint8_t size, uint8_t *data);
char *convert_sensor_status_name_map(int status);
const char *sdr_repo_type_to_str(enum gcq_sdr_repo_type sdr);

int read_thermal_sensors(struct pf_dev_struct *pf_dev, bool *fresh);
int read_current_sensors(struct pf_dev_struct *pf_dev, bool *fresh);
int read_voltage_sensors(struct pf_dev_struct *pf_dev, bool *fresh);
int read_power_sensors(struct pf_dev_struct *pf_dev, bool *fresh);

int read_fpt_hdr(struct pf_dev_struct *pf_dev, struct fpt_header *hdr);
int read_fpt_partition(struct pf_dev_struct *pf_dev, uint32_t partition_id,
                       struct fpt_partition *partition);
int read_board_info(struct pf_dev_struct *pf_dev, struct bd_info_record *bd_info);

#endif /* AMI_SENSOR_H */
