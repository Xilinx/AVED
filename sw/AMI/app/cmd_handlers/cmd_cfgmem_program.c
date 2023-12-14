// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_cfgmem_program.c - This file contains the implementation for the command "cfgmem_program"
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

/* API includes */
#include "ami.h"
#include "ami_program.h"

/* App includes */
#include "commands.h"
#include "apputils.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/
#define PROGRESS_BAR_WIDTH (100)

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_cfgmem_program() - "cfgmem_program" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_cfgmem_program(struct app_option *options, int num_args, char **args);

/**
 * progress_handler() - Event handler for the PDI download operation.
 * @status: Event status.
 * @ctr: Event counter - equal to the number of bytes written.
 * @data: Pointer to PDI progress struct.
 * 
 * Return: None.
 */
static void progress_handler(enum ami_event_status status, uint64_t ctr, void *data);

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * h: Help
 * d: Device
 * i: Image file
 * p: Partition number
 * y: Skip user confirmation
 * q: Quit after programming
 */
static const char short_options[] = "hd:i:p:yq";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"cfgmem_program - program a bitstream onto a device\r\n"
	"\r\nThis command requires root/sudo permissions.\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " cfgmem_program -d <bdf> -i <path> -p <n>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help             Show this screen\r\n"
	"\t-d <b>:[d].[f]        Specify the device BDF\r\n"
	"\t-i <path>             Path to image file\r\n"
	"\t-p <partition>        Partition to flash\r\n"
	"\t-y                    Skip confirmation\r\n"
	"\t-q                    Quit after programming\r\n"
;

struct app_cmd cmd_cfgmem_program = {
	.callback      = &do_cmd_cfgmem_program,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = true,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/*
 * Event handler for PDI download.
 */
static void progress_handler(enum ami_event_status status, uint64_t ctr, void *data)
{
	struct ami_pdi_progress *prog = NULL;

	if (!data)
		return;

	prog = (struct ami_pdi_progress*)data;

	if (status == AMI_EVENT_STATUS_OK)
		prog->bytes_written += ctr;

	prog->reserved = print_progress_bar(
		prog->bytes_written,
		prog->bytes_to_write,
		PROGRESS_BAR_WIDTH, '[', ']', '#', '.',
		prog->reserved
	);
}

/*
 * "program" command callback.
 */
static int do_cmd_cfgmem_program(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;

	/* Required options */
	struct app_option *device = NULL;
	struct app_option *image = NULL;
	struct app_option *partition = NULL;

	/* Required data */
	uint16_t bdf = 0;
	ami_device *dev = NULL;
	uint32_t partition_number = 0;

	/* For UUID checks */
	int found_current_uuid = AMI_STATUS_ERROR;
	int found_new_uuid = AMI_STATUS_ERROR;
	char new_uuid[AMI_LOGIC_UUID_SIZE] = { 0 };
	char current_uuid[AMI_LOGIC_UUID_SIZE] = { 0 };

	/* Must have at least an image and device. */
	if (!options) {
		APP_USER_ERROR("not enough options", help_msg);
		return EXIT_FAILURE;
	}

	/* Device and image are required. TODO: Should these be positional args? */
	device = find_app_option('d', options);
	image = find_app_option('i', options);
	partition = find_app_option('p', options);

	if (!device || !image || !partition) {
		APP_USER_ERROR("not enough arguments", help_msg);
		return AMI_STATUS_ERROR;
	}

	/* Check that the provided PDI image exists. */
	if (access(image->arg, F_OK) != AMI_LINUX_STATUS_OK) {
		/* File does not exist */
		APP_ERROR("provided image does not exist");
		return AMI_STATUS_ERROR;
	}

	/* Find device */
	if (ami_dev_find(device->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return AMI_STATUS_ERROR;
	}

	/* Check compatibility mode */
	warn_compat_mode(dev);

	ami_dev_get_pci_bdf(dev, &bdf);

	found_current_uuid = ami_dev_read_uuid(dev, current_uuid);
	found_new_uuid = find_logic_uuid(image->arg, new_uuid);
	partition_number =  (uint32_t)strtoul(partition->arg, NULL, 0);

	printf(
		"----------------------------------------------\r\n"
		"Device | %02x:%02x.%01x\r\n"
		"----------------------------------------------\r\n"
		"Current Configuration\r\n"
		"----------------------------------------------\r\n"
		"UUID   | %s\r\n"
		"----------------------------------------------\r\n"
		"Incoming Configuration\r\n"
		"----------------------------------------------\r\n"
		"UUID      | %s\r\n"
		"Path      | %s\r\n"
		"Partition | %d\r\n"
		"----------------------------------------------\r\n",
		AMI_PCI_BUS(bdf), AMI_PCI_DEV(bdf), AMI_PCI_FUNC(bdf),
		((found_current_uuid != AMI_STATUS_OK) ? ("N/A") : (current_uuid)),
		((found_new_uuid != AMI_STATUS_OK) ? ("N/A") : (new_uuid)),
		image->arg,
		partition_number
	);

	if ((NULL != find_app_option('y', options)) || confirm_action(APP_CONFIRM_PROMPT, 'Y', 3)) {
		printf("\r\nUpdating base flash image...\r\n");

		if (ami_prog_download_pdi(dev, image->arg, partition_number, progress_handler) == AMI_STATUS_OK) {
			printf("\r\nImage programming complete.\r\n");

			if (NULL == find_app_option('q', options)) {
				/* If we're not quitting, set the device boot partition */
				printf("Will do a hot reset to boot into partition %d. This may take a minute...\r\n",
					partition_number);

				if (ami_prog_device_boot(&dev, partition_number) == AMI_STATUS_OK) {
					ret = EXIT_SUCCESS;

					printf(
						"\r\nOK. Image has been programmed successfully.\r\n"
						"***********************************************\r\n"
						"Hot reset has been performed into partition %d.\r\n"
						"***********************************************\r\n",
						partition_number
					);
				} else {
					APP_API_ERROR("could not select boot partition");
				}
			} else {
				ret = EXIT_SUCCESS;

				printf(
					"\r\nOK. Image has been programmed successfully.\r\n"
					"****************************************************\r\n"
					"Cold reboot machine to load the new image on device.\r\n"
					"****************************************************\r\n"
				);
			}
		} else {
			APP_API_ERROR("could not program image");
		}
	} else {
		ret = EXIT_SUCCESS;
		printf("\r\nAborting...\r\n");
	}

	ami_dev_delete(&dev);
	return ret;
}
