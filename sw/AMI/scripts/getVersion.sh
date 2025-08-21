#!/bin/bash

##
# Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# E.g. To get the version info of a module named "ami":
#           ./getVersion ami
#
# This must be run from the top level of the repo for ami
# & from /src/driver/gcq-driver for gcq
#
# This cats the output file "<ID>_version.h"
# ami_version.h in the src/common/include directory
# gcq_version.h in the src/device_drivers/gcq_driver/src directory
#
# This file will not be tracked by git

GIT_HASH="$(git rev-parse HEAD)"
GIT_DATE="$(git log -1 --pretty=format:"%cd" --date=format:"%Y%m%d")"

if [ "$#" -ge 1 ]; then
    MODULE_NAME=$1
else
    MODULE_NAME=${PWD##*/}
fi
IN_DIR="${2:-.}"

AMI_SRC_VERSION_FILE="$IN_DIR/include/ami_version.h"
AMI_DST_VERSION_FILE="$IN_DIR/build/ami_version.h"
DRIVER_SRC_VERSION_FILE="$IN_DIR/ami_driver_version.h.in"
DRIVER_DST_VERSION_FILE="$IN_DIR/ami_driver_version.h"

AMI_STR="ami"
GCQ_VERSION_FILE="./src/gcq_version.h"
GCQ_STR="gcq"
DRIVER_STR="driver"

################################################################################
###                                 Functions                                ###
################################################################################

################################################################################
#
# This function creates the version.h file
#
################################################################################
function catVersionHeaderFile {

    if [ "$MODULE_NAME" = "$AMI_STR" ]; then
        cp $AMI_SRC_VERSION_FILE  $AMI_DST_VERSION_FILE
        sed -i -E "s/#define GIT_HASH.*/#define GIT_HASH                  \"${GIT_HASH}\"/" $AMI_DST_VERSION_FILE
        sed -i -E "s/#define GIT_DATE.*/#define GIT_DATE                  \"${GIT_DATE}\"/" $AMI_DST_VERSION_FILE
    elif [ "$MODULE_NAME" = "$GCQ_STR" ]; then
        cat $GCQ_VERSION_FILE
    elif [ "$MODULE_NAME" = "$DRIVER_STR" ]; then
        cp $DRIVER_SRC_VERSION_FILE  $DRIVER_DST_VERSION_FILE
        sed -i -E "s/#define GIT_HASH.*/#define GIT_HASH                  \"${GIT_HASH}\"/" $DRIVER_DST_VERSION_FILE
        sed -i -E "s/#define GIT_DATE.*/#define GIT_DATE                  \"${GIT_DATE}\"/" $DRIVER_DST_VERSION_FILE
    else
        echo "No version file found..."
    fi
}
################################################################################

function getVersionMain {
    echo -e "\r\nRunning $0"
    mkdir -p $IN_DIR/build
    catVersionHeaderFile
    echo -e "Done\r\n"
}
################################################################################

################################################################################
###                           Script Starting Point                          ###
################################################################################

getVersionMain

exit 0
