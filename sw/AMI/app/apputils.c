// SPDX-License-Identifier: GPL-2.0-only
/*
 * apputils.c - Utility functions for the AMI command line
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include <unistd.h>
#include <poll.h>  /* Linux only */

/* App includes */
#include "json.h"
#include "apputils.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define HEX_DATA_REALLOC_BUFFER	(100)
#define USER_PROMPT_TIMEOUT_MS	(300000) /* 5 minutes */

/*
 * The PDI version file is used to read PDI metadata and must be in
 * the same directory as the PDI image itself. This is used to check PDI UUID.
 */
#define PDI_VERSION_FILE	"version.json"

#define APP_DEV_COMPAT_STR	"COMPAT"

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/**
 * parse_logic_uuid() - Utility function to parse logic UUID JSON.
 * @json: Raw JSON string.
 * @uuid: Pointer to store parsed uuid string.
 * 
 * The expected JSON format is `{"design": {"logic_uuid": "..."}}`.
 * For best results, the logic UUID should be lowercase.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE
 */
static int parse_logic_uuid(const char *json, char *uuid)
{
	int ret = EXIT_FAILURE;

	/* Parsed JSON nodes. */
	JsonNode *parent = NULL;
	JsonNode *partition_meta = NULL;
	JsonNode *uuid_meta = NULL;

	if (!json || !uuid)
		return EXIT_FAILURE;
	
	parent = json_decode(json);

	if (parent) {
		partition_meta = json_find_member(parent, "design");

		if (partition_meta) {
			uuid_meta = json_find_member(partition_meta, "logic_uuid");

			if (uuid_meta && uuid_meta->string_) {
				ret = EXIT_SUCCESS;
				strncpy(
					uuid,
					uuid_meta->string_,
					AMI_LOGIC_UUID_SIZE
				);
			}
		}

		json_delete(parent);
	}

	return ret;
}

/*****************************************************************************/
/* Public function declarations                                              */
/*****************************************************************************/

/*
 * Read hex data from a file.
 */
int read_hex_data(const char *fname, void **values, uint32_t *num_values,
	size_t value_size)
{
	int ret = EXIT_FAILURE;
	FILE *file = NULL;
	char *line = NULL;
	size_t len = 0;
	uint32_t *buf = NULL;

	uint32_t n_lines_done = 0;
	uint32_t n_lines_allocated = HEX_DATA_REALLOC_BUFFER;

	if (!fname || !values || !num_values)
		return EXIT_FAILURE;

	buf = (uint32_t*)malloc(HEX_DATA_REALLOC_BUFFER * sizeof(uint32_t));

	if (!buf)
		return EXIT_FAILURE;

	file = fopen(fname, "r");

	if (file) {
		while (getline(&line, &len, file) != -1) {
			if (n_lines_done == n_lines_allocated) {
				buf = (uint32_t*)realloc(
					buf,
					(n_lines_allocated + HEX_DATA_REALLOC_BUFFER) * sizeof(uint32_t)
				);

				if (!buf)
					goto exit;

				n_lines_allocated += HEX_DATA_REALLOC_BUFFER;
			}

			buf[n_lines_done++] = (uint32_t)strtoul(
				line, NULL, 0
			);
		}

		if (line)
			free(line);

		/* Allocate output buffer and populate values */
		switch (value_size) {
		case sizeof(uint8_t):
		{
			int i = 0;
			uint8_t *v = (uint8_t*)malloc(n_lines_done * value_size);
			if (!v)
				goto exit;
			for (i = 0; i < n_lines_done; i++) {
				v[i] = (uint8_t)buf[i];
			}
			*((uint8_t**)values) = v;
		}
		break;

		case sizeof(uint16_t):
		{
			int i = 0;
			uint16_t *v = (uint16_t*)malloc(n_lines_done * value_size);
			if (!v)
				goto exit;
			for (i = 0; i < n_lines_done; i++) {
				v[i] = (uint16_t)buf[i];
			}
			*((uint16_t**)values) = v;
		}
		break;

		case sizeof(uint32_t):
		{
			int i = 0;
			uint32_t *v = (uint32_t*)malloc(n_lines_done * value_size);
			if (!v)
				goto exit;
			for (i = 0; i < n_lines_done; i++) {
				v[i] = (uint32_t)buf[i];
			}
			*((uint32_t**)values) = v;
		}
		break;

		default:
			goto exit;
		}

		*num_values = n_lines_done;
		ret = EXIT_SUCCESS;
	}

exit:
	if (file)
		fclose(file);

	if (buf)
		free(buf);
	
	return ret;
}

/*
 * Write hex data to a file.
 */
int write_hex_data(const char *fname, void *values, uint32_t num_values,
	size_t value_size)
{
	int ret = EXIT_FAILURE;
	FILE *file = NULL;

	if (!fname || !values)
		return EXIT_FAILURE;

	file = fopen(fname, "w");

	if (file) {
		int i = 0;
		for (i = 0; i < num_values; i++) {
			switch (value_size) {
			case sizeof(uint8_t):
				fprintf(file, "0x%02x\r\n", ((uint8_t*)values)[i]);
				break;

			case sizeof(uint16_t):
				fprintf(file, "0x%04x\r\n", ((uint16_t*)values)[i]);
				break;

			case sizeof(uint32_t):
				fprintf(file, "0x%08x\r\n", ((uint32_t*)values)[i]);
				break;

			default:
				goto exit;
			}
		}

		ret = EXIT_SUCCESS;
	}

exit:
	if (file)
		fclose(file);

	return ret;
}

/*
 * Read a file into a buffer.
 */
int read_file(const char *fname, uint8_t **buf, uint32_t *size)
{
	FILE *fp = NULL;
	long offset = 0;
	size_t len = 0;
	int ret = EXIT_FAILURE;
	uint8_t *buffer = NULL;

	if (!fname || !buf || !size)
		return EXIT_FAILURE;

	fp = fopen(fname, "rb");

	if (fp == NULL)
		return EXIT_FAILURE;
	
	/* Go to end of file. */
	if (fseek(fp, 0L, SEEK_END) != AMI_LINUX_STATUS_OK)
		goto close;

	offset = ftell(fp);
	if (offset == AMI_LINUX_STATUS_ERROR)
		goto close;

	buffer = (uint8_t*)malloc(sizeof(uint8_t) * (offset + 1));

	if (!buffer)
		goto close;

	/* Go back to the start of the file. */
	if (fseek(fp, 0L, SEEK_SET) != AMI_LINUX_STATUS_OK)
		goto del_buf;

	len = fread(buffer, sizeof(uint8_t), offset, fp);
	if (ferror(fp) == AMI_LINUX_STATUS_OK) {
		*size = (uint32_t)len;
		*buf = buffer;
		ret = EXIT_SUCCESS;
	}
	fclose(fp);
	return ret;

del_buf:
	free(buffer);

close:
	fclose(fp);
	return ret;
}

/*
 * Ask a user for confirmation.
 */
bool confirm_action(const char *prompt, char yes, int attempts)
{
	char response = 0;
	int num_attempts = 0;
	struct pollfd mypoll = { STDIN_FILENO, POLLIN | POLLPRI };

	do {
		fflush(stdin);
		printf("%s", prompt);
		fflush(stdout);

		/* Poll stdin before reading */
		if (poll(&mypoll, 1, USER_PROMPT_TIMEOUT_MS)) {
			scanf(" %c", &response);
			fflush(stdin);
			num_attempts++;
		} else {
			printf("No input. Aborting...\r\n");
			break;
		}
	} while ((response != yes) &&
		(num_attempts < attempts) &&
		(response != 'n'));

	return response == yes;
}

/*
 * Find the logic UUID of a PDI image.
 */
int find_logic_uuid(const char pdi[PATH_MAX], char uuid[AMI_LOGIC_UUID_SIZE])
{
	int ret = EXIT_FAILURE;
	char *dir = NULL;
	char pdi_path[PATH_MAX] = { 0 };
	char path[PATH_MAX] = { 0 };
	uint8_t *data = NULL;
	uint32_t data_size = 0;

	if (!pdi || !uuid)
		return EXIT_FAILURE;

	strncpy(pdi_path, pdi, PATH_MAX);
	dir = dirname(pdi_path);

	snprintf(
		path,
		PATH_MAX,
		"%s/%s",
		dir,
		PDI_VERSION_FILE
	);

	if (access(path, F_OK) == 0) {
		if (read_file(path, &data, &data_size) == AMI_STATUS_OK) {
			ret = parse_logic_uuid((const char *)data, uuid);
			free(data);
		}
	}

	return ret;
}

/*
 * Parse output related options.
 */
int parse_output_options(struct app_option *options, enum app_out_format *fmt,
	bool *verbose, FILE **stream, bool *fmt_given, bool *output_given)
{
	bool o_given = false, f_given = false;
	struct app_option *opt = NULL;

	/* `options` are not required */
	if (!options)
		return EXIT_SUCCESS;

	/* `fmt_given`, `output_given`, and `verbose` are optional */
	if (!fmt || !stream)
		return EXIT_FAILURE;

	/* Check output format. */
	if (NULL != (opt = find_app_option('f', options))) {
		f_given = true;

		/* Default to table format. */
		if (strcmp(opt->arg, "table") == 0)
			*fmt = APP_OUT_FORMAT_TABLE;
		else if (strcmp(opt->arg, "json") == 0)
			*fmt = APP_OUT_FORMAT_JSON;
		else
			APP_WARN("invalid output format");
	}

	/* Check if user specified output file. */
	if (NULL != (opt = find_app_option('o', options))) {
		if (access(opt->arg, F_OK) == 0) {
			APP_ERROR("output file already exists");
			return EXIT_FAILURE;
		}

		o_given = true;
		*stream = fopen(opt->arg, "w");
		
		/* Defaults to stdout */
		if (!(*stream))
			APP_WARN("could not open output file");
	}

	/* The format is only for the output file. */
	if (f_given && !o_given)
		APP_WARN("format specified but no output given");
	
	/* Check if user requested verbose output */
	if (verbose)
		*verbose = (NULL != find_app_option('v', options));

	if (output_given)
		*output_given = o_given;

	if (fmt_given)
		*fmt_given = f_given;

	return EXIT_SUCCESS;
}

/*
 * Warn the user if a device is running in compatibility mode
 */
void warn_compat_mode(ami_device *dev)
{
	char buf[AMI_DEV_STATE_SIZE] = { 0 };

	if (!dev)
		return;

	 if (ami_dev_get_state(dev, buf) == AMI_STATUS_ERROR) {
		APP_WARN(
			"could not check device state - you may experience issues!\r\n"
		);
		return;
	}

	if (strcmp(buf, APP_DEV_COMPAT_STR) == 0)
		APP_WARN(
			"device is running in compatibility mode - you may experience issues!\r\n"
		);
}
