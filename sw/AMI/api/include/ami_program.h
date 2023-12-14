// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_program.h- This file contains the public interface for device programming logic
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_PROGRAM_H
#define AMI_PROGRAM_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdint.h>

/* Public API includes */
#include "ami_device.h"

/*****************************************************************************/
/* Structs, Enums                                                            */
/*****************************************************************************/

/**
 * struct ami_fpt_header - The FPT header.
 * @version: FPT version.
 * @hdr_size: The size of the header in bytes.
 * @entry_size: The entry size in bytes.
 * @num_entries: The number of partitions.
 */
struct ami_fpt_header {
	uint8_t version;
	uint8_t hdr_size;
	uint8_t entry_size;
	uint8_t num_entries;
};

/**
 * enum ami_fpt_type - Different FPT partition types.
 * @AMI_FPT_TYPE_FPT: File partition table.
 * @AMI_FPT_TYPE_RECOVERY_FPT: Recovery file partition table.
 * @AMI_FPT_TYPE_EXTENSION_FPT: Extension file partition table.
 * @AMI_FPT_TYPE_PDI_BOOT: PDI boot.
 * @AMI_FPT_TYPE_PDI_BOOT_BACKUP: PDI boot backup.
 * @AMI_FPT_TYPE_PDI_XSABIN_META: XSABIN metadata.
 * @AMI_FPT_TYPE_PDI_GOLDEN: PDI golden image.
 * @AMI_FPT_TYPE_PDI_SYS_DTB: SYS DTB.
 * @AMI_FPT_TYPE_PDI_META: PDI metadata.
 * @AMI_FPT_TYPE_PDI_META_BACKUP: PDI metadata backup.
 * @AMI_FPT_TYPE_SC_FW: SC FW.
 */
enum ami_fpt_type {
	AMI_FPT_TYPE_FPT             = 0xFFFF,
	AMI_FPT_TYPE_RECOVERY_FPT    = 0xFFFE,
	AMI_FPT_TYPE_EXTENSION_FPT   = 0xFFFD,
	AMI_FPT_TYPE_PDI_BOOT        = 0x0E00,
	AMI_FPT_TYPE_PDI_BOOT_BACKUP = 0x0E01,
	AMI_FPT_TYPE_PDI_XSABIN_META = 0x0E02,
	AMI_FPT_TYPE_PDI_GOLDEN      = 0x0E03,
	AMI_FPT_TYPE_PDI_SYS_DTB     = 0x0E04,
	AMI_FPT_TYPE_PDI_META        = 0x0E05,
	AMI_FPT_TYPE_PDI_META_BACKUP = 0x0E06,
	AMI_FPT_TYPE_SC_FW           = 0x0C00,
};

/**
 * struct ami_fpt_partition - The individual partition information.
 * @type: Partition type.
 * @base_addr: Partition base address.
 * @size: Partition size.
 */
struct ami_fpt_partition {
	enum ami_fpt_type type;
	uint32_t base_addr;
	uint32_t size;
};

/**
 * struct ami_pdi_progress - Data struct for PDI download progress handlers
 * @bytes_to_write: Total number of bytes to write as part of the procedure
 * @bytes_written: Number of bytes written so far - this must be updated by
 *     the progress handler (initially set to 0)
 * @reserved: Generic field which can be used by the handler implementation
 */
struct ami_pdi_progress {
	uint32_t bytes_to_write;
	uint32_t bytes_written;
	uint64_t reserved;
};

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/**
 * ami_prog_download_pdi() - Program a .pdi bitstream onto a device.
 * @dev: Device handle.
 * @path: Full path to PDI file.
 * @partition: Partition number to flash to.
 * @progress_handler: An event handler to accept progress notifications.
 *
 * If a progress handler is given, a thread will be started to monitor driver
 * events - `ctr` will be equal to the number of bytes successfully written
 * and `data` will be a pointer to `struct ami_pdi_progress`.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_prog_download_pdi(ami_device *dev, const char *path, uint32_t partition,
	ami_event_handler progress_handler);

/**
 * ami_prog_update_fpt() - Program a PDI containing an FPT onto a device.
 * @dev: Device handle.
 * @path: Full path to PDI file.
 * @progress_handler: An event handler to accept progress notifications.
 *
 * Note that the provided PDI image must contain a valid FPT.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_prog_update_fpt(ami_device *dev, const char *path,
	ami_event_handler progress_handler);

/**
 * ami_prog_device_boot() - Set the device boot partition.
 * @dev: Device handle.
 * @partition: Partition number to set.
 *
 * NOTE: This function will update the given device handle.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_prog_device_boot(struct ami_device **dev, uint32_t partition);

/**
 * ami_prog_copy_partition() - Copy one device partition to another.
 * @dev: Device handle.
 * @src: Partition to copy from.
 * @dest: Partition to copy to.
 * @progress_handler: An event handler to accept progress notifications.
 *
 * The progress handler support is currently very limited for this function.
 * The driver does not currently support sending any events so the handler
 * will always receive a timeout status - `ctr` should not be used and `data`
 * will be NULL.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_prog_copy_partition(ami_device *dev, uint32_t src, uint32_t dest,
	ami_event_handler progress_handler);

/**
 * ami_prog_get_fpt_header() - Get the FPT header information.
 * @dev: Device handle.
 * @header: Struct to hold header information.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_prog_get_fpt_header(ami_device *dev, struct ami_fpt_header *header);

/**
 * ami_prog_get_fpt_partition() - Get FPT partition information.
 * @dev: Device handle.
 * @num: Partition number to fetch.
 * @partition: Struct to hold partition information.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR.
 */
int ami_prog_get_fpt_partition(ami_device *dev, uint32_t num, struct ami_fpt_partition *partition);

#ifdef __cplusplus
}
#endif

#endif  /* AMI_PROGRAM_H */
