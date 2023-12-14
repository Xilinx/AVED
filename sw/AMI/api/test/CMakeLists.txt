# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

cmake_minimum_required(VERSION 3.5.0)

project(ami-api)

include(CTest)
enable_testing()

option(UNIT_TESTING "" OFF)
option(WITH_EXAMPLES "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka ${CMAKE_BINARY_DIR}/cmocka)

# test_ami_device.c test setup

add_executable(test_ami_device
	test_ami_device.c
	${CMAKE_CURRENT_SOURCE_DIR}/../src/ami_device.c
)

target_include_directories(test_ami_device PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_ami_device
	cmocka
	-Wl,--wrap=ioctl
	-Wl,--wrap=open
	-Wl,--wrap=close
	-Wl,--wrap=read
	-Wl,--wrap=write
	-Wl,--wrap=fopen
	-Wl,--wrap=fclose
	-Wl,--wrap=calloc
	-Wl,--wrap=getline
	-Wl,--wrap=ami_set_last_error
	-Wl,--wrap=ami_parse_bdf
	-Wl,--wrap=ami_sensor_discover
	-Wl,--wrap=ami_get_driver_version
	-Wl,--wrap=ami_mem_bar_write
	-Wl,--wrap=readlink
	-Wl,--wrap=basename
	-Wl,--wrap=dirname
	-Wl,--wrap=sleep
	-Wl,--wrap=lseek
)

# See https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
target_compile_options(test_ami_device PRIVATE
	-fno-builtin-calloc
)

add_test(NAME test_ami_device
	COMMAND test_ami_device
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# test_ami_mem_access.c test setup

add_executable(test_ami_mem_access
	test_ami_mem_access.c
	${CMAKE_CURRENT_SOURCE_DIR}/../src/ami_mem_access.c
)

target_include_directories(test_ami_mem_access PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_ami_mem_access
	cmocka
	-Wl,--wrap=ami_set_last_error
	-Wl,--wrap=ami_open_cdev
	-Wl,--wrap=ioctl
)

add_test(NAME test_ami_mem_access
	COMMAND test_ami_mem_access
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# test_ami_program.c test setup

add_executable(test_ami_program
	test_ami_program.c
	${CMAKE_CURRENT_SOURCE_DIR}/../src/ami_program.c
)

target_include_directories(test_ami_program PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_ami_program
	cmocka
	-Wl,--wrap=ami_set_last_error
	-Wl,--wrap=ami_open_cdev
	-Wl,--wrap=ami_dev_pci_reload
	-Wl,--wrap=ami_dev_hot_reset
	-Wl,--wrap=ioctl
	-Wl,--wrap=fopen
	-Wl,--wrap=fseek
	-Wl,--wrap=ftell
	-Wl,--wrap=malloc
	-Wl,--wrap=fread
	-Wl,--wrap=ferror
	-Wl,--wrap=fclose
)

target_compile_options(test_ami_program PRIVATE
	-fno-builtin-malloc
)

add_test(NAME test_ami_program
	COMMAND test_ami_program
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# test_ami_sensor.c test setup

add_executable(test_ami_sensor
	test_ami_sensor.c
	${CMAKE_CURRENT_SOURCE_DIR}/../src/ami_sensor.c
)

target_include_directories(test_ami_sensor PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_ami_sensor
	cmocka
	-Wl,--wrap=ioctl
	-Wl,--wrap=ami_open_cdev
	-Wl,--wrap=ami_set_last_error
	-Wl,--wrap=ami_convert_num
	-Wl,--wrap=glob
	-Wl,--wrap=globfree
	-Wl,--wrap=stat
	-Wl,--wrap=open
	-Wl,--wrap=close
	-Wl,--wrap=read
	-Wl,--wrap=write
	-Wl,--wrap=calloc
	-Wl,--wrap=snprintf
)

target_compile_options(test_ami_sensor PRIVATE
	-fno-builtin-calloc
	-fno-builtin-snprintf
)

add_test(NAME test_ami_sensor
	COMMAND test_ami_sensor
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# test_ami.c test setup

add_executable(test_ami
	test_ami.c
	${CMAKE_CURRENT_SOURCE_DIR}/../src/ami.c
)

target_include_directories(test_ami PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_ami
	cmocka
	-Wl,--wrap=open
	-Wl,--wrap=read
	-Wl,--wrap=close
)

add_test(NAME test_ami
	COMMAND test_ami
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# unit test coverage setup

if (COVERAGE_ENABLE)
	include(${CMAKE_CURRENT_SOURCE_DIR}/../../test/CodeCoverage.cmake)
	append_coverage_compiler_flags()

	set(COVERAGE_EXCLUDES
		test_ami_device.c
		test_ami_mem_access.c
		test_ami_program.c
		test_ami_sensor.c
		test_ami.c
	)

	SETUP_TARGET_FOR_COVERAGE_LCOV(
		NAME test_coverage
		EXECUTABLE ctest
		DEPENDENCIES
			test_ami_device
			test_ami_mem_access
			test_ami_program
			test_ami_sensor
			test_ami
	)
endif()
