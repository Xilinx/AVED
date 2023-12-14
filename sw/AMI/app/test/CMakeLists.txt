# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

cmake_minimum_required(VERSION 3.5.0)

project(ami-app)

include(CTest)
enable_testing()

option(UNIT_TESTING "" OFF)
option(WITH_EXAMPLES "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka ${CMAKE_BINARY_DIR}/cmocka)

# test_table.c test setup

add_executable(test_table
	test_table.c
	${CMAKE_CURRENT_SOURCE_DIR}/../table.c
)

target_include_directories(test_table PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${CMAKE_CURRENT_SOURCE_DIR}/../cmd_handlers
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../api/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_table
	cmocka
	-Wl,--wrap=print_divider
	-Wl,--wrap=my_fprintf
	-Wl,--wrap=calloc
)

# See https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
target_compile_options(test_table PRIVATE
	-fno-builtin-calloc
)

add_test(NAME test_table
	COMMAND test_table
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# test_printer.c test setup

add_executable(test_printer
	test_printer.c
	${CMAKE_CURRENT_SOURCE_DIR}/../printer.c
)

target_include_directories(test_printer PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${CMAKE_CURRENT_SOURCE_DIR}/../cmd_handlers
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../api/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_printer
	cmocka
	-Wl,--wrap=print_table
	-Wl,--wrap=json_mkobject
	-Wl,--wrap=json_stringify
	-Wl,--wrap=json_delete
	-Wl,--wrap=vfprintf
	-Wl,--wrap=putc
	-Wl,--wrap=printf
	-Wl,--wrap=calloc
	-Wl,--wrap=fprintf
)

target_compile_options(test_printer PRIVATE
	-fno-builtin-calloc
	-fno-builtin-vfprintf
	-fno-builtin-printf
	-fno-builtin-fprintf
)

add_test(NAME test_printer
	COMMAND test_printer
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# test_sensors.c test setup

add_executable(test_sensors
	test_sensors.c
)

target_include_directories(test_sensors PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../
	${CMAKE_CURRENT_SOURCE_DIR}/../cmd_handlers
	${CMAKE_CURRENT_SOURCE_DIR}/../../test
	${CMAKE_CURRENT_SOURCE_DIR}/../../api/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../ext/CMocka/include
)

target_link_libraries(test_sensors
	m
	cmocka
	-Wl,--wrap=ami_sensor_get_temp_value
	-Wl,--wrap=ami_sensor_get_temp_unit_mod
	-Wl,--wrap=ami_sensor_get_temp_uptime_max
	-Wl,--wrap=ami_sensor_get_temp_uptime_average
	-Wl,--wrap=ami_sensor_get_power_value
	-Wl,--wrap=ami_sensor_get_power_unit_mod
	-Wl,--wrap=ami_sensor_get_power_uptime_max
	-Wl,--wrap=ami_sensor_get_power_uptime_average
	-Wl,--wrap=ami_sensor_get_current_value
	-Wl,--wrap=ami_sensor_get_current_unit_mod
	-Wl,--wrap=ami_sensor_get_current_uptime_max
	-Wl,--wrap=ami_sensor_get_current_uptime_average
	-Wl,--wrap=ami_sensor_get_voltage_value
	-Wl,--wrap=ami_sensor_get_voltage_unit_mod
	-Wl,--wrap=ami_sensor_get_voltage_uptime_max
	-Wl,--wrap=ami_sensor_get_voltage_uptime_average
	-Wl,--wrap=ami_sensor_get_type
	-Wl,--wrap=json_mkobject
	-Wl,--wrap=json_mknumber
	-Wl,--wrap=json_append_member
	-Wl,--wrap=json_mknull
	-Wl,--wrap=json_delete
	-Wl,--wrap=ami_dev_find
	-Wl,--wrap=ami_dev_delete
	-Wl,--wrap=ami_dev_get_pci_bdf
	-Wl,--wrap=ami_sensor_discover
	-Wl,--wrap=ami_dev_find_next
	-Wl,--wrap=ami_sensor_get_sensors
	-Wl,--wrap=ami_sensor_get_num_total
	-Wl,--wrap=print_table_data
	-Wl,--wrap=print_json_data
	-Wl,--wrap=print_json_obj
	-Wl,--wrap=gen_json_data
	-Wl,--wrap=find_app_option
	-Wl,--wrap=fclose
	-Wl,--wrap=malloc
	-Wl,--wrap=calloc
	-Wl,--wrap=parse_output_options
)

target_compile_options(test_printer PRIVATE
	-fno-builtin-malloc
	-fno-builtin-calloc
)

add_test(NAME test_sensors
	COMMAND test_sensors
	WORKING_DIRECTORY ${UNIT_TEST_BIN_OUTPUT_DIR}
)

# unit test coverage setup

if (COVERAGE_ENABLE)
	include(${CMAKE_CURRENT_SOURCE_DIR}/../../test/CodeCoverage.cmake)
	append_coverage_compiler_flags()

	set(COVERAGE_EXCLUDES
		test_table.c
		test_printer.c
		test_sensors.c
	)

	SETUP_TARGET_FOR_COVERAGE_LCOV(
		NAME test_coverage
		EXECUTABLE ctest
		DEPENDENCIES
			test_table
			test_printer
			test_sensors
	)
endif()
