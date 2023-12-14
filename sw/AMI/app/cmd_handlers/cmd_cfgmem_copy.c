// SPDX-License-Identifier: GPL-2.0-only
/*
 * cmd_cfgmem_copy.c - This file contains the implementation for the command "cfgmem_copy"
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

/* API includes */
#include "ami.h"
#include "ami_program.h"

/* App includes */
#include "commands.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define PDI_CHUNK_MULTIPLIER		(1024)
#define PDI_CHUNK_SIZE			(32)	/* Multiple of 1024 */
#define COPY_CHUNK_DUR_MS		(800)	/* Est duration for partition chunk copy (ms) */
#define MINUTE_IN_MS			(60000) 

/*****************************************************************************/
/* Function declarations                                                     */
/*****************************************************************************/

/**
 * do_cmd_cfgmem_copy() - "cfgmem_copy" command callback.
 * @options:  Ordered list of options passed in at the command line
 * @num_args:  Number of non-option arguments (excluding command)
 * @args:  List of non-option arguments (excluding command)
 * 
 * `args` may be an invalid pointer. It is the function's responsibility
 * to validate the `num_args` parameter.
 * 
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int do_cmd_cfgmem_copy(struct app_option *options, int num_args, char **args);

/**
 * calc_est_time() - Local function to calc est copy duration. 
 * @part_size: Size of src partition.
 * 
 * Return: Result of duration calc.
 */
static uint32_t calc_est_time(uint32_t part_size);

/**
 * progress_handler() - Event handler for the copy operation.
 * @status: Event status.
 * @ctr: Event counter, unused.
 * @data: NULL, unused.
 *
 * The progress handling is currently limited and simply prints out a character
 * regardless of the actual event that gets raised. This may still be useful,
 * however, to indicate that the application itself is not hanging.
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
 * i: Initial (source) partition
 * p: Destination partition
 */
static const char short_options[] = "hd:p:i:";

static const struct option long_options[] = {
	{ "help", no_argument, NULL, 'h' },  /* help screen */
	{ },
};

static const char help_msg[] = \
	"cfgmem_copy - copy one partition to another\r\n"
	"\r\nThis command requires root/sudo permissions.\r\n"
	"\r\nUsage:\r\n"
	"\t" APP_NAME " cfgmem_copy -d <bdf> -i <n> -p <n>\r\n"
	"\r\nOptions:\r\n"
	"\t-h --help            Show this screen\r\n"
	"\t-d <b>:[d].[f]       Specify the device BDF\r\n"
	"\t-i <partition>       Partition to copy from (source)\r\n"
	"\t-p <partition>       Partition to copy to (destination)\r\n"
;

struct app_cmd cmd_cfgmem_copy = {
	.callback      = &do_cmd_cfgmem_copy,
	.short_options = short_options,
	.long_options  = long_options,
	.root_required = true,
	.help_msg      = help_msg
};

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

/**
 * Display "dumb" progress to the user.
 */
static void progress_handler(enum ami_event_status status, uint64_t ctr, void *data)
{
	putchar('.');
	fflush(stdout);
}

/*
 * Local function to calc est copy duration.
 */
static uint32_t calc_est_time(uint32_t part_size)
{
	uint32_t est_dur_mins = 0;
	uint32_t est_num_chunks = 0;

	/* calc est copy duration */
	est_num_chunks = (part_size + ((PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER) - 1)) /
		                 (PDI_CHUNK_SIZE * PDI_CHUNK_MULTIPLIER);	
	est_dur_mins = ((est_num_chunks*COPY_CHUNK_DUR_MS) / MINUTE_IN_MS) + 1;
	return est_dur_mins;
}

/*
 * "cfgmem_copy" command callback.
 */
static int do_cmd_cfgmem_copy(struct app_option *options, int num_args, char **args)
{
	int ret = EXIT_FAILURE;

	/* Required options */
	struct app_option *device = NULL;
	struct app_option *source = NULL;
	struct app_option *dest = NULL;

	/* Required data */
	ami_device *dev = NULL;
	uint32_t source_partition = 0;
	uint32_t dest_partition = 0;
	struct ami_fpt_partition part = { 0 };
	uint32_t est_dur_mins = 0;

	/* Must have device, source partition, destination partition. */
	if (!options) {
		APP_USER_ERROR("not enough options", help_msg);
		return EXIT_FAILURE;
	}

	/* Device and partition are required. */
	device = find_app_option('d', options);
	source = find_app_option('i', options);
	dest = find_app_option('p', options);

	if (!device || !source | !dest) {
		APP_USER_ERROR("not enough arguments", help_msg);
		return AMI_STATUS_ERROR;
	}

	/* Find device */
	if (ami_dev_find(device->arg, &dev) != AMI_STATUS_OK) {
		APP_API_ERROR("could not find the requested device");
		return AMI_STATUS_ERROR;
	}

	source_partition = (uint32_t)strtoul(source->arg, NULL, 0);
	dest_partition = (uint32_t)strtoul(dest->arg, NULL, 0);

	printf("Copying partition %d to partition %d\r\n", source_partition, dest_partition);

	ret = ami_prog_get_fpt_partition(dev, source_partition, &part); /* get src partition size */

	if (ret == AMI_STATUS_ERROR) {
		APP_API_ERROR("could not get source fpt partition");
	} else {
		est_dur_mins = calc_est_time(part.size);
		printf("Estimated time to copy partition: %d (mins)...\r\n", est_dur_mins);
	}

	ret = ami_prog_copy_partition(dev, source_partition, dest_partition, progress_handler);
	printf("\r\n");

	if (ret == AMI_STATUS_ERROR)
		APP_API_ERROR("could not copy partition");
	else
		printf("Done. Partition copied successfully.\r\n");

	ami_dev_delete(&dev);
	return ret;
}
