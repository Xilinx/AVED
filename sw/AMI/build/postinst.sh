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

# Dkms status o/p differs with different versions
# So we need different way of parsing old module version string.
dkms_major=`dkms --version | tr -d " "[a-z-:] | awk -F. '{print $1}'`
if [ $dkms_major -ge 3 ]; then
    MODULE_VERSION_STRING_OLD=`dkms status -m ${MODULE_NAME} | awk -F, '{print $1}' | awk -F/ '{print $2}'`
else
    MODULE_VERSION_STRING_OLD=`dkms status -m ${MODULE_NAME} | awk -F, '{print $2}'`
fi

for OLD in $MODULE_VERSION_STRING_OLD; do
    echo "Unregistering old ${MODULE_NAME} Linux kernel module sources $OLD from dkms"
    dkms remove -m ${MODULE_NAME} -v $OLD --all
    break
done

find /lib/modules -type f -name ${MODULE_NAME}.ko -delete
find /lib/modules -type f -name ${MODULE_NAME}.ko.kz -delete
find /lib/modules -type f -name ${MODULE_NAME}.ko.xz -delete
depmod -A

dkms add -m ${MODULE_NAME} -v ${MODULE_VERSION_STRING}

if [ $? -ne 0 ]; then
    echo "ERROR: dkms add failed. Installation of ${MODULE_NAME} failed"
    exit 1
fi

dkms install -m ${MODULE_NAME} -v ${MODULE_VERSION_STRING}

if [ $? -ne 0 ]; then
    echo "ERROR: dkms install failed. Installation of ${MODULE_NAME} failed"
    exit 1
fi

echo "INFO: Loading new ${MODULE_NAME} Linux kernel modules"
modprobe ${MODULE_NAME}

if [ -z "`dkms status -m ${MODULE_NAME} -v ${MODULE_VERSION_STRING} | grep installed`" ]; then
    echo "ERROR: failed to install ${MODULE_NAME} drivers"
    exit 1
fi

echo "Done! If you are not running in bash you may need to refresh your environment before using AMI in the current shell."
echo "If using csh, run 'rehash'. Otherwise, refer to the manual for your shell or start a new session."
