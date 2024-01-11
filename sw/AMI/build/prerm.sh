#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

MODULE_NAME=$1
MODULE_VERSION_STRING=$2

if lsmod | grep -wq ${MODULE_NAME}; then
    echo "Unloading old ${MODULE_NAME} driver"
    rmmod ${MODULE_NAME}
else
    echo "Module ${MODULE_NAME} not loaded. Skipping 'rmmod'."
fi

echo "Unregistering ${MODULE_NAME} Linux kernel module sources ${MODULE_VERSION_STRING} from dkms"
dkms remove -m ${MODULE_NAME} -v ${MODULE_VERSION_STRING} --all

if [ $? -ne 0 ]; then
    echo "ERROR: dkms remove failed. Removal of ${MODULE_NAME} failed"
    exit 1
fi

find /lib/modules -type f -name ${MODULE_NAME}.ko -delete
find /lib/modules -type f -name ${MODULE_NAME}.ko.kz -delete
find /lib/modules -type f -name ${MODULE_NAME}.ko.xz -delete
depmod -A

exit 0
