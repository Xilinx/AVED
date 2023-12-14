// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami.h - This file contains generic public API definitions
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_H
#define AMI_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* Public API return codes */
#define AMI_STATUS_OK		(0)
#define AMI_STATUS_ERROR	(-1)

/* Placeholder for file descriptors */
#define AMI_INVALID_FD		(-1)

/* AMI character device files. */
#define AMI_DEV			"/dev/ami%d"
#define AMI_DEV_NAME_MAX	(16)

/* Driver attributes */
#define AMI_DRIVER_VERSION	"/sys/bus/pci/drivers/ami/version"
#define AMI_DEVICES_MAP		"/sys/bus/pci/drivers/ami/devices"

/* Utility macros for working with BDF numbers. */
#define AMI_MK_BDF(b, d, f) \
	(uint16_t)(((b << 8) & 0xFF00) | \
		((( d << 3) & ~0x07)   | \
		(f & 0x07)))

#define AMI_PCI_BUS(bdf)	(bdf >> 8)
#define AMI_PCI_DEV(bdf)	(((bdf & 0xFF) >> 3) & 0x1F)
#define AMI_PCI_FUNC(bdf)	((bdf & 0xFF) & 0x07)

#define AMI_BDF_STR_LEN		(7 + 1)
#define AMI_BDF_FIELDS_NUM	(3)
#define AMI_BDF_FORMAT		"%02x:%02x.%1x"

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum ami_bdf_map - positions of arguments in the bdf map file
 * @AMI_BDF_MAP_BUS: pci device bus number
 * @AMI_BDF_MAP_DEV: pci device number
 * @AMI_BDF_MAP_FUNC: pci function number
 * @AMI_BDF_MAP_DEVN: character device number
 * @AMI_BDF_MAP_HWMON: hwmon device number
 */
enum ami_bdf_map {
	AMI_BDF_MAP_BUS = 0,
	AMI_BDF_MAP_DEV,
	AMI_BDF_MAP_FUNC,
	AMI_BDF_MAP_DEVN,
	AMI_BDF_MAP_HWMON,

	AMI_BDF_MAP_MAX
};

/**
 * enum ami_linux_status - list of generic Linux return codes
 * @AMI_LINUX_STATUS_OK: success code
 * @AMI_LINUX_STATUS_ERROR: failure code
 */
enum ami_linux_status {
	AMI_LINUX_STATUS_OK    = 0,
	AMI_LINUX_STATUS_ERROR = -1,
};

/**
 * enum ami_event_status - list of driver event status codes
 * @AMI_EVENT_STATUS_OK: poll was ok and valid data was read back
 * @AMI_EVENT_STATUS_READ_ERROR: poll was ok but the read failed
 * @AMI_EVENT_STATUS_TIMEOUT: poll timed out (no event)
 */
enum ami_event_status {
	AMI_EVENT_STATUS_OK = 0,
	AMI_EVENT_STATUS_READ_ERROR,
	AMI_EVENT_STATUS_TIMEOUT,
};

/*****************************************************************************/
/* Typedefs                                                                  */
/*****************************************************************************/

/**
 * typedef ami_event_handler - Callback for AMI driver events.
 * @status: Event status
 * @ctr: Data read back from the event file descriptor
 * @data: Generic data pointer - may be NULL (depends on function)
 * 
 * Some API functions accept an event handler to receive AMI driver events.
 * This can be useful, for example, for tracking the progress of long running
 * commands, such as PDI downloads. Note that this function will be called
 * from within a thread.
 * 
 * Return: None
 */
typedef void (*ami_event_handler)(enum ami_event_status status, uint64_t ctr, void *data);

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
* struct ami_version - structure to hold AMI version information
* @major: Major software version
* @minor: Minor software version
* @patch: Patch number (if applicable)
* @dev_commits: Number of development commits since the version was released
* @status: 0 if unmodified, 1 if there are unversioned modifications
*/
struct ami_version {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint8_t dev_commits;
	uint8_t status;
};

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * ami_get_driver_version() - Get AMI Driver version info
 * @ami_version: Structure with AMI Driver version information
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_get_driver_version(struct ami_version *ami_version);

/**
 * ami_get_api_version() - Get AMI API version info
 * @ami_version: Structure with AMI API version information
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_get_api_version(struct ami_version *ami_version);

/**
 * ami_get_last_error() - Get a human readable string of the last error code.
 * 
 * This function should only be called if the function you called returned
 * AMI_STATUS_ERROR - otherwise, you may get the string for an error
 * from a previous, unrelated function call.
 * 
 * Return: Error code string.
 */
const char *ami_get_last_error(void);

/**
 * ami_parse_bdf() - Parse a BDF string into a uint16 representation.
 * @bdf: String representation of the BDf to parse
 * 
 * Note that this function tries to parse as much of a BDF string as it can.
 * This means that if the string "c1" is given, it will assume that the bus
 * number is 0xC1, while the device and function are both 0.
 *
 * Return: uint16_t representation of the parsed BDF (0 on fail)
 */
uint16_t ami_parse_bdf(const char *bdf);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_H */
