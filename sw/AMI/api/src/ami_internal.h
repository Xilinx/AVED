// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_internal.h - This file contains generic private API definitions
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_INTERNAL_H
#define AMI_INTERNAL_H

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/

/* Standard includes */
#include <pthread.h>

/* Public API includes */
#include "ami.h"  /* Used by other internal headers. */

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define AMI_BASE_10 (10)

/*
 * Utility macro to set the last error and return AMI_STATUS_ERROR.
 * We ignore the return value of `ami_set_last_error` and ALWAYS return
 * `AMI_STATUS_ERROR`. This is useful for public API functions which must set
 * an error code but return only `AMI_STATUS_ERROR`.
 */
#define AMI_API_ERROR_M(err, m, ...) ((ami_set_last_error(err, "%s:%d - " m, __func__, __LINE__, ##__VA_ARGS__)) ? \
	(AMI_STATUS_ERROR) : (AMI_STATUS_ERROR))

/* Same as above but with no error message. */
#define AMI_API_ERROR(err) ((ami_set_last_error(err, "%s:%d", __func__, __LINE__)) ? \
	(AMI_STATUS_ERROR) : (AMI_STATUS_ERROR))

/*****************************************************************************/
/* Enums, Structs                                                            */
/*****************************************************************************/

/**
 * enum ami_error - list of AMI error codes
 * @AMI_ERROR_NONE: no errors, to be used as a placeholder
 * @AMI_ERROR_EINVAL: invalid arguments
 * @AMI_ERROR_EBADF: bad file (could not open/close)
 * @AMI_ERROR_EIO: IO error (read/write/ioctl/etc...)
 * @AMI_ERROR_EFMT: bad format, parse error, etc...
 * @AMI_ERROR_ENOMEM: memory allocation failure
 * @AMI_ERROR_ERET: error return code from function call
 * @AMI_ERROR_ENODEV: no such device
 * @AMI_ERROR_EVER: version mismatch
 */
enum ami_error {
	AMI_ERROR_NONE = 0,
	AMI_ERROR_EINVAL,
	AMI_ERROR_EBADF,
	AMI_ERROR_EIO,
	AMI_ERROR_EFMT,
	AMI_ERROR_ENOMEM,
	AMI_ERROR_ERET,
	AMI_ERROR_ENODEV,
	AMI_ERROR_EVER,
};

/**
 * struct ami_event_data - internal data for event handlers
 * @efd: event file descriptor to watch
 * @quit: boolean indicating if the thread should stop
 * @thread_created: boolean indicating if the thread has been created
 * @event_thread: the thread reference
 * @callback_data: data to be passed into the user-provided callback
 * @callback: user-provided event callback
 */
struct ami_event_data {
	int                  efd;
	bool                 quit;
	bool                 thread_created;
	pthread_t            event_thread;
	void                *callback_data;
	ami_event_handler    callback;
};

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*
 * Global variable to keep track of the last API error.
 * Declared volatile to mimic the behaviour of errno, so it can be used
 * from signal handlers or separate threads.
 */
extern volatile enum ami_error ami_last_error;

/*****************************************************************************/
/* Private API function definitions                                          */
/*****************************************************************************/

/**
 * ami_convert_num() - Convert a string buffer to a signed long
 * @buf: String buffer to convert from.
 * @base: Base of the number
 * @out: Pointer to output variable
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_convert_num(const char *buf, int base, long *out);

/**
 * @ami_msleep() - Sleep for the specified number of milliseconds.
 * @msec: Milliseconds to sleep for.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_msleep(long msec);

/**
 * ami_set_last_error() - Set the last error global variable
 * @err: Error code to set.
 * @ctxt: Optional context string related to this error.
 *
 * Prefer to use the AMI_API_ERROR and AMI_API_ERROR_M macros instead of
 * calling this function directly.
 * 
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_set_last_error(enum ami_error err, const char *ctxt, ...);

/**
 * ami_watch_driver_events() - Begin watching driver events
 * @event_data: Internal event data struct.
 * @callback: The actual handler to be called when an event is received
 * @data: Data to be passed into the callback - may be NULL
 *
 * Note: This function creates a new thread but does not wait for it to finish.
 * Several fields inside the `event_data` struct are updated. Namely, on success,
 * the `efd` field is set to the event file descriptor - this must be given
 * to the driver so the communication channel can be initialised.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_watch_driver_events(struct ami_event_data *event_data,
	ami_event_handler callback, void *data);

/**
 * ami_stop_watching_events() - Stop watching driver events
 * @event_data: The same data struct pointer that was given to `ami_watch_driver_events`
 *
 * This function indicates to the underlying thread that it should exit and
 * waits for it to finish. It also cleans up the eventfd interface.
 *
 * Return: AMI_STATUS_OK or AMI_STATUS_ERROR
 */
int ami_stop_watching_events(struct ami_event_data *event_data);

#endif  /* AMI_INTERNAL_H */
