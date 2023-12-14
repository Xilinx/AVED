// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_log.h - This file contains functions to read AMC shared memory logs.
 * 
 * Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.
 */
 
#ifndef AMI_LOG_H
#define AMI_LOG_H

#include "ami_amc_control.h"

#define AMC_LOG_ENTRY_SIZE   (96)
#define AMC_LOG_MAX_RECS     (50)

/**
 * struct amc_msg_payload - AMC log message format.
 * @buff:		message buffer.
 */
struct amc_msg_payload {
	char buff[AMC_LOG_ENTRY_SIZE];
};

/**
 * dump_amc_log() - Prints incoming AMC logs.
 * @param amc_ctrl_ctxt Pointer to top level AMC data struct.
 */
void dump_amc_log(struct amc_control_ctxt *amc_ctrl_ctxt);

#endif  /* AMI_LOG_H */
