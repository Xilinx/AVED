// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_cdev.h - This file contains definitions related to AMI character device files.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_CDEV_H
#define AMI_CDEV_H

#include <linux/pci.h>
#include <linux/types.h>
#include <linux/kdev_t.h>  /* udev = device manager for the Linux kernel that creates/removes device nodes in the /dev */
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>  /* copy_to/from_user() */

#include "ami.h"

#define DEFAULT_CDEV_COUNT	1
#define DEV_NAME_SIZE		50
#define CLS_STR_SIZE		30
#define DEFAULT_CLS_NAME	KBUILD_MODNAME
#define DEFAULT_DEVICE_NAME	KBUILD_MODNAME
#define DEFAULT_CDEV_BASEMINOR	(unsigned)(0)

/* IOCTL data. Shared with userspace code. */

#define AMI_IOC_FPT_UPDATE_MAGIC	(0xAAAAAAAA)
#define AMI_IOC_SENSOR_STATUS_LEN	(40)

/**
 * struct ami_ioc_data_payload - payload struct for dynamically sized ioctl data
 * @size: Size of data buffer.
 * @addr: Location of data buffer in userspace memory.
 * @partition: Partition number to flash (also used for boot select).
 * @src: Source partition (for copy operation).
 * @dest: Destination partition (for copy operation).
 * @cap_override: Bypass permission checks. This may not apply to all IOCTL's.
 * @efd: File descriptor for event notifications (used for progress reporting when
 *     performing long running operations like PDI downloads) - optional
 *
 * Note that addr can be an address to any arbitrary data type,
 * depending on the context. This struct is reused for the boot select
 * command, with only the `partition` field populated.
 *
 * When performing a PDI download operation, if `partition` is equal to
 * `FPT_UPDATE_MAGIC`, the FPT will be overwritten with the provided
 * PDI image!
 */
struct ami_ioc_data_payload {
	uint32_t       size;
	unsigned long  addr;
	uint32_t       partition;
	uint32_t       src;
	uint32_t       dest;
	bool           cap_override;
	int            efd;
};

/**
 * struct ami_ioc_bar_data - payload struct for PCI BAR-related IOCTL calls
 * @num: Number of BAR registers (to read or write).
 * @addr: Userspace address of data payload (read or write).
 * @bar_idx: Bar number.
 * @offset: Offset within BAR.
 * @cap_override: Bypass permission checks. This may not apply to all IOCTL's.
 * 
 * Note that this struct can be used for either read or write transactions.
 *
 * For reading a BAR, `addr` is the userspace address of a uint32 buffer to be
 * populated with data read from the BAR and `num` is the number of values to read.
 * 
 * To write to a BAR, `addr` is the userspace address of the uint32 buffer to
 * write and `num` is the number of values to write.
 */
struct ami_ioc_bar_data {
	uint32_t       num;
	unsigned long  addr;
	uint8_t        bar_idx;
	uint64_t       offset;
	bool           cap_override;
};

/**
 * enum ami_ioc_sensor_type - list of supported sensor types
 * @IOC_SENSOR_TYPE_TEMP: Temperature sensor ("temp" in hwmon)
 * @IOC_SENSOR_TYPE_VOLTAGE: Voltage sensor ("in" in hwmon)
 * @IOC_SENSOR_TYPE_CURRENT: Current sensor ("curr" in hwmon)
 * @IOC_SENSOR_TYPE_POWER: Power sensor ("power" in hwmon)
 * 
 * This enum is to be use in conjunction with `struct ami_ioc_sensor_value`.
 */
enum ami_ioc_sensor_type {
	IOC_SENSOR_TYPE_TEMP,
	IOC_SENSOR_TYPE_VOLTAGE,
	IOC_SENSOR_TYPE_CURRENT,
	IOC_SENSOR_TYPE_POWER,
};

/**
 * struct ami_ioc_sensor_value - single sensor value and status
 * @val: Sensor value. Populated by the driver.
 * @status: Sensor status. Populated by the driver.
 * @fresh: Whether or not the value/status were read from the cache. Populated by the driver.
 * @hwmon_channel: The hwmon sensor channel number. This is passed in from userspace.
 * @sensor_type: Sensor type. This is passed in from userspace.
 * 
 * This API is provided as an alternative to hwmon due to a possible race condition
 * where the user would have to read two separate hwmon files to fetch both the
 * sensor value and its status. Currently, this is only supported for the
 * instantaneous value of a sensor.
 */
struct ami_ioc_sensor_value {
	long    val;
	char    status[AMI_IOC_SENSOR_STATUS_LEN];
	bool    fresh;
	int     hwmon_channel;
	int     sensor_type;
};

/**
 * struct ami_ioc_fpt_hdr_value - the fpt header
 * @version: The version. Populated by the driver.
 * @hdr_size: The size of the header in bytes. Populated by the driver.
 * @entry_size: The entry size in bytes. Populated by the driver.
 * @num_entries: The number of partitions. Populated by the driver.
 * 
 * This struct is to be use in conjunction with `struct ami_ioc_fpt_hdr_value` to get the
 * individual partition information.
 */
struct ami_ioc_fpt_hdr_value {
	uint8_t version;
	uint8_t hdr_size;
	uint8_t entry_size;
	uint8_t num_entries;
};

/**
 * struct ami_ioc_fpt_partition_value - the individual partition information.
 * @partition: This is passed in from userspace.
 * @type: The partition type. Populated by the driver.
 * @base_addr: The partition base address. Populated by the driver.
 * @partition_size: The parition size. Populated by the driver.
 */
struct ami_ioc_fpt_partition_value {
	uint32_t partition;
	uint32_t type;
	uint32_t base_addr;
	uint32_t partition_size;
};

/**
 * struct ami_ioc_eeprom_payload - payload struct for dynamically sized ioctl eeprom data
 * @addr: Location of data buffer in userspace memory.
 * @len: The number of bytes to read/write
 * @offset: Offset from the EEPROM base address.
 */
struct ami_ioc_eeprom_payload {
        unsigned long  addr;
	uint8_t        len;
	uint8_t        offset;
};

/**
 * struct ami_ioc_module_payload - payload struct for dynamically sized ioctl qsfp data
 * @addr: Location of data buffer in userspace memory.
 * @device_id: Module device ID.
 * @page: Page number to access.
 * @len: Number of bytes to read/write.
 * @offset: Offset within page.
 */
struct ami_ioc_module_payload {
	unsigned long addr;
	uint8_t       device_id;
	uint8_t       page;
	uint8_t       len;
	uint8_t       offset;
};

/**
 * enum ami_ioc_app_setup - accepted values for the AMI_IOC_APP_SETUP IOCTL
 * @IOC_APP_SETUP_REGISTER: Register a process with a device.
 * @IOC_APP_SETUP_DEREGISTER: Deregister a process from a device.
 */
enum ami_ioc_app_setup {
	IOC_APP_SETUP_REGISTER,
	IOC_APP_SETUP_DEREGISTER,
};

#define AMI_IOC_MAGIC			'a'
#define AMI_IOC_DOWNLOAD_PDI		_IOW(AMI_IOC_MAGIC, 0, struct ami_ioc_data_payload*)
#define AMI_IOC_READ_BAR		_IOWR(AMI_IOC_MAGIC, 1, struct ami_ioc_bar_data*)
#define AMI_IOC_WRITE_BAR		_IOW(AMI_IOC_MAGIC, 2, struct ami_ioc_bar_data*)
#define AMI_IOC_GET_SENSOR_VALUE	_IOWR(AMI_IOC_MAGIC, 3, struct ami_ioc_sensor_value*)
#define AMI_IOC_DEVICE_BOOT		_IOW(AMI_IOC_MAGIC, 4, struct ami_ioc_data_payload*)
#define AMI_IOC_COPY_PARTITION		_IOW(AMI_IOC_MAGIC, 5, struct ami_ioc_data_payload*)
#define AMI_IOC_SET_SENSOR_REFRESH	_IOW(AMI_IOC_MAGIC, 6, uint16_t)
#define AMI_IOC_GET_FPT_HDR		_IOR(AMI_IOC_MAGIC, 7, struct ami_ioc_fpt_hdr_value*)
#define AMI_IOC_GET_FPT_PARTITION	_IOWR(AMI_IOC_MAGIC, 8, struct ami_ioc_fpt_partition_value*)
#define AMI_IOC_READ_EEPROM		_IOWR(AMI_IOC_MAGIC, 9, struct ami_ioc_eeprom_payload*)
#define AMI_IOC_WRITE_EEPROM		_IOW(AMI_IOC_MAGIC, 10, struct ami_ioc_eeprom_payload*)
#define AMI_IOC_APP_SETUP		_IOW(AMI_IOC_MAGIC, 11, enum ami_ioc_app_setup)
#define AMI_IOC_READ_MODULE		_IOW(AMI_IOC_MAGIC, 12, struct ami_ioc_module_payload*)
#define AMI_IOC_WRITE_MODULE		_IOW(AMI_IOC_MAGIC, 13, struct ami_ioc_module_payload*)
#define AMI_IOC_MAX			(14)

/* End shared data. */

/**
 * struct drv_cdev_struct - character device data.
 * 
 * @dev_name: Character device name.
 * @drv_cls_str: Character device class name.
 * @count: Number of minor numbers corresponding to this device. Should be 1.
 * @cdev_num: Character device number.
 * @cdev: Character device struct.
 * @dev_class: Pointer to character device class struct.
 * @device: Pointer to character device struct.
 */
struct drv_cdev_struct {
	char		dev_name[DEV_NAME_SIZE];
	char		drv_cls_str[CLS_STR_SIZE];
	unsigned	count;
	dev_t		cdev_num;
	struct cdev	cdev;
	struct class	*dev_class;
	struct device	*device;
};

/* Standard Linux callbacks */
int dev_open(struct inode *inode, struct file *filp);
int dev_close(struct inode *inode, struct file *filp);
long dev_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

/**
 * create_cdev() - Create a character device file.
 * @baseminor: The minor number to use for this device
 * @drv_cdev: The drv_cdev object for this device
 * @parent: Parent device - this is used to set the parent kobj of the cdev
 * @fops: file operations struct
 * 
 * Note that this will only ever create a single character device.
 * (count == 1). This function tries to do the following:
 *      * Alloc / register chrdev region
 *      * If first device, create class
 *      * cdev init
 *      * cdev add
 * 
 * Return: 0 or negative error code
 */
int create_cdev(unsigned baseminor, struct drv_cdev_struct *drv_cdev,
	struct device *parent, const struct file_operations *fops);

#endif /* AMI_CDEV_H */
