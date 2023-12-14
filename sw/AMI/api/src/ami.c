// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami.c - This file contains the implementation of misc. API functions
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <time.h>
#include <sys/eventfd.h>

/* Private API includes */
#include "ami_internal.h"
#include "ami_version.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define VERSION_ATTR_FMT	"%hhd.%hhd.%hhd +%hhd *%hhd"
#define VERSION_ATTR_FIELDS	(5)
#define VERSION_ATTR_SIZE	(16)

#define BDF_DOMAIN_END		(4)
#define AMI_BDF_FORMAT_FULL	"%*04x:" AMI_BDF_FORMAT  /* ignore domain */

#define MAX_ERROR_DEFAULT_STR	(128)
#define MAX_ERROR_CTXT_STR	(256)
#define MAX_ERROR_STR		(MAX_ERROR_DEFAULT_STR + MAX_ERROR_CTXT_STR)

#define EVENT_POLL_FDS		(1)
#define EVENT_POLL_TIMEOUT_MS	(1000)

#define MSEC_TO_SEC(msec)	(msec / 1000)
#define MSEC_TO_NSEC(msec)	((msec % 1000) * 1000000)

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

volatile enum ami_error ami_last_error = AMI_ERROR_NONE;
static char last_error_str[MAX_ERROR_STR] = { 0 };

/*****************************************************************************/
/* Local function definitions                                                */
/*****************************************************************************/

/**
 * ami_event_thread() - Internal event watcher thread.
 * @data: Pointer to `struct ami_event_data`
 *
 * Return: None.
 */
static void *ami_event_thread(void *data)
{
	struct ami_event_data *d = NULL;
	struct pollfd mypoll = { 0 };
	uint64_t efd_ctr = 0;
	enum ami_event_status status = AMI_EVENT_STATUS_TIMEOUT;

	if (!data)
		return NULL;

	d = (struct ami_event_data*)data;

	/* callback is necessary */
	if (!(d->callback))
		return NULL;

	mypoll.fd = d->efd;
	mypoll.events = POLLIN;

	while (!d->quit) {
		if (poll(&mypoll, EVENT_POLL_FDS, EVENT_POLL_TIMEOUT_MS)) {
			if (read(d->efd, &efd_ctr, sizeof(uint64_t)) == sizeof(uint64_t))
				status = AMI_EVENT_STATUS_OK;
			else
				status = AMI_EVENT_STATUS_READ_ERROR;
		} else {
			status = AMI_EVENT_STATUS_TIMEOUT;
		}

		d->callback(
			status,
			efd_ctr,
			d->callback_data
		);
	}

	return NULL;
}

/*****************************************************************************/
/* Private API Definitions                                                   */
/*****************************************************************************/

/*
 * Sleep for the specified number of milliseconds.
 */
int ami_msleep(long msec)
{
	int ret = AMI_LINUX_STATUS_ERROR;
	struct timespec ts = { 0 };

	if (msec < 0)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	ts.tv_sec = MSEC_TO_SEC(msec);
	ts.tv_nsec = MSEC_TO_NSEC(msec);

	do {
		ret = nanosleep(&ts, &ts);
	} while (ret && (errno == EINTR));

	if (ret)
		return AMI_API_ERROR(AMI_ERROR_ERET);

	return AMI_STATUS_OK;
}

/*
 * Convert string to a signed long
 */
int ami_convert_num(const char *buf, int base, long *out)
{
	long result = 0;
	char *end = NULL;
	int ret = AMI_STATUS_ERROR;

	if (!out || !buf)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	errno = 0;  /* For error checking */
	result = strtol(buf, &end, base);

	if ((errno == 0) && (*end != 0)) {
		*out = result;
		ret = AMI_STATUS_OK;
	}

	return ret;
}

/*
 * Set the last error.
 */
int ami_set_last_error(enum ami_error err, const char *ctxt, ...)
{
	ami_last_error = err;
	char error_ctxt[MAX_ERROR_CTXT_STR] = { 0 };

	if (ctxt != NULL) {
		va_list args;
		va_start(args, ctxt);
		vsnprintf(error_ctxt, MAX_ERROR_CTXT_STR, ctxt, args);
		va_end(args);
	} else {
		/* Default to just the numeric error code. */
		sprintf(
			error_ctxt,
			"%d",
			(int)err
		);
	}

	memset(last_error_str, 0x00, MAX_ERROR_STR);

	switch (err) {
	case AMI_ERROR_EINVAL:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"EINVAL: Invalid arguments [%s].\r\n",
			error_ctxt
		);
		break;
	
	case AMI_ERROR_EBADF:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"EBADF: File could not be opened and/or closed [%s].\r\n",
			error_ctxt
		);
		break;

	case AMI_ERROR_EIO:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"EIO: File could not be read or written [%s].\r\n",
			error_ctxt
		);
		break;

	case AMI_ERROR_EFMT:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"EFMT: Bad format; data could not be parsed [%s].\r\n",
			error_ctxt
		);
		break;

	case AMI_ERROR_ENOMEM:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"ENOMEM: Could not allocate memory [%s].\r\n",
			error_ctxt
		);
		break;

	case AMI_ERROR_ERET:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"ERET: Invalid return code from function call [%s].\r\n",
			error_ctxt
		);
		break;
	
	case AMI_ERROR_ENODEV:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"ENODEV: No such device [%s].\r\n",
			error_ctxt
		);
		break;

	case AMI_ERROR_EVER:
		snprintf(
			last_error_str,
			MAX_ERROR_STR,
			"EVER: Version does not match expected value [%s].\r\n",
			error_ctxt
		);
		break;

	default:
		sprintf(
			last_error_str,
			"Unknown error (%d).\r\n",
			(int)err
		);
		break;
	}

	return AMI_STATUS_OK;
}

/*****************************************************************************/
/* Public API Definitions                                                    */
/*****************************************************************************/

/*
* Get AMI Driver version info
*/
int ami_get_driver_version(struct ami_version *ami_version)
{
	int ret = AMI_STATUS_ERROR;
	int fd = AMI_INVALID_FD;
	char raw_buf[VERSION_ATTR_SIZE] = { 0 };

	if (!ami_version)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	fd = open(AMI_DRIVER_VERSION, O_RDONLY | O_NONBLOCK);

	if (fd != AMI_INVALID_FD) {
		if (read(fd, raw_buf, VERSION_ATTR_SIZE) != AMI_LINUX_STATUS_ERROR) {
			int scan = sscanf(
				raw_buf,
				VERSION_ATTR_FMT,
				&ami_version->major,
				&ami_version->minor,
				&ami_version->patch,
				&ami_version->dev_commits,
				&ami_version->status
			);

			if (scan == VERSION_ATTR_FIELDS)
				ret = AMI_STATUS_OK;
			else
				ret = AMI_API_ERROR(AMI_ERROR_EFMT);
		} else {
			ret = AMI_API_ERROR(AMI_ERROR_EIO);
		}

		close(fd);
	} else {
		ret = AMI_API_ERROR(AMI_ERROR_EBADF);
	}

	return ret;
}

/*
* Get AMI API version info
*/
int ami_get_api_version(struct ami_version *ami_version)
{
	if (ami_version != NULL) {
		ami_version->major = GIT_TAG_VER_MAJOR;
		ami_version->minor = GIT_TAG_VER_MINOR;
		ami_version->patch = GIT_TAG_VER_PATCH;
		ami_version->dev_commits = GIT_TAG_VER_DEV_COMMITS;
		ami_version->status = (0 == GIT_STATUS) ? (0) : (1);

		return AMI_STATUS_OK;
	}

	return AMI_API_ERROR(AMI_ERROR_EINVAL);
}

/*
 * Get the last error as a string.
 */
const char *ami_get_last_error(void)
{
	return last_error_str;
}

/*
 * Parse a BDF string.
 */
uint16_t ami_parse_bdf(const char *bdf)
{
	uint16_t bdf_num = 0;
	int bus = 0, dev = 0, fn = 0;
	int ret = 0;

	/* Need to determine if the user passed in a PCI domain. */
	char *c = NULL;

	if (!bdf)
		return ret;
	
	/* Find position of first colon. */
	c = strchr(bdf, ':');

	if (c && ((size_t)(c - bdf) == BDF_DOMAIN_END))
		ret = sscanf(bdf, AMI_BDF_FORMAT_FULL, &bus, &dev, &fn);
	else
		ret = sscanf(bdf, AMI_BDF_FORMAT, &bus, &dev, &fn);

	if ((ret >= 1) && (ret <= AMI_BDF_FIELDS_NUM))
		bdf_num = AMI_MK_BDF(bus, dev, fn);

	return bdf_num;
}

/*
 * Start the event watcher thread.
 */
int ami_watch_driver_events(struct ami_event_data *event_data,
	ami_event_handler callback, void *data)
{
	int ret = AMI_LINUX_STATUS_ERROR;
	int efd = AMI_INVALID_FD;

	/* callback must be given */
	if (!event_data || !callback)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	/* Setup eventfd */
	if ((efd = eventfd(0, 0)) == AMI_INVALID_FD) 
		return AMI_API_ERROR(EBADF);

	/* Initialise thread data */
	event_data->quit = false;
	event_data->callback = callback;
	event_data->callback_data = data;
	event_data->efd = efd;

	/* Create thread */
	ret = pthread_create(
		&event_data->event_thread,
		NULL,
		ami_event_thread,
		(void*)event_data
	);

	if (ret) 
		return AMI_API_ERROR(AMI_ERROR_ERET);

	event_data->thread_created = true;
	return AMI_STATUS_OK;
}

/*
 * Stop the event watcher thread and cleanup.
 */
int ami_stop_watching_events(struct ami_event_data *event_data)
{
	if (!event_data)
		return AMI_API_ERROR(AMI_ERROR_EINVAL);

	if (event_data->thread_created) {
		event_data->quit = true;
		pthread_join(event_data->event_thread, NULL);
	}

	if (event_data->efd != AMI_INVALID_FD)
		close(event_data->efd);

	return AMI_STATUS_OK;
}
