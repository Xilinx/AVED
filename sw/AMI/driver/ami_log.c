// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_log.c - This file contains functions to read AMC shared memory logs.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/types.h>

#include "ami_log.h"
#include "ami_amc_control.h"


/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

/*
 * Handles and prints incoming AMC logs
 */
void dump_amc_log(struct amc_control_ctxt *amc_ctrl_ctxt)
{
	int i = 0;
	uintptr_t msg_idx_addr = 0;
	uint32_t current_log_idx = 0;

	if (!amc_ctrl_ctxt)
		return;
	
	msg_idx_addr = (uintptr_t)amc_ctrl_ctxt->gcq_payload_base_virt_addr +
		offsetof(struct amc_shared_mem, log_msg.log_msg_index);

	current_log_idx = ioread32((void *)msg_idx_addr);

	/*
	 * When PCI memory is corrupted (e.g. by doing an 'rst -sys' or attempting
	 * a HW manager flash while AMI is loaded), the data read back is 0xFF,
	 * which causes the logging thread to loop forever.
	 */
	if (AMC_LOG_MAX_RECS < current_log_idx) {
		AMI_ERR_ONCE(
			amc_ctrl_ctxt,
			"Logging thread error - invalid PCI data read!"
		);
		return;
	}

	i = amc_ctrl_ctxt->last_printed_msg_index;

	while (i != current_log_idx) {
		struct amc_msg_payload msg = { 0 };

		/* Calculate the address of the log message for the current index */
		uintptr_t log_msg_addr = (uintptr_t)amc_ctrl_ctxt->gcq_payload_base_virt_addr +
			amc_ctrl_ctxt->amc_shared_mem.log_msg.log_msg_buf_off + (i * sizeof(struct amc_msg_payload));

		memcpy_fromio(&msg, (void*)log_msg_addr, sizeof(struct amc_msg_payload));

		if(strchr(msg.buff, '\0') && strlen(msg.buff))
			AMI_VDBG(amc_ctrl_ctxt, "%s", msg.buff);

		i = (i + 1) % AMC_LOG_MAX_RECS;
	}

	amc_ctrl_ctxt->last_printed_msg_index = current_log_idx;

	return;
}
