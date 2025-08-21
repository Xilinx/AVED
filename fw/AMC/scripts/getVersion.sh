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

if [ "$#" -eq 1 ]; then
    MODULE_NAME=$1
else
    MODULE_NAME=${PWD##*/}
fi

AMC_SRC_VERSION_FILE="./src/common/include/amc_version.h"
AMC_DST_VERSION_FILE="./build/amc_version.h"
AMC_STR="amc"
VERSION_GCQ_FILE="./src/gcq_version.h"
GCQ_STR="gcq"

################################################################################
###                                 Functions                                ###
################################################################################

################################################################################
#
# This function creates the version.h file
#
################################################################################
function catVersionHeaderFile {

    if [ "$MODULE_NAME" = "$AMC_STR" ]; then
        cp $AMC_SRC_VERSION_FILE  $AMC_DST_VERSION_FILE
        sed -i -E "s/#define GIT_HASH.*/#define GIT_HASH                  \"${GIT_HASH}\"/" $AMC_DST_VERSION_FILE
        sed -i -E "s/#define GIT_DATE.*/#define GIT_DATE                  \"${GIT_DATE}\"/" $AMC_DST_VERSION_FILE
    elif [ "$MODULE_NAME" = "$GCQ_STR" ]; then
        cat $VERSION_GCQ_FILE
    else
        echo "No version file found..."
    fi
}
################################################################################

function getVersionMain {
    echo -e "\r\nRunning $0"
    catVersionHeaderFile
    echo -e "Done\r\n"
}
################################################################################

################################################################################
###                           Script Starting Point                          ###
################################################################################

getVersionMain

exit 0
