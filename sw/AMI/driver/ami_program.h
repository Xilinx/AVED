// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_program.h - This file contains functions to program (flash) devices.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */
 
#ifndef AMI_PROGRAM_H
#define AMI_PROGRAM_H

#include <linux/types.h>
#include <linux/eventfd.h>

#include "ami_top.h"
#include "ami_amc_control.h"

#define MAX_PARTITION			(15)

/*
 * Format of flags:
 * 0xAAAABBBB where:
 *   0xAAAA is the source partition (16 bits)
 *   0xBBBB is the destination partition (16 bits)
 */
#define MK_PARTITION_FLAGS(src, dest)	(((uint16_t)src << 16) | ((uint16_t)dest))
#define PARTITION_SRC(flags)		((uint16_t)(flags >> 16))
#define PARTITION_DEST(flags)		((uint16_t)(flags & 0x0000ffff))

#define FPT_UPDATE_FLAG			(0xAAAA)  /* uint16 - the other 2 bytes are the chunk num */
#define FPT_UPDATE_MAGIC		(0xAAAAAAAA)
#define PDI_CHUNK_MULTIPLIER		(1024)
#define PDI_CHUNK_SIZE			(32)  /* Multiple of 1024 */

/*
 * Format of flags:
 * 0xAAAABBBB where:
 *   0xAAAA is the partition number (16 bits) - this is 0xFFFF when updating the FPT
 *   0xBBBB is the current chunk number (15 bits) with the MSB set to 1 if this is the last chunk (1 bit)
 *
 * `last` in this macro should be a bool.
 */
#define MK_PDI_FLAGS(part, chunk, last)	(((uint16_t)part << 16) | ((last) ? \
						((uint16_t)chunk | ((uint16_t)1 << 15)) : \
						((uint16_t)chunk & ~((uint16_t)1 << 15))))
#define PDI_PARTITION(flags)		((uint16_t)(flags >> 16))
#define PDI_CHUNK(flags)		(((uint16_t)(flags & 0x0000ffff)) & ~((uint16_t)1 << 15))
#define PDI_CHUNK_IS_LAST(flags)	((uint16_t)(flags & 0x0000ffff) >> 15)  /* either 1 or 0 */

/**
 * download_pdi() - Download a PDI bitstream onto a device.
 * @amc_ctrl_ctxt: Pointer to top level AMC data struct.
 * @buf: Bitstream byte buffer.
 * @size: Size of bitstream buffer.
 * @partition: Partition number to flash.
 * @efd_ctx: eventfd context for reporting progress (optional).
 * 
 * Return: 0 or negative error code.
 */
int download_pdi(struct amc_control_ctxt *amc_ctrl_ctxt, uint8_t *buf, uint32_t size,
	uint32_t partition, struct eventfd_ctx *efd_ctx);

/**
 * update_fpt() - Download a PDI containing an FPT onto a device.
 * @pf_dev: Device data.
 * @buf: Bitstream byte buffer - must contain valid FPT.
 * @size: Size of bitstream buffer.
 * @efd_ctx: eventfd context for reporting progress (optional).
 * 
 * Return: 0 or negative error code.
 */
int update_fpt(struct pf_dev_struct *pf_dev, uint8_t *buf, uint32_t size,
	struct eventfd_ctx *efd_ctx);

/**
 * device_boot() - Set the device boot partition.
 * @pf_dev: Device data.
 * @partition: Partition number to select.
 * 
 * Return: 0 or negative error code.
 */
int device_boot(struct pf_dev_struct *pf_dev, uint32_t partition);

/**
 * copy_partition() - Copy a device partition.
 * @pf_dev: Device data.
 * @src: Partition to copy from.
 * @dest: Partition to copy to.
 *
 * Return: 0 or negative error code.
 */
int copy_partition(struct pf_dev_struct *pf_dev, uint32_t src, uint32_t dest);

#endif  /* AMI_PROGRAM_H */
