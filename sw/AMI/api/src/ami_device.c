// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_device.c - This file contains the implementation of device related logic
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

/* Private API includes */
#include "ami_internal.h"
#include "ami_device_internal.h"
#include "ami_sensor_internal.h"
#include "ami_ioctl.h"
#include "ami_version.h"
#include "ami_mem_access.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* Device sysfs attributes */
#define SYSFS_LOGIC_UUID		"logic_uuid"
#define SYSFS_PCI_VENDOR		"vendor"
#define SYSFS_PCI_DEVICE		"device"
#define SYSFS_PCI_NUMA_NODE		"numa_node"
#define SYSFS_PCI_CPULIST		"local_cpulist"
#define SYSFS_DEV_STATE			"dev_state"
#define SYSFS_DEV_NAME			"dev_name"
#define SYSFS_AMC_VERSION		"amc_version"

#define AMC_VERSION_ATTR_FMT		"%hhd.%hhd.%hhd +%hd *%hhd"
#define AMC_VERSION_ATTR_FIELDS		(5)
#define AMC_VERSION_ATTR_SIZE		(16)

/*
 * The AMI driver creates link speed/width attributes because
 * the pre-existing attributes created by the PCIe core return invalid
 * values on some systems.
 */
#define SYSFS_PCI_LINK_SPEED_C		"link_speed_current"
#define SYSFS_PCI_LINK_SPEED_M		"link_speed_max"
#define SYSFS_PCI_LINK_WIDTH_C		"link_width_current"
#define SYSFS_PCI_LINK_WIDTH_M		"link_width_max"

/* For PCI reloading */
#define HOT_RESET_GPIO_BAR		(0)
#define HOT_RESET_GPIO_OFFSET		(0x1040000)
#define PCI_ENABLE			(1)
/*
 * NOTE: The following delays may need tweaking.
 * It is possible that a device may not show up after a rescan
 * if improper delays are used!
 */
#define HOT_RESET_SBR_SET_DELAY_MS	(2)
#define HOT_RESET_RESCAN_DELAY_MS	(4000)
#define HOT_RESET_GPIO_SET_DELAY_MS	(1)

#define PCI_DEV_DIR			"/sys/bus/pci/devices/0000:%02x:%02x.%1x"
#define PCI_BRIDGE_CONTROL		(0x3e)
#define PCI_BRIDGE_CTL_BUS_RESET	(0x40)

#define SYSFS_ENABLE			"1"
#define SYSFS_PCI_CONFIG		"/sys/bus/pci/devices/%s/config"
#define SYSFS_PCI_REMOVE		PCI_DEV_DIR "/remove"
#define SYSFS_PCI_RESCAN		"/sys/bus/pci/rescan"

/*****************************************************************************/
/* Local function declarations                                               */
/*****************************************************************************/

/**
 * open_sysfs() - Open a sysfs node for either reading or writing (or both).
 * @dev: Device handle.
 * @attr: Attribute name.
 * @mode: File mode (O_RDONLY, O_WRONLY, O_RDWR).
 * 
 * Return: The file descriptor (-1 on error).
 */
static int open_sysfs(ami_device *dev, const char *attr, int mode);

/**
 * get_new_device_handle() - Get a fresh handle for a specific device.
 * @new_dev: Variable to store new handle.
 * @bdf: BDF of old handle.
 * @with_sensors: Did the old handle have sensor data?
 *
 * This function is essentially a wrapper for `ami_dev_find_next` which performs
 * some additional setup to ensure that the new device handle is in the same
 * state as the old handle (i.e., if the old handle had sensor data, an attempt
 * will be made to fetch sensor data for the new handle also).
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int get_new_device_handle(ami_device **new_dev, uint16_t bdf, bool with_sensors);

/**
 * pci_remove() - Remove a device from the PCI tree.
 * @bdf: Numeric BDF of the device to remove.
 *
 * Note that the device to be removed need not be a device attached to the AMI
 * driver - it can be any generic PCI device.
 *
 * Sudo/root permissions are required.
 *
 * If the provided device is attached to the AMI driver, it must be deleted
 * before calling this function, otherwise a SIGBUS will be sent to the calling
 * application. If this is not handled, the process will be terminated.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
static int pci_remove(uint16_t bdf);

/**
 * pci_rescan() - Rescan the PCI tree.
 *
 * Sudo/root permissions are required. This function will not return
 * until any new devices have been fully initialized/probed.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
static int pci_rescan(void);

/**
 * do_app_setup() - Utility function to execute an app setup IOCTL.
 * @dev: Device handle.
 * @arg: Type of setup action to perform.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
static int do_app_setup(ami_device *dev, enum ami_ioc_app_setup arg);

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/*
 * Open a sysfs node.
 */
static int open_sysfs(ami_device *dev, const char *attr, int mode)
{
	int file = AMI_INVALID_FD;
	char path[AMI_SYSFS_PATH_MAX] = { 0 };

	if (!dev || !attr)
		return file;
	
	snprintf(
		path,
		AMI_SYSFS_PATH_MAX,
		AMI_DEV_SYSFS_NODE,
		AMI_PCI_BUS(dev->bdf),
		AMI_PCI_DEV(dev->bdf),
		AMI_PCI_FUNC(dev->bdf),
		attr
	);

	return open(path, mode);
}

/*
 * Get a fresh handle for a specific device.
 */
static int get_new_device_handle(ami_device **new_dev, uint16_t bdf, bool with_sensors)
{
	int ret = AMI_STATUS_ERROR;
	ami_device *dev = NULL;

	if (!new_dev || (*new_dev))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	ret =  ami_dev_find_next(
		&dev,
		AMI_PCI_BUS(bdf),
		AMI_PCI_DEV(bdf),
		AMI_PCI_FUNC(bdf),
		NULL
	);

	if (ret == AMI_STATUS_OK) {
		/* Check if we need to setup sensors. */
		if (!with_sensors || ((ret = ami_sensor_discover(dev)) == AMI_STATUS_OK))
			*new_dev = dev;
	}

	return ret;
}

/*
 * Remove a device from the PCI tree.
 */
static int pci_remove(uint16_t bdf)
{
	int ret = AMI_STATUS_ERROR;
	int file = AMI_INVALID_FD;
	char path[AMI_SYSFS_PATH_MAX] = { 0 };

	snprintf(
		path,
		AMI_SYSFS_PATH_MAX,
		SYSFS_PCI_REMOVE,
		AMI_PCI_BUS(bdf),
		AMI_PCI_DEV(bdf),
		AMI_PCI_FUNC(bdf)
	);

	if ((file = open(path, O_WRONLY)) != AMI_INVALID_FD) {
		if (write(file, SYSFS_ENABLE, strlen(SYSFS_ENABLE)) != AMI_LINUX_STATUS_ERROR)
			ret = AMI_STATUS_OK;
		else
			ret = AMI_API_ERROR(AMI_ERROR_EIO);

		close(file);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}
	
	return ret;
}

/*
 * Rescan the PCI bus.
 */
static int pci_rescan(void)
{
	int ret = AMI_STATUS_ERROR;
	int file = AMI_INVALID_FD;

	if ((file = open(SYSFS_PCI_RESCAN, O_WRONLY)) != AMI_INVALID_FD) {
		if (write(file, SYSFS_ENABLE, strlen(SYSFS_ENABLE)) != AMI_LINUX_STATUS_ERROR)
			ret = AMI_STATUS_OK;
		else
			ret = AMI_API_ERROR(AMI_ERROR_EIO);

		close(file);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}

	return ret;
}

/*
 * Execute a generic IOCTL.
 */
static int do_app_setup(ami_device *dev, enum ami_ioc_app_setup arg)
{
	int ret = AMI_STATUS_ERROR;

	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR;

	if (ioctl(dev->cdev, AMI_IOC_APP_SETUP, arg) == AMI_LINUX_STATUS_ERROR)
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	else
		ret = AMI_STATUS_OK;

	return ret;
}

/*****************************************************************************/
/* Private API function definitions                                          */
/*****************************************************************************/

/*
 * Open a character device file.
*/
int ami_open_cdev(ami_device *dev)
{
	int ret = AMI_STATUS_ERROR;
	int fd = AMI_INVALID_FD;
	char path[AMI_DEV_NAME_MAX] = { 0 };

	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (dev->cdev != AMI_INVALID_FD)
		return AMI_STATUS_OK;  /* Device already opened */

	snprintf(path, AMI_DEV_NAME_MAX, AMI_DEV, dev->cdev_num);
	fd = open(path, O_RDWR | O_NONBLOCK);

	if (fd != AMI_INVALID_FD) {
		dev->cdev = fd;
		ret = AMI_STATUS_OK;
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}

	return ret;
}

/*
 * Close a character device file.
 */
int ami_close_cdev(ami_device *dev)
{
	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (dev->cdev == AMI_INVALID_FD)
		return AMI_STATUS_OK;  /* Device already closed */

	if (close(dev->cdev) != AMI_LINUX_STATUS_ERROR)
		return AMI_STATUS_OK;

	return AMI_API_ERROR(AMI_ERROR_EBADF);
}

/*
 * Read a sysfs attribute.
 */
int ami_read_sysfs(ami_device *dev, const char *attr, char *buf)
{
	int file = AMI_INVALID_FD;
	int ret = AMI_STATUS_ERROR;

	if (!dev || !attr || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	file = open_sysfs(dev, attr, O_RDONLY);

	if (file != AMI_INVALID_FD) {
		if (read(file, buf, AMI_SYSFS_STR_MAX) != AMI_LINUX_STATUS_ERROR)
			ret = AMI_STATUS_OK;
		else
			ret = AMI_API_ERROR(AMI_ERROR_EIO);
		
		close(file);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}

	return ret;
}

/*
 * Register the current process with a driver device.
 */
int ami_dev_register(ami_device *dev)
{
	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_app_setup(dev, IOC_APP_SETUP_REGISTER);
}

/*
 * Deregister the current process from a driver device.
 */
int ami_dev_deregister(ami_device *dev)
{
	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_app_setup(dev, IOC_APP_SETUP_DEREGISTER);
}

/*****************************************************************************/
/* Public API function definitions                                           */
/*****************************************************************************/

/*
 * Find the next PCI device attached to the AMI driver.
 */
int ami_dev_find_next(ami_device **dev, int b, int d, int f, ami_device *prev)
{
	int ret = AMI_STATUS_ERROR;

	/* For reading the device file. */
	FILE *file = NULL;
	char *line = NULL;
	size_t len = 0;
	bool passed_prev = false;
	int previous_dev = 0;
	int current_line = 0;
	struct ami_version driver_ver = { 0 };
	/* Parsed values. */
	int map[AMI_BDF_MAP_MAX] = { 0, 0, 0, AMI_STATUS_ERROR, AMI_STATUS_ERROR };

	if (!dev || *dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	/* Check driver version */
	if (ami_get_driver_version(&driver_ver) != AMI_STATUS_ERROR) {
		if ((GIT_TAG_VER_MAJOR != driver_ver.major) || (GIT_TAG_VER_MINOR != driver_ver.minor))
			return AMI_API_ERROR(AMI_ERROR_EVER);
	} else {
		return AMI_STATUS_ERROR;
	}

	file = fopen(AMI_DEVICES_MAP, "r");

	if (file) {
		int nread = 0;
		bool found = false;

		while ((nread = getline(&line, &len, file)) != AMI_LINUX_STATUS_ERROR) {
			/* First line is the number of devices. */
			if (0 == current_line++)
				continue;

			int iScan = sscanf(
				line,
				"%02x:%02x.%1x %d %d",
				&map[AMI_BDF_MAP_BUS],
				&map[AMI_BDF_MAP_DEV],
				&map[AMI_BDF_MAP_FUNC],
				&map[AMI_BDF_MAP_DEVN],
				&map[AMI_BDF_MAP_HWMON]
			);

			if (iScan == AMI_BDF_MAP_MAX) {
				if (!passed_prev && (!prev || (previous_dev == prev->cdev_num))) {
					passed_prev = true;
				}

				if ((passed_prev) &&
					((b == AMI_ANY_DEV) || (map[AMI_BDF_MAP_BUS] == b)) &&
					((d == AMI_ANY_DEV) || (map[AMI_BDF_MAP_DEV] == d)) &&
					((f == AMI_ANY_DEV) || (map[AMI_BDF_MAP_FUNC] == f))) {
					/* Initialise device attributes. */
					found = true;
					*dev = (ami_device*)calloc(1, sizeof(ami_device));

					if (*dev) {
						(*dev)->bdf = AMI_MK_BDF(
							map[AMI_BDF_MAP_BUS],
							map[AMI_BDF_MAP_DEV],
							map[AMI_BDF_MAP_FUNC]
						);
						(*dev)->cdev = AMI_INVALID_FD;
						(*dev)->cdev_num = map[AMI_BDF_MAP_DEVN];
						(*dev)->hwmon_num = map[AMI_BDF_MAP_HWMON];
						ret = ami_dev_register(*dev);
					} else {
						ret = AMI_API_ERROR(AMI_ERROR_ENOMEM);
					}
					
					break;
				}

				previous_dev = map[AMI_BDF_MAP_DEVN];
			} else {
				ret = AMI_API_ERROR(AMI_ERROR_EFMT);
				break;
			}
		}

		fclose(file);
		
		if (line)
			free(line);
		
		/* Check if device was found. */
		if ((nread == AMI_LINUX_STATUS_ERROR) && !found)
			ret = AMI_API_ERROR(AMI_ERROR_ENODEV);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}

	return ret;
}

/*
 * Find a PCIe device with a specific BDF.
 */
int ami_dev_find(const char *bdf, ami_device **dev)
{
	uint16_t bdf_num = 0;

	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	bdf_num = ami_parse_bdf(bdf);

	return ami_dev_find_next(
		dev,
		AMI_PCI_BUS(bdf_num),
		AMI_PCI_DEV(bdf_num),
		AMI_PCI_FUNC(bdf_num),
		NULL
	);
}

/*
 * Find a device and perform setup logic on it.
 */
int ami_dev_bringup(const char *bdf, ami_device **dev)
{
	int ret = AMI_STATUS_ERROR;

	if (!bdf || !dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_dev_find(bdf, dev) == AMI_STATUS_OK)
		ret = ami_sensor_discover(*dev);

	return ret;
}

/*
 * Free device memory.
 */
void ami_dev_delete(ami_device **dev)
{
	if (dev && *dev) {
		/* Free sensor data. */
		if ((*dev)->sensors) {
			struct ami_sensor *sensor = (*dev)->sensors;
			struct ami_sensor *next = NULL;

			while (sensor) {
				next = sensor->next;

				/* Free private data. */
				free(sensor->sensor_data->temp);
				free(sensor->sensor_data->power);
				free(sensor->sensor_data->current);
				free(sensor->sensor_data->voltage);
				free(sensor->sensor_data);

				/* Free sensor. */
				free(sensor);
				sensor = next;
			}

			(*dev)->num_sensors = 0;
			(*dev)->num_total_sensors = 0;
			(*dev)->sensors = NULL;
		}

		/* Cleanup device. */
		ami_dev_deregister(*dev);
		ami_close_cdev(*dev);
		free(*dev);
		*dev = NULL;
	}
}

/*
 * Request elevated device permissions.
 */
int ami_dev_request_access(ami_device *dev)
{
	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	dev->cap_override = true;
	return AMI_STATUS_OK;
}

/*
 * Remove a device from the PCI tree and rescan the bus.
 */
int ami_dev_pci_reload(ami_device **dev, const char *bdf)
{
	int ret = AMI_STATUS_ERROR;
	uint16_t bdf_num = 0;
	bool has_sensors = false;

	/* Only `bdf` or `dev` may be specified (not both); *dev must be non-NULL */
	if ((bdf && dev) || (!dev && !bdf) || (dev && (!(*dev))))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (dev) {
		bdf_num = (*dev)->bdf;
		has_sensors = ((*dev)->sensors != NULL);
		ami_dev_delete(dev);
	} else {
		bdf_num = ami_parse_bdf(bdf);
	}

	if (pci_remove(bdf_num) == AMI_STATUS_OK) {
		if ((ret = pci_rescan()) == AMI_STATUS_OK) {
			/* Check if we need to update the device handle. */
			if (dev)
				ret = get_new_device_handle(
					dev,
					bdf_num,
					has_sensors
				);
		}
	}

	return ret;
}

/*
 * Perform a hot reset on a device.
 */
int ami_dev_hot_reset(ami_device **dev)
{
	int ret = AMI_STATUS_ERROR;
	char config_path[AMI_SYSFS_PATH_MAX] = { 0 };
	int config = AMI_INVALID_FD;
	char port[AMI_DEV_PCI_PORT_SIZE] = { 0 };
	uint16_t bridge_ctl = 0;

	/* Store data so we can restore the handle later. */
	uint16_t bdf = 0;
	bool has_sensors = false;

	if (!dev || !(*dev))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	/* Need the port so we can write to PCI config space. */
	ret = ami_dev_get_pci_port(*dev, port);

	if (ret)
		return ret;
	
	snprintf(
		config_path,
		AMI_SYSFS_PATH_MAX,
		SYSFS_PCI_CONFIG,
		port
	);

	config = open(config_path, O_RDWR | O_SYNC);

	if (config == AMI_INVALID_FD)
		return AMI_API_ERROR(AMI_ERROR_EBADF);

	/* Set PMC GPIO */
	ret = ami_mem_bar_write(
		*dev, HOT_RESET_GPIO_BAR, HOT_RESET_GPIO_OFFSET, PCI_ENABLE
	);

	if (ret)
		goto close_config;

	/* Remove device */
	has_sensors = ((*dev)->sensors != NULL);
	bdf = (*dev)->bdf;
	ami_dev_delete(dev);
	ret = pci_remove(bdf);
	if (ret)
		goto close_config;
	
	/*
	 * On some systems, the device that is being reset disappears from the host,
	 * forcing a system reboot - adding a delay before setting the SBR seems
	 * to mitigate this issue.
	 */
	ami_msleep(HOT_RESET_GPIO_SET_DELAY_MS);

	/* Toggle SBR */
	/* Read current BRIDGE_CONTROL */
	errno = 0;
	if (lseek(config, PCI_BRIDGE_CONTROL, SEEK_SET) != AMI_LINUX_STATUS_ERROR) {
		errno = 0;
		if (read(config, &bridge_ctl, sizeof(uint16_t)) == AMI_LINUX_STATUS_ERROR) {
			ret = AMI_API_ERROR_M(
				AMI_ERROR_EIO,
				"errno %d (%s)",
				errno,
				strerror(errno)
			);
			goto close_config;
		}
	} else {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
		goto close_config;
	}

	/* Set SBR - PDI will reload here */
	errno = 0;
	bridge_ctl |= PCI_BRIDGE_CTL_BUS_RESET;
	if (lseek(config, PCI_BRIDGE_CONTROL, SEEK_SET) != AMI_LINUX_STATUS_ERROR) {
		errno = 0;
		if (write(config, &bridge_ctl, sizeof(uint16_t)) == AMI_LINUX_STATUS_ERROR) {
			ret = AMI_API_ERROR_M(
				AMI_ERROR_EIO,
				"errno %d (%s)",
				errno,
				strerror(errno)
			);
			goto close_config;
		}
	} else {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
		goto close_config;
	}

	/* Wait a short while before resetting the SBR */
	ami_msleep(HOT_RESET_SBR_SET_DELAY_MS);

	/* Reset SBR */
	errno = 0;
	bridge_ctl &= ~PCI_BRIDGE_CTL_BUS_RESET;
	if (lseek(config, PCI_BRIDGE_CONTROL, SEEK_SET) != AMI_LINUX_STATUS_ERROR) {
		errno = 0 ;
		if (write(config, &bridge_ctl, sizeof(uint16_t)) == AMI_LINUX_STATUS_ERROR) {
			ret = AMI_API_ERROR_M(
				AMI_ERROR_EIO,
				"errno %d (%s)",
				errno,
				strerror(errno)
			);
			goto close_config;
		}
	} else {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
		goto close_config;
	}

	/* Give enough time for the PDI to initialize */
	ami_msleep(HOT_RESET_RESCAN_DELAY_MS);

	/* Finished with the config */
	close(config);

	/* Rescan bus */
	ret = pci_rescan();

	/* Update handle */
	return get_new_device_handle(dev, bdf, has_sensors);

close_config:
	close(config);
	return ret;
}

/*
 * Read the logic uuid of a device.
 */
int ami_dev_read_uuid(ami_device *dev, char buf[AMI_LOGIC_UUID_SIZE])
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMI_SYSFS_STR_MAX] = { 0 };

	if (!dev || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_LOGIC_UUID, raw_buf) == AMI_STATUS_OK) {
		/*
		 * This will also strip the new line because we're specifying 
		 * the maximum size of the logic UUID.
		 */
		strncpy(buf, raw_buf, AMI_LOGIC_UUID_SIZE - 1);
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Get the number of attached PCI devices.
 */
int ami_dev_get_num_devices(uint16_t *num)
{
	int ret = AMI_STATUS_ERROR;

	/* For reading the device file. */
	FILE *file = NULL;
	char *line = NULL;
	size_t len = 0;

	if (!num)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	file = fopen(AMI_DEVICES_MAP, "r");

	if (file) {
		if (getline(&line, &len, file) != AMI_LINUX_STATUS_ERROR) {
			int num_devices = 0;
			
			if (sscanf(line, "%d", &num_devices) == 1) {
				*num = num_devices;
				ret = AMI_STATUS_OK;
			} else {
				ret = AMI_API_ERROR(AMI_ERROR_ERET);
			}
		}

		fclose(file);

		if (line)
			free(line);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}

	return ret;
}

/*
 * Get the PCI link speed.
 */
int ami_dev_get_pci_link_speed(ami_device *dev, uint8_t *current, uint8_t *max)
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf_c[AMI_SYSFS_STR_MAX] = { 0 };
	char raw_buf_m[AMI_SYSFS_STR_MAX] = { 0 };

	if (!dev || !current || !max)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_PCI_LINK_SPEED_M, raw_buf_m) == AMI_STATUS_OK) {
		if (ami_read_sysfs(dev, SYSFS_PCI_LINK_SPEED_C, raw_buf_c) == AMI_STATUS_OK) {
			uint8_t c = 0, m = 0;

			if (sscanf(raw_buf_c, "%hhd", &c) != 1)
				return AMI_API_ERROR(AMI_ERROR_ERET);
			
			if (sscanf(raw_buf_m, "%hhd", &m) != 1)
				return AMI_API_ERROR(AMI_ERROR_ERET);
			
			*current = c;
			*max = m;
			ret = AMI_STATUS_OK;
		}
	}

	return ret;
}

/*
 * Get the PCI link width.
 */
int ami_dev_get_pci_link_width(ami_device *dev, uint8_t *current, uint8_t *max)
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf_c[AMI_SYSFS_STR_MAX] = { 0 };
	char raw_buf_m[AMI_SYSFS_STR_MAX] = { 0 };

	if (!dev || !current || !max)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_PCI_LINK_WIDTH_M, raw_buf_m) == AMI_STATUS_OK) {
		if (ami_read_sysfs(dev, SYSFS_PCI_LINK_WIDTH_C, raw_buf_c) == AMI_STATUS_OK) {
			uint8_t c = 0, m = 0;

			if (sscanf(raw_buf_c, "%hhd", &c) != 1)
				return AMI_API_ERROR(AMI_ERROR_ERET);
			
			if (sscanf(raw_buf_m, "%hhd", &m) != 1)
				return AMI_API_ERROR(AMI_ERROR_ERET);

			*current = c;
			*max = m;
			ret = AMI_STATUS_OK;
		}
	}

	return ret;
}

/*
 * Get the PCI vendor.
 */
int ami_dev_get_pci_vendor(ami_device *dev, uint16_t *vendor)
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMI_SYSFS_STR_MAX] = { 0 };
	
	if (!dev || !vendor)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_PCI_VENDOR, raw_buf) == AMI_STATUS_OK) {
		uint16_t v = 0;

		if (sscanf(raw_buf, "0x%hx", &v) != 1)
			return AMI_API_ERROR(AMI_ERROR_ERET);
		
		*vendor = v;
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Get the PCI device number.
 */
int ami_dev_get_pci_device(ami_device *dev, uint16_t *device)
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMI_SYSFS_STR_MAX] = { 0 };
	
	if (!dev || !device)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_PCI_DEVICE, raw_buf) == AMI_STATUS_OK) {
		uint16_t d = 0;

		if (sscanf(raw_buf, "0x%hx", &d) != 1)
			return AMI_API_ERROR(AMI_ERROR_ERET);
		
		*device = d;
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Get the PCI NUMA node.
 */
int ami_dev_get_pci_numa_node(ami_device *dev, uint8_t *node)
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMI_SYSFS_STR_MAX] = { 0 };
	
	if (!dev || !node)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_PCI_NUMA_NODE, raw_buf) == AMI_STATUS_OK) {
		uint8_t n = 0;

		if (sscanf(raw_buf, "%hhd", &n) != 1)
			return AMI_API_ERROR(AMI_ERROR_ERET);
		
		*node = n;
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Get the PCI CPU affinity.
 */
int ami_dev_get_pci_cpulist(ami_device *dev, char buf[AMI_PCI_CPULIST_SIZE])
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMI_SYSFS_STR_MAX] = { 0 };

	if (!dev || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_PCI_CPULIST, raw_buf) == AMI_STATUS_OK) {
		/* Strip newline */
		raw_buf[strcspn(raw_buf, "\r\n")] = 0;
		memset(buf, 0x00, AMI_PCI_CPULIST_SIZE);
		strncpy(buf, raw_buf, AMI_PCI_CPULIST_SIZE - 1);
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Get the device state.
 */
int ami_dev_get_state(ami_device *dev, char buf[AMI_DEV_STATE_SIZE])
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMI_DEV_STATE_SIZE] = { 0 };

	if (!dev || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_DEV_STATE, raw_buf) == AMI_STATUS_OK) {
		/* Strip newline */
		raw_buf[strcspn(raw_buf, "\r\n")] = 0;
		memset(buf, 0x00, AMI_DEV_STATE_SIZE);
		strncpy(buf, raw_buf, AMI_DEV_STATE_SIZE - 1);
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Get the device name.
 */
int ami_dev_get_name(ami_device *dev, char buf[AMI_DEV_NAME_SIZE])
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMI_DEV_NAME_SIZE] = { 0 };

	if (!dev || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_DEV_NAME, raw_buf) == AMI_STATUS_OK) {
		/* Strip newline */
		raw_buf[strcspn(raw_buf, "\r\n")] = 0;
		memset(buf, 0x00, AMI_DEV_NAME_SIZE);
		strncpy(buf, raw_buf, AMI_DEV_NAME_SIZE - 1);
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Get AMC version.
 */
int ami_dev_get_amc_version(ami_device *dev, struct amc_version *amc_version)
{
	int ret = AMI_STATUS_ERROR;
	char raw_buf[AMC_VERSION_ATTR_SIZE] = { 0 };

	if (!dev || !amc_version)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_read_sysfs(dev, SYSFS_AMC_VERSION, raw_buf) == AMI_STATUS_OK) {
		int scan = sscanf(
			raw_buf,
			AMC_VERSION_ATTR_FMT,
			&amc_version->major,
			&amc_version->minor,
			&amc_version->patch,
			&amc_version->dev_commits,
			&amc_version->local_changes
		);

		if (scan == AMC_VERSION_ATTR_FIELDS)
			ret = AMI_STATUS_OK;
		else
			ret = AMI_API_ERROR(AMI_ERROR_EFMT);
	}

	return ret;
}

/*
 * Get the PCI port of a device.
 */
int ami_dev_get_pci_port(ami_device *dev, char buf[AMI_DEV_PCI_PORT_SIZE])
{
	int ret = AMI_STATUS_ERROR;
	char dev_path[AMI_SYSFS_PATH_MAX] = { 0 };
	char link_path[AMI_SYSFS_PATH_MAX] = { 0 };

	if (!dev || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	snprintf(
		dev_path,
		AMI_SYSFS_PATH_MAX,
		PCI_DEV_DIR,
		AMI_PCI_BUS(dev->bdf),
		AMI_PCI_DEV(dev->bdf),
		AMI_PCI_FUNC(dev->bdf)
	);

	errno = 0;
	if (readlink(dev_path, link_path, AMI_SYSFS_PATH_MAX) == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR_M(
				AMI_ERROR_ERET,
				"errno %d (%s)",
				errno,
				strerror(errno)
			);
	} else {
		strncpy(
			buf,
			basename(dirname(link_path)),
			AMI_DEV_PCI_PORT_SIZE
		);
		
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/****************************** Device Getters ********************************/

/*
 * Get device BDF.
 */
int ami_dev_get_pci_bdf(ami_device *dev, uint16_t *bdf)
{
	if (!dev || !bdf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	*bdf = dev->bdf;
	return AMI_STATUS_OK;
}

/*
 * Get cdev number.
 */
int ami_dev_get_cdev_num(ami_device *dev, int *num)
{
	if (!dev || !num)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	*num = dev->cdev_num;
	return AMI_STATUS_OK;
}

/*
 * Get hwmon number.
 */
int ami_dev_get_hwmon_num(ami_device *dev, int *num)
{
	if (!dev || !num)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	*num = dev->hwmon_num;
	return AMI_STATUS_OK;
}
