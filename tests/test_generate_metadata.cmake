############################################################
# @file test_generate_metadata.cmake
# @brief Test script for fmi_ls_bus_generate_metadata CMake function
#
# Copyright (c) 2026 Kurtis Thrush. All rights reserved.
# For license terms, see the LICENSE file at the root of this project.
############################################################

include("${FMI_LS_BUS_CMAKE_MODULE}")

fmi_ls_bus_generate_metadata(
    OUTPUT_DIRECTORY "${TEST_OUTPUT_DIR}"
    BUSES CAN1 CAN2 CAN3
    TERMINALS_FOLDER_VAR term_dir
    EXTRA_FOLDER_VAR extra_dir
)

# Verify output paths
if(NOT term_dir STREQUAL "${TEST_OUTPUT_DIR}/terminalsAndIcons")
    message(FATAL_ERROR "term_dir mismatch: ${term_dir}")
endif()

if(NOT extra_dir STREQUAL "${TEST_OUTPUT_DIR}/extra")
    message(FATAL_ERROR "extra_dir mismatch: ${extra_dir}")
endif()

# Verify terminalsAndIcons.xml content
set(term_file "${term_dir}/terminalsAndIcons.xml")
if(NOT EXISTS "${term_file}")
    message(FATAL_ERROR "Missing file: ${term_file}")
endif()

file(READ "${term_file}" term_content)
foreach(bus CAN1 CAN2 CAN3)
    if(NOT term_content MATCHES "<Terminal name=\"${bus}\"")
        message(FATAL_ERROR "Terminal ${bus} missing in ${term_file}")
    endif()
    if(NOT term_content MATCHES "variableName=\"${bus}\\.Tx_Data\"")
        message(FATAL_ERROR "Tx_Data for ${bus} missing in ${term_file}")
    endif()
    if(NOT term_content MATCHES "variableName=\"${bus}\\.BusNotifications\"")
        message(FATAL_ERROR "BusNotifications for ${bus} missing in ${term_file}")
    endif()
endforeach()

# Verify fmi-ls-manifest.xml content
set(manifest_file "${extra_dir}/org.fmi-standard.fmi-ls-bus/fmi-ls-manifest.xml")
if(NOT EXISTS "${manifest_file}")
    message(FATAL_ERROR "Missing file: ${manifest_file}")
endif()

file(READ "${manifest_file}" manifest_content)
if(NOT manifest_content MATCHES "fmi-ls:fmi-ls-name=\"org\\.fmi-standard\\.fmi-ls-bus\"")
    message(FATAL_ERROR "fmi-ls-name mismatch in manifest")
endif()
if(NOT manifest_content MATCHES "isBusSimulationFMU=\"false\"")
    message(FATAL_ERROR "isBusSimulationFMU mismatch in manifest")
endif()

message(STATUS "fmi_ls_bus_generate_metadata test passed successfully.")
