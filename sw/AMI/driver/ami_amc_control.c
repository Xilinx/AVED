// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_amc_control.c - This file contains AMC control implementation.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/types.h>

#include "gcq.h"
#include "ami_top.h"
#include "ami_amc_control.h"
#include "amc_proxy.h"
#include "ami_program.h"
#include "ami_eeprom.h"
#include "ami_log.h"
#include "ami_module.h"
#include "ami_driver_version.h"

/*****************************************************************************/
/* Local Varaiables                                                          */
/*****************************************************************************/

static DEFINE_XARRAY_ALLOC(cid_xarray);


/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define MAX_COMMAND_IDS    		        (255)

#define DEVICE_READY_SLEEP_INTERVAL		(100)
#define DEVICE_READY_RETRY_COUNT		(5)
#define REQUEST_MSQ_TIMEOUT			(msecs_to_jiffies(30000))   /* 30 seconds */
/*
 * Timeout for the PDI download can be small as we are packetising the
 * image and sending across one chunk at a time.
 */
#define REQUEST_DOWNLOAD_TIMEOUT		(msecs_to_jiffies(30000))   /* 30 seconds */
#define REQUEST_COPY_TIMEOUT			(msecs_to_jiffies(3600000)) /* 60 minutes - based on example max parition size of 128MB */
#define REQUEST_HEARTBEAT_TIMEOUT		(msecs_to_jiffies(500))     /* 0.5 seconds */
#define HEARTBEAT_REQUEST_INTERVAL		(500)
#define LOGGING_SLEEP_INTERVAL			(1000)


/* AMC Identify Command Version Major and Minor Numbers */
#define AMC_GCQ_IDENTIFY_CMD_MAJOR		(1)
#define AMC_GCQ_IDENTIFY_CMD_MINOR		(0)
#define AMC_GCQ_MAGIC_NO                        (0x564D5230)
#define VERSION_BUF_SIZE	                (8)

/* Number of permitted failures before raising a fatal event */
#define HEARTBEAT_FAIL_THRESHOLD		(3)


/*****************************************************************************/
/* Private functions                                                         */
/*****************************************************************************/

/**
 * get_sensor_id() - Get the sensor ID.
 * @cmd_req: The command to be submitted.
 * @flags: The command specific flags.
 *
 * This function is different from `get_sid` in that `get_sid`
 * returns the repo type, whereas this returns the individual
 * sensor ID, thus it only applies to the GET_SINGLE_SENSOR
 * API.
 *
 * Return: the sensor ID.
 */
static int get_sensor_id(int cmd_req, int flags)
{
	int id = 0;

	switch(cmd_req) {
	case GCQ_SUBMIT_CMD_GET_INLET_TEMP_SENSOR:
		id = INLET_TEMP_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_OUTLET_TEMP_SENSOR:
		id = OUTLET_TEMP_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_BOARD_TEMP_SENSOR:
		id = BOARD_TEMP_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_FPGA_TEMP_SENSOR:
		id = FPGA_TEMP_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_VCCINT_VOLTAGE:
		id = VCCINT_VOLT_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_VCC1V2_VOLTAGE:
		id = VCC1V2_VOLT_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_12V_PEX_VOLTAGE:
		id = VCC12V_PEX_VOLT_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_12V_AUX0_VOLTAGE:
		id = VCC12V_AUX0_VOLT_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_VCCINT_CURRENT:
		id = VCCINT_CURRENT_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_VCC1V2_CURRENT:
		id = VCC1V2_CURRENT_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_12V_PEX_CURRENT:
		id = VCCPEX_CURRENT_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_TOTAL_POWER:
		id = TOTAL_POWER_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_SDR:
	case GCQ_SUBMIT_CMD_GET_SDR_SIZE:
		id = ALL_SENSOR_ID;
		break;

	case GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR:
		id = ALL_SENSOR_ID;
		break;

	default:
		id = UNKNOWN_SENSOR_ID;
		break;
	}

	return id;
}

/**
 * get_aid() - Get the API ID.
 * @cmd_req: The command to be submitted.
 * @flags: The command specific flags.
 *
 * Return: the AID.
 */
int get_aid(int cmd_req, int flags)
{
	enum amc_proxy_cmd_sensor_request id = AMC_PROXY_CMD_SENSOR_REQUEST_UNKNOWN;

	switch(cmd_req) {
	case GCQ_SUBMIT_CMD_GET_SDR:
		id = AMC_PROXY_CMD_SENSOR_REQUEST_GET_SDR;
		break;

	case GCQ_SUBMIT_CMD_GET_SDR_SIZE:
		id = AMC_PROXY_CMD_SENSOR_REQUEST_GET_SIZE;
		break;

	case GCQ_SUBMIT_CMD_GET_INLET_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_OUTLET_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_BOARD_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_FPGA_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_VCCINT_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_VCC1V2_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_12V_PEX_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_12V_AUX0_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_VCCINT_CURRENT:
	case GCQ_SUBMIT_CMD_GET_VCC1V2_CURRENT:
	case GCQ_SUBMIT_CMD_GET_12V_PEX_CURRENT:
	case GCQ_SUBMIT_CMD_GET_TOTAL_POWER:
		id = AMC_PROXY_CMD_SENSOR_REQUEST_GET_SINGLE_SDR;
		break;

	case GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR:
		id = AMC_PROXY_CMD_SENSOR_REQUEST_ALL_SDR;
		break;

	default:
		break;
	}

	return id;
}

/**
 * gcq_device_is_ready() - check that the GCQ is ready.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Wait for gcq service is fully ready after a reset.
 *
 * Return: None.
 */
bool gcq_device_is_ready(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	int i = 0, retry = DEVICE_READY_RETRY_COUNT, interval = DEVICE_READY_SLEEP_INTERVAL;

	if (!amc_ctrl_ctxt)
		return -EINVAL;

	for (i = 0; i < retry; i++) {
		msleep(interval);
		memcpy_fromio(&(amc_ctrl_ctxt->amc_shared_mem),
			amc_ctrl_ctxt->gcq_payload_base_virt_addr, sizeof(amc_ctrl_ctxt->amc_shared_mem));

		AMI_VDBG(amc_ctrl_ctxt, "************ AMC Shared Memory ***************, virt_addr : %p",
			amc_ctrl_ctxt->gcq_payload_base_virt_addr);

		AMI_VDBG(amc_ctrl_ctxt, "Magic number                                   : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.amc_magic_no);

		AMI_VDBG(amc_ctrl_ctxt, "Offset of gcq ring buffer inited by gcq server : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.ring_buffer.ring_buffer_off);

		AMI_VDBG(amc_ctrl_ctxt, "Length of gcq ring buffer inited by gcq server : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.ring_buffer.ring_buffer_len);

		AMI_VDBG(amc_ctrl_ctxt, "Offset of amc device status                    : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.status.amc_status_off);

		AMI_VDBG(amc_ctrl_ctxt, "Length of amc device status                    : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.status.amc_status_len);

		AMI_VDBG(amc_ctrl_ctxt, "Current index of ring buffer log               : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.log_msg.log_msg_index);

		AMI_VDBG(amc_ctrl_ctxt, "Offset of dbg log                              : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.log_msg.log_msg_buf_off);

		AMI_VDBG(amc_ctrl_ctxt, "Length of dbg log                              : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.log_msg.log_msg_buf_len);

		AMI_VDBG(amc_ctrl_ctxt, "Offset of data buffer started                  : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.data.amc_data_start);

		AMI_VDBG(amc_ctrl_ctxt, "Offset of data buffer ended                    : 0x%x",
			amc_ctrl_ctxt->amc_shared_mem.data.amc_data_end);

		if (amc_ctrl_ctxt->amc_shared_mem.amc_magic_no == AMC_GCQ_MAGIC_NO) {

			uint32_t amc_status = 0;

			AMI_VDBG(amc_ctrl_ctxt, "AMC Magic Number found");

			/* Read the device status (amc_status_off from payload) */
			amc_status = ioread32(amc_ctrl_ctxt->gcq_payload_base_virt_addr +
				amc_ctrl_ctxt->amc_shared_mem.status.amc_status_off);

			AMI_VDBG(amc_ctrl_ctxt, "Device status value : %x", amc_status);
			if (amc_status) {
				AMI_VDBG(amc_ctrl_ctxt,
					"AMC GCQ service ready after %d ms",
					interval * i);
				return true;
			}
		}
	}

	AMI_ERR(amc_ctrl_ctxt, "AMC GCQ service not ready after %d ms",
		interval * retry);

	return false;
}

/**
 * start_gcq_services() - start the service running.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Start service to allow incoming requests to be handled
 *
 * Return: None.
 */
static int start_gcq_services(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	int ret = 0;
	bool dev_rdy = false;

	if (!amc_ctrl_ctxt)
		return -EINVAL;

	AMI_VDBG(amc_ctrl_ctxt, "Starting AMC services");

	/* Check if device is ready */
	dev_rdy = gcq_device_is_ready(amc_ctrl_ctxt);
	if (!dev_rdy) {
		AMI_ERR(amc_ctrl_ctxt, "Device not ready, reboot required!");
		ret = -ENODEV;
		goto fail;
	}

	amc_ctrl_ctxt->gcq_ring_buf_base_virt_addr = amc_ctrl_ctxt->gcq_payload_base_virt_addr +
		amc_ctrl_ctxt->amc_shared_mem.ring_buffer.ring_buffer_off;

	AMI_VDBG(amc_ctrl_ctxt, "\t- GCQ ring buffer virtual addr   : 0x%p",
		amc_ctrl_ctxt->gcq_ring_buf_base_virt_addr);

	/* Start receiving incoming commands */
	mutex_lock(&amc_ctrl_ctxt->lock);
	amc_ctrl_ctxt->gcq_halted = false;
	mutex_unlock(&amc_ctrl_ctxt->lock);

	AMI_VDBG(amc_ctrl_ctxt, "Successfully started AMC service");
	return SUCCESS;

fail:
	AMI_ERR(amc_ctrl_ctxt, "Failed to start AMC service");
	return ret;
}

/**
 * get_gcq_cid() - get the next CID (unique id).
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @id: Variable to store allocated ID.
 *
 * Return: 0 on success or negative error code
 */
static int get_gcq_cid(struct amc_control_ctxt *amc_ctrl_ctxt, uint16_t *id)
{
	int ret = 0;
        uint32_t cid_id = 0;
	static uint32_t next_cid_id = 0;

        /*
         * If we support 16 cards each with a heartbeat msg, one other
         * synchronous message per card then 32 Id's would be enough.
         */
        struct xa_limit limit = XA_LIMIT(0, MAX_COMMAND_IDS);
	if (!amc_ctrl_ctxt || !id)
                return -EINVAL;

        /*
         * Allocate the next free id, will use the first available
         * after `next_cid_id`, wrapping around if necessary.
	 * If no values left -EBUSY will be returned.
         */
        ret = xa_alloc_cyclic(&cid_xarray, &cid_id, amc_ctrl_ctxt, limit,
		&next_cid_id, GFP_KERNEL);
	if (ret < 0)
	        return ret;

	*id = cid_id;
	return 0;
}

/**
 * remove_gcq_cid() - free the CID (unique id).
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @id: the CID to remove
 *
 * Return: 0 on success or negative error code
 */
static int remove_gcq_cid(struct amc_control_ctxt *amc_ctrl_ctxt, uint16_t id)
{
        struct amc_control_ctxt *amc_ctrl_ctxt_returned = NULL;

	if (!amc_ctrl_ctxt)
		return -EINVAL;

        amc_ctrl_ctxt_returned = xa_erase(&cid_xarray, id);
        if (amc_ctrl_ctxt_returned != amc_ctrl_ctxt) {
                AMI_ERR(amc_ctrl_ctxt, "cid released context not as expected");
                return -EIO;
        }

        return 0;
}

/**
 * amc_proxy_callback() - the event callback.
 * @proxy_id: the associated proxy id.
 * @event_id: the event that cause the callback to be invoked.
 * @arg: void ptr containing the proxy command used to raise request.
 *
 * Return: errno or success code.
 */
static int amc_proxy_callback(uint8_t proxy_id, uint8_t event_id, void* arg)
{
        struct amc_proxy_cmd_struct *amc_proxy_cmd = NULL;
        struct completion *req_complete = NULL;
        int ret = 0;

        if (!arg)
		return -EINVAL;

	amc_proxy_cmd = (struct amc_proxy_cmd_struct*)arg;

        if (amc_proxy_cmd->cmd_opcode == AMC_CMD_ID_HEARTBEAT) {
                req_complete = &amc_proxy_cmd->cmd_complete_heartbeat;
        } else {
                req_complete = &amc_proxy_cmd->cmd_complete;
        }

	switch(event_id) {
	case AMC_PROXY_EVENT_RESPONSE_COMPLETE:
		/* Signal condvar to unblock & fetch the response */
                complete(req_complete);
		break;

	case AMC_PROXY_EVENT_RESPONSE_TIMEOUT:
		/* Signal condvar to unblock & fetch the failed response */
                complete(req_complete);
		amc_proxy_cmd->timed_out = true;
		break;

	default:
        	/* Unknown response */
		ret = -EIO;
		break;
	}

        return ret;
}

/**
 * get_gcq_log_page_addr() - get the log page address.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Return: errno or success code.
 */
static uint32_t get_gcq_log_page_addr(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	if (!amc_ctrl_ctxt)
		return 0;

	return amc_ctrl_ctxt->amc_shared_mem.data.amc_data_start + AMC_LOG_ADDR_OFF;
}

/**
 * acquire_gcq_log_page_sema() - acquire the log page semaphore.
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @addr: the log page address.
 * @len: the log page length.
 *
 * Return: errno or success code.
 */
static int acquire_gcq_log_page_sema(struct amc_control_ctxt *amc_ctrl_ctxt, uint32_t *addr, uint32_t *len)
{
	if (!amc_ctrl_ctxt || !addr || !len)
		return -EINVAL;

	if (down_interruptible(&(amc_ctrl_ctxt->gcq_log_page_sema))) {
		AMI_ERR(amc_ctrl_ctxt, "Log page acquire cancelled");
		return -EIO;
	}

	*addr = get_gcq_log_page_addr(amc_ctrl_ctxt);
	*len = AMC_LOG_PAGE_SIZE;

	return SUCCESS;
}

/**
 * release_amc_log_page_sema() - release the log page semaphore.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Return: None.
 */
static void release_amc_log_page_sema(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	if (amc_ctrl_ctxt)
		up(&(amc_ctrl_ctxt->gcq_log_page_sema));
}

/**
 * amc_shared_mem_size() - Get the size of AMC shared memory.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Return: Shared memory size or 0.
 */
static size_t inline amc_shared_mem_size(struct amc_control_ctxt *amc_ctrl_ctxt)
{	
	if (amc_ctrl_ctxt)
		return amc_ctrl_ctxt->amc_shared_mem.data.amc_data_end -
			amc_ctrl_ctxt->amc_shared_mem.data.amc_data_start + 1;
	
	return 0;
}

/**
 * shm_size_log_page() - Get the total AMC log page size.
 *
 * Return: Total size (page size * pages).
 */
static size_t inline shm_size_log_page(void)
{
	return (AMC_LOG_PAGE_SIZE * AMC_LOG_PAGE_NUM);
}

/**
 * shm_size_data() - Get the size of AMC shared data memory.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Return: Data size or 0.
 */
static size_t inline shm_size_data(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	if (amc_ctrl_ctxt)
		return amc_shared_mem_size(amc_ctrl_ctxt) - shm_size_log_page();
	
	return 0;
}

/**
 * shm_addr_data() - Get the offset of AMC shared data.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Return: Offset or 0.
 */
static u32 inline shm_addr_data(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	if (amc_ctrl_ctxt)
		return amc_ctrl_ctxt->amc_shared_mem.data.amc_data_start + AMC_DATA_ADDR_OFF;
	
	return 0;
}

/**
 * acquire_gcq_data() - Request access to AMC shared data memory.
 * @amc_ctrl_ctxt: AMC struct instance.
 * @addr: Pointer to variable which will hold address of memory.
 * @len: Pointer to variable which will hold length of memory region.
 * 
 * Return: 0 or negative error code.
 */
static int acquire_gcq_data(struct amc_control_ctxt *amc_ctrl_ctxt, u32 *addr, u32 *len)
{
	if (!amc_ctrl_ctxt || !addr || !len)
		return -EINVAL;

	if (down_interruptible(&(amc_ctrl_ctxt->gcq_data_sema))) {
		AMI_ERR(amc_ctrl_ctxt, "Data page acquire cancelled");
		return -EIO;
	}

	*addr = shm_addr_data(amc_ctrl_ctxt);
	*len = shm_size_data(amc_ctrl_ctxt);

	return SUCCESS;
}

/**
 * release_gcq_data() - Release access to the AMC shared data memory.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Return: None.
 */
static void release_gcq_data(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	if (amc_ctrl_ctxt)
		up(&amc_ctrl_ctxt->gcq_data_sema);
}

/**
 * memcpy_gcq_payload_from_device() - copy data from shared memory.
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @offset: the offset.
 * @dst: the destination address.
 * @len: the length.
 *
 * Return: None.
 */
static void memcpy_gcq_payload_from_device(struct amc_control_ctxt *amc_ctrl_ctxt, uint32_t offset,
                                           void *dst, size_t len)
{
	if (!amc_ctrl_ctxt || !dst)
		return;

        if ((offset+len) > amc_shared_mem_size(amc_ctrl_ctxt)) {
                AMI_ERR(amc_ctrl_ctxt, "Shared memory read access outside of range");
                return;
        }

	memcpy_fromio(dst,
		(void __iomem *)(amc_ctrl_ctxt->gcq_payload_base_virt_addr + offset),
		len);
}

/**
 * memcpy_gcq_payload_to_device() - copy data to shared memory.
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @offset: the offset.
 * @data: the data pointer.
 * @len: the length.
 *
 * Return: None.
 */
static void memcpy_gcq_payload_to_device(struct amc_control_ctxt *amc_ctrl_ctxt, uint32_t offset,
                                         const void *data, size_t len)
{
	if (!amc_ctrl_ctxt || !data)
		return;

        if ((offset+len) > amc_shared_mem_size(amc_ctrl_ctxt)) {
                AMI_ERR(amc_ctrl_ctxt, "Shared memory write access outside of range");
                return;
        }

	memcpy_toio(
		(void __iomem *)(amc_ctrl_ctxt->gcq_payload_base_virt_addr + offset),
		data,
		len
	);
}

/**
 * get_gcq_version() - get the GCQ version.
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @data_buf: the offset.
 *
 * Return: the errno code.
 */
static int get_gcq_version(struct amc_control_ctxt *amc_ctrl_ctxt, char *data_buf)
{
	int ret = SUCCESS;

	if (!amc_ctrl_ctxt || !data_buf)
		return -EINVAL;

	ret = submit_gcq_command(amc_ctrl_ctxt, GCQ_SUBMIT_CMD_GET_GCQ_VERSION,
		GCQ_CMD_FLAG_NONE, data_buf, 0);

	if (ret) {
		AMI_ERR(amc_ctrl_ctxt, "Failed to get the GCQ version");
        }

	return ret;
}

/**
 * check_gcq_supported_version() - check if the GCQ version is valid.
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @major: the GCQ major version number
 * @minor: the GCQ minor version number
 *
 * Return: the errno code.
 */
static int check_gcq_supported_version(struct amc_control_ctxt *amc_ctrl_ctxt, const uint16_t major,
	                               const uint16_t minor)
{
	GCQ_VERSION_TYPE ver = { 0 };

	if (!amc_ctrl_ctxt)
		return -EINVAL;
	
	if (iGCQGetVersion(&ver) == SUCCESS) {
		if ((ver.ucVerMajor == major) && (ver.ucVerMinor == minor)) {
			AMI_INFO(amc_ctrl_ctxt, "GCQ Supported Version : %d.%d", major, minor);
			return SUCCESS;
		}
	}

	AMI_ERR(amc_ctrl_ctxt, "Unsupported GCQ Version (expected %d.%d but got %d.%d)",
		ver.ucVerMajor, ver.ucVerMinor, major, minor);
	return -ENOTSUPP;
}

/**
 * check_amc_supported_version() - check if the AMC version is valid.
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @major: the AMC major version number
 * @minor: the AMC minor version number
 *
 * This check is only used to inform the user of a potential incompatibility.
 * The driver will carry on even if this check fails.
 *
 * Return: the errno code.
 */
static int check_amc_supported_version(struct amc_control_ctxt *amc_ctrl_ctxt, const uint16_t major,
	                               const uint16_t minor)
{
	if (!amc_ctrl_ctxt)
		return -EINVAL;
	
	if ((GIT_TAG_VER_MAJOR == major) && (GIT_TAG_VER_MINOR == minor)) {
		AMI_INFO(amc_ctrl_ctxt, "AMC Supported Version : %d.%d", major, minor);
		return SUCCESS;
	}

	AMI_WARN(
		amc_ctrl_ctxt,
		"Potentially incompatible AMC version detected (expected %d.%d but got %d.%d)",
		GIT_TAG_VER_MAJOR, GIT_TAG_VER_MINOR, major, minor
	);
	return -ENOTSUPP;
}

/**
 * get_cmd_command_id() - map the request command.
 * @cmd_req: the request.
 * 
 * Return: the mapped id.
 */
static enum amc_cmd_id get_cmd_command_id(enum gcq_submit_cmd_req cmd_req)
{
	enum amc_cmd_id id = AMC_CMD_ID_UNKNOWN;

	switch(cmd_req) {
	case GCQ_SUBMIT_CMD_DOWNLOAD_PDI:
		id = AMC_CMD_ID_DOWNLOAD_PDI;
		break;

	case GCQ_SUBMIT_CMD_GET_GCQ_VERSION:
		id = AMC_CMD_ID_IDENTIFY;
		break;

	case GCQ_SUBMIT_CMD_GET_SDR:
	case GCQ_SUBMIT_CMD_GET_SDR_SIZE:
		id = AMC_CMD_ID_SENSOR;
		break;

	case GCQ_SUBMIT_CMD_DEVICE_BOOT:
		id = AMC_CMD_ID_DEVICE_BOOT;
		break;
	
	case GCQ_SUBMIT_CMD_COPY_PARTITION:
		id = AMC_CMD_ID_COPY_PARTITION;
		break;

	case GCQ_SUBMIT_CMD_GET_INLET_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_OUTLET_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_BOARD_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_FPGA_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_VCCINT_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_VCC1V2_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_12V_PEX_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_12V_AUX0_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR:
	case GCQ_SUBMIT_CMD_GET_VCCINT_CURRENT:
	case GCQ_SUBMIT_CMD_GET_VCC1V2_CURRENT:
	case GCQ_SUBMIT_CMD_GET_12V_PEX_CURRENT:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR:
	case GCQ_SUBMIT_CMD_GET_TOTAL_POWER:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR:
		id = AMC_CMD_ID_SENSOR;
		break;

        case GCQ_SUBMIT_CMD_GET_HEARTBEAT:
		id = AMC_CMD_ID_HEARTBEAT;
                break;

        case GCQ_SUBMIT_CMD_EEPROM_READ_WRITE:
		id = AMC_CMD_ID_EEPROM_READ_WRITE;
		break;
	
	case GCQ_SUBMIT_CMD_MODULE_READ_WRITE:
		id = AMC_CMD_ID_MODULE_READ_WRITE;
		break;

	default:
		id = AMC_CMD_ID_UNKNOWN;
		break;

	}

	return id;
}

/**
 * map_amc_endpoints() - map the IP based on the endpoint.
 * @dev: the device.
 * @amc_ctrl_ctxt: AMC data struct instance.
 * @ep_gcq: The rpu endpoint info.
 * @ep_gcq_payload: The mgmt endpoint info.
 * 
 * Return: the errno.
 */
static int map_amc_endpoints(struct pci_dev *dev, struct amc_control_ctxt *amc_ctrl_ctxt,
	endpoint_info_struct ep_gcq, endpoint_info_struct ep_gcq_payload)
{
	int ret = 0;
	struct pf_dev_struct *pf_dev = NULL;

	if (!amc_ctrl_ctxt || !dev)
		return -EINVAL;

	AMI_VDBG(amc_ctrl_ctxt, "Mapping GCQ endpoints");

	pf_dev = dev_get_drvdata(&dev->dev);

	if (!pf_dev) {
		ret = -EINVAL;
		goto fail;
	}

	if (pf_dev->pcie_config->header->bar[PCIE_BAR0].requested) {
		ret = -EINVAL;
		goto fail;
	}

	/* TODO: do not hardcode which BAR is requested */
	ret = pci_request_region(amc_ctrl_ctxt->pcie_dev, PCIE_BAR0,
		PCIE_BAR_NAME[PCIE_BAR0]);
	if (ret) {
		AMI_ERR(amc_ctrl_ctxt, "Could not request %s region (SQ_BASE)",
			PCIE_BAR_NAME[PCIE_BAR0]);
		ret = -EIO;
		goto fail;
	}

	pf_dev->pcie_config->header->bar[PCIE_BAR0].requested = true;

	/* Map the GCQ IP Region */
	amc_ctrl_ctxt->gcq_base_virt_addr = pci_iomap_range(amc_ctrl_ctxt->pcie_dev,
		ep_gcq.bar_num, ep_gcq.start_addr, ep_gcq.bar_len);

	if (!(amc_ctrl_ctxt->gcq_base_virt_addr)) {
		AMI_ERR(amc_ctrl_ctxt, "Could not map GCQ IP into virtual memory");
		ret = -EIO;
		goto fail;
	}

	AMI_VDBG(amc_ctrl_ctxt, "\t- gcq_start_phy          : 0x%llx",
		ep_gcq.start_addr);
	AMI_VDBG(amc_ctrl_ctxt, "\t- gcq_len                : 0x%llx",
		ep_gcq.bar_len);
	AMI_VDBG(amc_ctrl_ctxt, "\t- gcq_bar_num            : 0x%x",
		ep_gcq.bar_num);

	/* Map the GCQ Payload Region */
	amc_ctrl_ctxt->gcq_payload_base_virt_addr = pci_iomap_range(amc_ctrl_ctxt->pcie_dev,
		ep_gcq_payload.bar_num,
		ep_gcq_payload.start_addr,
		ep_gcq_payload.bar_len);

	if (!(amc_ctrl_ctxt->gcq_payload_base_virt_addr)) {
		AMI_ERR(amc_ctrl_ctxt, "Could not map GCQ payload into virtual memory");
		ret = -EIO;
		goto fail;
	}

	/* Map the Ring Buffer base address */
	AMI_VDBG(amc_ctrl_ctxt, "Successfully mapped GCQ payload");
	AMI_VDBG(amc_ctrl_ctxt, "\t- gcq_payload_start_phy          : 0x%llx",
		ep_gcq_payload.start_addr);
	AMI_VDBG(amc_ctrl_ctxt, "\t- gcq_payload_len                : 0x%llx",
		ep_gcq_payload.bar_len);
	AMI_VDBG(amc_ctrl_ctxt, "\t- gcq_payload_bar_num            : 0x%x",
		ep_gcq_payload.bar_num);
	AMI_VDBG(amc_ctrl_ctxt, "\t- GCQ payload virtual addr       : 0x%p",
		amc_ctrl_ctxt->gcq_payload_base_virt_addr);

	AMI_VDBG(amc_ctrl_ctxt, "Successfully mapped GCQ endpoints");
	return SUCCESS;

fail:
	AMI_ERR(amc_ctrl_ctxt, "Failed to map GCQ endpoints");
	return ret;
}

/**
 * unmap_pci_io() - unmap the PCI IO.
 * @dev: the device.
 * @virt_addr: the virtual address.
 *
 * Return: None.
 */
static void unmap_pci_io(struct pci_dev *dev, void __iomem **virt_addr)
{
	if (dev && virt_addr && *virt_addr) {
		pci_iounmap(dev, *virt_addr);
		*virt_addr = NULL;
	}
}

/**
 * release_amc() - free resources.
 * @amc_ctrl_ctxt: AMC data struct instance.
 *
 * Return: None.
 */
void release_amc(struct amc_control_ctxt **amc_ctrl_ctxt)
{
	if (amc_ctrl_ctxt && *amc_ctrl_ctxt) {
		kfree(*amc_ctrl_ctxt);
		*amc_ctrl_ctxt = NULL;
	}
}

/**
 * heartbeat_health_thread() - the heartbeat health thread
 *
 * @data: the data pointer to the amc control context
 *
 * Periodically send out the heartbeat message
 *
 * Return: the errno return code if thread exits
 */
static int heartbeat_health_thread(void *data)
{
        struct amc_control_ctxt *amc_ctxt = NULL;
        uint8_t request_id = 0;
        uint8_t response_id = 0;
        int ret = 0;
	int fail_count = 0;
	bool fatal_event_raised = false;

        if (!data) {
                PR_ERR("Heartbeat health thread null data arg");
		/*
		 * Set the failure count to the threshold but do not set
		 * `fatal_event_raised` yet so we can trigger the callback.
		 */
                fail_count = HEARTBEAT_FAIL_THRESHOLD;
        } else {
                amc_ctxt = (struct amc_control_ctxt *)data;
        }

        while(1)
        {
                if (!fatal_event_raised && (fail_count < HEARTBEAT_FAIL_THRESHOLD))
                {
                        ret = submit_gcq_command(amc_ctxt,
                                                 GCQ_SUBMIT_CMD_GET_HEARTBEAT,
                                                 request_id,
                                                 &response_id,
                                                 sizeof(response_id));
                        if (ret) {
                                PR_ERR("Failed to get the heartbeat msg!");
                                if (amc_ctxt->event_cb) {
                                        amc_ctxt->event_cb(
						AMC_EVENT_ID_HEARTBEAT_EXPIRED,
						amc_ctxt->event_cb_data
					);
                                }
                                fail_count++;

                        } else if (response_id != request_id) {
                                PR_ERR("Heartbeat validation failed!");
                                if (amc_ctxt->event_cb) {
                                        amc_ctxt->event_cb(
						AMC_EVENT_ID_HEARTBEAT_VALIDATION,
						amc_ctxt->event_cb_data
					);
                                }
                                fail_count++;

                        } else {
				/* Reset fail count */
				fail_count = 0;
			}

                        /* Increment or rollover counter */
                        request_id += 1;
                } else {
			if (!fatal_event_raised) {
				PR_ERR("Heartbeat fail count above threshold! Raising fatal event...");
				amc_ctxt->event_cb(
					AMC_EVENT_ID_HEARTBEAT_FATAL,
					amc_ctxt->event_cb_data
				);
				fatal_event_raised = true;
			}
		}

                msleep(HEARTBEAT_REQUEST_INTERVAL);

                /* only exit from the thread is within the unset_amc context */
                if (kthread_should_stop()) {
			break;
                }
        }
        return 0;
}

/**
 * logging_thread() - the AMC logging thread
 * @data: the data pointer to the amc control context
 *
 * Periodically checks for incoming AMC logs and prints them to dmesg
 *
 * Return: 0 if the thread exits
 */
static int logging_thread(void *data)
{
	struct amc_control_ctxt *amc_ctxt = NULL;
	bool logging_failed = false;

	if (!data) {
		PR_ERR("AMC Logging thread null data arg");
		logging_failed = true;
	} else {
		amc_ctxt = (struct amc_control_ctxt *)data;
		amc_ctxt->last_printed_msg_index = 0;
	}

	while (1) {
		if (logging_failed == false) {
			dump_amc_log(amc_ctxt);
		}
		msleep(LOGGING_SLEEP_INTERVAL);

		/* only exit from the thread is within the unset_amc context */
		if (kthread_should_stop()) {
			break;
		}
	}
	return 0;
}


/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

/*
 * Get the SID (repo type).
 */
int get_sid(int cmd_req, int flags)
{
	enum amc_proxy_cmd_sensor_repo id = AMC_PROXY_CMD_SENSOR_REPO_UNKNOWN;

	switch(cmd_req) {
	case GCQ_SUBMIT_CMD_GET_SDR:
	case GCQ_SUBMIT_CMD_GET_SDR_SIZE:
		if (flags & GCQ_CMD_FLAG_REPO_TYPE_TEMP)
			id = AMC_PROXY_CMD_SENSOR_REPO_TEMP;
		else if (flags & GCQ_CMD_FLAG_REPO_TYPE_CURRENT)
			id = AMC_PROXY_CMD_SENSOR_REPO_CURRENT;
		else if (flags & GCQ_CMD_FLAG_REPO_TYPE_POWER)
			id = AMC_PROXY_CMD_SENSOR_REPO_POWER;
		else if (flags & GCQ_CMD_FLAG_REPO_TYPE_TOTAL_POWER)
			id = AMC_PROXY_CMD_SENSOR_REPO_TOTAL_POWER;
		else if (flags & GCQ_CMD_FLAG_REPO_TYPE_VOLTAGE)
			id = AMC_PROXY_CMD_SENSOR_REPO_VOLTAGE;
		else if (flags & GCQ_CMD_FLAG_REPO_TYPE_BD_INFO)
			id = AMC_PROXY_CMD_SENSOR_REPO_BDINFO;
		else if (flags & GCQ_CMD_FLAG_REPO_TYPE_FPT)
			id = AMC_PROXY_CMD_SENSOR_REPO_FPT;
		else
			id = AMC_PROXY_CMD_SENSOR_REPO_UNKNOWN;
		break;

	case GCQ_SUBMIT_CMD_GET_INLET_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_OUTLET_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_BOARD_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_FPGA_TEMP_SENSOR:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_TEMP_SENSOR:
		id = AMC_PROXY_CMD_SENSOR_REPO_TEMP;
		break;

	case GCQ_SUBMIT_CMD_GET_VCCINT_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_VCC1V2_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_12V_PEX_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_12V_AUX0_VOLTAGE:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_VOLTAGE_SENSOR:
		id = AMC_PROXY_CMD_SENSOR_REPO_VOLTAGE;
		break;

	case GCQ_SUBMIT_CMD_GET_VCCINT_CURRENT:
	case GCQ_SUBMIT_CMD_GET_VCC1V2_CURRENT:
	case GCQ_SUBMIT_CMD_GET_12V_PEX_CURRENT:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_CURRENT_SENSOR:
		id = AMC_PROXY_CMD_SENSOR_REPO_CURRENT;
		break;

	case GCQ_SUBMIT_CMD_GET_TOTAL_POWER:
	case GCQ_SUBMIT_CMD_GET_ALL_INST_POWER_SENSOR:
		id = (SDR_TYPE_TOTAL_POWER == SDR_POWER_TYPE) ?
			(AMC_PROXY_CMD_SENSOR_REPO_TOTAL_POWER) : (AMC_PROXY_CMD_SENSOR_REPO_POWER);
		break;

	default:
		break;
	}

	return id;
}

/*
 * Get the repo ID.
 */
int get_rid(int repo_type)
{
	enum amc_proxy_cmd_sensor_repo id = AMC_PROXY_CMD_SENSOR_REPO_UNKNOWN;

	switch(repo_type) {
	case AMC_CL_SENSOR_GET_SIZE:
		id = AMC_PROXY_CMD_SENSOR_REPO_GET_SIZE;
                break;

	case AMC_CL_SENSOR_BOARD_INFO:
		id = AMC_PROXY_CMD_SENSOR_REPO_BDINFO;
                break;

	case AMC_CL_SENSOR_TEMPERATURE:
		id = AMC_PROXY_CMD_SENSOR_REPO_TEMP;
                break;

	case AMC_CL_SENSOR_VOLTAGE:
		id = AMC_PROXY_CMD_SENSOR_REPO_VOLTAGE;
                break;

	case AMC_CL_SENSOR_CURRENT:
		id = AMC_PROXY_CMD_SENSOR_REPO_CURRENT;
                break;

	case AMC_CL_SENSOR_POWER:
		id = AMC_PROXY_CMD_SENSOR_REPO_POWER;
                break;

	case AMC_CL_SENSOR_TOTAL_POWER:
		id = AMC_PROXY_CMD_SENSOR_REPO_TOTAL_POWER;
                break;

	default:
		break;
	}

	return id;
}

/*
 * Top level function to submit a request and wait on response
 */
int submit_gcq_command(struct amc_control_ctxt *amc_ctrl_ctxt, enum gcq_submit_cmd_req cmd_req,
	uint32_t flags, uint8_t *data_buf, uint32_t data_size)
{
	int ret = SUCCESS;
	enum amc_cmd_id cmd_id = AMC_CMD_ID_UNKNOWN;
	bool log_page_acquired = false, data_page_acquired = false;
	uint32_t length = 0;
	enum amc_proxy_cmd_sensor_repo sid = AMC_PROXY_CMD_SENSOR_REPO_UNKNOWN;
	enum amc_proxy_cmd_sensor_request aid = AMC_PROXY_CMD_SENSOR_REQUEST_UNKNOWN;
	int sensor_id = 0;
	struct amc_proxy_cmd_struct *amc_proxy_cmd = NULL;
	uint32_t payload_size = 0;
	uint64_t payload_address = 0;
	uint16_t cid = 0;
        struct completion *req_complete = NULL;

	/* data_buf is required only for some commands */
	if (!amc_ctrl_ctxt)
		return -EINVAL;


	if (amc_ctrl_ctxt->gcq_halted) {
		AMI_ERR(amc_ctrl_ctxt, "Service is halted");
		ret = -EIO;
		goto done;
	}

	cmd_id = get_cmd_command_id(cmd_req);
	if (cmd_id == AMC_CMD_ID_UNKNOWN) {
		AMI_ERR(amc_ctrl_ctxt, "Unable to allocate command id");
		ret = -EIO;
		goto done;
	}

        if (cmd_id != AMC_CMD_ID_HEARTBEAT) {
                AMI_VDBG(amc_ctrl_ctxt, "Submitting command [%d] with resp len %d", cmd_id, data_size);
        }

	switch(cmd_id) {
	case AMC_CMD_ID_DOWNLOAD_PDI:
	case AMC_CMD_ID_IDENTIFY:
	case AMC_CMD_ID_SENSOR:
        case AMC_CMD_ID_HEARTBEAT:
        case AMC_CMD_ID_EEPROM_READ_WRITE:
	case AMC_CMD_ID_MODULE_READ_WRITE:
		if (!data_buf) {
			ret = -EINVAL;
			goto done;
		}
		break;
	
	case AMC_CMD_ID_COPY_PARTITION:
		/* data_buf not required - must check length */
		if (!data_size) {
			ret = -EINVAL;
			goto done;
		}
		break;

	case AMC_CMD_ID_DEVICE_BOOT:
                /* data_buf not required */
		break;

	default:
		AMI_ERR(amc_ctrl_ctxt, ",command id %d not supported", cmd_id);
		ret = -EIO;
		goto done;
	}

	amc_proxy_cmd = kzalloc(sizeof(struct amc_proxy_cmd_struct), GFP_KERNEL);
        if (!amc_proxy_cmd) {
		AMI_ERR(amc_ctrl_ctxt, "Failed to allocate kernel memory for amc_proxy_cmd");
		ret = -ENOMEM;
		goto done;
	}

	/* Payload formation */
	switch(cmd_id) {
	case AMC_CMD_ID_IDENTIFY:
		break; /* No Payload */
	
	case AMC_CMD_ID_DEVICE_BOOT:
		break; /* No Payload */

        case AMC_CMD_ID_HEARTBEAT:
                break; /* No Payload */
	
	case AMC_CMD_ID_COPY_PARTITION:
	{
		/*
		 * For COPY_PARTITION we allocate max memory - it will be populated
		 * by AMC and used by AMC only. This gets released again by AMI
		 * when the command is complete/aborted. As part of the payload,
		 * however, we pass in the value of `data_size` - this must be set
		 * to the size of the source partition by the caller.
		 */
		if (acquire_gcq_data(amc_ctrl_ctxt, (uint32_t *)&(payload_address), &length)) {
			ret = -EIO;
			goto done;
		}

		data_page_acquired = true;
		payload_size = data_size;

		AMI_VDBG(amc_ctrl_ctxt, "Copy partition max data size = %d, actual size = %d",
			length, payload_size);
		if (length < payload_size) {
			AMI_ERR(amc_ctrl_ctxt,
				"Data request length is %d but allocated length is %d",
				payload_size, length);
			ret = -ENOMEM;
			goto done;
		}
	}
	break;

	case AMC_CMD_ID_DOWNLOAD_PDI:
	{
		if (acquire_gcq_data(amc_ctrl_ctxt, (uint32_t *)&(payload_address), &length)) {
			ret = -EIO;
			goto done;
		}

		data_page_acquired = true;
		payload_size = data_size;

		AMI_VDBG(amc_ctrl_ctxt, "Payload size = %d, page length = %d",
			payload_size, length);
		if (length < payload_size) {
			AMI_WARN(amc_ctrl_ctxt,
				"Data request length is %d but allocated length is %d",
				payload_size, length);
			payload_size = length;
		}

		/* Copy payload data to address */
		memcpy_gcq_payload_to_device(amc_ctrl_ctxt, payload_address, data_buf, data_size);
	}
	break;

	case AMC_CMD_ID_SENSOR:
	{
		if (acquire_gcq_log_page_sema(amc_ctrl_ctxt,
			(uint32_t *)&(payload_address), &length)) {
			ret = -EIO;
			goto done;
		}
		log_page_acquired = true;

		payload_size = data_size;
		if (length < payload_size) {
			AMI_WARN(amc_ctrl_ctxt,
				"Sensor request length is %d but allocated length is %d",
				payload_size, length);
			payload_size = length;
		}

		/* Sensor request ID */
		sid = get_sid(cmd_req, flags);
		if (sid == AMC_PROXY_CMD_SENSOR_REPO_UNKNOWN) {
			AMI_ERR(amc_ctrl_ctxt, "Sensor request ID not defined for command 0x%X", cmd_req);
			ret = -EIO;
			goto done;
		}

		/* Sensor API ID */
		aid = get_aid(cmd_req, flags);
		if (aid == AMC_PROXY_CMD_SENSOR_REQUEST_UNKNOWN) {
			AMI_ERR(amc_ctrl_ctxt, "Sensor API ID not defined for command 0x%X", cmd_req);
			ret = -EIO;
			goto done;
		}

		/* Sensor ID */
		sensor_id = get_sensor_id(cmd_req, flags);
		if (sensor_id == UNKNOWN_SENSOR_ID) {
			AMI_ERR(amc_ctrl_ctxt, "Sensor ID not defined for command 0x%X", cmd_req);
			ret = -EIO;
			goto done;
		}

		AMI_VDBG(amc_ctrl_ctxt, "Sensor payload sensor_id=%d api=%d sid=%d", sensor_id, aid, sid);
	}
	break;

        case AMC_CMD_ID_EEPROM_READ_WRITE:
	case AMC_CMD_ID_MODULE_READ_WRITE:
        {
		int req_type = MAX_AMC_PROXY_CMD_RW_REQUEST;

                if (acquire_gcq_data(amc_ctrl_ctxt, (uint32_t *)&(payload_address), &length)) {
			ret = -EIO;
			goto done;
		}

		data_page_acquired = true;
		payload_size = data_size;

		AMI_VDBG(amc_ctrl_ctxt, "Payload size = %d, page length = %d",
			payload_size, length);
		if (length < payload_size) {
			AMI_WARN(amc_ctrl_ctxt,
				 "Data request length is %d but allocated length is %d",
				 payload_size, length);
			payload_size = length;
		}

		/* Check if we need to copy the payload data */
		switch (cmd_id) {
		case AMC_CMD_ID_EEPROM_READ_WRITE:
			req_type = EEPROM_GET_TYPE(flags);
			break;
		
		case AMC_CMD_ID_MODULE_READ_WRITE:
			req_type = MODULE_RW_TYPE(flags);
			break;
		
		default:
			break;
		}

		if (req_type == AMC_PROXY_CMD_RW_REQUEST_WRITE) {
			/* Copy payload data to address */
			memcpy_gcq_payload_to_device(amc_ctrl_ctxt, payload_address, data_buf, data_size);
		}
        }
        break;

        default:
                break;
	}

	/* Allocate unique id to the command */
	if ((ret = get_gcq_cid(amc_ctrl_ctxt, &cid))) {
		AMI_ERR(amc_ctrl_ctxt,
			"Error: Allocation of unique cid failed");
		goto done;
	}
	amc_proxy_cmd->cmd_cid = cid;

	/* Init condition variable */
        if (cmd_id == AMC_CMD_ID_HEARTBEAT) {
                req_complete = &amc_proxy_cmd->cmd_complete_heartbeat;
        } else {
                req_complete = &amc_proxy_cmd->cmd_complete;
        }
        init_completion(req_complete);

	/* Set timeout in ms */
	amc_proxy_cmd->cmd_timeout_jiffies = jiffies + REQUEST_MSQ_TIMEOUT;
	amc_proxy_cmd->cmd_arg = amc_ctrl_ctxt;
	amc_proxy_cmd->cmd_fw_if_gcq = &amc_ctrl_ctxt->fw_if_cfg;
	amc_proxy_cmd->cmd_rcode = 0;
        amc_proxy_cmd->cmd_suppress_dbg = false;
        amc_proxy_cmd->cmd_opcode = cmd_id;

        /* Multiple thread now generating gcq command requests, protect concurrent access */
        mutex_lock(&amc_ctrl_ctxt->gcq_cmd_lock);

	switch(cmd_id) {
	case AMC_CMD_ID_IDENTIFY:
		ret = amc_proxy_request_identity(amc_proxy_cmd);
		break;

	case AMC_CMD_ID_SENSOR:
	{
		struct amc_proxy_sensor_request sensor_req = {0};
		sensor_req.sensor_id = sensor_id;
		sensor_req.repo = sid;
		sensor_req.req = aid;
		sensor_req.length = payload_size;
		sensor_req.address = payload_address;
		ret = amc_proxy_request_sensor(amc_proxy_cmd, &sensor_req);
	}
	break;

	case AMC_CMD_ID_DOWNLOAD_PDI:
	{
		struct amc_proxy_pdi_download_request pdi_download_request = {0};
		pdi_download_request.length = payload_size;
		pdi_download_request.address = payload_address;

		/* Using the `flags` argument to select the partition. */
		if (PDI_PARTITION(flags) != FPT_UPDATE_FLAG)
			pdi_download_request.partition = PDI_PARTITION(flags);
		else
			pdi_download_request.partition = FPT_UPDATE_MAGIC;

		pdi_download_request.last_chunk = PDI_CHUNK_IS_LAST(flags);
		pdi_download_request.chunk = PDI_CHUNK(flags);
		pdi_download_request.chunk_size = PDI_CHUNK_SIZE;
		/* Set longer timeout for the PDI download */
		amc_proxy_cmd->cmd_timeout_jiffies = jiffies + REQUEST_DOWNLOAD_TIMEOUT;
		ret = amc_proxy_request_pdi_download(amc_proxy_cmd, &pdi_download_request);
	}
	break;

	case AMC_CMD_ID_DEVICE_BOOT:
	{
		/* Using same request format as DOWNLOAD_PDI */
		struct amc_proxy_pdi_download_request device_boot_request = {0};
		/* Using the `flags` argument to select the partition. */
		device_boot_request.partition = flags;
		ret = amc_proxy_request_device_boot(amc_proxy_cmd, &device_boot_request);
	}
	break;

	case AMC_CMD_ID_COPY_PARTITION:
	{
		struct amc_proxy_partition_copy_request partition_copy_request = {0};
		/* Using the `flags` argument to select the partition. */
		partition_copy_request.src = PARTITION_SRC(flags);
		partition_copy_request.dest = PARTITION_DEST(flags);
		partition_copy_request.length = payload_size;
		partition_copy_request.address = payload_address;
		/* Set longer timeout for partition copy request. */
		amc_proxy_cmd->cmd_timeout_jiffies = jiffies + REQUEST_COPY_TIMEOUT;
		ret = amc_proxy_request_partition_copy(amc_proxy_cmd, &partition_copy_request);
	}
	break;

	case AMC_CMD_ID_HEARTBEAT:
	{
		struct amc_proxy_hearbeat_request heartbeat_req = {0};
                amc_proxy_cmd->cmd_suppress_dbg = true;
		/* Using the `flags` argument to set the request id. */
		heartbeat_req.request_id = flags;
                amc_proxy_cmd->cmd_timeout_jiffies = jiffies + REQUEST_HEARTBEAT_TIMEOUT;
		ret = amc_proxy_request_heartbeat(amc_proxy_cmd, &heartbeat_req);
                break;
	}

        case AMC_CMD_ID_EEPROM_READ_WRITE:
        {
                struct amc_proxy_eeprom_rw_request eeprom_req = {0};
                eeprom_req.address = payload_address;
                eeprom_req.length = payload_size;
                eeprom_req.type = EEPROM_GET_TYPE(flags);
                eeprom_req.offset = EEPROM_GET_OFFSET(flags);
                ret = amc_proxy_request_eeprom_read_write(amc_proxy_cmd, &eeprom_req);
                break;
        }

	case AMC_CMD_ID_MODULE_READ_WRITE:
	{
		struct amc_proxy_module_rw_request module_req = {0};
                module_req.address = payload_address;
		module_req.device_id = MODULE_RW_DEVICE(flags);
		module_req.page = MODULE_RW_PAGE(flags);
		module_req.offset = MODULE_RW_OFFSET(flags);
                module_req.length = payload_size;
		module_req.type = MODULE_RW_TYPE(flags);
                ret = amc_proxy_request_module_read_write(amc_proxy_cmd, &module_req);
		break;
	}

	default:
		ret = -EINVAL;
		AMI_ERR(amc_ctrl_ctxt, "Unsupported request %d", cmd_id);
		break;
	}

        mutex_unlock(&amc_ctrl_ctxt->gcq_cmd_lock);

	/* Wait for command completion */
        if (ret || wait_for_completion_killable(req_complete)) {
                ret = -ERESTARTSYS;
                AMI_ERR(amc_ctrl_ctxt, "Submitted command killed, abort.");
                amc_proxy_request_abort(amc_proxy_cmd);
                goto done;
        }

	if (amc_proxy_cmd->timed_out) {
		AMI_ERR(amc_ctrl_ctxt, "Submitted command timed out");
		amc_proxy_request_abort(amc_proxy_cmd);
		ret = -ETIMEDOUT;
		goto done;
	}

	/* Check return code before reading response */
	if (amc_proxy_cmd->cmd_rcode != 0) {
		ret = -EINVAL;
		AMI_ERR(amc_ctrl_ctxt, "Request failed %d", amc_proxy_cmd->cmd_rcode);
		amc_proxy_request_abort(amc_proxy_cmd);
		goto done;
	}

        if (cmd_id != AMC_CMD_ID_HEARTBEAT) {
                AMI_VDBG(amc_ctrl_ctxt, "Submitted command was processed successfully, rcode : %d",
                                amc_proxy_cmd->cmd_rcode);
        }

	switch(cmd_id) {
	case AMC_CMD_ID_IDENTIFY:
	{
		struct amc_proxy_identify_response identity = {0};
		ret = amc_proxy_get_response_identity(amc_proxy_cmd, &identity);
		if (!ret) {
			memcpy(&data_buf[0], &(identity.ver_major), sizeof(uint8_t));
			memcpy(&data_buf[1], &(identity.ver_minor), sizeof(uint8_t));
			memcpy(&data_buf[2], &(identity.ver_patch), sizeof(uint8_t));
			memcpy(&data_buf[3], &(identity.local_changes), sizeof(uint8_t));
			memcpy(&data_buf[4], &(identity.dev_commits), sizeof(uint16_t));
			memcpy(&data_buf[6], &(identity.link_ver_major), sizeof(uint8_t));
			memcpy(&data_buf[7], &(identity.link_ver_minor), sizeof(uint8_t));
		}
	}
	break;

	case AMC_CMD_ID_SENSOR:
	{
		ret = amc_proxy_get_response_sensor(amc_proxy_cmd);
		if (!ret) {
			memcpy_gcq_payload_from_device(amc_ctrl_ctxt, payload_address, data_buf, data_size);
		}
	}
	break;
		
	case AMC_CMD_ID_DOWNLOAD_PDI:
		ret = amc_proxy_get_response_pdi_download(amc_proxy_cmd);
		break;
	
	case AMC_CMD_ID_DEVICE_BOOT:
		ret = amc_proxy_get_response_device_boot(amc_proxy_cmd);
		break;
	
	case AMC_CMD_ID_COPY_PARTITION:
		ret = amc_proxy_get_response_partition_copy(amc_proxy_cmd);
		break;
	
	case AMC_CMD_ID_HEARTBEAT:
	{
		struct amc_proxy_heartbeat_response heartbeat = {0};
		ret = amc_proxy_get_response_heartbeat(amc_proxy_cmd, &heartbeat);
                memcpy(&data_buf[0], &(heartbeat.request_id), sizeof(uint8_t));
		break;
	}

        case AMC_CMD_ID_EEPROM_READ_WRITE:
        {
		ret = amc_proxy_get_response_eeprom_read_write(amc_proxy_cmd);
                if (!ret) {
                        if(EEPROM_GET_TYPE(flags) == AMC_PROXY_CMD_RW_REQUEST_READ) {
			        memcpy_gcq_payload_from_device(amc_ctrl_ctxt, payload_address, data_buf, data_size);
                        }
		}
		break;
        }

	case AMC_CMD_ID_MODULE_READ_WRITE:
	{
		ret = amc_proxy_get_response_module_read_write(amc_proxy_cmd);
                if (!ret) {
                        if(MODULE_RW_TYPE(flags) == AMC_PROXY_CMD_RW_REQUEST_READ) {
			        memcpy_gcq_payload_from_device(amc_ctrl_ctxt, payload_address, data_buf, data_size);
                        }
		}
		break;
	}

	default:
		AMI_ERR(amc_ctrl_ctxt, "Unsupported response %d", cmd_id);
		break;
	}

done:

	if (amc_proxy_cmd)
		remove_gcq_cid(amc_ctrl_ctxt, amc_proxy_cmd->cmd_cid);

	if (log_page_acquired)
		release_amc_log_page_sema(amc_ctrl_ctxt);

	if (data_page_acquired)
		release_gcq_data(amc_ctrl_ctxt);

	if (amc_proxy_cmd)
		kfree(amc_proxy_cmd);

	return ret;
}

/*
 * Stop the GCQ service.
 */
void stop_gcq_services(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	if (!amc_ctrl_ctxt)
		return;

        /* Check if already stopped */
        if (amc_ctrl_ctxt->gcq_halted == false) {

                AMI_VDBG(amc_ctrl_ctxt, "Attempting stopping gcq services...");

                /* stop receiving incoming commands */
                mutex_lock(&(amc_ctrl_ctxt->lock));
                amc_ctrl_ctxt->gcq_halted = true;
                mutex_unlock(&(amc_ctrl_ctxt->lock));

                AMI_VDBG(amc_ctrl_ctxt, "AMC services are stopped...");
        }
}


/*
 * Init setup & configuration for the AMC
 */
int setup_amc(struct pci_dev *dev, struct amc_control_ctxt **amc_ctrl_ctxt,
	      endpoint_info_struct ep_gcq, endpoint_info_struct ep_gcq_payload,
              amc_event_callback event_cb, void *event_cb_data)
{
	int ret = 0;
	char *version_buf = NULL;
        const char *amc_hb_thread_name = "amc heartbeat";
	const char *amc_log_thread_name = "amc logging";

	if (!dev || !amc_ctrl_ctxt)
		return -EINVAL;

	DEV_VDBG(dev, "Setting up AMC GCQ");

	*amc_ctrl_ctxt = kzalloc(sizeof(struct amc_control_ctxt), GFP_KERNEL);
	if (!(*amc_ctrl_ctxt)) {
		DEV_ERR(dev, "Failed to allocate kernel memory for amc_ctrl_ctxt");
		ret = -ENOMEM;
		goto fail;
	}

	(*amc_ctrl_ctxt)->pcie_dev = dev;
	(*amc_ctrl_ctxt)->gcq_halted = true;
	(*amc_ctrl_ctxt)->gcq_base_virt_addr = NULL;
	(*amc_ctrl_ctxt)->gcq_payload_base_virt_addr = NULL;
        (*amc_ctrl_ctxt)->heartbeat_thread_created = false;
        (*amc_ctrl_ctxt)->logging_thread_created = false;

	mutex_init(&((*amc_ctrl_ctxt)->lock));
        mutex_init(&((*amc_ctrl_ctxt)->gcq_cmd_lock));
	sema_init(&((*amc_ctrl_ctxt)->gcq_log_page_sema), 1);
	sema_init(&((*amc_ctrl_ctxt)->gcq_data_sema), 1);

	/* Map Endpoints */
	ret = map_amc_endpoints(dev, *amc_ctrl_ctxt, ep_gcq, ep_gcq_payload);
	if (ret)
		goto fail;

	/* Start Service */
	ret = start_gcq_services(*amc_ctrl_ctxt);
	if (ret)
		goto fail;

        /* Create GCQ instance */
	(*amc_ctrl_ctxt)->fw_if_gcq_consumer.ullBaseAddress = (uint64_t)(*amc_ctrl_ctxt)->gcq_base_virt_addr;
	(*amc_ctrl_ctxt)->fw_if_gcq_consumer.xInterruptMode = FW_IF_GCQ_INTERRUPT_MODE_NONE;
	(*amc_ctrl_ctxt)->fw_if_gcq_consumer.xMode = FW_IF_GCQ_MODE_CONSUMER;
	(*amc_ctrl_ctxt)->fw_if_gcq_consumer.ullRingAddress = (uint64_t)(*amc_ctrl_ctxt)->gcq_ring_buf_base_virt_addr;
	(*amc_ctrl_ctxt)->fw_if_gcq_consumer.ulRingLength = (uint64_t)(*amc_ctrl_ctxt)->amc_shared_mem.ring_buffer.ring_buffer_len;
	(*amc_ctrl_ctxt)->fw_if_gcq_consumer.ulSubmissionQueueSlotSize = AMC_PROXY_REQUEST_SIZE;
	(*amc_ctrl_ctxt)->fw_if_gcq_consumer.ulCompletionQueueSlotSize = AMC_PROXY_RESPONSE_SIZE;
	ret = ulFW_IF_GCQ_create(&(*amc_ctrl_ctxt)->fw_if_cfg, &(*amc_ctrl_ctxt)->fw_if_gcq_consumer);
	if (ret != FW_IF_ERRORS_NONE) {
		DEV_ERR(dev, "FW_IF_GCQ_create() failed %d", ret);
		goto fail;
	}

        /* Init proxy and bind in callback */
	ret = amc_proxy_init(0, &(*amc_ctrl_ctxt)->fw_if_cfg);
	if (ret) {
		DEV_ERR(dev, "amc_proxy_init() failed %d", ret);
		goto fail;
	}

	ret = amc_proxy_bind_callback(&(*amc_ctrl_ctxt)->fw_if_cfg, amc_proxy_callback);
	if (ret) {
		DEV_ERR(dev, "amc_proxy_bind_callback() failed %d", ret);
		goto fail;
	}

	/* Getting GCQ Version, Check the GCQ Version so that we don't
	 * send unsupported commands to AMC firmware */
	version_buf = vzalloc(sizeof(char) * VERSION_BUF_SIZE);
	if (!version_buf) {
		DEV_ERR(dev, "Failed to allocate memory for GCQ version buffer");
		ret = -ENOMEM;
		goto fail;
	}

	ret = get_gcq_version(*amc_ctrl_ctxt, version_buf);
	if (ret) {
		ret = -EIO;
		goto fail;
	}

	(*amc_ctrl_ctxt)->version.ver_major = (uint8_t)version_buf[0];
	(*amc_ctrl_ctxt)->version.ver_minor = (uint8_t)version_buf[1];
	(*amc_ctrl_ctxt)->version.ver_patch = (uint8_t)version_buf[2];
	(*amc_ctrl_ctxt)->version.local_changes = (uint8_t)version_buf[3];
	(*amc_ctrl_ctxt)->version.dev_commits = (uint16_t)(version_buf[4] & 0x00FF) |
		(uint16_t)((version_buf[5] << 8) & 0xFF00);

	DEV_VDBG(dev, "amc version = %d.%d.%d-%d gcq version = %d.%d",
		(*amc_ctrl_ctxt)->version.ver_major,
		(*amc_ctrl_ctxt)->version.ver_minor,
		(*amc_ctrl_ctxt)->version.ver_patch,
		(*amc_ctrl_ctxt)->version.dev_commits,
		(uint8_t)version_buf[6],
		(uint8_t)version_buf[7]
	);

	ret = check_gcq_supported_version(*amc_ctrl_ctxt, version_buf[6], version_buf[7]);
	if (ret)
		goto fail;
	
	/* Check AMC version */
	if (check_amc_supported_version(*amc_ctrl_ctxt, (*amc_ctrl_ctxt)->version.ver_major,
			(*amc_ctrl_ctxt)->version.ver_minor)) {
		
		(*amc_ctrl_ctxt)->compat_mode = true;
		DEV_WARN(
			dev,
			"Running in compatibility mode - most features will be unavailable!"
			" Update your AMC to version %d.%d.",
			GIT_TAG_VER_MAJOR,
			GIT_TAG_VER_MINOR
		);
	}

        vfree(version_buf);

	/*
	 * COMPAT MODE: heartbeat disabled, logging disabled.
	 * When the AMC version does not match the current AMI version, we run
	 * in compatibility mode - this disables most driver features,
	 * including the heartbeat and logging threads.
	 */
	if (!(*amc_ctrl_ctxt)->compat_mode) {
		/* Spawn the heartbeat thread once version is verified */
		(*amc_ctrl_ctxt)->event_cb = event_cb;
		(*amc_ctrl_ctxt)->event_cb_data = event_cb_data;
		(*amc_ctrl_ctxt)->heartbeat_thread = kthread_create(
			heartbeat_health_thread,
			*amc_ctrl_ctxt,
			amc_hb_thread_name
		);

		if (IS_ERR((*amc_ctrl_ctxt)->heartbeat_thread)) {
			DEV_ERR(dev, "Unable to create the %s thread", amc_hb_thread_name);
			ret = PTR_ERR((*amc_ctrl_ctxt)->heartbeat_thread);
			goto fail;
		} else {
			DEV_VDBG(dev, "Successfully created %s hearbeat thread", amc_hb_thread_name);
			(*amc_ctrl_ctxt)->heartbeat_thread_created = true;
			wake_up_process((*amc_ctrl_ctxt)->heartbeat_thread);
		}

		/* Spawn logging thread. */
		(*amc_ctrl_ctxt)->logging_thread = kthread_create(
			logging_thread,
			*amc_ctrl_ctxt,
			amc_log_thread_name
		);

		if (IS_ERR((*amc_ctrl_ctxt)->logging_thread)) {
			DEV_ERR(dev, "Unable to create the %s thread", amc_log_thread_name);
			ret = PTR_ERR((*amc_ctrl_ctxt)->logging_thread);
			goto fail;
		} else {
			DEV_VDBG(dev, "Successfully created %s thread", amc_log_thread_name);
			(*amc_ctrl_ctxt)->logging_thread_created = true;
			wake_up_process((*amc_ctrl_ctxt)->logging_thread);
		}
	}

	if (ret)
		goto fail;

	return SUCCESS;

fail:
	if (version_buf)
		vfree(version_buf);

	unset_amc(dev, amc_ctrl_ctxt);
	release_amc_mem(amc_ctrl_ctxt);
	DEV_ERR(dev, "Failed to setup AMC GCQ");

	return ret;
}

/* 
 * Stop the service, close proxy and tidy up PCI
 */
int unset_amc(struct pci_dev *dev, struct amc_control_ctxt **amc_ctrl_ctxt)
{
	int ret = SUCCESS;
	struct pf_dev_struct *pf_dev = NULL;

	if (!dev || !amc_ctrl_ctxt)
		return FAILURE;
	
	pf_dev = dev_get_drvdata(&dev->dev);

	if (!pf_dev)
		return -EINVAL;

	if (*amc_ctrl_ctxt) {

                /* Stop the heartbeat thread if it's been created */
                if ((*amc_ctrl_ctxt)->heartbeat_thread_created == true)
                {
                        ret = kthread_stop((*amc_ctrl_ctxt)->heartbeat_thread);
                        if (ret) {
                                DEV_ERR(dev, "kthread_stop() failed for heartbeat thread: %d", ret);
                        }
                }

                /* Stop the logging thread if it's been created */
                if ((*amc_ctrl_ctxt)->logging_thread_created == true)
                {
                        ret = kthread_stop((*amc_ctrl_ctxt)->logging_thread);
                        if (ret) {
                                DEV_ERR(dev, "kthread_stop() failed for logging thread: %d", ret);
                        }
                }

		/* Stop the Services */
		stop_gcq_services(*amc_ctrl_ctxt);

		/* Close the proxy */
		ret = amc_proxy_close(&((*amc_ctrl_ctxt)->fw_if_cfg));
		if (ret)
			DEV_ERR(dev, "Failed to close the amc proxy %d", ret);

		unmap_pci_io(dev, &((*amc_ctrl_ctxt)->gcq_payload_base_virt_addr));
		unmap_pci_io(dev, &((*amc_ctrl_ctxt)->gcq_base_virt_addr));
	}

	pci_release_region(dev, PCIE_BAR0);
	pf_dev->pcie_config->header->bar[PCIE_BAR0].requested = false;

	return ret;
}

/*
 * Free resources
 */
void release_amc_mem(struct amc_control_ctxt **amc_ctrl_ctxt)
{
	if (amc_ctrl_ctxt)
		release_amc(amc_ctrl_ctxt);
}
