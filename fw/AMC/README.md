Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT

# AVED Management Control (AMC)

Refer to https://pages.gitenterprise.xilinx.com/XBB/AMC/ for the Architecture and API descriptions.

## Overview

The AVED Management Controller (AMC) provides management and control of the AVED RPU. Its basic features include, but are not limited to:

- In-Band Telemetry
- Built in Monitoring 
- Host (AMI) communication
- Sensor Control
- QSFP Control
- Clock Control
- Download and Programming to Flash

In addition, the AMC is fully abstracted from:

- The OS (Operating System Abstraction Layer (OSAL))
- The Firmware Driver (Firmware Interface Abstraction Layer (FAL))

Event driven architecture is provided by the Event Library (EVL).


---

## Building

### Set-up

Copy source (ssh recommended):
```
git clone --recurse-submodules git@gitenterprise.xilinx.com:XBB/AMC.git
```
SSH key passphrase will be required to clone each submodule.

cd into top-level of the cloned repo

Enable Xilinx software command-line tools:
```
module switch xilinx/ta/<selected_version>
```

### Build

To build from clean:
```
$ ./scripts/build.sh -os <target os (freertos10_xilinx, standalone etc.)> -profile <target profile (v70, v80 etc.)> -xsa <path to xsa>
```
-xsa parameter is not required for Linux builds, or if building with the "-amc" option.

This will generate a BSP (on non-Linux builds) using vitis build tools to get all the libraries required by AMC, then build using CMake with the CMakeLists file on the top level.


To see the full list of options:
```
$ ./scripts/build.sh -help
```

A minimum CMake version of 3.3.0 is required therefore you must ensure that this is installed and used in the place where you are building AMC i.e. server, VM or your local machine.

Compiled binary and auto-generated version information will be available:
```
build/
├── amc.elf
├── amc_version.json
```

### Compile AMC separately

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Clean

```
$ rm -rf build/
```

---

## Unit testing

Compile Unit Tests:
```
$ cd build
$ cmake .. -DTEST_ENABLE=true
$ make
```
