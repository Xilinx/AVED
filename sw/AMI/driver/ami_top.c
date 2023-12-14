// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_top.c - This file contains the main entry point for the AMI driver.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <uapi/linux/pci_regs.h>
#include <linux/ioport.h>
#include <asm-generic/pci_iomap.h>
#include <uapi/asm-generic/errno-base.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/signal.h>

#include "ami.h"
#include "ami_top.h"
#include "ami_sysfs.h"
#include "ami_hwmon.h"
#include "ami_cdev.h"
#include "ami_utils.h"
#include "ami_cdev.h"
#include "ami_pcie.h"
#include "ami_vsec.h"
#include "ami_amc_control.h"
#include "ami_driver_version.h"

/* RHEL fix */
#ifndef fallthrough
#define fallthrough do {} while (0)  /* fallthrough */
#endif

static struct drv_cdev_struct driver_dev = {{ 0 }};  /* Global device */
static unsigned pf_dev_index = DEFAULT_CDEV_BASEMINOR;
static FW_IF_GCQ_INIT_CFG fw_if_gcq_init_cfg = { 0 };

/*
 * We need a global device mutex to fetch/delete device handles in situations
 * where we do not already hold a valid device pointer. This can't be embedded
 * inside the pf_dev_struct because any pointer we access may already be invalid
 * by the time we attempt to acquire a lock.
 */
static DEFINE_MUTEX(pf_dev_lock);


int register_driver_kernel(void);
void unregister_driver_kernel(void);

int pcie_device_probe(struct pci_dev *dev, const struct pci_device_id *id);
void pcie_device_remove(struct pci_dev *dev);
int register_driver_pcie(void);


static struct file_operations dev_fops = {
	.owner          = THIS_MODULE,
	.open           = dev_open,
	.release        = dev_close,
	.unlocked_ioctl = dev_unlocked_ioctl,
};

int register_driver_kernel(void)
{
	int ret = 0;

	/* Register the global driver character device. */
	ret = create_cdev(pf_dev_index, &driver_dev, NULL, &dev_fops);
	if(ret) {
		driver_dev.count = 0;
		PR_ERR("Failed to register character device to the kernel");
	} else {
		pf_dev_index++;
		ret = SUCCESS;
	}

	return ret;
}

void unregister_driver_kernel(void)
{
	PR_DBG("Delete cdev");

	if(driver_dev.count) {
		cdev_del(&driver_dev.cdev);
		device_destroy(driver_dev.dev_class, driver_dev.cdev_num);
		class_destroy(driver_dev.dev_class);
		unregister_chrdev_region(driver_dev.cdev_num, driver_dev.count);
	}

	driver_dev.count = 0;
}

static void amc_event_cb(enum amc_event_id id, void *data)
{
	switch(id) {
	case AMC_EVENT_ID_HEARTBEAT_EXPIRED:
		PR_ERR("AMC Heartbeat expired event received");
		break;

	case AMC_EVENT_ID_HEARTBEAT_VALIDATION:
		PR_ERR("AMC Heartbeat validation event received");
		break;

	/* This event will only be raised once */
	case AMC_EVENT_ID_HEARTBEAT_FATAL:
	{
		struct pci_dev *dev = NULL;
		struct pf_dev_struct *pf_dev = NULL;

		PR_ERR("AMC Heartbeat fatal event received, stopping GCQ...");

		if (!data) {
			PR_ERR("AMC Heartbeat callback received invalid data!");
		} else {
			dev = (struct pci_dev*)data;
			pf_dev = pci_get_drvdata(dev);

			if (!pf_dev) {
				PR_ERR("AMC Heartbeat callback cannot recover!");
			} else {
				stop_gcq_services(pf_dev->amc_ctrl_ctxt);
				/* Overwrite the device state */
				pf_dev->state = PF_DEV_STATE_NO_AMC;
			}
		}
		
	}
	break;

	default:
		break;
	}
}

/*
 * The below registration will export the pci_device_id structure to userspace 
 * to allow the hot-plug and module loading system know what module works with
 * which hardware devices. This information will help hot-plug system to find
 * and load the proper driver when kernel tells the hotplug system that new PCI
 * device is found. The module entry is added to the file -
 * /lib/modules/KERNEL_VERSION/modules.pcimap
 */

static struct pci_device_id device_id[] = {
	{
		PCI_DEVICE(PCIE_VENDOR_ID, PCIE_DEVICE_ID),
		.subvendor  = PCIE_SUBVENDOR_ID,
		.subdevice  = PCIE_SUBDEVICE_ID,
		.class      = PCIE_CLASS_ID,
	},
	{}
};
MODULE_DEVICE_TABLE(pci, device_id);

static struct pci_driver pcie_driver_core = {
	.name       = DEFAULT_DEVICE_NAME,  /* This name appears in /sys/bus/pci/drivers when driver is loaded into kernel */
	.id_table   = device_id,
	.probe      = pcie_device_probe,    /* Kernel calls this when it thinks our device is being inserted */
	.remove     = pcie_device_remove,
};

/**
 * create_pf_dev_data() - Create a pf_dev struct and initialize all services.
 * @dev: Parent PCI device struct.
 *
 * This function should only be called from the probe callback.
 * 
 * Return: 0 or negative error code.
 */
static int create_pf_dev_data(struct pci_dev *dev)
{
	int ret = SUCCESS;
	int empty_sdr_count = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev)
		return -EINVAL;

	/* Allocating and zeroing kernel memory */
	pf_dev = kzalloc(sizeof(struct pf_dev_struct), GFP_KERNEL);

	if (!pf_dev) {
		PR_ERR("Failed to allocate kernel memory for pf_dev_struct");
		ret = -ENOMEM;
		goto fail;
	}

	pf_dev->pci = dev;
	pf_dev->sensor_refresh = SENSOR_REFRESH_TIMEOUT_MS;
	pf_dev->hwmon_id = -1;
	pf_dev->pcie_config = NULL;
	pf_dev->endpoints = NULL;
	pf_dev->amc_ctrl_ctxt = NULL;
	pf_dev->pcie_bus_num = dev->bus->number;
	pf_dev->pcie_device_num = PCI_SLOT(dev->devfn);
	pf_dev->pcie_function_num = PCI_FUNC(dev->devfn);
	sema_init(&pf_dev->ioctl_sema, 1);
	sema_init(&pf_dev->remove_sema, 0);  /* init to 0 so we can block in the remove callback */
	mutex_init(&pf_dev->app_lock);
	kref_init(&pf_dev->refcount);
	INIT_LIST_HEAD(&pf_dev->apps);

	sprintf(pf_dev->bdf_str,
		"%02x:%02x.%1x",
		pf_dev->pcie_bus_num,
		pf_dev->pcie_device_num,
		pf_dev->pcie_function_num);

	DEV_INFO(dev, "PCIE device BDF %s", pf_dev->bdf_str);

	pci_set_drvdata(dev, pf_dev);

	/* Read the configuration registers */
	ret = read_pcie_configuration(dev, &pf_dev->pcie_config);
	if (ret)
		goto delete_data;

	/* Setting PCIE Config */
	ret = write_pcie_configuration(dev);
	if (ret)
		goto delete_data;

	/* Read vendor specific information */
	if (pf_dev->pcie_config->ext_cap->vsec_base_addr_found) {
		ret = read_vsec(dev,
			pf_dev->pcie_config->ext_cap->vsec_base_addr,
			&pf_dev->endpoints);
		if (ret)
			goto delete_data;
	} else {
		ret = -EINVAL;
		goto delete_data;
	}

	/* AMC Setup */
	if (pf_dev->pcie_function_num == 0) {
		/*
		 * If this fails, simply set the context to NULL and try to
		 * continue with the probe function as normal.
		 */
		ret = setup_amc(dev, &pf_dev->amc_ctrl_ctxt,
			pf_dev->endpoints->gcq,
			pf_dev->endpoints->gcq_payload,
			amc_event_cb,
			(void*)dev);
		if (ret) {
			pf_dev->amc_ctrl_ctxt = NULL;
			pf_dev->state = PF_DEV_STATE_NO_AMC;
		} else {
			if (pf_dev->amc_ctrl_ctxt->compat_mode)
				pf_dev->state = PF_DEV_STATE_COMPAT;
		}
	} else if (pf_dev->pcie_function_num == 1) {
		/* CG TODO: try to peek poke xbtest CU */
	}

	/*
	 * Attempt sensor discovery only if AMC was initialised correctly. 
	 * COMPAT MODE: No sensor data and no hwmon entries.
	 */
	if ((pf_dev->pcie_function_num == 0) && pf_dev->amc_ctrl_ctxt &&
			!(pf_dev->amc_ctrl_ctxt->compat_mode)) {

		/* We don't bail out if the sensor discover fails - the user
		 * should still be able to access the device regardless. We do
		 * bail out, however, if the hwmon init fails as this should not
		 * happen. NOTE: Both, the sensor data and hwmon data use managed
		 * memory so no cleanup is necessary.
		 */
		if (!discover_sensors(pf_dev, &empty_sdr_count)) {
			ret = register_hwmon(&dev->dev, pf_dev);
			if (ret)
				goto remove_pf_dev;
		} else {
			pf_dev->state = PF_DEV_STATE_INIT_ERROR;
		}
	}

	/*
	 * Create extra sysfs attributes.
	 * COMPAT MODE: sysfs allowed.
	 */
	if (pf_dev->pcie_function_num == 0) {
		ret = register_sysfs(&dev->dev);
		if (ret)
			goto remove_pf_dev;
	}

	/*
	 * Create character device.
	 * COMPAT MODE: cdev allowed but only certain functions will succeed.
	 */
	pf_dev->cdev.dev_class = driver_dev.dev_class;
	strncpy(pf_dev->cdev.drv_cls_str, driver_dev.drv_cls_str,
			strlen(driver_dev.drv_cls_str));

	ret = create_cdev(pf_dev_index, &pf_dev->cdev, &dev->dev, &dev_fops);
	if(ret)
		goto delete_sysfs;

	pf_dev_index++;

	if (pf_dev->state == PF_DEV_STATE_INIT) {
		if(empty_sdr_count) {
			pf_dev->state = PF_DEV_STATE_MISSING_INFO;
		}
		else {
			pf_dev->state = PF_DEV_STATE_READY;
		}
	}

	DEV_VDBG(dev, "Successfully probed device: 0x%X", dev->device);
	pf_dev->enabled = true;  /* This is safe if we are called from the probe callback. */
	return SUCCESS;

delete_sysfs:
	if (pf_dev->pcie_function_num == 0)
		remove_sysfs(&dev->dev);

remove_pf_dev:
	pf_dev->cdev.count = 0;

	if (pf_dev->amc_ctrl_ctxt) {
		unset_amc(dev, &pf_dev->amc_ctrl_ctxt);
		release_amc_mem(&pf_dev->amc_ctrl_ctxt);
	}

delete_data:
	release_vsec_mem(&pf_dev->endpoints);
	release_pcie_mem(&pf_dev->pcie_config);
	kfree(pf_dev);
	pci_set_drvdata(dev, NULL);

fail:
	DEV_VDBG(dev, "Failed to create pf_dev data: 0x%X", dev->device);
	return ret;
}

/**
 * pcie_device_probe() - Probe a new device.
 * @dev: PCI device handle.
 * @id: PCI device ID.
 * 
 * This probe function gets called (during execution of pci_register_driver() for already
 * existing devices or later if a new pci device gets inserted) for all PCI devices which
 * match the ID table and are not "owned" by the other drivers yet. The probe function
 * always get called from process context, so it can sleep.
 * 
 * Return: 0 or negative error code.
 */
int pcie_device_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int ret = SUCCESS;

	if (!dev || !id)
		return -EINVAL;

	if (!is_supported_pcie_device_id(dev->device)) {
		DEV_VDBG(dev, "Device ID not supported: 0x%X", dev->device);
		return -EINVAL;
	}

	DEV_VDBG(dev, "Probing PCIE device (vendor id : 0x%X, device id : 0x%X)",
		dev->vendor, dev->device);

	ret = pci_enable_device(dev);
	if (ret) {
		DEV_ERR(dev, "PCIE device enable failed");
		return ret;
	}
	DEV_VDBG(dev, "Successfully enabled PCIE device");

	ret = create_pf_dev_data(dev);

	if (ret) {
		pci_disable_device(dev);
		DEV_VDBG(dev, "Failed to probe device: 0x%X", dev->device);
	} else {
		DEV_VDBG(dev, "Successfully probed device: 0x%X", dev->device);
	}

	return ret;
}

/*
 * Shutdown all device services but don't delete any other data.
 */
void shutdown_pf_dev_services(struct pf_dev_struct *pf_dev)
{
	if (!pf_dev || (pf_dev->state == PF_DEV_STATE_SHUTDOWN))
		return;

	/* Shutdown AMC. */
	if (pf_dev->amc_ctrl_ctxt) {
		unset_amc(pf_dev->pci, &pf_dev->amc_ctrl_ctxt);
		release_amc_mem(&pf_dev->amc_ctrl_ctxt);  /* NOTE: This sets the pointer to NULL. */
	}

	pf_dev->state = PF_DEV_STATE_SHUTDOWN;
}

/**
 * delete_pf_dev_data() - Delete a pf_dev struct and remove all services.
 * @pf_dev: Device data struct.
 * @delete_managed: If true, also delete managed data (e.g., sensors, hwmon).
 * 
 * Return: None.
 */
void delete_pf_dev_data(struct pf_dev_struct *pf_dev, bool delete_managed)
{
	struct pf_dev_application *pos = NULL, *next = NULL;

	if (!pf_dev)
		return;

	/* Prevent new app registrations */
	mutex_lock(&pf_dev->app_lock);

	/* Delete all apps */
	list_for_each_entry_safe(pos, next, &pf_dev->apps, list) {
		put_pid(pos->pid);
		list_del(&pos->list);
		
		if (delete_managed) {
			devm_kfree(&pf_dev->pci->dev, pos);
		}
	}

	shutdown_pf_dev_services(pf_dev);
	
	if (pf_dev->cdev.count) {
		/* Don't destroy the class here. */
		cdev_del(&pf_dev->cdev.cdev);
		device_destroy(pf_dev->cdev.dev_class, pf_dev->cdev.cdev_num);
		unregister_chrdev_region(pf_dev->cdev.cdev_num, pf_dev->cdev.count);
	}

	release_vsec_mem(&pf_dev->endpoints);
	release_pcie_mem(&pf_dev->pcie_config);

	remove_sysfs(&pf_dev->pci->dev);

	/* Managed data does not need to be manually freed if a device has been removed. */
	if (delete_managed) {
		delete_sensors(pf_dev);
		remove_hwmon(pf_dev->hwmon_dev);
	}

	pci_set_drvdata(pf_dev->pci, NULL);
	mutex_unlock(&pf_dev->app_lock);
	kfree(pf_dev);
}

void pcie_device_remove(struct pci_dev *dev)
{
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev)
		return;
	
	pf_dev = pci_get_drvdata(dev);

	if (pf_dev) {
		mutex_lock(&pf_dev_lock);
		/*
		 * We can access pf_dev directly because we have a
		 * reference from the probe function - we will set the device
		 * to disabled to prevent any further reference increases and also release
		 * our own reference. The device data will only be deleted when the
		 * refcount reaches 0.
		 */
		pf_dev->enabled = false;
		mutex_unlock(&pf_dev_lock);
		put_pf_dev_entry(pf_dev);
	}

	pci_disable_device(dev);  /* Disable bus mastering regardless of the refcount */
	kill_pf_dev_apps(pf_dev, SIGBUS);  /* Kill any applications that may still be running */
	down_interruptible(&pf_dev->remove_sema);  /* Wait until the refcount reaches 0 */
	delete_pf_dev_data(pf_dev, false);  /* Safe to delete data */
	DEV_INFO(dev, "Successfully removed PCIe device");
}

int register_driver_pcie(void)
{
	int ret = 0;

	ret = pci_register_driver(&pcie_driver_core);
	if (ret)
		goto fail;

	PR_DBG("Successfully registered module with PCIE Core");
	return SUCCESS;

fail:
	PR_ERR("Module registration with PCIE Core failed");
	return ret;
}

/**
 * release_pf_dev_entry() - Callback for freeing device data when the refcount is 0.
 * @ref: refcount struct of the device to be removed
 *
 * Note that this function doesn't actually delete any data, but rather it
 * instructs the `pcie_device_remove` callback that it is safe to remove
 * the device.
 *
 * Return: None
 */
static void release_pf_dev_entry(struct kref *ref)
{
	struct pf_dev_struct *pf_dev = NULL;

	if (!ref)
		return;

	pf_dev = container_of(ref, struct pf_dev_struct, refcount);

	/* No need to acquire pf_dev_lock here */
	up(&pf_dev->remove_sema);
}

/*
 * Get a pf_dev_struct pointer and increment the reference count.
 */
struct pf_dev_struct *get_pf_dev_entry(void *cache, enum pf_dev_cache_type cache_type)
{
	struct pf_dev_struct *entry = NULL;
 
	if (!cache)
		return NULL;

	mutex_lock(&pf_dev_lock);

	switch (cache_type) {
	case PF_DEV_CACHE_PCI_DEV:
	{
		struct pci_dev *pci_dev = (struct pci_dev*)cache;
		entry = pci_get_drvdata(pci_dev);
		break;
	}
	
	case PF_DEV_CACHE_FILP:
	{
		struct file *filp = (struct file*)cache;
		if (iminor(filp->f_inode) != DEFAULT_CDEV_BASEMINOR)
			entry = filp->private_data;
		break;
	}

	case PF_DEV_CACHE_INODE:
	{
		struct inode *inode = (struct inode*)cache;
		struct drv_cdev_struct *cdev = NULL; 
		if (iminor(inode) != DEFAULT_CDEV_BASEMINOR) {
			cdev = container_of(inode->i_cdev, struct drv_cdev_struct, cdev);
			entry = container_of(cdev, struct pf_dev_struct, cdev);
		}
		break;
	}

	case PF_DEV_CACHE_DEV:
	{
		struct device *dev = (struct device*)cache;
		entry = dev_get_drvdata(dev);
		break;
	}

	default:
		break;
	}

	if (entry) {
		if (!entry->enabled || !kref_get_unless_zero(&entry->refcount))
			entry = NULL;
	}

	mutex_unlock(&pf_dev_lock);
	return entry;
}

/*
 * Decrement the refcount of a pf_dev_struct.
 */
void put_pf_dev_entry(struct pf_dev_struct *pf_dev)
{
	if (pf_dev)
		kref_put(&pf_dev->refcount, release_pf_dev_entry);
}

/*
 * Register a user application with a driver device.
 */
int add_pf_dev_app(struct pf_dev_struct *pf_dev, struct task_struct *task)
{
	int ret = 0;
	struct pid *pid = NULL;
	struct pf_dev_application *app = NULL, *pos = NULL;

	if (!pf_dev || !task)
		return -EINVAL;
	
	mutex_lock(&pf_dev->app_lock);
	pid = get_task_pid(task, PIDTYPE_PID);

	if (!pid) {
		ret = -EINVAL;
		goto done;
	}

	/* Do nothing if app already exists. */
	list_for_each_entry(pos, &pf_dev->apps, list) {
		if (pid == pos->pid)
			goto done;
	}

	app = devm_kzalloc(&pf_dev->pci->dev,
		sizeof(struct pf_dev_application), GFP_KERNEL);

	if (!app) {
		ret = -ENOMEM;
		goto done;
	}

	app->pid = pid;
	INIT_LIST_HEAD(&app->list);
	list_add(&app->list, &pf_dev->apps);

done:
	/* Don't call `put_pid` here */
	mutex_unlock(&pf_dev->app_lock);
	return ret;
}

/*
 * Deregister a user application from a driver device.
 */
int delete_pf_dev_app(struct pf_dev_struct *pf_dev, struct task_struct *task)
{
	struct pid *pid = NULL;
	struct pf_dev_application *pos = NULL, *next = NULL;

	if (!pf_dev || !task)
		return -EINVAL;

	mutex_lock(&pf_dev->app_lock);
	pid = get_task_pid(task, PIDTYPE_PID);

	/* Do nothing if app is not in list. */
	list_for_each_entry_safe(pos, next, &pf_dev->apps, list) {
		if (pid == pos->pid) {
			put_pid(pos->pid);
			list_del(&pos->list);
			devm_kfree(&pf_dev->pci->dev, pos);
			break;
		}
	}

	put_pid(pid);
	mutex_unlock(&pf_dev->app_lock);
	return 0;
}

/*
 * Attempt to kill all user applications attached to a device.
 */
int kill_pf_dev_apps(struct pf_dev_struct *pf_dev, int sig)
{
	int ret = 0, r = 0;
	struct pf_dev_application *pos = NULL, *next = NULL;

	if (!pf_dev)
		return -EINVAL;

	mutex_lock(&pf_dev->app_lock);

	list_for_each_entry_safe(pos, next, &pf_dev->apps, list) {
		if ((r = kill_pid(pos->pid, sig, 0))) {
			/* Could not send signal. */
			if (!ret)
				ret = r;
		} else {
			put_pid(pos->pid);
			list_del(&pos->list);
			devm_kfree(&pf_dev->pci->dev, pos);
		}
	}

	mutex_unlock(&pf_dev->app_lock);
	return ret;
}

/**
 * create_map_str() - Create a device mapping string.
 * @map_str: Output parameter for the string - must be NULL
 * @map_str_sz: Output parameter for the string size - must be 0
 * 
 * This function is used to allow the user to see what BDF numbers
 * correspond to which device files. For example, the string:
 *      2
 *      c1:00.0 1 3
 *      21:00.0 2 4
 * ...means that there are two devices attached to the driver;
 * the device c1:00.0 has a character device file at /dev/ami1 and a hwmon tree
 * at /sys/class/hwmon3; the device 21:00.0 has a character device file at
 * /dev/ami2 and a hwmon tree at /sys/class/hwmon4.
 * 
 * Return: 0 or negative error code
 */
int create_map_str(char **map_str, int *map_str_sz)
{
	int ret = 0;
	int num_entries = 0;
	struct pci_dev *pci = NULL;
	struct pf_dev_struct *pf_dev = NULL;
	char temp[BDF_STR_LEN + 11] = { 0 };  /* +11 for " %d %d\n\0" (3 digits) */
	char *map = NULL;
	int map_sz = 1;

	if(!map_str || !map_str_sz || (*map_str) || (*map_str_sz != 0)) {
		ret = -EINVAL;
		goto fail;
	}

	map_sz = 1;
	map = kzalloc(map_sz, GFP_KERNEL);

	if(!map) {
		ret = -ENOMEM;
		goto fail;
	}

	while ((pci = pci_get_device(PCIE_VENDOR_ID, PCIE_DEVICE_ID, pci)) != NULL) {
		if((pci->driver) && (strcmp(pci->driver->name, DEFAULT_DEVICE_NAME) == 0)) {
			pf_dev = pci_get_drvdata(pci);
			sprintf(temp, "%02x:%02x.%1x %d %d\n", pci->bus->number,
				PCI_SLOT(pci->devfn), PCI_FUNC(pci->devfn),
				MINOR(pf_dev->cdev.cdev_num), pf_dev->hwmon_id);

			strconcat(&map, temp, &map_sz);
			num_entries++;
		}
	}

	/* Add number of entries to top of map. */
	*map_str_sz = 1;
	*map_str = kzalloc(*map_str_sz, GFP_KERNEL);

	if (!(*map_str)) {
		ret = -ENOMEM;
		goto fail;
	}

	sprintf(temp, "%d\n", num_entries);
	strconcat(map_str, temp, map_str_sz);
	strconcat(map_str, map, map_str_sz);
	kfree(map);
	return SUCCESS;

fail:
	if (*map_str) {
		kfree(*map_str);
		*map_str = NULL;
		*map_str_sz = 0;
	}

	if (map)
		kfree(map);

	return ret;
}

/**
 * devices_show() - Sysfs read callback for 'devices' attribute.
 * @drv: Driver that this attribute belongs to.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t devices_show(struct device_driver *drv, char *buf)
{
	int ret = 0;
	char *map_str = NULL;
	int map_str_sz = 0;
	int n = 0;

	if (!drv || !buf)
		return 0;

	ret = create_map_str(&map_str, &map_str_sz);

	if (!ret && map_str) {
		n = sprintf(buf, "%s", map_str);
		kfree(map_str);
	}

	return n;
}
static DRIVER_ATTR_RO(devices);

/**
 * version_show() - Sysfs read callback for 'version' attribute.
 * @drv: Driver that this attribute belongs to.
 * @buf: Output character buffer.
 * 
 * Return: Number of bytes written to output buffer.
 */
static ssize_t version_show(struct device_driver *drv, char *buf)
{
	if (!drv || !buf)
		return 0;

	/* Format is MAJOR.MINOR.PATCH +COMMITS *STATUS */
	return sprintf(
		buf,
		"%hhd.%hhd.%hhd +%hhd *%hhd\n",
		GIT_TAG_VER_MAJOR,
		GIT_TAG_VER_MINOR,
		GIT_TAG_VER_PATCH,
		GIT_TAG_VER_DEV_COMMITS,
		GIT_STATUS
	);
}
static DRIVER_ATTR_RO(version);

int __init vmc_entry(void)
{
	int ret = 0;

	/* Init FAL for GCQ */
	ret = ulFW_IF_GCQ_init(&fw_if_gcq_init_cfg);
	if(ret != FW_IF_ERRORS_NONE)
		goto fail;

	PR_DBG("Loading driver to the kernel");

	/* Register the device driver with the kernel */
	ret = register_driver_kernel();
	if (ret)
		goto fail;

	/* Register the device driver with the PCIE Core */
	ret = register_driver_pcie();
	if (ret)
		goto unreg_drv_krnl_pf0;
	
	/* Create 'devices' attribute */
	ret = driver_create_file(&pcie_driver_core.driver, &driver_attr_devices);
	if (ret)
		goto unreg_drv_pci;

	/* Create 'version' attribute */
	ret = driver_create_file(&pcie_driver_core.driver, &driver_attr_version);
	if (ret)
		goto remove_device_attr;

	PR_INFO("Successfully loaded driver to the kernel");
	return SUCCESS;

remove_device_attr:
	driver_remove_file(&pcie_driver_core.driver, &driver_attr_devices);

unreg_drv_pci:
	pci_unregister_driver(&pcie_driver_core);

unreg_drv_krnl_pf0:
	unregister_driver_kernel();

fail:
	PR_ERR("Failed to load driver to the kernel");
	return ret;
}

/* Module cleanup function
 * __exit will put this in separate section of elf file
 * to enable this code only at module removal time.
 * Note - Do not call this function from anywhere in the code
 */
void __exit vmc_exit(void)
{
	PR_DBG("Removing driver from the kernel");
	PR_DBG("Unregister driver from PCIE Stack");

	/* Remove attributes */
	driver_remove_file(&pcie_driver_core.driver, &driver_attr_devices);
	driver_remove_file(&pcie_driver_core.driver, &driver_attr_version);

	/* Unregister driver */
	pci_unregister_driver(&pcie_driver_core);
	unregister_driver_kernel();

	PR_INFO("Successfully removed driver");
}

module_init(vmc_entry);
module_exit(vmc_exit);

MODULE_AUTHOR(MDL_AUTHOR);
MODULE_VERSION(MDL_VERSION);
MODULE_DESCRIPTION(MDL_DESCRIPTION);
MODULE_LICENSE(MDL_LICENSE);

//This should contain the minimum firmware version on versal device for this driver to communicate to Versal over PCIE
//MODULE_FIRMWARE(" ");
