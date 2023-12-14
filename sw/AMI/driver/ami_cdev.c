// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_cdev.c - This file contains logic related to AMI character device files.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/pci.h>     /* pci_dev */
#include <linux/errno.h>   /* error codes */
#include <linux/kernel.h>  /* container_of */
#include <linux/slab.h>    /* kzalloc... */
#include <linux/string.h>  /* string funcs */
#include <linux/fs.h>      /* file_operations */
#include <linux/types.h>
#include <linux/hwmon.h>
#include <linux/eventfd.h>

#include "ami.h"
#include "ami_hwmon.h"
#include "ami_top.h"
#include "ami_cdev.h"
#include "ami_utils.h"
#include "ami_pcie.h"
#include "ami_program.h"
#include "ami_eeprom.h"
#include "ami_utils.h"
#include "ami_module.h"

#define ROOT_USER                (0)
#define READ_WRITE               (0666)
#define IS_ROOT_USER(uid, euid)  (capable(CAP_DAC_OVERRIDE) || (uid == ROOT_USER) || (euid == ROOT_USER))


static int dev_major = 0;  /* This will be overriden. */


/**
 * devnode() - Callback to return device permissions.
 * @dev: Pointer to device struct.
 * @mode: Pointer to store permission bits.
 * 
 * Return: NULL.
 */
static char *devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = READ_WRITE;

	return NULL;
}

/*
 * Open a device file - this increments the pf_dev refcount.
 */
int dev_open(struct inode *inode, struct file *filp)
{
	if (!inode || !filp)
		return -EINVAL;
	
	/* This already checks the minor number */
	filp->private_data = get_pf_dev_entry((void*)inode, PF_DEV_CACHE_INODE);

	if (!filp->private_data)
		return -ENODEV;

	return 0;
}

/*
 * Close a device file - this decrements the pf_dev refcount.
 */
int dev_close(struct inode *inode, struct file *filp)
{
	if (!inode || !filp)
		return -EINVAL;

	if (filp->private_data)
		put_pf_dev_entry((struct pf_dev_struct*)filp->private_data);

	return 0;
}

/*
 * This function will be called when we use IOCTL with command on the Device file
 */
long dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;
	/* eventfd is used for sending notifications to the user */
	struct eventfd_ctx *efd_ctx = NULL;

	if (!filp)
		return -EINVAL;

	/*
	 * Extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok(  )
	 */
	if(_IOC_TYPE(cmd) != AMI_IOC_MAGIC) return -ENOTTY;
	if(_IOC_NR(cmd) > AMI_IOC_MAX) return -ENOTTY;

	if(!access_ok((void __user*)arg, _IOC_SIZE(cmd)))
		return -ENOTTY;
	
	/* This is is already reference counted  */
	pf_dev = filp->private_data;

	/* Check device data */
	if (!pf_dev) {
		PR_ERR("dev_unlocked_ioctl: unable to find card");
		return -ENODEV;
	}

	/* Check device state */
	switch (cmd) {
	/* READY, MISSING_INFO or COMPAT only */
	case AMI_IOC_DOWNLOAD_PDI:
	case AMI_IOC_DEVICE_BOOT:
		switch (pf_dev->state) {
		case PF_DEV_STATE_COMPAT:
		case PF_DEV_STATE_READY:
		case PF_DEV_STATE_MISSING_INFO:
			break;
		
		default:
			return -EPERM;
		}
		break;

	/* Any state except INIT and SHUTDOWN */
	case AMI_IOC_READ_BAR:
	case AMI_IOC_WRITE_BAR:
	case AMI_IOC_APP_SETUP:
		switch (pf_dev->state) {
		case PF_DEV_STATE_INIT:
		case PF_DEV_STATE_SHUTDOWN:
			return -EPERM;

		default:
			break;
		}
		break;

	/* READY or MISSING_INFO only */
	case AMI_IOC_GET_SENSOR_VALUE:
	case AMI_IOC_COPY_PARTITION:
	case AMI_IOC_SET_SENSOR_REFRESH:
	case AMI_IOC_GET_FPT_HDR:
	case AMI_IOC_GET_FPT_PARTITION:
	case AMI_IOC_READ_EEPROM:
	case AMI_IOC_WRITE_EEPROM:
	case AMI_IOC_READ_MODULE:
	case AMI_IOC_WRITE_MODULE:
		switch (pf_dev->state) {
		case PF_DEV_STATE_READY:
		case PF_DEV_STATE_MISSING_INFO:
			break;
		
		default:
			return -EPERM;
		}
		break;

	default:
		break;
	}

	/* Acquire semaphore */
	if (down_interruptible(&(pf_dev->ioctl_sema)))
		return -ERESTARTSYS;
	
	if (pf_dev->state == PF_DEV_STATE_COMPAT)
		PR_WARN("Performing IOCTL request in compatibility mode - you may experience issues!");

	/* Handle command */
	switch (cmd) {
	case AMI_IOC_SET_SENSOR_REFRESH:
	{
		/*
		 * This does the same thing as writing to the `update_interval`
		 * hwmon file, but we do not require any sudo permissions here.
		 */
		pf_dev->sensor_refresh = (uint16_t)arg;
		break;
	}

	case AMI_IOC_DOWNLOAD_PDI:
	{
		/*
		 * `arg` is a pointer to the `ami_ioc_data_payload` struct
		 * This struct contains the address of the actual data buffer.
		 */
		struct ami_ioc_data_payload data = { 0 };
		uint8_t *buf = NULL;

		/* Check PF - currently only PF0 supported for this command. */
		if (pf_dev->pcie_function_num != 0) {
			ret = -ENODEV;
			goto done;
		}

		/* Read data payload from user. */
		if (copy_from_user(&data, (struct ami_ioc_data_payload*)arg, sizeof(data))) {
			ret = -EFAULT; /* Bad address */
			goto done;
		}

		/* Check permissions. */
		if (!(data.cap_override || IS_ROOT_USER(current_uid().val, current_euid().val))) {
			ret = -EPERM;
			goto done;
		}
		
		if ((data.size <= 0) || (data.addr == 0)) {
			ret = -EINVAL;
			goto done;
		}

		/*
		 * Using vzalloc because the PDI buffer will be too large
		 * for kzalloc (around 4-6MB)
		 */
		buf = vzalloc(data.size);

		if (!buf) {
			ret = -ENOMEM;
			goto done;
		}

		if (data.efd >= 0)
			efd_ctx = eventfd_ctx_fdget(data.efd);

		/* Read actual data buffer. `addr` is a pointer to uint8_t */
		if(!copy_from_user(buf, (uint8_t*)data.addr, data.size)) {
			if (data.partition == AMI_IOC_FPT_UPDATE_MAGIC)
				ret = update_fpt(
					pf_dev,
					buf,
					data.size,
					efd_ctx
				);
			else
				ret = download_pdi(
					pf_dev->amc_ctrl_ctxt,
					buf,
					data.size,
					data.partition,
					efd_ctx
				);
		} else {
			ret = -EFAULT;
		}

		vfree(buf);
		break;
	}

	case AMI_IOC_DEVICE_BOOT:
	{
		/*
		 * `arg` is a pointer to the `ami_ioc_data_payload` struct.
		 * Only the `partition` field should be populated.
		 */
		struct ami_ioc_data_payload data = { 0 };

		/* Check PF - currently only PF0 supported for this command. */
		if (pf_dev->pcie_function_num != 0) {
			ret = -ENODEV;
			goto done;
		}

		/* Read data payload from user. */
		if (copy_from_user(&data, (struct ami_ioc_data_payload*)arg, sizeof(data))) {
			ret = -EFAULT; /* Bad address */
			goto done;
		}

		/* Check permissions. */
		if (!(data.cap_override || IS_ROOT_USER(current_uid().val, current_euid().val))) {
			ret = -EPERM;
			goto done;
		}

		ret = device_boot(pf_dev, data.partition);

		break;
	}

	case AMI_IOC_COPY_PARTITION:
	{
		/*
		 * `arg` is a pointer to the `ami_ioc_data_payload` struct.
		 * Only the `partition` field should be populated.
		 */
		struct ami_ioc_data_payload data = { 0 };

		/* Check PF - currently only PF0 supported for this command. */
		if (pf_dev->pcie_function_num != 0) {
			ret = -ENODEV;
			goto done;
		}

		/* Read data payload from user. */
		if (copy_from_user(&data, (struct ami_ioc_data_payload*)arg, sizeof(data))) {
			ret = -EFAULT; /* Bad address */
			goto done;
		}

		/* Check permissions. */
		if (!(data.cap_override || IS_ROOT_USER(current_uid().val, current_euid().val))) {
			ret = -EPERM;
			goto done;
		}

		ret = copy_partition(pf_dev, data.src, data.dest);
		break;
	}

        case AMI_IOC_READ_BAR:
	{
		/*
		 * `arg` is a pointer to the `ami_ioc_bar_data` struct.
		 */
		uint32_t *buf = NULL;
		struct ami_ioc_bar_data data = { 0 };

		/* Read data payload. */
		if (copy_from_user(&data, (struct ami_ioc_bar_data*)arg, sizeof(data))) {
			ret = -EFAULT;
			goto done;
		}

		/* Check permissions. */
		if (!(data.cap_override || IS_ROOT_USER(current_uid().val, current_euid().val))) {
			ret = -EPERM;
			goto done;
		}

		if ((data.num <= 0) || (data.addr == 0)) {
			ret = -EINVAL;
			goto done;
		}

		/* Allocate memory for response buffer. */
		buf = vzalloc(data.num * sizeof(uint32_t));

		if (!buf) {
			ret = -ENOMEM;
			goto done;
		}
		
		/* We will write the response to the userspace address. */
		ret = read_pcie_bar(pf_dev->pci, data.bar_idx,
			data.offset, data.num, buf);

		if (!ret)
			ret = copy_to_user((uint32_t*)data.addr, buf,
				data.num * sizeof(uint32_t));

		vfree(buf);
		break;
	}

	case AMI_IOC_WRITE_BAR:
	{
		/*
		 * `arg` is a pointer to the `ami_ioc_bar_data` struct.
		 */
		uint32_t *buf = NULL;
		struct ami_ioc_bar_data data = { 0 };

		/* Read data payload. */
		if (copy_from_user(&data, (struct ami_ioc_bar_data*)arg, sizeof(data))) {
			ret = -EFAULT;
			goto done;
		}

		/* Check permissions. */
		if (!(data.cap_override || IS_ROOT_USER(current_uid().val, current_euid().val))) {
			ret = -EPERM;
			goto done;
		}

		if ((data.num <= 0) || (data.addr == 0)) {
			ret = -EINVAL;
			goto done;
		}
		
		/* Allocate memory for payload buffer. */
		buf = vzalloc(data.num * sizeof(uint32_t));

		if (!buf) {
			ret = -ENOMEM;
			goto done;
		}
		
		/* Copy payload data. */
		if (!copy_from_user(buf, (uint32_t*)data.addr, data.num * sizeof(uint32_t)))
			ret = write_pcie_bar(pf_dev->pci, data.bar_idx,
				data.offset, data.num, buf);
		else
			ret = -EFAULT;
		
		vfree(buf);
		break;
	}

	case AMI_IOC_GET_SENSOR_VALUE:
	{
		/* `arg` is a pointer to `struct ami_ioc_sensor_value` */
		struct ami_ioc_sensor_value data = { 0 };
		enum hwmon_sensor_types hwmon_type = 0;
		uint32_t hwmon_attr = 0;

		if (copy_from_user(&data, (struct ami_ioc_sensor_value*)arg, sizeof(data))) {
			ret = -EFAULT;
			goto done;
		}

		/* Currently, only the instant sensor value is supported with this API. */
		switch (data.sensor_type) {
		case IOC_SENSOR_TYPE_TEMP:
			hwmon_type = hwmon_temp;
			hwmon_attr = hwmon_temp_input;
			break;
		
		case IOC_SENSOR_TYPE_POWER:
			hwmon_type = hwmon_power;
			hwmon_attr = hwmon_power_input;
			break;
		
		case IOC_SENSOR_TYPE_CURRENT:
			hwmon_type = hwmon_curr;
			hwmon_attr = hwmon_curr_input;
			break;
		
		case IOC_SENSOR_TYPE_VOLTAGE:
			hwmon_type = hwmon_in;
			hwmon_attr = hwmon_in_input;
			break;
		
		default:
			ret = -EINVAL;
			break;
		}

		if (ret)
			goto done;

		ret = read_sensor_val(
			pf_dev,
			hwmon_type,
			hwmon_attr,
			data.hwmon_channel,
			&data.val,
			data.status,
			&data.fresh
		);

		if (!ret)
			ret = copy_to_user((struct ami_ioc_sensor_value*)arg,
				&data, sizeof(data));

		break;
	}

	case AMI_IOC_GET_FPT_HDR:
	{
                /* `arg` is a pointer to `struct ami_ioc_fpt_hdr_value` */
                struct ami_ioc_fpt_hdr_value data = { 0 };
                struct fpt_header hdr = { 0 };

                ret = read_fpt_hdr(pf_dev, &hdr);
                if (!ret) {
                        data.version = hdr.version;
                        data.hdr_size = hdr.header_size;
                        data.entry_size = hdr.entry_size;
                        data.num_entries = hdr.num_entries;
                        ret = copy_to_user((struct ami_ioc_fpt_hdr_value*)arg,
					   &data, sizeof(data));
                }
                break;
	}

	case AMI_IOC_GET_FPT_PARTITION:
	{
                /* `arg` is a pointer to `struct ami_ioc_fpt_partition_value` */
                struct ami_ioc_fpt_partition_value data = { 0 };
                struct fpt_partition partition = { 0 };

                if (copy_from_user(&data, (struct ami_ioc_fpt_partition_value*)arg, sizeof(data))) {
                        ret = -EFAULT;
                        goto done;
                }

                ret = read_fpt_partition(pf_dev,
                                         data.partition,
                                         &partition);
                if (!ret) {
                        data.type = partition.type;
                        data.base_addr = partition.base_addr;
                        data.partition_size = partition.partition_size;
                        ret = copy_to_user((struct ami_ioc_fpt_partition_value*)arg,
					   &data, sizeof(data));
                }
                break;
	}

        case AMI_IOC_READ_EEPROM:
        {
                struct ami_ioc_eeprom_payload data = { 0 };
                uint8_t *buf = NULL;

                /* Read data payload. */
                if (copy_from_user(&data, (struct ami_ioc_eeprom_payload*)arg, sizeof(data))) {
                        ret = -EFAULT;
                        goto done;
                }

                if ((data.len <= 0) || (data.addr == 0)) {
			ret = -EINVAL;
			goto done;
		}

                /* Allocate memory for response buffer. */
		buf = vzalloc(data.len * sizeof(uint8_t));

		if (!buf) {
			ret = -ENOMEM;
			goto done;
		}

                ret = eeprom_read(pf_dev->amc_ctrl_ctxt, buf, data.len, data.offset);
                if (!ret) {
                        ret = copy_to_user((uint8_t*)data.addr, buf,
				data.len * sizeof(uint8_t));
                }
                vfree(buf);
                break;
        }

        case AMI_IOC_WRITE_EEPROM:
        {
                struct ami_ioc_eeprom_payload data = { 0 };
		uint8_t *buf = NULL;

		/* Read data payload. */
		if (copy_from_user(&data, (struct ami_ioc_eeprom_payload*)arg, sizeof(data))) {
			ret = -EFAULT;
			goto done;
		}

		if ((data.len <= 0) || (data.addr == 0)) {
			ret = -EINVAL;
			goto done;
		}

		/* Allocate memory for payload buffer. */
		buf = vzalloc(data.len * sizeof(uint8_t));

		if (!buf) {
			ret = -ENOMEM;
			goto done;
		}

		/* Copy payload data. */
		if (!copy_from_user(buf, (uint8_t*)data.addr, data.len * sizeof(uint8_t)))
			ret = eeprom_write(pf_dev->amc_ctrl_ctxt, buf, data.len, data.offset);
                else
			ret = -EFAULT;

		vfree(buf);
                break;
        }

	case AMI_IOC_APP_SETUP:
		switch ((enum ami_ioc_app_setup)arg) {
		case IOC_APP_SETUP_REGISTER:
			ret = add_pf_dev_app(pf_dev, get_current());
			break;
		
		case IOC_APP_SETUP_DEREGISTER:
			ret = delete_pf_dev_app(pf_dev, get_current());
			break;
		
		default:
			ret = -EINVAL;
			break;
		}
		break;

	case AMI_IOC_READ_MODULE:
	{
		struct ami_ioc_module_payload data = { 0 };
		uint8_t *buf = NULL;

		/* Read data payload. */
		if (copy_from_user(&data, (struct ami_ioc_module_payload*)arg, sizeof(data))) {
			ret = -EFAULT;
			goto done;
		}

		if ((data.len <= 0) || (data.addr == 0)) {
			ret = -EINVAL;
			goto done;
		}

		/* Allocate memory for response buffer. */
		buf = vzalloc(data.len * sizeof(uint8_t));

		if (!buf) {
			ret = -ENOMEM;
			goto done;
		}

		ret = module_read(
			pf_dev->amc_ctrl_ctxt,
			data.device_id,
			data.page,
			data.offset,
			buf,
			data.len
		);

		if (!ret) {
			ret = copy_to_user((uint8_t*)data.addr, buf,
				data.len * sizeof(uint8_t));
		}
		vfree(buf);
		break;
	}

	case AMI_IOC_WRITE_MODULE:
	{
		struct ami_ioc_module_payload data = { 0 };
		uint8_t *buf = NULL;

		/* Read data payload. */
		if (copy_from_user(&data, (struct ami_ioc_module_payload*)arg, sizeof(data))) {
			ret = -EFAULT;
			goto done;
		}

		if ((data.len <= 0) || (data.addr == 0)) {
			ret = -EINVAL;
			goto done;
		}

		/* Allocate memory for payload buffer. */
		buf = vzalloc(data.len * sizeof(uint8_t));

		if (!buf) {
			ret = -ENOMEM;
			goto done;
		}

		/* Copy payload data. */
		if (!copy_from_user(buf, (uint8_t*)data.addr, data.len * sizeof(uint8_t)))
			ret = module_write(
				pf_dev->amc_ctrl_ctxt,
				data.device_id,
				data.page,
				data.offset,
				buf,
				data.len
			);
		else
			ret = -EFAULT;

		vfree(buf);
		break;
	}

	default:
		PR_ERR("Unknown command, do nothing");
		ret = -ENOTTY;
		break;
	}

done:
	if (efd_ctx)
		eventfd_ctx_put(efd_ctx);

	up(&(pf_dev->ioctl_sema));
	return ret;
}

/*
 * Create a character device.
 */
int create_cdev(unsigned baseminor, struct drv_cdev_struct *drv_cdev,
	struct device *parent, const struct file_operations *fops)
{
	int ret = 0;
	bool cls_created = false;

	/* parent may be NULL */
	if (!drv_cdev || !fops) {
		return -EINVAL;
	}

	/* Allocate chrdev region */
	drv_cdev->count = DEFAULT_CDEV_COUNT;
	if(dev_major) {
		drv_cdev->cdev_num = MKDEV(dev_major, baseminor);
		ret = register_chrdev_region(drv_cdev->cdev_num, DEFAULT_CDEV_COUNT,
			(const char*)DEFAULT_DEVICE_NAME);
	} else {
		/* This is the first device. */
		ret = alloc_chrdev_region(&(drv_cdev->cdev_num), baseminor,
			 DEFAULT_CDEV_COUNT, (const char*)DEFAULT_DEVICE_NAME);
		dev_major = MAJOR(drv_cdev->cdev_num);
	}

	if(ret)
		goto fail;

	/* If first device, create class. */
	strncpy(drv_cdev->drv_cls_str, (const char*)DEFAULT_CLS_NAME, CLS_STR_SIZE);

	if(!drv_cdev->dev_class) {
		cls_created = true;
		drv_cdev->dev_class = class_create(THIS_MODULE, drv_cdev->drv_cls_str);
		if (IS_ERR(drv_cdev->dev_class)) {
			ret = PTR_ERR(drv_cdev->dev_class);
			PR_ERR("Failed to create class %s. ret : %d",
				drv_cdev->drv_cls_str, ret);
			goto unreg_cdev_reg;
		}
		drv_cdev->dev_class->devnode = devnode;
	}

	/* Create device */
	snprintf(drv_cdev->dev_name, DEV_NAME_SIZE, "%s%d", DEFAULT_DEVICE_NAME, baseminor);
	drv_cdev->device = device_create(drv_cdev->dev_class, NULL,
		drv_cdev->cdev_num, NULL, drv_cdev->dev_name);
	if (IS_ERR(drv_cdev->device)) {
		ret = PTR_ERR(drv_cdev->device);
		PR_ERR("Failed to create device %s. ret : %d", drv_cdev->dev_name, ret);
		goto del_class;
	}

	/* Initialize the cdev structure */
	cdev_init(&(drv_cdev->cdev), fops);
	drv_cdev->cdev.owner = THIS_MODULE;
	drv_cdev->cdev.ops = fops;

	/*
	 * Setting the parent is necessary so that the kobject is referenced
	 * appropriately and the parent is not freed before the cdev.
	 */
	if (parent)
		cdev_set_parent(&drv_cdev->cdev, &parent->kobj);
	
	/* Register cdev to the kernel */
	ret = cdev_add(&(drv_cdev->cdev), drv_cdev->cdev_num, drv_cdev->count);
	if (ret) {
		PR_ERR("Failed to register cdev to the kernel, err_code : %d", ret);
		goto del_device;
	}

	return SUCCESS;

del_device:
	device_destroy(drv_cdev->dev_class, drv_cdev->cdev_num);

del_class:
	if(cls_created)
		class_destroy(drv_cdev->dev_class);

unreg_cdev_reg:
	unregister_chrdev_region(drv_cdev->cdev_num, DEFAULT_CDEV_COUNT);

fail:
	return ret;
}
