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

# Discover fmu4cpp if present locally
set(FMU4CPP_INCLUDE_DIRS "")
set(FMU4CPP_TEST_SOURCES "")

foreach(CANDIDATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../fmu4cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../fmu4cpp"
    "${PROJECT_SOURCE_DIR}/../fmu4cpp"
    "${PROJECT_SOURCE_DIR}/fmu4cpp"
)
    if(EXISTS "${CANDIDATE}/export/include/fmu4cpp/fmu_base.hpp")
        set(FMU4CPP_INCLUDE_DIRS
            "${CANDIDATE}/export/include"
            "${CANDIDATE}/export/src"
            "${CANDIDATE}/export/src/fmu4cpp"
        )
        if(EXISTS "${CANDIDATE}/export/src/fmu4cpp/fmu_base.cpp")
            set(FMU4CPP_TEST_SOURCES
                "${CANDIDATE}/export/src/fmu4cpp/fmu_base.cpp"
                "${CANDIDATE}/export/src/fmu4cpp/fmu_variable.cpp"
            )
        endif()
        break()
    endif()
endforeach()

function(add_fmi_ls_bus_test test_name test_source)
    add_executable(${test_name} ${test_source} ${ARGN})
    target_link_libraries(${test_name} PRIVATE
        fmi_ls_bus_cpp
        GTest::gtest_main
    )
    if(FMU4CPP_INCLUDE_DIRS)
        target_include_directories(${test_name} PRIVATE ${FMU4CPP_INCLUDE_DIRS})
    endif()
    gtest_discover_tests(${test_name})
endfunction()

add_fmi_ls_bus_test(test_endian test_endian.cpp)
add_fmi_ls_bus_test(test_codec test_codec.cpp)
add_fmi_ls_bus_test(test_hex_codec test_hex_codec.cpp)
add_fmi_ls_bus_test(test_can_channel test_can_channel.cpp)
add_fmi_ls_bus_test(test_metadata test_metadata.cpp)
add_fmi_ls_bus_test(test_fmu4cpp_adapter test_fmu4cpp_adapter.cpp ${FMU4CPP_TEST_SOURCES})
