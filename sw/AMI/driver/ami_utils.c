// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_utils.c - This file contains AMI driver utilities.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#include <linux/slab.h>    /* kmalloc */
#include <linux/string.h>  /* memcpy, strlen */

#include "ami.h"
#include "ami_utils.h"


int my_krealloc(void **buf, int old_size, int new_size, gfp_t flags)
{
	void *tmp = NULL;
	int tmp_size = 0;

	if (!buf || (new_size <= 0) || (old_size < 0))
		return -EINVAL;

	if (!(*buf)) {
		*buf = kmalloc(new_size, flags);
		if (!(*buf))
			return -ENOMEM;
		else
			return SUCCESS;
	}

	if (old_size < new_size)
		tmp_size = old_size;
	else
		tmp_size = new_size;

	tmp = kmalloc(tmp_size, flags);
	if (!tmp)
		return -ENOMEM;

	memcpy(tmp, *buf, tmp_size);

	kfree(*buf);
	*buf = NULL;

	*buf = kmalloc(new_size, flags);
	if (!(*buf))
		return -ENOMEM;

	memcpy(*buf, tmp, tmp_size);

	kfree(tmp);
	tmp = NULL;

	return SUCCESS;
}

int strconcat(char **dst, char src[], int *size)
{
	int ret = SUCCESS;

	if (!size || !dst || ((*size) < 0))
		return -EINVAL;

	ret = my_krealloc((void **)dst, *size, *size + strlen(src), GFP_KERNEL);
	if (ret)
		return ret;

	if (!(*dst))
		return -ENOMEM;

	memcpy(*dst + *size - 1, src, strlen(src) + 1);
	*size += strlen(src);
	return SUCCESS;
}
