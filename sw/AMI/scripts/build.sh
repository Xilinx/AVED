#!/bin/bash

# Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: GPL-2.0-only

set -e

################################################################################
###                                 Variables                                ###
################################################################################

# Command line options
DRIVER_ONLY=0
APP_ONLY=0
NO_VER=0
NO_WORKSPACE=0

# Output files
DRIVER_BIN="ami.ko"
APP_BIN="ami_tool"
API_BIN="libami.a"

# This script assumes that it is run from the top level project directory
ROOT_DIR=$(pwd)
DRIVER_DIR=$ROOT_DIR/driver
API_DIR=$ROOT_DIR/api
APP_DIR=$ROOT_DIR/app

SCRIPT_START_TIME=$SECONDS

################################################################################
###                                 Functions                                ###
################################################################################

function clean_driver() {
    SECTION_START=$SECONDS
    echo "=== Removing driver artifacts ==="
    cd $DRIVER_DIR
    make clean
    cd $ROOT_DIR
    echo "*** Driver cleaning took $((SECONDS - $SECTION_START)) S ***"
}

function clean_app() {
    SECTION_START=$SECONDS
    echo "=== Removing API and ami_tool artifacts ==="
    rm -rf $API_DIR/build
    rm -rf $APP_DIR/build
    echo "*** App cleaning took $((SECONDS - $SECTION_START)) S ***"
}

function print_help() {
    echo "=========================================== AMI Build script =========================================="
    echo
    echo "-clean        : remove all build files"
    echo "-driver       : only builds the AMI driver (app and API untouched)"
    echo "-app          : only builds the AMI API and ami_tool binary (driver untouched)"
    echo "-no_ver       : Don't execute the getVersion.sh script (useful for development)"
    echo "-no_workspace : Skip the workspace setup (the workspace setup is only needed during active development)"
    echo
    echo "If no options are specified, the default behaviour is to clean and build everything"
    echo
    echo "E.g.: To build the driver, API, and ami_tool:"
    echo "./scripts/build.sh"
    echo
    echo "E.g.: To build only the driver:"
    echo "./scripts/build.sh -driver"
    echo
    echo "========================================================================================================="
}

function build_driver() {
    echo "=== Executing driver build process ==="
    SECTION_START=$SECONDS
    cd $DRIVER_DIR && make
    cd ..
    # Check if the build succeeded
    if [[ ! -f "$DRIVER_DIR/$DRIVER_BIN" ]]; then
        echo "ERROR: Some files failed to build!"
        return 1
    else
        echo "*** Driver build took $((SECONDS - $SECTION_START)) S ***"
    fi
}

function build_app() {
    echo "=== Executing application build process ==="
    SECTION_START=$SECONDS
    cd $API_DIR && make
    cd $APP_DIR && make
    cd ..
    # Check if the build succeeded
    if [[ (! -f "$API_DIR/build/$API_BIN") || (! -f "$APP_DIR/build/$APP_BIN") ]]; then
        echo "ERROR: Some files failed to build!"
        return 1
    else
        echo "*** App build took $((SECONDS - $SECTION_START)) S ***"
    fi
}

function fetch_ami_version() {
    echo "=== Fetching AMI version ==="
    SECTION_START=$SECONDS
    cd $ROOT_DIR
    ./scripts/getVersion.sh ami
    echo "*** AMI version update took $((SECONDS - $SECTION_START)) S ***"
}

function fetch_gcq_version() {
    echo "=== Fetching GCQ version ==="
    SECTION_START=$SECONDS
    cd $DRIVER_DIR/gcq-driver
    ./getVersion.sh gcq
    cd $ROOT_DIR
    echo "*** GCQ version update took $((SECONDS - $SECTION_START)) S ***"
}

################################################################################

################################################################################
###                           Script Starting Point                          ###
################################################################################

# Check that the necessary directories exist
if [[ (! -d "$DRIVER_DIR") || (! -d "$API_DIR") || (! -d "$APP_DIR") ]]; then
    echo "Some directories are missing! Are you running from the top-level project directory?"
    exit 1
fi

### while-done structure defines a loop ###
### that executes once for each passed option ###
while [ $# -gt 0 ]; do
    case "$1" in
    -help)
        print_help
        exit 0
        ;;
    -clean)
        # Clean build files
        clean_driver
        clean_app
        exit 0
        ;;
    -app)
        # Build application only (and API)
        if [ $DRIVER_ONLY -eq 1 ]; then
            echo "-app and -driver options cannot be used together"
            exit 1
        fi
        APP_ONLY=1
        ;;
    -driver)
        # Build driver only
        if [ $APP_ONLY -eq 1 ]; then
            echo "-app and -driver options cannot be used together"
            exit 1
        fi
        DRIVER_ONLY=1
        ;;
    -no_ver)
        NO_VER=1
        ;;
    -no_workspace)
        NO_WORKSPACE=1
        ;;
    esac
    shift ### shift to next passed option ###
done

### start initial build ###
echo "$(date)"

if [ $NO_WORKSPACE -eq 0 ]; then
    echo "Setting up workspace"
    ./scripts/setupWorkspace.sh
fi

if [ $DRIVER_ONLY -eq 1 ]; then
    echo "Building ami.ko only"
    if [ $NO_VER -eq 0 ]; then
        fetch_ami_version
        fetch_gcq_version
    fi
    clean_driver && build_driver
    echo
    echo "Done, built the following files:"
    echo "  - $DRIVER_DIR/$DRIVER_BIN"
    echo
elif [ $APP_ONLY -eq 1 ]; then
    echo "Building ami_tool only"
    if [ $NO_VER -eq 0 ]; then
        fetch_ami_version
    fi
    clean_app && build_app
    echo
    echo "Done, built the following files:"
    echo "  - $API_DIR/build/$API_BIN"
    echo "  - $APP_DIR/build/$APP_BIN"
    echo
else
    echo "Building ami.ko and ami_tool"
    if [ $NO_VER -eq 0 ]; then
        fetch_ami_version
        fetch_gcq_version
    fi
    clean_driver && build_driver
    clean_app && build_app
    echo
    echo "Done, built the following files:"
    echo "  - $API_DIR/build/$API_BIN"
    echo "  - $APP_DIR/build/$APP_BIN"
    echo "  - $DRIVER_DIR/$DRIVER_BIN"
    echo
fi

echo "*** Complete build time: $((SECONDS - $SCRIPT_START_TIME)) S ***"
