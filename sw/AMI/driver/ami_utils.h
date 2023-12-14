// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_utils.h - This file contains AMI driver utility definitions.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_UTILS_H
#define AMI_UTILS_H

#include <linux/cred.h>
#include "ami_top.h"

int my_krealloc(void **buf, int old_size, int new_size, gfp_t flags);
int strconcat(char **dst, char src[], int *size);

#endif  /* AMI_UTILS_H */
