#!/bin/bash
################################################################################
#                                                                              #
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.        #
# SPDX-License-Identifier: GPL-2.0-only                                        #
#                                                                              #
################################################################################

################################################################################
#                                                                              #
# E.g. To get the version info of a module named "ami":                        #
#           ./getVersion ami                                                   #
#                                                                              #
# This must be run from the top level of the repo                              #
#                                                                              #
# This will create a file called "ami_version.h" in the api/include/ directory #
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

TEMP_FILE="$0.tmp"
VERSION_H_FILE="api/include/${MODULE_NAME}_version.h"
DRIVER_H_FILE="driver/${MODULE_NAME}_driver_version.h"
VERSION_J_FILE="${MODULE_NAME}_version.json"

COMMIT_HASH="unversioned"
COMMIT_DATE="n/a"
COMMIT_BRANCH="n/a"
COMMIT_STATUS=0
COMMIT_TAG="x.y.z_SRC"

SPDX="SPDX-License-Identifier: GPL-2.0-only"
COPYRIGHT="Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved."

################################################################################
###                                 Functions                                ###
################################################################################

################################################################################
#
# This function ensures the temporary file doesn't exist, and removes it
# if it does
#
################################################################################
function cleanupData {
    ### Delete temporary git output file if it exists ###
    if [ -f $TEMP_FILE ]; then
        rm $TEMP_FILE
    fi
}
################################################################################

################################################################################
#
# This function retrieves the git info and parses it
#
################################################################################
function getGitInfo {

    ### check if anything has been modified locally ###
    if [[ $(git status --porcelain) ]]; then
        COMMIT_STATUS=1
    fi

    ### store git commit in temp file ###
    git log -n 1 --date=format:'%Y%m%d' >> $TEMP_FILE

    ### retrieve the git hash from it ###
    COMMIT_HASH=$(grep -m 1 'commit ' $TEMP_FILE | sed 's/^.* //')

    ### retrieve the git hash date from it ###
    COMMIT_DATE=$(grep -m 1 'Date: ' $TEMP_FILE | sed 's/^.*Date:\s*//')

    ### store git status in temp file ###
    git status >> $TEMP_FILE

    ### retrieve the git branch from it ###
    COMMIT_BRANCH=$(grep -m 1 'On branch ' $TEMP_FILE | sed 's/^.*On branch //')

    ### fetch all branches and tags from the remote
    git fetch --all

    ### store git tag info in temp file ###
    COMMIT_TAG="$(git describe --tag)" >> $TEMP_FILE
    if [ -z "$COMMIT_TAG" ]; then
        ### No Git Tag ###
        COMMIT_TAG="0.0.0"
        COMMIT_TAG_A=0
        COMMIT_TAG_B=0
        COMMIT_TAG_C=0
        COMMIT_TAG_EXTRA=0
    else
        ### Parse Git Tag ###
        COMMIT_TAG_A=`echo "$COMMIT_TAG" | cut -d '.' -f 1`
        COMMIT_TAG_B=`echo "$COMMIT_TAG" | cut -d '.' -f 2`
        COMMIT_TAG_C=`echo "$COMMIT_TAG" | cut -d '.' -f 3`
        COMMIT_TAG_C=`echo "$COMMIT_TAG_C" | cut -d '-' -f 1`
        COMMIT_TAG_C=`echo "$COMMIT_TAG_C" | cut -d '_' -f 1`
        COMMIT_TAG_C=`echo "$COMMIT_TAG_C" | cut -d ' ' -f 1`

        echo "$COMMIT_TAG" | grep '-'
        if [ $? -eq 0 ]; then
            COMMIT_TAG_EXTRA=`echo "$COMMIT_TAG" | cut -d '-' -f 2`
        else
            COMMIT_TAG_EXTRA=0
        fi
    fi
}
################################################################################

################################################################################
#
# This function outputs the git info into the version.h file
#
################################################################################
function outputVariables {
    
    echo -e "#define GIT_TAG                   \"$COMMIT_TAG\"" >> $VERSION_H_FILE
    echo -e "#define GIT_TAG_VER_MAJOR         ($COMMIT_TAG_A)" >> $VERSION_H_FILE
    echo -e "#define GIT_TAG_VER_MINOR         ($COMMIT_TAG_B)" >> $VERSION_H_FILE
    echo -e "#define GIT_TAG_VER_PATCH         ($COMMIT_TAG_C)" >> $VERSION_H_FILE
    echo -e "#define GIT_TAG_VER_DEV_COMMITS   ($COMMIT_TAG_EXTRA)" >> $VERSION_H_FILE
    echo >> $VERSION_H_FILE
    echo -e "#define GIT_HASH                  \"$COMMIT_HASH\"" >> $VERSION_H_FILE
    echo -e "#define GIT_DATE                  \"$COMMIT_DATE\"" >> $VERSION_H_FILE
    echo -e "#define GIT_BRANCH                \"$COMMIT_BRANCH\"" >> $VERSION_H_FILE
    echo -e "#define GIT_STATUS                ($COMMIT_STATUS)" >> $VERSION_H_FILE
}

################################################################################
#
# This function creates the version.h file
#
################################################################################
function createVersionHeaderFile {
    
    GUARD_NAME=${MODULE_NAME^^}

    echo > $VERSION_H_FILE
    echo -e "// ${SPDX}" >> $VERSION_H_FILE
    echo -e "// ${COPYRIGHT}" >> $VERSION_H_FILE    
    echo >> $VERSION_H_FILE
    echo -e "#ifndef _${GUARD_NAME}_VERSION_H_" >> $VERSION_H_FILE
    echo -e "#define _${GUARD_NAME}_VERSION_H_" >> $VERSION_H_FILE
    echo >> $VERSION_H_FILE

    outputVariables

    echo  >> $VERSION_H_FILE
    echo -e "#endif // _${GUARD_NAME}_VERSION_H_" >> $VERSION_H_FILE
    echo  >> $VERSION_H_FILE

    cp $VERSION_H_FILE $DRIVER_H_FILE
    sed -i "s/${GUARD_NAME}/${GUARD_NAME}_DRIVER/g" $DRIVER_H_FILE
}
################################################################################

################################################################################
#
# This function creates the version.json file
#
################################################################################
function createVersionJsonFile {
    
    echo -e "{" > $VERSION_J_FILE

    echo -e "  \"BUILD_BRANCH\":\"$COMMIT_BRANCH\"," >> $VERSION_J_FILE
    echo -e "  \"VERSION_HASH\":\"$COMMIT_HASH\"," >> $VERSION_J_FILE
    echo -e "  \"VERSION_HASH_DATE\":\"$COMMIT_DATE\"," >> $VERSION_J_FILE
    echo -e "  \"AMI_VERSION_RELEASE\":\"$COMMIT_TAG\"," >> $VERSION_J_FILE
    echo -e "  \"AMI_VERSION_MAJOR\":$COMMIT_TAG_A," >> $VERSION_J_FILE
    echo -e "  \"AMI_VERSION_MINOR\":$COMMIT_TAG_B," >> $VERSION_J_FILE
    echo -e "  \"AMI_VERSION_PATCH\":$COMMIT_TAG_C," >> $VERSION_J_FILE
    echo -e "  \"AMI_DEV_COMMITS\":$COMMIT_TAG_EXTRA," >> $VERSION_J_FILE
    echo -e "  \"AMI_DEV_STATUS\":$COMMIT_STATUS" >> $VERSION_J_FILE

    echo -e "}" >> $VERSION_J_FILE
}
################################################################################

################################################################################
#
# This function calls each step in order
#
################################################################################
function getVersionMain {
    echo -e "\r\nRunning $0"
    cleanupData
    getGitInfo
    echo -e "Creating $VERSION_H_FILE"
    createVersionHeaderFile
    createVersionJsonFile
    cleanupData
    echo -e "Done\r\n"
}
################################################################################

################################################################################
###                           Script Starting Point                          ###
################################################################################
getVersionMain
