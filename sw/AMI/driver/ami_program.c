// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_program.c - This file contains functions to program (flash) devices.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/eventfd.h>

#include "ami_top.h"
#include "ami_program.h"
#include "ami_amc_control.h"
#include "ami_sensor.h"
#include "ami_pcie.h"

/* For invalidating the first PDI chunk in case the download fails */
#define INVALID_BOOT_TAG	(0xFFFFFFFF)
#define BOOT_TAG_CHUNK		(0)


/**
 * do_image_download() - Perform an image download operation.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @buf: Bitstream byte buffer.
 * @size: Size of bitstream buffer.
 * @partition: Partition number to flash.
 * @efd_ctx: eventfd context for reporting progress (optional).
 *
 * If `partition` is equal to `FPT_UPDATE_MAGIC` will update the FPT.
 *
 * Return: 0 or negative error code.
 */
static int do_image_download(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t *buf, uint32_t size,
	uint32_t partition, struct eventfd_ctx *efd_ctx)
{
	int ret = SUCCESS;
	uint16_t chunk = 0;
	uint16_t part = 0;
	uint32_t bytes_written = 0;
	uint32_t bytes_to_write = 0;
	bool rewrite_boot_tag = false;
	/* Round up the total number of chunks */
	uint16_t num_chunks = (size + ((PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER) - 1)) /
		(PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER);
	
	if (!size || !amc_ctrl_ctxt || !buf)
		return -EINVAL;

	if (partition == FPT_UPDATE_MAGIC) {
		part = FPT_UPDATE_FLAG;
	} else {
		/* Basic sanity check */
		if (partition > MAX_PARTITION)
			return -EINVAL;

		part = (uint16_t)partition;
	}

	AMI_VDBG(
		amc_ctrl_ctxt,
		"Attempting to download PDI bitstream with image size %d to partition %d num_chunks = %d",
		size, part, num_chunks
	);

	while (bytes_written < size) {
		if ((PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER) > (size - bytes_written))
			bytes_to_write = (size - bytes_written);
		else
			bytes_to_write = (PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER);
		
		/*
		 * Don't invalidate the boot tag if we're updating the FPT
		 * or if there is only a single chunk.
		 */
		if ((part == FPT_UPDATE_FLAG) || (num_chunks == 1) ||
			((num_chunks > 1) && (chunk != BOOT_TAG_CHUNK))) {
			/*
			* This will copy the bitstream buffer into shared memory and submit
			* the GCQ command. Using `flags` to pass in partition and chunk numbers.
			*/
			ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_DOWNLOAD_PDI,
				MK_PDI_FLAGS(part, chunk, (!rewrite_boot_tag && (chunk == (num_chunks - 1)))),
				&buf[bytes_written], bytes_to_write);

			if (ret) 
				break;

			if (efd_ctx)
				eventfd_signal(efd_ctx, bytes_to_write);
		} else {
			uint32_t boot_tag = INVALID_BOOT_TAG;

			AMI_VDBG(
				amc_ctrl_ctxt,
				"Invalidating the PDI boot tag"
			);

			/*
			 * If there's more than one chunk we must invalidate the boot
			 * tag and write the first chunk last.
			 */
			ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_DOWNLOAD_PDI,
				MK_PDI_FLAGS(part, BOOT_TAG_CHUNK, false),
				(uint8_t*)&boot_tag, sizeof(uint32_t));
			
			/*
			 * Don't signal to the user here but we do increment
			 * the chunk and number of bytes written so we can continue
			 * with the loop as normal.
			 */
			if (ret)
				break;
			
			rewrite_boot_tag = true;
		}

		AMI_VDBG(
			amc_ctrl_ctxt,
			"Done with chunk %d",
			chunk
		);

		chunk++;
		bytes_written += bytes_to_write;
	}

	/* Check if we need to re-write the first chunk */
	if (!ret && rewrite_boot_tag) {
		AMI_VDBG(
			amc_ctrl_ctxt,
			"Rewriting first chunk"
		);

		/*
		 * If there's more than one chunk, the first chunk is guaranteed
		 * to have the full chunk size.
		 */
		ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_DOWNLOAD_PDI,
			MK_PDI_FLAGS(partition, BOOT_TAG_CHUNK, true),
			buf, (PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER));

		if (!ret && efd_ctx)
			eventfd_signal(efd_ctx, (PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER));
	}

	if (ret)
		AMI_ERR(amc_ctrl_ctxt, "Failed to download PDI");

	return ret;
}

/*
 * Download a PDI bitstream.
 */
int download_pdi(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t *buf, uint32_t size,
	uint32_t partition, struct eventfd_ctx *efd_ctx)
{
	if (!amc_ctrl_ctxt || !size || !buf || (partition == FPT_UPDATE_MAGIC))
		return -EINVAL;

	return do_image_download(
		amc_ctrl_ctxt,
		buf,
		size,
		partition,
		efd_ctx
	);
}

/*
 * Update device FPT.
 */
int update_fpt(struct pf_dev_struct *pf_dev, uint8_t *buf, uint32_t size,
	struct eventfd_ctx *efd_ctx)
{
	int ret = 0;

	if (!pf_dev || !size || !buf)
		return -EINVAL;

	ret = do_image_download(
		pf_dev->amc_ctrl_ctxt,
		buf,
		size,
		FPT_UPDATE_MAGIC,
		efd_ctx
	);

	if (!ret) {
		/* Must update FPT SDR */
		ret = update_sdr(pf_dev, SDR_TYPE_FPT);

		if (ret)
			AMI_ERR(
				pf_dev->amc_ctrl_ctxt,
				"Could not update FPT SDR"
			);
		else
			AMI_WARN(
				pf_dev->amc_ctrl_ctxt,
				"Updated FPT SDR"
			);
	}

	return ret;
}

/*
 * Select a device boot partition.
 */
int device_boot(struct pf_dev_struct *pf_dev, uint32_t partition)
{
	int ret = SUCCESS;

	AMI_VDBG(
		pf_dev->amc_ctrl_ctxt,
		"Attempting to select device boot partition (%d)",
		partition
	);

	ret = submit_gcq_command(pf_dev->amc_ctrl_ctxt, GCQ_SUBMIT_CMD_DEVICE_BOOT,
		partition, NULL, 0);
	
	if (ret)
		AMI_ERR(pf_dev->amc_ctrl_ctxt, "Failed to select boot partition");

	return ret;
}

/*
 * Copy a device partition.
 */
int copy_partition(struct pf_dev_struct *pf_dev, uint32_t src, uint32_t dest)
{
	int ret = SUCCESS;
	struct fpt_partition src_partition = { 0 };
	struct fpt_partition dest_partition = { 0 };

	AMI_VDBG(
		pf_dev->amc_ctrl_ctxt,
		"Attempting to copy partition %d to %d",
		src, dest
	);

	/* Basic sanity check */
	if ((src > MAX_PARTITION) || (dest > MAX_PARTITION))
		return -EINVAL;

	/* Check that the partitions exist. */
	if (read_fpt_partition(pf_dev, src, &src_partition) ||
		read_fpt_partition(pf_dev, dest, &dest_partition)) {
			AMI_ERR(pf_dev->amc_ctrl_ctxt, "Partition not found");
			return -EINVAL;
	}
	
	/* Sanity check partition size */
	if (dest_partition.partition_size < src_partition.partition_size) {
		AMI_ERR(pf_dev->amc_ctrl_ctxt, "Destination partition is too small for copy");
		return -EINVAL;
	}

	/*
	 * Using `flags` to pass in source and destination partitions.
	 * No data buffer is given and length is set to the size of the source partition.
	 */
	ret = submit_gcq_command(pf_dev->amc_ctrl_ctxt, GCQ_SUBMIT_CMD_COPY_PARTITION,
		MK_PARTITION_FLAGS(src, dest), NULL, src_partition.partition_size);
	
	if (ret)
		AMI_ERR(pf_dev->amc_ctrl_ctxt, "Failed to copy partition");

	return ret;
}
