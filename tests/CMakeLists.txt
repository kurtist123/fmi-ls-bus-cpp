############################################################
# @file CMakeLists.txt
# @brief CMake configuration and test suite definition for fmi-ls-bus-cpp
#
# Copyright (c) 2026 Kurtis Thrush. All rights reserved.
# For license terms, see the LICENSE file at the root of this project.
############################################################

cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

function(add_fmi_ls_bus_test test_name test_source)
    add_executable(${test_name} ${test_source} ${ARGN})
    target_link_libraries(${test_name} PRIVATE
        fmi_ls_bus_cpp
        GTest::gtest_main
    )
    gtest_discover_tests(${test_name})
endfunction()

add_fmi_ls_bus_test(endian_test endian_test.cpp)
add_fmi_ls_bus_test(codec_test codec_test.cpp)
add_fmi_ls_bus_test(hex_codec_test hex_codec_test.cpp)
add_fmi_ls_bus_test(can_channel_test can_channel_test.cpp)
add_fmi_ls_bus_test(metadata_test metadata_test.cpp)

add_test(
    NAME FmiLsBusGenerateMetadataCMakeTest
    COMMAND ${CMAKE_COMMAND}
        -DFMI_LS_BUS_CMAKE_MODULE=${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FmiLsBusGenerateMetadata.cmake
        -DTEST_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/test_metadata
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test_generate_metadata.cmake
)
