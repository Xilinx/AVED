#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

MODULE_NAME=$1
MODULE_VERSION_STRING=$2

OS_RELEASE=$(cat /etc/os-release)

if [[ "$OS_RELEASE" == *"Ubuntu"* ]]; then
    if dpkg -s xrt &> /dev/null; then
        echo "Error: found XRT installation. Exiting..."
        exit 1
    fi
else
    if rpm -qa | grep xrt &> /dev/null; then
        echo "Error: found XRT installation. Exiting..."
        exit 1
    fi
fi

exit 0
