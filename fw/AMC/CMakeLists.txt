# Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.3)

project( amc )

add_definitions( -DRUN_TIME )

#adds test directories if test flag set to true
if( TEST_ENABLE )
    add_subdirectory( ./ext/CMocka )
    add_subdirectory( ./src/test )
    add_subdirectory( ./src/apps/in_band/test )
    add_subdirectory( ./src/osal/src/test/unittest )
    add_subdirectory( ./src/proxy_drivers/acc/test )
    add_subdirectory( ./src/proxy_drivers/apc/test )
    add_subdirectory( ./src/proxy_drivers/axc/test )
    add_subdirectory( ./src/proxy_drivers/ami/test )
    add_subdirectory( ./src/proxy_drivers/asc/test )
    add_subdirectory( ./src/proxy_drivers/rmi_handler/test )
else()
    #if OS not specified, default to FreeRTOS
    if(NOT OS)
        set(OS "FreeRTOS")
    else()
        if(OS MATCHES "^(Linux)$")
            set(OS "Linux")
            set(PROFILE "Linux")
        else()
            set(OS "FreeRTOS")
        endif()
    endif()

    if(OS MATCHES "^(Linux)$")
        set(CMAKE_C_COMPILER gcc)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set(CMAKE_BUILD_TYPE Debug)
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wno-missing-braces")

        find_package(Threads REQUIRED)
        set(OSAL_PATH "src/osal/src/linux/osal.c")
        set(FW_IF_PATH "src/fal/gcq/fw_if_gcq_linux.c")
        set(FW_I2C_PATH "src/device_drivers/i2c/linux/i2c.c")
        set(FW_EMMC_PATH "src/device_drivers/emmc/linux/emmc.c")
        set(FW_SYS_MON_PATH "src/device_drivers/sensors/sys_mon/linux/sys_mon.c")
        set(FW_OSPI_PATH "src/device_drivers/ospi/linux/ospi.c")
        set(FW_IF_OSPI_PATH "src/fal/ospi/fw_if_ospi_stub.c")
        set(FW_IF_MUXED_DEVICE_PATH "src/fal/muxed_device/fw_if_muxed_device_stub.c")
        set(FW_IF_MUXED_DEVICE_DEBUG_PATH "src/fal/muxed_device/debug/fw_if_muxed_device_debug.c")
        set(LINUX_HAL_MEM_BASE "src/profiles/Linux/profile_hal_memory_base.c")
        set(EEPROM_PATH "src/device_drivers/eeprom/linux/eeprom.c")
        set(FW_IF_EMMC_PATH "src/fal/emmc/fw_if_emmc_stub.c")
        set(NAME "amc")
    endif()

    if(OS MATCHES "^(FreeRTOS)$")
        set(CMAKE_C_COMPILER armr5-none-eabi-gcc)
        set(CMAKE_SYSTEM_NAME Generic)
        set(CMAKE_SYSTEM_PROCESSOR arm)
        set(CMAKE_CROSSCOMPILING 1)
        set(CMAKE_C_COMPILER_WORKS TRUE)
        set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -Werror -Wall")
        set(OSAL_PATH "src/osal/src/freeRTOS/osal.c")
        set(FW_IF_PATH "src/fal/gcq/fw_if_gcq_amc.c")
        set(FW_I2C_PATH "src/device_drivers/i2c/aved/i2c.c")
        set(FW_EMMC_PATH "src/device_drivers/emmc/aved/emmc.c")
        set(FW_SYS_MON_PATH "src/device_drivers/sensors/sys_mon/aved/sys_mon.c")
        set(FW_OSPI_PATH "src/device_drivers/ospi/aved/ospi.c")
        set(FW_IF_OSPI_PATH "src/fal/ospi/fw_if_ospi_amc.c")
        set(FW_IF_MUXED_DEVICE_PATH "src/fal/muxed_device/fw_if_muxed_device_amc.c")
        set(FW_IF_MUXED_DEVICE_DEBUG_PATH "src/fal/muxed_device/debug/fw_if_muxed_device_debug.c")
        set(EEPROM_PATH "src/device_drivers/eeprom/aved/eeprom.c")
        set(FW_IF_EMMC_PATH "src/fal/emmc/fw_if_emmc_stub.c")
        set(NAME "amc.elf")

        if( FREERTOS_DEBUG )
            add_definitions(-DFREERTOS_DEBUG)
        endif()

    endif()

    #If build type not specified, set default as "Debug", check if build type set is valid
    if(NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    else()
        if(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
            message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
        endif()
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "^(Debug)$")
        add_definitions(-DDEBUG_BUILD)
    endif()

    #set custom profile with -DPROFILE flag (v80, Linux, etc), Defaults to v80 if not specified
    if(NOT PROFILE)
        include_directories(src/profiles/v80)
        set(PROFILE "v80")
        set(PROFILE_FAL_PATH "src/profiles/v80/profile_fal.c")
    else()
        if(PROFILE MATCHES "^(v80*)$")
            include_directories(src/profiles/v80)
            set(PROFILE "v80")
            set(PROFILE_FAL_PATH "src/profiles/v80/profile_fal.c")
        elseif(PROFILE MATCHES "^(v70)$")
            include_directories(src/profiles/v70)
            set(FW_IF_QSFP_PATH "")
            set(FW_IF_QSFP_DEBUG_PATH "")
            set(PROFILE "v70")
            set(PROFILE_FAL_PATH "src/profiles/v70/profile_fal.c")
        elseif(PROFILE MATCHES "^(Linux)$")
            include_directories(src/profiles/Linux)
            set(PROFILE "Linux")
            set(PROFILE_FAL_PATH "src/profiles/Linux/profile_fal.c")
        else()
            include_directories(src/profiles/v80)
            set(PROFILE "v80")
            set(PROFILE_FAL_PATH "src/profiles/v80/profile_fal.c")
        endif()
    endif()


    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    message("=================================================")
    message("                 BUILD SETTINGS                  ")
    message("=================================================")
    message("OS: ${OS}")
    message("Build Type: ${CMAKE_BUILD_TYPE}")
    message("Profile: ${PROFILE}")
    message("Output Dir: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    #Running prerequisite script
    execute_process(COMMAND ./scripts/getVersion.sh amc WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(COMMAND ./getVersion.sh gcq WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/device_drivers/gcq_driver )

    #adding static libraries
    find_library(FREERTOS_LIB NAMES freertos PATHS amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/lib)
    find_library(XIL_LIB NAMES xil PATHS amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/lib)
    find_library(XIL_FPGA NAMES xilfpga PATHS amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/lib)
    find_library(XIL_MAILBOX NAMES xilmailbox PATHS amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/lib)

    #Include directories
    include_directories(amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/include)
    include_directories(amc_bsp/psv_cortexr5_0/freertos10_xilinx_domain/bsp/psv_cortexr5_0/lib)
    include_directories(src/common/include)
    include_directories(src/common/core_libs/pll)
    include_directories(src/common/core_libs/evl)
    include_directories(src/common/core_libs/dal)
    include_directories(src/osal/src)
    include_directories(src/fal)
    include_directories(src/fal/test)
    include_directories(src/fal/gcq)
    include_directories(src/fal/ospi)
    include_directories(src/fal/muxed_device)
    include_directories(src/fal/emmc)
    include_directories(src/device_drivers/gcq_driver/src)
    include_directories(src/device_drivers/ospi)
    include_directories(src/device_drivers/sensors/ina3221)
    include_directories(src/device_drivers/sensors/isl68221)
    include_directories(src/device_drivers/sensors/cat34ts02)
    include_directories(src/device_drivers/sensors/sys_mon)
    include_directories(src/device_drivers/i2c)
    include_directories(src/device_drivers/emmc)
    include_directories(src/device_drivers/eeprom)
    include_directories(src/proxy_drivers/acc)
    include_directories(src/proxy_drivers/axc)
    include_directories(src/proxy_drivers/apc)
    include_directories(src/proxy_drivers/asc)
    include_directories(src/proxy_drivers/ami)
    include_directories(src/proxy_drivers/rmi_handler)
    include_directories(src/proxy_drivers/rmi_handler/lib)
    include_directories(src/apps/asdm)
    include_directories(src/apps/in_band)
    include_directories(src/apps/out_of_band)
    include_directories(src/apps/bim)

    if(CMAKE_BUILD_TYPE MATCHES "^(Debug)$")
        include_directories(src/osal/debug)
        include_directories(src/common/core_libs/pll/debug)
        include_directories(src/common/core_libs/dal/debug)
        include_directories(src/common/core_libs/evl/debug)
        include_directories(src/device_drivers/sensors/ina3221/debug)
        include_directories(src/device_drivers/sensors/isl68221/debug)
        include_directories(src/device_drivers/sensors/cat34ts02/debug)
        include_directories(src/device_drivers/sensors/sys_mon/debug)
        include_directories(src/device_drivers/eeprom/debug)
        include_directories(src/device_drivers/ospi/debug)
        include_directories(src/device_drivers/i2c/debug)
        include_directories(src/device_drivers/emmc/debug)
        include_directories(src/fal/gcq/debug)
        include_directories(src/fal/ospi/debug)
        include_directories(src/fal/muxed_device/debug)
        include_directories(src/proxy_drivers/acc/debug)
        include_directories(src/proxy_drivers/axc/debug)
        include_directories(src/proxy_drivers/apc/debug)
        include_directories(src/proxy_drivers/asc/debug)
        include_directories(src/proxy_drivers/ami/debug)
        include_directories(src/proxy_drivers/rmi_handler/debug)
        include_directories(src/apps/asdm/debug)
        include_directories(src/apps/in_band/debug)
        include_directories(src/apps/out_of_band/debug)
        include_directories(src/apps/bim/debug)
    endif()

    #Add main app executable
    add_executable(${NAME}
                src/amc.c
                ${OSAL_PATH}
                src/common/core_libs/pll/pll.c
                src/common/core_libs/evl/evl.c
                src/common/core_libs/dal/dal.c
                src/fal/test/fw_if_test_stub.c
                ${FW_IF_PATH}
                ${FW_IF_OSPI_PATH}
                ${FW_IF_MUXED_DEVICE_PATH}
                ${FW_IF_EMMC_PATH}
                src/device_drivers/gcq_driver/src/gcq_driver.c
                src/device_drivers/gcq_driver/src/gcq_features.c
                src/device_drivers/gcq_driver/src/gcq_hw.c
                src/device_drivers/sensors/ina3221/ina3221.c
                src/device_drivers/sensors/isl68221/isl68221.c
                src/device_drivers/sensors/cat34ts02/cat34ts02.c
                ${FW_SYS_MON_PATH}
                ${FW_OSPI_PATH}
                ${FW_I2C_PATH}
                ${FW_EMMC_PATH}
                ${EEPROM_PATH}
                src/proxy_drivers/acc/acc_proxy_driver.c
                src/proxy_drivers/axc/axc_proxy_driver.c
                src/proxy_drivers/apc/apc_proxy_driver.c
                src/proxy_drivers/asc/asc_proxy_driver.c
                src/proxy_drivers/ami/ami_proxy_driver.c
                src/proxy_drivers/rmi_handler/rmi_handler.c
                src/apps/asdm/asdm.c
                src/apps/in_band/in_band_telemetry.c
                src/apps/out_of_band/out_of_band_telemetry.c
                src/apps/bim/bim.c
                ${LINUX_HAL_MEM_BASE}
                ${PROFILE_FAL_PATH}
                )

    if(CMAKE_BUILD_TYPE MATCHES "^(Debug)$")
        target_sources(${NAME}
                PRIVATE src/osal/debug/osal_debug.c
                PRIVATE src/common/core_libs/pll/debug/pll_debug.c
                PRIVATE src/common/core_libs/dal/debug/dal_debug.c
                PRIVATE src/common/core_libs/evl/debug/evl_debug.c
                PRIVATE src/device_drivers/sensors/ina3221/debug/ina3221_debug.c
                PRIVATE src/device_drivers/sensors/isl68221/debug/isl68221_debug.c
                PRIVATE src/device_drivers/sensors/cat34ts02/debug/cat34ts02_debug.c
                PRIVATE src/device_drivers/sensors/sys_mon/debug/sys_mon_debug.c
                PRIVATE src/device_drivers/eeprom/debug/eeprom_debug.c
                PRIVATE src/device_drivers/ospi/debug/ospi_debug.c
                PRIVATE src/device_drivers/i2c/debug/i2c_debug.c
                PRIVATE src/device_drivers/emmc/debug/emmc_debug.c
                PRIVATE src/fal/gcq/debug/fw_if_gcq_debug.c
                PRIVATE src/fal/ospi/debug/fw_if_ospi_debug.c
                PRIVATE ${FW_IF_MUXED_DEVICE_DEBUG_PATH}
                PRIVATE src/proxy_drivers/acc/debug/acc_proxy_driver_debug.c
                PRIVATE src/proxy_drivers/ami/debug/ami_proxy_driver_debug.c
                PRIVATE src/proxy_drivers/apc/debug/apc_proxy_driver_debug.c
                PRIVATE src/proxy_drivers/axc/debug/axc_proxy_driver_debug.c
                PRIVATE src/proxy_drivers/asc/debug/asc_proxy_driver_debug.c
                PRIVATE src/proxy_drivers/rmi_handler/debug/rmi_handler_debug.c
                PRIVATE src/apps/asdm/debug/asdm_debug.c
                PRIVATE src/apps/in_band/debug/in_band_telemetry_debug.c
                PRIVATE src/apps/out_of_band/debug/out_of_band_telemetry_debug.c
                PRIVATE src/apps/bim/debug/bim_debug.c)
    endif()

    #linking libraries with executable
    if(OS MATCHES "^(FreeRTOS)$")
        target_link_libraries(amc.elf "-Wl,-T -Wl,../src/lscript.ld" -Wl,--start-group,${XIL_LIB},${FREERTOS_LIB},-lgcc,-lc,--end-group -Wl,--start-group,${XIL_FPGA},${XIL_LIB},-lgcc,-lc,--end-group -Wl,--start-group,${XIL_MAILBOX},${XIL_LIB},-lgcc,-lc,--end-group)
        target_compile_definitions(amc.elf PRIVATE $<$<CONFIG:Debug>:DEBUG_PRINT> GCQ_MAX_INSTANCES=1)
    endif()

    if(OS MATCHES "^(Linux)$")
    target_link_libraries(amc Threads::Threads rt)
    target_compile_definitions(amc PRIVATE $<$<CONFIG:Debug>:DEBUG_PRINT>)
    endif()
endif()
