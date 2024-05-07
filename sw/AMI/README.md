SPDX-License-Identifier: GPL-2.0-only
Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

# AVED Management Interface (AMI)

Refer to https://pages.gitenterprise.xilinx.com/XBB/AMI/ for the Architecture and API descriptions.

# Overview

**NOTE:** Unless stated otherwise, this project uses the Linux kernel coding style.
See:

* https://www.kernel.org/doc/html/v4.10/process/coding-style.html
* https://www.kernel.org/doc/html/next/doc-guide/kernel-doc.html

## Building

AMI is built using standard Makefiles for the API and command line app. For the driver, a kernel Makefile is used.
To compile AMI yourself, run the following commands from the top level project directory.

When finished with the build process, you should be left with the following build artifacts:

* Kernel module (ami.ko)
* API static library (libami.a)
* Command line application (ami_tool)

### Build script

The easiest way to build AMI is to use the `./scripts/build.sh` script which automates the process.
Try running `./scripts/build.sh -help` for usage and examples. **Note that this script must be run from the top-level
project directory.**

If you wish to build the source manually without using this script, see the build steps
described below (this is roughly the same procedure as that performed by the build script).

### 1. Driver

```
./scripts/getVersion.sh ami
cd driver/gcq-driver
./getVersion.sh gcq
cd ..
make
```

After running the above commands, you should see a file **ami.ko** in the driver directory. This is the AMI kernel module.

### 2. API

```
./scripts/getVersion.sh ami
cd api
make clean && make
```

This creates a build directory and generates a **libami.a** file which you can link your applications against to use the
AMI API. The public header files are located in **api/include**. Note, if you already ran the 'getVersion.sh' script as
part of the driver build steps, you do not need to run it again here.

### 3. CLI App

This step **must** be performed after building the API as the application is linked against the AMI static library.

```
cd app
make clean && make
```

This creates a binary at **build/ami_tool** - this is the AMI command line application.
For usage you can run `./build/ami_tool --help`.

## Running

1. Make sure XRT is disabled.
   1. `sudo rmmod xclmgmt xocl`
2. Follow the above build instructions to compile AMI.
3. Load the AMI kernel module.
   1. `sudo insmod ami.ko`
   2. Verify correct installation by looking at dmesg.
4. Build your application using the AMI API or run the provided command line tool.

## Installation

To build .deb/.rpm packages, use the provided `gen_package.py` script.

```
./scripts/gen_package.py
cd output/<timestamp>/
sudo apt install ./ami_xxx.deb
```

This script may be run from either the top-level project directory or the scripts directory itself. Note, however,
that the output directory (containing the built package) will be created in your current working directory.

The generated packages produce the following artifacts:

 * Driver source code at /usr/src/ami-\<version\>/driver
 * dkms config at /usr/src/ami-\<version\>
 * API header files at /usr/include/ami
 * ami_tool binary at /usr/local/bin
 * libami.a at /usr/local/lib

## Testing

### API

The API is tested using *CMocka* using a series of unit tests. Code coverage is provided by *lcov*.

```
cd api/test
mkdir build
cd build
cmake -DCOVERAGE_ENABLE=true -DCMAKE_BUILD_TYPE=Debug ..
make test_coverage
```

Code coverage can be viewed at `build/test_coverage/index.html`.
