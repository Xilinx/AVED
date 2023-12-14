// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_device.h - This file contains the public interface for device related functionality.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_DEVICE_H
#define AMI_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes. */
#include <stdint.h>

/* API includes */
#include "ami.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* For iterating over devices, this is used as a placeholder value. */
#define AMI_ANY_DEV		(-1)

/* String returned by `ami_dev_get_state` when the device is fully initialised. */
#define AMI_DEV_READY_STR	"READY"

/* Sysfs attributes. */
#define AMI_LOGIC_UUID_SIZE	(32 + 1)
#define AMI_PCI_CPULIST_SIZE	(32 + 1)
#define AMI_DEV_STATE_SIZE	(32 + 1)
#define AMI_DEV_NAME_SIZE	(32 + 1)
#define AMI_DEV_PCI_PORT_SIZE	(13)  /* 0000:00:00.0 + NULL */

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/* Opaque declaration of `struct ami_device`. */
typedef struct ami_device ami_device;

/**
* struct amc_version - structure to hold AMC version information
* @major: Major software version
* @minor: Minor software version
* @patch: Patch number (if applicable)
* @local_changes: 0 for no changes, 1 for changes
* @dev_commits: Number of development commits since the version was released
*/
struct amc_version {
	uint8_t   major;
	uint8_t   minor;
	uint8_t   patch;
	uint8_t   local_changes;
	uint16_t  dev_commits;
};

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/**
 * ami_dev_find_next() - Find the next device that matches the specified criteria.
 * @dev: Pointer to device struct.
 * @b: Device must have this bus number (can be AMI_ANY_DEV)
 * @d: Device must have this device number (can be AMI_ANY_DEV)
 * @f: Device must have this function number (can be AMI_ANY_DEV)
 * @prev: Previous device handle (may be NULL)
 * 
 * This function attempts to find a compatible PCIe device which is
 * attached to the AMI driver. Upon success, it populates all necessary
 * fields in the device struct that gets passed into this function. This
 * can then be used as a device handle to all other API calls that need it.
 * If no device is found, or the end of list has been reached an error is
 * returned and the `dev` parameter is not modified.
 *
 * Note that `prev` must not be a handle to a device which was deleted.
 *
 * This function will fail if the driver version cannot be checked
 * or if it is incompatible with the current API version.
 *
 * If you already have a handle to an AMI device and request another handle,
 * the function will work as expected, however, it is recommended that you only
 * use a single handle per device to avoid data inconsistencies. All device
 * handles must be deleted when finished by calling `ami_dev_delete`.
 *
 * If a device is removed from the driver (this should normally not happen) and
 * there still exists a valid handle registered with the driver, a SIGBUS will be
 * generated, killing the process. The same will happen if a device exceeds
 * its fatal sensor thresholds.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_find_next(ami_device **dev, int b, int d, int f, ami_device *prev);

/**
 * ami_dev_find() - Wrapper around `ami_dev_find_next`.
 * @bdf: Human readable BDF of the device to search for.
 * @dev: Pointer to device handle.
 * 
 * This function is a (slight) abstraction of `ami_dev_find_next` -
 * it simply tries to find a device which matches the given BDF
 * and returns the handle if successful. No extra logic or setup is
 * performed. For a higher level function, see `ami_dev_bringup`.
 * 
 * The format of the BDF string is "bb:dd.f" where bb is the bus,
 * dd is the device, and f is the function. This is automatically
 * converted to a numeric representation internally
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_find(const char *bdf, ami_device **dev);

/**
 * ami_dev_bringup() - Find a device and perform additional setup logic on it.
 * @bdf: Human readable BDF of the device to search for.
 * @dev: Pointer to device handle.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_bringup(const char *bdf, ami_device **dev);

/**
 * ami_dev_delete() - Free the memory held by a device struct
 * @dev: Device handle.
 * 
 * Return: None
 */
void ami_dev_delete(ami_device **dev);

/**
 * ami_dev_request_access() - Request elevated device permissions.
 * @dev: Device handle.
 *
 * Calling this function adds the currently active user ID to the list
 * of device "owners". This is valid for the lifetime of the AMI driver and
 * allows the user to carry out certain restricted operations (e.g., PDI
 * download, PCI memory access, etc.) without superuser permissions. The root
 * user is always allowed and does not need to call this function.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_request_access(ami_device *dev);

/**
 * ami_dev_pci_reload() - Remove a device from the PCI tree and rescan the bus.
 * @dev: Device handle.
 * @bdf: Device BDF.
 *
 * This function accepts either a device handle or a BDF string for the device
 * to be reloaded. If a device handle is given, an attempt is made to update
 * the handle once the reload is complete. If both arguments are provided,
 * an error will be returned. Unlike the other reloading functions, here
 * we allow a string BDF to be given in case a device is no longer attached
 * to the AMI driver and must be recovered. It is important that you
 * do not have any open device handles pointing to the same device - this
 * could lead to stability issues!
 *
 * This function requires sudo/root permissions.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_pci_reload(ami_device **dev, const char *bdf);

/**
 * ami_dev_hot_reset() - Trigger a PCI hot reset for this device.
 * @dev: Device handle.
 *
 * The provided handle will be deleted and an attempt will be made to
 * re-initialise it once the hot reset is complete. It is important that you
 * do not have any other open device handles pointing to the same device - this
 * could lead to stability issues!
 *
 * This function requires sudo/root permissions.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_hot_reset(ami_device **dev);

/**
 * ami_dev_read_uuid() - Read the logic uuid sysfs node.
 * @dev: Device handle.
 * @buf: Buffer to hold output string.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_dev_read_uuid(ami_device *dev, char buf[AMI_LOGIC_UUID_SIZE]);

/**
 * ami_dev_get_num_devices() - Get the number of devices attached to the driver.
 * @num: Variable to hold output number.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_num_devices(uint16_t *num);

/**
 * ami_dev_get_pci_link_speed() - Get the PCI link speed.
 * @dev: Device handle.
 * @current: Variable to store the PCI generation number corresponding to the
 *   current link speed.
 * @max: Variable to store the PCI generation number corresponding to the
 *   maximum link speed.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_pci_link_speed(ami_device *dev, uint8_t *current, uint8_t *max);

/**
 * ami_dev_get_pci_link_width() - Get the PCI link width.
 * @dev: Device handle.
 * @current: Variable to store current link width.
 * @max: Variable to store max link width.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_pci_link_width(ami_device *dev, uint8_t *current, uint8_t *max);

/**
 * ami_dev_get_pci_vendor() - Get the PCI vendor ID.
 * @dev: Device handle.
 * @vendor: Variable to store vendor ID.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
*/
int ami_dev_get_pci_vendor(ami_device *dev, uint16_t *vendor);

/**
 * ami_dev_get_pci_device() - Get the PCI device ID.
 * @dev: Device handle.
 * @device: Variable to store device ID.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
*/
int ami_dev_get_pci_device(ami_device *dev, uint16_t *device);

/**
 * ami_dev_get_pci_numa_node() - Get the PCI device NUMA node.
 * @dev: Device handle.
 * @node: Variable to store node number.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
*/
int ami_dev_get_pci_numa_node(ami_device *dev, uint8_t *node);

/**
 * ami_dev_get_pci_cpulist() - Get the PCI CPU affinity.
 * @dev: Device handle.
 * @buf: Variable to store CPU affinity string.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
*/
int ami_dev_get_pci_cpulist(ami_device *dev, char buf[AMI_PCI_CPULIST_SIZE]);

/**
 * ami_dev_get_state() - Get the device state.
 * @dev: Device handle.
 * @buf: Variable to store device state string.
 *
 * The returned string can be checked against `AMI_DEV_READY_STR` to
 * verify that the device is fully initialised.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_state(ami_device *dev, char buf[AMI_DEV_STATE_SIZE]);

/**
 * ami_dev_get_name() - Get the device name.
 * @dev: Device handle.
 * @buf: Variable to store device name string.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_name(ami_device *dev, char buf[AMI_DEV_NAME_SIZE]);

/**
 * ami_dev_get_amc_version() - Get AMC version info if available.
 * @dev: Device handle.
 * @amc_version: Structure to hold version information.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_amc_version(ami_device *dev, struct amc_version *amc_version);

/**
 * ami_dev_get_pci_port() - Get the BDF of the PCI port for the given device.
 * @dev: Device handle.
 * @buf: Variable to store port BDF string.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_pci_port(ami_device *dev, char buf[AMI_DEV_PCI_PORT_SIZE]);

/****************************** Device Getters ********************************/

/**
 * ami_dev_get_pci_bdf() - Get PCI BDF for this device.
 * @dev: Device handle.
 * @bdf: Variable to store BDF.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_pci_bdf(ami_device *dev, uint16_t *bdf);

/**
 * ami_dev_get_cdev_num() - Get the character device number for this device.
 * @dev: Device handle
 * @num: Variable to store number.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_cdev_num(ami_device *dev, int *num);

/**
 * ami_dev_get_hwmon_num() - Get the HWMON number for this device.
 * @dev: Device handle
 * @num: Variable to store number.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_dev_get_hwmon_num(ami_device *dev, int *num);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_DEVICE_H */
