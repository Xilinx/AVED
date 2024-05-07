#!/bin/bash

################################################################################
#                                                                              #
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.        #
# SPDX-License-Identifier: X11                                                 #
#                                                                              #
################################################################################

################################################################################
#                                                                              #
# E.g. To get the version info of a module named "smbus":                      #
#           ./getVersion smbus                                                 #
#                                                                              #
# This must be run from the top level of the repo for smbus                    #
# & from /src/device_drivers/smbus_driver for smbus                            #
#                                                                              #
# This cats the output file "<ID>_version.h"                                   #
# gcq_version.h in the src/device_drivers/smbus_driver/src directory           #
#                                                                              #
# This file will not be tracked by git                                         #
#                                                                              #
################################################################################

################################################################################
###                                 Variables                                ###
################################################################################

if [ "$#" -eq 1 ]; then
    MODULE_NAME=$1
else
    MODULE_NAME=${PWD##*/}
fi

VERSION_SMBUS_FILE="./src/smbus_version.h"
SMBUS_STR="smbus"

################################################################################
###                                 Functions                                ###
################################################################################

################################################################################
#
# This function creates the version.h file
#
################################################################################
function catVersionHeaderFile {

    if [ "$MODULE_NAME" = "$SMBUS_STR" ]; then
        cat $VERSION_SMBUS_FILE
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