// SPDX-License-Identifier: GPL-2.0-only
/*
 * amc_proxy.c - This file contains AMC proxy layer implementation.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>


#include "amc_proxy.h"
#include "ami_program.h"  /* Need some #defines from here */


/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

/* Create an artificial assertion via a bad divide by zero assertion. */
#define AP_ASSERT_CONCAT_(a, b)       a##b
#define AP_CONCAT(a, b)               AP_ASSERT_CONCAT_(a, b)
#define AP_STATIC_ASSERT(e,m)         enum { AP_CONCAT(ap_assert_line_, __LINE__) = 1/(int)(!!(e)) }

#define AMC_PROXY_REQUEST_CMD_NEW     (1)

#define AMC_PROXY_MSLEEP_1S	      (1000)


/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 * enum amc_proxy_cmd_opcode - AMC command opcode
 * @AMC_PROXY_CMD_OPCODE_HEARTBEAT: heartbeat request
 * @AMC_PROXY_CMD_OPCODE_PDI_DOWNLOAD: pdi download
 * @AMC_PROXY_CMD_OPCODE_SENSOR: sensor request
 * @AMC_PROXY_CMD_OPCODE_IDENTIFY: identity request
 * @AMC_PROXY_CMD_OPCODE_DEVICE_BOOT: boot partition select
 * @AMC_PROXY_CMD_OPCODE_PARTITION_COPY: partition copy request
 * @AMC_PROXY_CMD_OPCODE_EEPROM_READ_WRITE: eeprom read/write request
 * @AMC_PROXY_CMD_OPCODE_MODULE_READ_WRITE: module read/write request
 */
enum amc_proxy_cmd_opcode {

	AMC_PROXY_CMD_OPCODE_HEARTBEAT = 0x2,
	AMC_PROXY_CMD_OPCODE_EEPROM_READ_WRITE = 0x3,
	AMC_PROXY_CMD_OPCODE_MODULE_READ_WRITE = 0x4,
	AMC_PROXY_CMD_OPCODE_PDI_DOWNLOAD = 0xA,
	AMC_PROXY_CMD_OPCODE_SENSOR = 0xC,
	AMC_PROXY_CMD_OPCODE_IDENTIFY = 0x202,
	AMC_PROXY_CMD_OPCODE_DEVICE_BOOT = 0x00,
	AMC_PROXY_CMD_OPCODE_PARTITION_COPY = 0xD,

	/* Other commands to be added here */
	MAX_AMC_PROXY_CMD_OPCODE
};

/**
 * enum amc_proxy_cmd_flash - sensor repo request types
 * @AMC_PROXY_CMD_FLASH_DEFAULT: default flash
 * @AMC_PROXY_CMD_FLASH_NO_BACKUP: backup
 * @AMC_PROXY_CMD_FLASH_TO_LEGACY: legacy
 */
enum amc_proxy_cmd_flash {
	AMC_PROXY_CMD_FLASH_DEFAULT = 0,
	AMC_PROXY_CMD_FLASH_NO_BACKUP,
	AMC_PROXY_CMD_FLASH_TO_LEGACY,

        MAX_AMC_PROXY_CMD_FLASH
};

/**
 * enum amc_proxy_result - list of return codes from AMC
 * @AMC_PROXY_RESULT_SUCCESS: no errors
 * @AMC_PROXY_RESULT_FAILURE: generic failure
 * @AMC_PROXY_RESULT_INVALID_VALUE: invalid value provided
 * @AMC_PROXY_RESULT_GET_REQUEST_FAILED: AMC failed to properly fetch the GCQ request
 * @AMC_PROXY_RESULT_PROCESS_REQUEST_FAILED: AMC failed to process the GCQ request
 * @AMC_PROXY_RESULT_ALREADY_IN_PROGRESS: command already in progress
 * @AMC_PROXY_RESULT_INVALID_CONFIGURATION: generic error for invalid config (e.g., no device)
 */
enum amc_proxy_result {
        AMC_PROXY_RESULT_SUCCESS = 0,
        AMC_PROXY_RESULT_FAILURE,
        AMC_PROXY_RESULT_INVALID_VALUE,
        AMC_PROXY_RESULT_GET_REQUEST_FAILED,
        AMC_PROXY_RESULT_PROCESS_REQUEST_FAILED,
        AMC_PROXY_RESULT_ALREADY_IN_PROGRESS,
        AMC_PROXY_RESULT_INVALID_CONFIGURATION,

        MAX_AMI_PROXY_RESULT
};


/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

/**
 * struct amc_proxy_cmd_request_hdr: The request (submission) queue entry header format
 *
 * @opcode:     [15-0]      command opcode identifying specific command
 * @count:      [30-16]     number of bytes representing packet payload
 * @state:      [31]        flag indicates this is a new entry
 * @cid:                    unique command id
 * @rsvd:                   reserved for future use
 * @cu_idx:     [11-0]      CU index for certain start CU op codes
 * @cu_domain:  [3-0]       CU domain for certain start CU op codes
 * @header:                 Used to convert between data buffer and header structure
 *
 * Any command in request queue shares same command header.
 * An command ID is used to identify the command. When the command
 * is completed, the same command ID is put into the completion
 * queue entry.
 */
struct amc_proxy_cmd_request_hdr {
        union {
                struct {
                        uint32_t opcode:16; /* [15-0]   */
                        uint32_t count:15;  /* [30-16] */
                        uint32_t state:1;   /* [31] */
                        uint16_t cid;
                        union {
                                uint16_t rsvd;
                                struct {
                                        uint16_t cu_idx:12;
                                        uint16_t cu_domain:4;
                                        };
                                };
                        };
                        uint32_t header[2];
                };
};
AP_STATIC_ASSERT(sizeof(struct amc_proxy_cmd_request_hdr) == 8,\
                "cmd_request_hdr structure no longer is 8 bytes in size");

/**
 * struct amc_proxy_cmd_response_hdr: the response (completion) queue entry header format
 *
 * @cid:        unique command id
 * @cstate:     command state
 * @specific:   flag indicates there is command specific info in result
 * @state:      flag indicates this is a new entry
 * @header:     Used to convert between data buffer and header structure
 * 
 * This is the header of the completion queue entry. A generic command
 * state is put into cstate. The command is identified by cid which
 * matches the cid in submission queue.
 */
struct amc_proxy_cmd_response_hdr {
        union {
                struct {
                        uint16_t cid;
                        uint16_t cstate:14;
                        uint16_t specific:1;
                        uint16_t state:1;
                };
                uint32_t header[1];
        };
};
AP_STATIC_ASSERT(sizeof(struct amc_proxy_cmd_response_hdr) == 4,\
                "cmd_response_hdr structure no longer is 4 bytes in size");

/**
 * struct com_queue_entry: completion queue entry format
 *
 * @hdr: header of the entry
 * @result: command specific result
 * @resvd: reserved
 * @rcode: POSIX error return code
 * @data:  command data
 * 
 * When a command is completed, a completion entry is put into completion
 * queue. A generic command state is put into cstate. The command is
 * identified by cid which matches the cid in submission queue.
 * More command specific result is put into result field. POSIX error code
 * can be put into rcode. This is useful for some case like PS kernel.
 *
 * All completion queue entries have a same fixed size of 4 words.
 */
struct com_queue_entry {
	union {
		struct {
			struct amc_proxy_cmd_response_hdr hdr;
			uint32_t result;
			uint32_t resvd;
			uint32_t rcode;
		};
		uint32_t data[4];
	};
};
AP_STATIC_ASSERT(sizeof(struct com_queue_entry) == AMC_PROXY_RESPONSE_SIZE,\
                "com_queue_entry structure no longer is 16 bytes in size");

/**
 * struct amc_proxy_cmd_req_sensor_payload: sensor_page request command
 *
 * @address: pre-allocated sensor data, device writes sensor data at this address
 * @size: size of pre-allocated sensor data
 * @offset: offset of returned device data
 * @aid: Sensor API ID which decides API in AMC (amc_proxy_cmd_sensor_request)
 * @sid: sensor request id (amc_proxy_cmd_sensor_repo)
 * @addr_type: pre-allocated address type
 * @sensor_id: sensor id values used to get single instantaneous sensor data
 * @resvd: reserved for future use
 * @pad: padding for alignment
 *
 * This payload is used for sensor data report.
 */
struct amc_proxy_cmd_req_sensor_payload {
        uint64_t address;
        uint32_t size;
        uint32_t offset;
        uint32_t aid:8;
        uint32_t sid:8;
        uint32_t addr_type:3;
        uint32_t sensor_id:8;
        uint32_t resvd:5;
        uint32_t pad;
};

/**
 * struct amc_proxy_cmd_data_payload: data request payload command
 *
 * @address: data that needs to be transferred
 * @size: data size
 * @remain_size: the remaining size
 * @src_partition: partition to copy from (applicable only to copy operation)
 * @dest_partition: partition to copy to (applicable only to copy operation)
 * @partition_sel: partition number to flash (also used for boot select)
 * @update_fpt: 1 to indicate that the image contains an FPT
 * @partition_resvd: reserved for future use
 * @last_chunk: 1 to indicate that this is the last data chunk
 * @chunk: current chunk (used only for download operation)
 * @chunk_size: chunk size in KB (used for download operation)
 */
struct amc_proxy_cmd_data_payload {
	uint64_t address;
	uint32_t size;
	uint32_t remain_size;
	uint32_t src_partition:4;
	uint32_t dest_partition:4;
	uint32_t partition_sel:4;
	uint32_t update_fpt:1;
	uint32_t partition_resvd:19;
	uint16_t last_chunk:1;
	uint16_t chunk:15;
	uint16_t chunk_size;
};

/**
 * struct amc_proxy_cmd_eeprom_payload: eeprom request payload command
 *
 * @address: data that needs to be transferred
 * @req_type: the request type, read or write
 * @len: the number of bytes to read/write
 * @offset: the offset into the eeprom address space
 * @resvd: reserved for future use
 */
struct amc_proxy_cmd_eeprom_payload {
        uint64_t address;
        uint32_t req_type:1;
        uint32_t len:8;
        uint32_t offset:8;
        uint32_t resvd:15;
};

/**
 * struct amc_proxy_cmd_eeprom_payload: eeprom request payload command
 *
 * @address: address in shared memory of data to be transferred
 * @device_id: the module device ID
 * @page: page number to read/write
 * @offset: offset within page to read/write
 * @len: number of bytes to read/write
 * @req_type: the request type, read or write
 * @resvd: reserved for future use
 */
struct amc_proxy_cmd_module_payload {
        uint64_t address;
        uint8_t  device_id;
        uint8_t  page;
        uint8_t  offset;
        uint8_t  len;
        uint32_t req_type:1;
        uint32_t resvd:31;
};

/**
 * struct amc_proxy_cmd_heartbeat_payload: heartbeat request payload command
 *
 * @request_id: the id 
 */
struct amc_proxy_cmd_heartbeat_payload {
	uint8_t request_id;
};

/**
 * struct amc_proxy_cmd_request: request command, header & payload (if applicable)
 *
 * @hdr: request command header
 * @sensor_payload: the sensor payload
 * @pdi_payload: the pdi download payload (also used for boot select)
 * @heartbeat_payload: the heartbeat request payload
 * @eeprom_payload: the eeprom read/write request payload
 * @module_payload: the module read/write request payload
 */
struct amc_proxy_cmd_request {
	struct amc_proxy_cmd_request_hdr hdr;
	union {
	        struct amc_proxy_cmd_req_sensor_payload sensor_payload;
                struct amc_proxy_cmd_data_payload pdi_payload;
                struct amc_proxy_cmd_heartbeat_payload heartbeat_payload;
                struct amc_proxy_cmd_eeprom_payload eeprom_payload;
                struct amc_proxy_cmd_module_payload module_payload;
	};
};

/**
 * struct amc_proxy_cmd_resp_default_payload: default completion payload
 *
 * @resvd0: reserved
 * @resvd1: reserved
 */
struct amc_proxy_cmd_resp_default_payload {
        uint32_t resvd0;
        uint32_t resvd1;
};

/**
 * struct amc_proxy_cmd_resp_identify_payload: identify completion payload
 *
 * AMC Identify Command
 * @ver_major: AMC major version number
 * @ver_minor: AMC minor version number
 * @ver_patch: AMC patch version number
 * @local_changes: 0 for no change, 1 for changes
 * @dev_commits: AMC number of dev commits
 * @link_ver_major: GCQ major version number
 * @link_ver_minor: GCQ minor version number
 */
struct amc_proxy_cmd_resp_identify_payload {
        uint8_t ver_major;
        uint8_t ver_minor;
        uint8_t ver_patch;
        uint8_t local_changes;
        uint16_t dev_commits;
        uint8_t link_ver_major;
        uint8_t link_ver_minor;
};

/**
 * struct amc_proxy_cmd_resp_sensor_payload: sensor completion payload
 * 
 * @result: result code
 * @resvd: reserved
 */
struct amc_proxy_cmd_resp_sensor_payload {
        uint32_t result;
        uint32_t resvd;
};

/**
 * struct amc_proxy_cmd_resp_data_payload: pdi data payload
 *
 * @count: how many data returned in bytes
 * @resvd: reserved
 */
struct amc_proxy_cmd_resp_data_payload {
	uint32_t count;
	uint32_t resvd;
};

/**
 * struct amc_proxy_cmd_resp_heartbeat_payload: heartbeat response payload
 *
 * @request_id: the request id
 * @resvd: reserved
 */
struct amc_proxy_cmd_resp_heartbeat_payload {
	uint32_t request_id;
        uint32_t resvd;
};

/**
 * struct amc_proxy_cmd_resp_eeprom_read_write_payload: eeprom read/write completion payload
 *
 * @result: result code
 * @resvd: reserved
 */
struct amc_proxy_cmd_resp_eeprom_read_write_payload {
        uint32_t result;
        uint32_t resvd;
};

/**
 * struct amc_proxy_cmd_resp_module_read_write_payload: module read/write completion payload
 *
 * @result: result code
 * @resvd: reserved
 */
struct amc_proxy_cmd_resp_module_read_write_payload {
        uint32_t result;
        uint32_t resvd;
};

/**
 * struct amc_proxy_cmd_response: response command, header & payload (if applicable)
 * 
 * @hdr: response command header
 * @default_payload: default completion payload
 * @identity_payload: identify completion payload
 * @sensor_payload: sensor completion payload
 * @pdi_payload: pdi download payload
 * @heartbeat_payload: heartbeat completion payload
 * @ret: response return code
 */
struct amc_proxy_cmd_response {
	struct amc_proxy_cmd_response_hdr hdr;
	union {
                struct amc_proxy_cmd_resp_default_payload default_payload;
	        struct amc_proxy_cmd_resp_identify_payload identity_payload;
                struct amc_proxy_cmd_resp_sensor_payload sensor_payload;
                struct amc_proxy_cmd_resp_data_payload pdi_payload;
                struct amc_proxy_cmd_resp_heartbeat_payload heartbeat_payload;
	};
        uint32_t ret;
};

/**
 * struct amc_proxy_instance - internal proxy instance
 *
 * @fw_if_handle: handle to the FW IF
 * @proxy_id: unique proxy id
 * @event_cb: the event callback to be invoked when event generated
 * @lock: lock to protect access to internal lists
 * @response_thread: thread to poll and handle responses
 * @response_thread_created: flag used to determine if thread has been created
 * @submitted_cmds: internal list of submitted commands
 * @initialised: flag to indicate layer has been init
 */
struct amc_proxy_instance {
        FW_IF_CFG                       *fw_if_handle;
        uint8_t                         proxy_id;
        amc_proxy_event_callback        *event_cb;
        struct mutex                    lock;
        struct task_struct              *response_thread;
        bool                            response_thread_created;
        struct list_head                submitted_cmds;
        bool                            initialised;
};

/**
 * struct amc_proxy_list_entry - internal proxy instance
 *
 * @list: kernel list associated with the instance
 * @inst: the proxy instance
 */
struct amc_proxy_list_entry {
        struct list_head                list;
        struct amc_proxy_instance       inst;
};


/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*Global list of proxy instances */
LIST_HEAD(amc_proxy_list_head);


/*****************************************************************************/
/* Local Functions                                                           */
/*****************************************************************************/

/**
 * amc_result_to_linux_errno() - map an AMC return code to a Linux errno
 * @res: Return code received from AMC over GCQ.
 *
 * Return: The corresponding errno or -EINVAL if not recognised.
 */
static int amc_result_to_linux_errno(enum amc_proxy_result res)
{
	switch (res) {
	case AMC_PROXY_RESULT_SUCCESS:
		return 0;

	case AMC_PROXY_RESULT_FAILURE:
		return -EIO;  /* generic I/O error */

	case AMC_PROXY_RESULT_INVALID_VALUE:
		return -EINVAL;  /* invalid value */

	case AMC_PROXY_RESULT_GET_REQUEST_FAILED:
		return -EPROTO;  /* protocol error */

	case AMC_PROXY_RESULT_PROCESS_REQUEST_FAILED:
		return -EPROTO;  /* protocol error */

	case AMC_PROXY_RESULT_ALREADY_IN_PROGRESS:
		return -EALREADY;  /* operation already in progress */

	case AMC_PROXY_RESULT_INVALID_CONFIGURATION:
		return -ENODATA;  /* generic no data error */

	default:
		break;
	}

	return -EINVAL;
}

/**
 * amc_proxy_cmd_complete() - handle the completion command response
 * @ccmd: the completion command
 * @inst: the proxy instance
 *
 * Find the matching cmd from submitted_cmd list using the cid
 * remove it and then invoke registered dcallback
 *
 */
static void amc_proxy_cmd_complete(struct amc_proxy_instance *inst, struct com_queue_entry *ccmd)
{
	struct amc_proxy_cmd_struct *cmd = NULL;
	struct list_head *pos = NULL, *next = NULL;

	if (!inst || !ccmd) {
		return;
        }

        mutex_lock(&(inst->lock));
	list_for_each_safe(pos, next, &(inst->submitted_cmds)) {

		cmd = list_entry(pos, struct amc_proxy_cmd_struct, cmd_list);
		if (cmd->cmd_cid == ccmd->hdr.cid) {

                        struct amc_proxy_cmd_response *cmd_resp =
                                (struct amc_proxy_cmd_response*)ccmd;

                        /* Make a copy of the response before removing from the list */
                        memcpy(&cmd->cmd_response, &cmd_resp->default_payload,
		                sizeof(cmd_resp->default_payload));
                        
                        cmd->cmd_response_code = cmd_resp->ret;

                        /* Suppress hearbeat message so as not to flood dmesg */
                        if (cmd->cmd_suppress_dbg == false) {
                                PR_DBG(
                                        "cmd=%d cid=0x%X cstate=0x%X specific=0x%X state=0x%X res=0x%X ret=0x%X",
                                        cmd->cmd_cid,
                                        ccmd->hdr.cid,
                                        ccmd->hdr.cstate,
                                        ccmd->hdr.specific,
                                        ccmd->hdr.state,
                                        ccmd->result,
                                        ccmd->rcode
                                );
                        }

			list_del(pos);

                        if (inst->event_cb) {
                                inst->event_cb(inst->proxy_id,
                                               AMC_PROXY_EVENT_RESPONSE_COMPLETE,
                                               cmd);
                        }
                        mutex_unlock(&(inst->lock));
			return;
		}
	}
        PR_ERR("No matching cid %d found, unexpected response", ccmd->hdr.cid);
        mutex_unlock(&(inst->lock));
}

/**
 * amc_proxy_submitted_cmds_empty() - check if the submitted queue is empty
 *
 * @inst: the proxy instance
 *
 * Return: boolean true if empty else false
 */
static bool amc_proxy_submitted_cmds_empty(struct amc_proxy_instance *inst)
{
        if (!inst) {
                /* returning true to not block */
                return true;
        }

	mutex_lock(&(inst->lock));
	if (list_empty(&(inst->submitted_cmds))) {
		mutex_unlock(&(inst->lock));
		return true;
	}
	mutex_unlock(&(inst->lock));
	return false;
}

/**
 * amc_proxy_submitted_cmds_drain() - drain the submitted queue list checking for timeouts
 *
 * @inst: the proxy instance
 *
 */
static void amc_proxy_submitted_cmds_drain(struct amc_proxy_instance *inst)
{
	struct amc_proxy_cmd_struct *cmd = NULL;
	struct list_head *pos = NULL, *next = NULL;

        if (!inst) {
                return;
        }

	mutex_lock(&(inst->lock));
	list_for_each_safe(pos, next, &(inst->submitted_cmds)) {

		cmd = list_entry(pos, struct amc_proxy_cmd_struct, cmd_list);

		/* Find any timed out commands */
		if (time_before(cmd->cmd_timeout_jiffies, jiffies)) {

                        /* Remove from list and invoke callback */
			list_del(pos);

                        cmd->cmd_rcode = -ETIME;
                        PR_CRIT_WARN("cmd id: %d timed out(drain), hot reset is required", cmd->cmd_cid);

                        if (inst->event_cb) {
                                inst->event_cb(inst->proxy_id,
                                               AMC_PROXY_EVENT_RESPONSE_TIMEOUT,
                                               cmd);
                        }
		}
	}
	mutex_unlock(&(inst->lock));
}

/**
 * amc_proxy_remove_submitted_cmd() - remove an already submitted command
 *
 * @inst: the proxy instance
 * @ccmd: the completion command
 *
 */
static void amc_proxy_remove_submitted_cmd(struct amc_proxy_instance *inst,
                                           struct amc_proxy_cmd_struct *ccmd)
{
	struct amc_proxy_cmd_struct *cmd = NULL;
	struct list_head *pos = NULL, *next = NULL;

	if (!inst || !ccmd) {
		return;
        }

	mutex_lock(&(inst->lock));
	list_for_each_safe(pos, next, &(inst->submitted_cmds)) {

		cmd = list_entry(pos, struct amc_proxy_cmd_struct, cmd_list);

		/* Finding the matching command and remove from list, don't invoke callback */
		if (cmd == ccmd) {
			list_del(pos);
                        cmd->cmd_rcode = -EIO;
			PR_DBG("cmd id: %d removed", cmd->cmd_cid);
		}
	}
	mutex_unlock(&(inst->lock));
}

/**
 * amc_proxy_submitted_cmd_check_timeout() - check for any timed out requests
 *
 * @inst: the proxy instance
 *
 */
static void amc_proxy_submitted_cmd_check_timeout(struct amc_proxy_instance *inst)
{
	struct amc_proxy_cmd_struct *cmd = NULL;
	struct list_head *pos = NULL, *next = NULL;

        if (!inst) {
		return;
        }

	mutex_lock(&(inst->lock));
	list_for_each_safe(pos, next, &(inst->submitted_cmds)) {
		cmd = list_entry(pos, struct amc_proxy_cmd_struct, cmd_list);

		/* Finding timed out cmds */
                if (time_before(cmd->cmd_timeout_jiffies, jiffies)) {

                        PR_CRIT_WARN("cmd id: %d timed out(timeout), hot reset is required", cmd->cmd_cid);
                        cmd->cmd_rcode = -ETIME;
                        if (inst->event_cb) {
                                inst->event_cb(inst->proxy_id,
                                               AMC_PROXY_EVENT_RESPONSE_TIMEOUT,
                                               cmd);
                        }
		}
	}
	mutex_unlock(&(inst->lock));
}

/**
 * complete_response_thread() - the response thread
 *
 * @data: the data pointer to the proxy instance
 *
 * Thread to check if response queue has new command to consume.
 * If there is one, completed it by reading entry, performing
 * callback and notifying peer
 *
 * Return: the errno return code
 */
static int complete_response_thread(void *data)
{
        struct com_queue_entry ccmd;
        uint32_t ccmd_size = sizeof(struct com_queue_entry);
        struct amc_proxy_instance *amc_proxy_inst = NULL;
        bool response_failed = false;

        if (!data) {
                PR_ERR("Response thread null data arg");
		response_failed = true;
        } else {
                amc_proxy_inst = (struct amc_proxy_instance *)data;
        }

        while(1) {

                if (response_failed == false) {

                        /* Perform the read from the FW_IF */
                        if (amc_proxy_inst->fw_if_handle->read(amc_proxy_inst->fw_if_handle, 0,
                                                        (uint8_t*)&ccmd,
                                                        &ccmd_size, 0) == FW_IF_ERRORS_NONE) {
                                /*
                                * Get the entry from submitted_cmds list,
                                * remove and invoke callback
                                */
                                amc_proxy_cmd_complete(amc_proxy_inst, &ccmd);
                        }

                        /* Check for any commands that might have timed out & notify via callback */
                        amc_proxy_submitted_cmd_check_timeout(amc_proxy_inst);
                }

                /* Check if a stop has been requested */
                if (kthread_should_stop()) {
			break;
                }

                usleep_range(1000, 2000);
        }

        /* Return will be passed to kthread_stop() */
        return 0;
}

/**
 * amc_proxy_find_matching_proxy_instance() - Find matching proxy list entry
 *
 * @fw_if_gcq: GCQ consumer configuration
 *
 * Loop around the global proxy instance list looking for a matching entry
 * based of the FW_IF instance.
 *
 * Return: the proxy instance if found or NULL is not found
 */
static struct amc_proxy_list_entry * amc_proxy_find_matching_proxy_instance(const FW_IF_CFG *fw_if_gcq)
{
       struct amc_proxy_list_entry *amc_proxy = NULL;
       struct list_head *pos = NULL, *next = NULL;

        if (!fw_if_gcq) {
                return(NULL);
        }

        list_for_each_safe(pos, next, &amc_proxy_list_head) {

               amc_proxy = list_entry(pos, struct amc_proxy_list_entry, list);
                /* Find the matching list based on handle */
                if (amc_proxy->inst.fw_if_handle == fw_if_gcq) {
                        return (amc_proxy);
                }
        }

        return (NULL);
}


/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

/*
 * Initialise the AMC proxy layer, creating resources and opening
 * a handle to the FW_IF abstraction
 */
int amc_proxy_init(uint8_t proxy_id, FW_IF_CFG *fw_if_handle)
{
        int ret = 0;

        if (!fw_if_handle) {
               return(-EINVAL);
        }

        ret = fw_if_handle->open(fw_if_handle);
        if (ret == FW_IF_ERRORS_NONE) {

                const char *ap_thread_name = "amc proxy";
                struct amc_proxy_list_entry *amc_proxy_entry = NULL;
                amc_proxy_entry = kzalloc(sizeof(struct amc_proxy_list_entry), GFP_KERNEL);

                if (!amc_proxy_entry) {
                        PR_ERR("Unable to allocate memory for amc context");
                        return(-EINVAL);
                }
                amc_proxy_entry->inst.fw_if_handle = fw_if_handle;
                amc_proxy_entry->inst.proxy_id = proxy_id;
                amc_proxy_entry->inst.response_thread_created = false;
                
                mutex_init(&(amc_proxy_entry->inst.lock));
                INIT_LIST_HEAD(&(amc_proxy_entry->list));
                INIT_LIST_HEAD(&(amc_proxy_entry->inst.submitted_cmds));

                /* Create thread to handle command responses */
                amc_proxy_entry->inst.response_thread = kthread_create(complete_response_thread,
                                                                       &amc_proxy_entry->inst,
                                                                       ap_thread_name);
	        if (IS_ERR(amc_proxy_entry->inst.response_thread)) {
		        PR_ERR("Unable to create the %s thread", ap_thread_name);
		        ret = PTR_ERR(amc_proxy_entry->inst.response_thread);
	        } else {
	                PR_INFO("Successfully created %s kernel thread", ap_thread_name);
                        amc_proxy_entry->inst.response_thread_created = true;
                }

                if (!ret) {

                        /* Add entry onto the global instance list */
                        list_add_tail(&amc_proxy_entry->list, &(amc_proxy_list_head));

                        /* Start response & heartbeat threads */
                        wake_up_process(amc_proxy_entry->inst.response_thread);

                        /* Set flag to indicate all initialisation has completed */
                        amc_proxy_entry->inst.initialised = true;
                }

        } else {
                PR_ERR("FW_IF open request failed: %d", ret);
                ret = -EIO;
        }

        return ret;
}

/*
 * Bind in a callback to be invoked when an event occurs
 */
int amc_proxy_bind_callback(FW_IF_CFG *fw_if_handle, amc_proxy_event_callback *event_cb)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!fw_if_handle || !event_cb) {
               return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(fw_if_handle);
        if (amc_ctxt && amc_ctxt->inst.initialised) {
                amc_ctxt->inst.event_cb = event_cb;
                ret = 0;
        }

        return ret;
}

/*
 * Close the AMC proxy layer, free any resources used and close
 * the FW_IF handle
 */
int amc_proxy_close(const FW_IF_CFG *fw_if_handle)
{
        int ret = -EPERM;
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        struct list_head *pos = NULL, *next = NULL;

        list_for_each_safe(pos, next, &amc_proxy_list_head) {

               amc_ctxt = list_entry(pos, struct amc_proxy_list_entry, list);

                /* Find the matching FW_IF */
                if (amc_ctxt && amc_ctxt->inst.initialised) {

                        if (amc_ctxt->inst.fw_if_handle == fw_if_handle) {

                                /*
                                 * Drain the submitted command queue before killing
                                 * the worker thread
                                 */
                                while (amc_proxy_submitted_cmds_empty(&amc_ctxt->inst) != true) {
                                        msleep(AMC_PROXY_MSLEEP_1S);
                                        amc_proxy_submitted_cmds_drain(&amc_ctxt->inst);
                                }

                                if(amc_ctxt->inst.response_thread_created == true)
                                {
                                        ret = kthread_stop(amc_ctxt->inst.response_thread);
                                        if (!ret) {
                                                ret = amc_ctxt->inst.fw_if_handle->close(amc_ctxt->inst.fw_if_handle);
                                                if(ret) {
                                                        PR_ERR("FW_IF close request failed: %d", ret);
                                                        ret = -EIO;
                                                }
                                        } else {
                                                PR_ERR("kthread_stop() failed for thread: %d", ret);
                                        }
                                }

                                /* Remove from list and free the memory */
                                list_del(&amc_ctxt->list);
                                kfree(amc_ctxt);

                                amc_ctxt->inst.initialised = false;
                                break;
                        }
                }
        }

        return ret;
}

/*
 * Abort an in progress request
 */
int amc_proxy_request_abort(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = 0;

        if (!cmd) {
               return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {
                /* Remove the command from the submitted queue */
                amc_proxy_remove_submitted_cmd(&amc_ctxt->inst, cmd);
        }

        return ret;
}

/*
 * Generate an identity request
 */
int amc_proxy_request_identity(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
               return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_IDENTIFY;
                request_hdr->count = 0; /* No payload for identity request */
                request_hdr->cid = cmd->cmd_cid;
                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                                (uint8_t*)&(request_cmd_entry),
                                                                sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                        ret = 0;
                } else {
                        PR_ERR("FW_IF write request failed: %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Generate a sensor request
 */
int amc_proxy_request_sensor(struct amc_proxy_cmd_struct *cmd,
                             struct amc_proxy_sensor_request *sensor_req)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !sensor_req) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_SENSOR;
                request_hdr->count = sizeof(request_cmd_entry.sensor_payload);
                request_hdr->cid = cmd->cmd_cid;

                request_cmd_entry.sensor_payload.address = sensor_req->address;
                request_cmd_entry.sensor_payload.size = sensor_req->length;
                request_cmd_entry.sensor_payload.aid = sensor_req->req;
                request_cmd_entry.sensor_payload.sid = sensor_req->repo;
                request_cmd_entry.sensor_payload.offset = 0;
                request_cmd_entry.sensor_payload.addr_type = 0;
                request_cmd_entry.sensor_payload.sensor_id = sensor_req->sensor_id;

                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                                (uint8_t*)&(request_cmd_entry),
                                                                sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                } else {
                        PR_ERR("FW_IF write request failed; %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Generate a PDI download request
 */
int amc_proxy_request_pdi_download(struct amc_proxy_cmd_struct *cmd,
                                   struct amc_proxy_pdi_download_request *pdi_download)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !pdi_download) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_PDI_DOWNLOAD;
                request_hdr->count = sizeof(request_cmd_entry.pdi_payload);
                request_hdr->cid = cmd->cmd_cid;

                request_cmd_entry.pdi_payload.address = pdi_download->address;
                request_cmd_entry.pdi_payload.size = pdi_download->length;
                request_cmd_entry.pdi_payload.remain_size = 0;
                request_cmd_entry.pdi_payload.last_chunk = pdi_download->last_chunk;
                request_cmd_entry.pdi_payload.chunk = pdi_download->chunk;
                request_cmd_entry.pdi_payload.chunk_size = pdi_download->chunk_size;

                if (pdi_download->partition == FPT_UPDATE_MAGIC) {
                        request_cmd_entry.pdi_payload.update_fpt = 1;
                } else {
                        request_cmd_entry.pdi_payload.update_fpt = 0;
                        request_cmd_entry.pdi_payload.partition_sel = pdi_download->partition;
                }

                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                                (uint8_t*)&(request_cmd_entry),
                                                                sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                } else {
                        PR_ERR("FW_IF write request failed; %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Generate a device boot select request
 */
int amc_proxy_request_device_boot(struct amc_proxy_cmd_struct *cmd,
                                   struct amc_proxy_pdi_download_request *device_boot)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !device_boot)
                return -EINVAL;

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_DEVICE_BOOT;
                request_hdr->count = sizeof(request_cmd_entry.pdi_payload);
                request_hdr->cid = cmd->cmd_cid;
                
                /* Only set the partition */
                request_cmd_entry.pdi_payload.partition_sel = device_boot->partition;

                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                                (uint8_t*)&(request_cmd_entry),
                                                                sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                } else {
                        PR_ERR("FW_IF write request failed; %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Generate a partition copy request
 */
int amc_proxy_request_partition_copy(struct amc_proxy_cmd_struct *cmd,
                                struct amc_proxy_partition_copy_request *partition_copy)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !partition_copy)
                return -EINVAL;

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_PARTITION_COPY;
                request_hdr->count = sizeof(request_cmd_entry.pdi_payload);
                request_hdr->cid = cmd->cmd_cid;

                request_cmd_entry.pdi_payload.src_partition = partition_copy->src;
                request_cmd_entry.pdi_payload.dest_partition = partition_copy->dest;
                request_cmd_entry.pdi_payload.address = partition_copy->address;
                request_cmd_entry.pdi_payload.size = partition_copy->length;

                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                                (uint8_t*)&(request_cmd_entry),
                                                                sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                } else {
                        PR_ERR("FW_IF write request failed; %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Generate heartbeat request
 */
int amc_proxy_request_heartbeat(struct amc_proxy_cmd_struct *cmd,
                                struct amc_proxy_hearbeat_request *heartbeat)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !heartbeat)
                return -EINVAL;

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_HEARTBEAT;
                request_hdr->count = sizeof(request_cmd_entry.heartbeat_payload);
                request_hdr->cid = cmd->cmd_cid;
                
                /* Only set the count used to identify the heartbeat message id */
                request_cmd_entry.heartbeat_payload.request_id = heartbeat->request_id;

                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                         (uint8_t*)&(request_cmd_entry),
                                                         sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                } else {
                        PR_ERR("FW_IF write request failed; %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Generate eeprom read/write request
 */
int amc_proxy_request_eeprom_read_write(struct amc_proxy_cmd_struct *cmd,
                                        struct amc_proxy_eeprom_rw_request *eeprom_rw)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !eeprom_rw)
                return -EINVAL;

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_EEPROM_READ_WRITE;
                request_hdr->count = sizeof(request_cmd_entry.eeprom_payload);
                request_hdr->cid = cmd->cmd_cid;
                request_cmd_entry.eeprom_payload.req_type = eeprom_rw->type;
                request_cmd_entry.eeprom_payload.address = eeprom_rw->address;
                request_cmd_entry.eeprom_payload.len= eeprom_rw->length;
                request_cmd_entry.eeprom_payload.offset = eeprom_rw->offset;
                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                         (uint8_t*)&(request_cmd_entry),
                                                         sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                } else {
                        PR_ERR("FW_IF write request failed; %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Generate a module read/write request
 */
int amc_proxy_request_module_read_write(struct amc_proxy_cmd_struct *cmd,
                                        struct amc_proxy_module_rw_request *module_rw)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !module_rw)
                return -EINVAL;

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_request request_cmd_entry = {{{{0}}}};
                struct amc_proxy_cmd_request_hdr *request_hdr = NULL;
                request_hdr = &(request_cmd_entry.hdr);
                request_hdr->state = AMC_PROXY_REQUEST_CMD_NEW;
                request_hdr->opcode = AMC_PROXY_CMD_OPCODE_MODULE_READ_WRITE;
                request_hdr->count = sizeof(request_cmd_entry.module_payload);
                request_hdr->cid = cmd->cmd_cid;
                request_cmd_entry.module_payload.address = module_rw->address;
                request_cmd_entry.module_payload.device_id = module_rw->device_id;
                request_cmd_entry.module_payload.page = module_rw->page;
                request_cmd_entry.module_payload.offset = module_rw->offset;
                request_cmd_entry.module_payload.len = module_rw->length;
                request_cmd_entry.module_payload.req_type = module_rw->type;
                ret = amc_ctxt->inst.fw_if_handle->write(amc_ctxt->inst.fw_if_handle, 0,
                                                         (uint8_t*)&(request_cmd_entry),
                                                         sizeof(request_cmd_entry), 0);
                if (ret == FW_IF_ERRORS_NONE) {
                        list_add_tail(&(cmd->cmd_list), &(amc_ctxt->inst.submitted_cmds));
                } else {
                        PR_ERR("FW_IF write request failed; %d", ret);
                        ret = -EIO;
                }
        }

        return ret;
}

/*
 * Read back the identity response
 */
int amc_proxy_get_response_identity(struct amc_proxy_cmd_struct *cmd,
                                    struct amc_proxy_identify_response *identity)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd || !identity) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_resp_identify_payload *identity_payload =
                        (struct amc_proxy_cmd_resp_identify_payload *)&cmd->cmd_response;
                
                /* AMC version */
                identity->ver_major = identity_payload->ver_major;
                identity->ver_minor = identity_payload->ver_minor;
                identity->ver_patch = identity_payload->ver_patch;
                identity->local_changes = identity_payload->local_changes;
                identity->dev_commits = identity_payload->dev_commits;

                /* GCQ version */
                identity->link_ver_major = identity_payload->link_ver_major;
                identity->link_ver_minor = identity_payload->link_ver_minor;

                ret = 0;
        }

        return ret;
}

/*
 * Read back the sensor response
 */
int amc_proxy_get_response_sensor(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {

                struct amc_proxy_cmd_resp_sensor_payload *sensor_payload =
                        (struct amc_proxy_cmd_resp_sensor_payload *)&cmd->cmd_response;
                ret = sensor_payload->result;
        }

        return ret;
}

/*
 * Read back the pdi download response
 */
int amc_proxy_get_response_pdi_download(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised) {
                /* Payload is currently unused. */
                // struct amc_proxy_cmd_resp_data_payload *pdi_payload =
                //         (struct amc_proxy_cmd_resp_data_payload *)&cmd->cmd_response;
                /* TODO: the count field in the response is not currently being populated */

                ret = amc_result_to_linux_errno(cmd->cmd_response_code);
        }

        return ret;
}

/*
 * Read back the boot select response
 */
int amc_proxy_get_response_device_boot(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised)
                ret = amc_result_to_linux_errno(cmd->cmd_response_code);

        return ret;
}

/*
 * Read back the partition copy response
 */
int amc_proxy_get_response_partition_copy(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised)
                ret = amc_result_to_linux_errno(cmd->cmd_response_code);

        return ret;
}

/*
 * Read back the heartbeat response
 */
int amc_proxy_get_response_heartbeat(struct amc_proxy_cmd_struct *cmd,
                                     struct amc_proxy_heartbeat_response *heartbeat)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised)
        {
                struct amc_proxy_cmd_resp_heartbeat_payload *heartbeat_payload =
                        (struct amc_proxy_cmd_resp_heartbeat_payload *)&cmd->cmd_response;

                heartbeat->request_id = heartbeat_payload->request_id;
                ret = amc_result_to_linux_errno(cmd->cmd_response_code);
        }

        return ret;
}

/*
 * Read back the eeprom read/write response
 */
int amc_proxy_get_response_eeprom_read_write(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised)
                ret = amc_result_to_linux_errno(cmd->cmd_response_code);

        return ret;
}

/*
 * Read back the module read/write response
 */
int amc_proxy_get_response_module_read_write(struct amc_proxy_cmd_struct *cmd)
{
        struct amc_proxy_list_entry *amc_ctxt = NULL;
        int ret = -EPERM;

        if (!cmd) {
                return(-EINVAL);
        }

        amc_ctxt = amc_proxy_find_matching_proxy_instance(cmd->cmd_fw_if_gcq);
        if (amc_ctxt && amc_ctxt->inst.initialised)
                ret = amc_result_to_linux_errno(cmd->cmd_response_code);
        
        return ret;
}
