SPDX-License-Identifier: GPL-2.0-only
Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

# AVED Management Interface (AMI)

Refer to https://pages.gitenterprise.xilinx.com/XBB/AMI/ for the Architecture and API descriptions.

# Overview

**NOTE:** Unless stated otherwise, this project uses the Linux kernel coding style.
See:

* https://www.kernel.org/doc/html/v4.10/process/coding-style.html
* https://www.kernel.org/doc/html/next/doc-guide/kernel-doc.html

To get started, you must clone the repo into your working directory.

```
git clone --recurse-submodules https://gitenterprise.xilinx.com/XBB/AMI.git
```

If you've already cloned the repo and want to fetch only the required submodules, run:

```
git submodule update --init --recursive
```

## Building

AMI is built using standard Makefiles for the API and command line app. For the driver, a kernel Makefile is used.
To compile AMI yourself, run the following commands from the top level project directory.

When finished with the build process, you should be left with the following build artifacts:

* Kernel module (ami.ko)
* API static library (libami.a)
* Command line application (ami_tool)

### 1. Driver

```
cd driver
make
```

After running the above commands, you should see a file **ami.ko** in the driver directory. This is the AMI kernel module.

#### Supported Platforms

The following systems have been tested and are confirmed to work as expected with the AMI kernel driver. If a system is not listed
in the below list, you can assume that AMI will not work!

* Ubuntu 20.04

### 2. API

```
./getVersion.sh ami
cd api
make clean && make
```

This creates a build directory and generates a **libami.a** file which you can link your applications against to use the
AMI API. The public header files are located in **api/include**.

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
./gen_package.py
cd output/<timestamp>/
sudo apt install ./ami_xxx.deb
```

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
