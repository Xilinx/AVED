// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_program.c - This file contains the implementation of device programming logic.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* Public API includes */
#include "ami_program.h"

/* Private API includes */
#include "ami_ioctl.h"
#include "ami_internal.h"
#include "ami_device_internal.h"

/*****************************************************************************/
/* Private functions                                                         */
/*****************************************************************************/

/**
 * read_file() - Read an entire file into a byte buffer.
 * @fname: Full path to file.
 * @buf: Pointer to byte buffer.
 * @size: Pointer to variable which will hold buffer size.
 *
 * Note that the caller is responsible for freeing the memory allocated
 * by this function.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
static int read_file(const char *fname, uint8_t **buf, uint32_t *size)
{
	FILE *fp = NULL;
	long offset = 0;
	size_t len = 0;
	int ret = AMI_STATUS_ERROR;
	uint8_t *buffer = NULL;

	if (!fname || !buf || !size)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	fp = fopen(fname, "rb");

	if (fp == NULL)
		return AMI_API_ERROR(AMI_ERROR_EBADF);
	
	/* Go to end of file. */
	if (fseek(fp, 0L, SEEK_END) != AMI_LINUX_STATUS_OK) {
		ret = AMI_API_ERROR(AMI_ERROR_EIO);
		goto close;
	}

	offset = ftell(fp);
	if (offset == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR(AMI_ERROR_EIO);
		goto close;
	}

	buffer = (uint8_t*)malloc(sizeof(uint8_t) * (offset + 1));

	if (!buffer) {
		ret = AMI_API_ERROR(AMI_ERROR_ENOMEM);
		goto close;
	}

	/* Go back to the start of the file. */
	if (fseek(fp, 0L, SEEK_SET) != AMI_LINUX_STATUS_OK) {
		ret = AMI_API_ERROR(AMI_ERROR_EIO);
		goto del_buf;
	}

	len = fread(buffer, sizeof(uint8_t), offset, fp);

	if (ferror(fp) == AMI_LINUX_STATUS_OK) {
		*size = (uint32_t)len;
		*buf = buffer;
		ret = AMI_STATUS_OK;
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EIO);
	}
	fclose(fp);
	return ret;

del_buf:
	free(buffer);

close:
	fclose(fp);
	return ret;
}

/**
 * do_image_download() - Perform an image download operation.
 * @dev: Device handle.
 * @path: Path to image file.
 * @partition: Partition number to program.
 * @progress_handler: Progress handler callback (optional).
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
static int do_image_download(ami_device *dev, const char *path, uint32_t partition,
	ami_event_handler progress_handler)
{
	uint8_t *img_data = NULL;
	uint32_t img_size = 0;
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_data_payload payload = { 0 };
	
	/* For progress tracking */
	struct ami_event_data evt_data = { 0 };
	struct ami_pdi_progress progress = { 0 };

	if (!dev || !path)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR;  /* last error is set by ami_open_cdev */
	
	if (read_file(path, &img_data, &img_size) == AMI_STATUS_OK) {
		payload.size = img_size;
		payload.addr = (unsigned long)(&img_data[0]);
		payload.cap_override = dev->cap_override;
		payload.partition = partition;
		payload.efd = AMI_INVALID_FD;

		if (progress_handler) {
			progress.bytes_to_write = img_size;

			if (ami_watch_driver_events(&evt_data, progress_handler, (void*)&progress) == AMI_STATUS_OK)
				payload.efd = evt_data.efd;
		}

		errno = 0;
		if (ioctl(dev->cdev, AMI_IOC_DOWNLOAD_PDI, &payload) == AMI_LINUX_STATUS_ERROR)
			ret = AMI_API_ERROR_M(
				AMI_ERROR_EIO,
				"errno %d (%s)",
				errno,
				strerror(errno)
			);
		else
			ret = AMI_STATUS_OK;

		free(img_data);  /* allocated by `read_file` */

		if (progress_handler && (evt_data.efd != AMI_INVALID_FD))
			ami_stop_watching_events(&evt_data);
	}

	return ret;
}

/*****************************************************************************/
/* Public API function definitions                                           */
/*****************************************************************************/

/*
 * Program a pdi bitstream onto a device.
 */
int ami_prog_download_pdi(ami_device *dev, const char *path, uint32_t partition,
	ami_event_handler progress_handler)
{
	if (!dev || !path || (partition == AMI_IOC_FPT_UPDATE_MAGIC))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_image_download(
		dev,
		path,
		partition,
		progress_handler
	);
}

/*
 * Update the device FPT.
 */
int ami_prog_update_fpt(ami_device *dev, const char *path,
	ami_event_handler progress_handler)
{
	if (!dev || !path)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	return do_image_download(
		dev,
		path,
		AMI_IOC_FPT_UPDATE_MAGIC,
		progress_handler
	);
}

/*
 * Set the device boot partition.
 */
int ami_prog_device_boot(struct ami_device **dev, uint32_t partition)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_data_payload payload = { 0 };

	if (!dev || !(*dev))
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_open_cdev(*dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR; /* last error is set by ami_open_cdev */
	
	payload.partition = partition;
	
	if (ioctl((*dev)->cdev, AMI_IOC_DEVICE_BOOT, &payload) == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	} else {
		/* Perform hot reset. This will update the device handle. */
		ret = ami_dev_hot_reset(dev);
	}

	return ret;
}

/*
 * Copy a device partition.
 */
int ami_prog_copy_partition(ami_device *dev, uint32_t src, uint32_t dest,
	ami_event_handler progress_handler)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_data_payload payload = { 0 };

	/* For progress tracking. */
	struct ami_event_data evt_data = { 0 };

	if (!dev)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR; /* last error is set by ami_open_cdev */
	
	payload.src = src;
	payload.dest = dest;

	/* NOTE: Progress tracking is currently not implemented driver side. */
	if (progress_handler)
		ami_watch_driver_events(
			&evt_data,
			progress_handler,
			NULL
		);
	
	if (ioctl(dev->cdev, AMI_IOC_COPY_PARTITION, &payload) == AMI_LINUX_STATUS_ERROR)
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	else
		ret = AMI_STATUS_OK;

	if (progress_handler && (evt_data.efd != AMI_INVALID_FD))
		ami_stop_watching_events(&evt_data);

	return ret;
}

/*
 * Get the FPT header.
 */
int ami_prog_get_fpt_header(ami_device *dev, struct ami_fpt_header *header)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_fpt_hdr_value data = { 0 };

	if (!dev || !header)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR; /* last error is set by ami_open_cdev */
	
	errno = 0;
	if (ioctl(dev->cdev, AMI_IOC_GET_FPT_HDR, &data) == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	} else {
		ret = AMI_STATUS_OK;
		header->version = data.version;
		header->hdr_size = data.hdr_size;
		header->entry_size = data.entry_size;
		header->num_entries = data.num_entries;
	}

	return ret;
}

/*
 * Get FPT partition information.
 */
int ami_prog_get_fpt_partition(ami_device *dev, uint32_t num, struct ami_fpt_partition *partition)
{
	int ret = AMI_STATUS_ERROR;
	struct ami_ioc_fpt_partition_value data = { 0 };

	if (!dev || !partition)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);
	
	if (ami_open_cdev(dev) != AMI_STATUS_OK)
		return AMI_STATUS_ERROR; /* last error is set by ami_open_cdev */

	data.partition = num;
	
	errno = 0;
	if (ioctl(dev->cdev, AMI_IOC_GET_FPT_PARTITION, &data) == AMI_LINUX_STATUS_ERROR) {
		ret = AMI_API_ERROR_M(
			AMI_ERROR_EIO,
			"errno %d (%s)",
			errno,
			strerror(errno)
		);
	} else {
		ret = AMI_STATUS_OK;
		partition->type = (enum ami_fpt_type)data.type;
		partition->base_addr = data.base_addr;
		partition->size = data.partition_size;
	}

	return ret;
}
