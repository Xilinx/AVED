// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_top.h - This file contains common AMI driver definitions.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_TOP_H
#define AMI_TOP_H

#include <linux/pci.h>
#include <linux/pid.h>
#include <linux/list.h>
#include <linux/kref.h>
#include <linux/semaphore.h>

#include "ami.h"
#include "ami_vsec.h"
#include "ami_pcie.h"
#include "ami_sensor.h"
#include "ami_cdev.h"
#include "ami_amc_control.h"

/* Device state strings. */
#define STATE_NAME_INIT       	"INIT"
#define STATE_NAME_READY      	"READY"
#define STATE_NAME_MISSING_INFO "MISSING_INFO"
#define STATE_NAME_NO_AMC     	"NO_AMC"
#define STATE_NAME_INIT_ERROR 	"INIT_ERROR"
#define STATE_NAME_SHUTDOWN   	"SHUTDOWN"
#define STATE_NAME_COMPAT     	"COMPAT"

/**
 * enum pf_dev_state - List of possible device states.
 * @PF_DEV_STATE_INIT: Device is initialising.
 * @PF_DEV_STATE_READY: Device is fully initialised and ready to use.
 * @PF_DEV_STATE_MISSING_INFO: AMC setup and device may be used, but with some missing data.
 * @PF_DEV_STATE_NO_AMC: Device may be used but has no AMC instance.
 * @PF_DEV_STATE_INIT_ERROR: AMC setup and device may be used but with no data.
 * @PF_DEV_STATE_SHUTDOWN: All services have been shutdown.
 * @PF_DEV_STATE_COMPAT: Compatibility mode - most functions unavailable.
 * 
 * Note that, currently, PF_DEV_STATE_INIT is only used temporarily within
 * the device probe function so a user is unlikely to ever see this state.
 */
enum pf_dev_state {
	PF_DEV_STATE_INIT = 0,
	PF_DEV_STATE_READY,
	PF_DEV_STATE_MISSING_INFO,
	PF_DEV_STATE_NO_AMC,
	PF_DEV_STATE_INIT_ERROR,
	PF_DEV_STATE_SHUTDOWN,
	PF_DEV_STATE_COMPAT
};

/**
 * struct pf_dev_application - A single user application registered with the driver.
 * @list: Linked list node.
 * @pid: PID struct of the application - this is safe to store as it is not reused.
 */
struct pf_dev_application {
	struct list_head list;
	struct pid *pid;
};

/**
 * enum pf_dev_cache_type() - List of possible pf_dev_struct cache locations
 * @PF_DEV_CACHE_PCI_DEV: pf_dev is stored inside struct pci_dev private data
 * @PF_DEV_CACHE_FILP: pf_dev_ is stored inside struct file private data
 * @PF_DEV_CACHE_INODE: pf_dev is stored inside cdev struct which is inside an inode
 * @PF_DEV_CACHE_DEV: pf_dev is stored inside a generic device struct
 *
 * Pointers to device data structs are often stored, or "cached", for future use.
 * For example, character devices may fetch a pointer to a pf_dev_struct through
 * the inode pointer, however, these pointers are not reference counted and must
 * not be used directly. Instead, `get_pf_dev_entry` should be called to fetch
 * a reference counted pointer and `put_pf_dev_entry` should be called when finished.
 * The cache simply serves as a lookup for the actual pointer that should be used.
 */
enum pf_dev_cache_type {
	PF_DEV_CACHE_PCI_DEV,
	PF_DEV_CACHE_FILP,
	PF_DEV_CACHE_INODE,
	PF_DEV_CACHE_DEV,
};

/* Same device structure reused for both PF0 and PF1 */
/**
 * struct pf_dev_struct - Top level struct for a PCI device.
 * @state: Current device state.
 * @pci: PCI device struct.
 * @pcie_config: PCI specific data
 * @endpoints: PCI endpoints (UUID, GCQ, etc...)
 * @amc_ctrl_ctxt: AMC data struct.
 * @ioctl_sema: Semaphore used by the IOCTL handler.
 * @sensor_refresh: Sensor update interval in milliseconds.
 * @num_sensor_repos: Number of discovered sensor repos.
 * @sensor_repos: Discovered sensor repos.
 * @cdev: Character device data.
 * @hwmon_id: Hwmon number.
 * @pcie_bus_num: Bus number.
 * @pcie_device_num: Device number.
 * @pcie_function_num: Function number.
 * @bdf_str: BDF string.
 * @hwmon_dev: Hwmon device struct.
 * @apps: List of applications registered with the driver.
 * @app_lock: Mutex protecting list of applications.
 * @enabled: Boolean indicating if this device is enabled - when the top level
 *   remove callback is called, this is set to false and no more device
 *   handles will be given out. Do not modify this directly.
 * @refcount: Reference counter. Anybody who wishes to hold a reference to a
 *   pf_dev_struct pointer must call `get_pf_dev_entry` and `put_pf_dev_entry`.
 *   Do not use this directly.
 * @remove_sema: Semaphore to allow blocking in the `pcie_device_remove`
 *   callback. This is initialised to 0; when the refcount reaches 0, the semaphore
 *   is incremented and all device data gets deleted. Do not use this directly.
 */
struct pf_dev_struct {
	enum pf_dev_state           state;
	struct pci_dev             *pci;
	pcie_config_struct         *pcie_config;
	endpoints_struct           *endpoints;
	struct amc_control_ctxt    *amc_ctrl_ctxt;  /* Only applicable for PF0 */
	struct semaphore            ioctl_sema;
	uint16_t                    sensor_refresh;
	uint8_t                     num_sensor_repos;
	struct sdr_repo            *sensor_repos;
	struct drv_cdev_struct      cdev;  /* Not a pointer so we can use `container_of` */
	int                         hwmon_id;
	uint8_t                     pcie_bus_num;
	uint8_t                     pcie_device_num;
	uint8_t                     pcie_function_num;
	char                        bdf_str[BDF_STR_LEN];
	struct device              *hwmon_dev;
	struct list_head            apps;
	struct mutex                app_lock;
	bool                        enabled;
	struct kref                 refcount;
	struct semaphore            remove_sema;
};

/**
 * shutdown_pf_dev_services() - Shutdown all device services.
 * @pf_dev: Device data struct.
 *
 * This function does not delete any non-service related data.
 * Any services will become unusable after this function call until
 * the device is manually removed and bus rescanned.
 *
 * Return: None.
 */
void shutdown_pf_dev_services(struct pf_dev_struct *pf_dev);

/**
 * kill_pf_dev_apps() - Attempt to stop all applications registered with a device.
 * @pf_dev: The device handle.
 * @sig: Signal to send to applications.
 *
 * Return: 0 if signal sent to all applications, errno otherwise
 */
int kill_pf_dev_apps(struct pf_dev_struct *pf_dev, int sig);

/**
 * add_pf_dev_app() - Add a user application to a device.
 * @pf_dev: The device handle.
 * @task: The task struct this application belongs to.
 *
 * If the application already exists, this function does nothing and
 * returns success.
 *
 * Return: 0 or negative error code
 */
int add_pf_dev_app(struct pf_dev_struct *pf_dev, struct task_struct *task);

/**
 * delete_pf_dev_app() - Remove a user application from a device.
 * @pf_dev: The device handle.
 * @task: The task struct this application belongs to.
 * 
 * If the application does not exist, this function does nothing and
 * returns success.
 *
 * Return: 0 or negative error code
 */
int delete_pf_dev_app(struct pf_dev_struct *pf_dev, struct task_struct *task);

/**
 * get_pf_dev_entry() - Request a reference counted pointer to a pf_dev_struct
 * @cache: Data cache in which to lookup the pointer.
 * @cache_type: Type of data cache.
 *
 * Return: Pointer to data struct or NULL if unavailable.
 */
struct pf_dev_struct *get_pf_dev_entry(void *cache, enum pf_dev_cache_type cache_type);

/**
 * put_pf_dev_entry() - Decrement the reference count of a pf_dev_struct
 * @pf_dev: Pointer to data struct.
 * 
 * If this is the last pointer reference, all device data will be deleted.
 *
 * Return: None
 */
void put_pf_dev_entry(struct pf_dev_struct *pf_dev);

#endif /* AMI_TOP_H */
