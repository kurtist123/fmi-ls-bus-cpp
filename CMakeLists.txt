############################################################
# @file CMakeLists.txt
# @brief Top-level CMake configuration for fmi-ls-bus-cpp library
#
# Copyright (c) 2026 Kurtis Thrush. All rights reserved.
# For license terms, see the LICENSE file at the root of this project.
############################################################

cmake_minimum_required(VERSION 3.20)
project(fmi_ls_bus_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Library sources
set(FMI_LS_BUS_SOURCES
    src/serializer.cpp
    src/deserializer.cpp
    src/hex_codec.cpp
    src/can_channel.cpp
    src/metadata.cpp
)

# Header files for IDE visibility
set(FMI_LS_BUS_HEADERS
    include/fmi_ls_bus/fmi_ls_bus.hpp
    include/fmi_ls_bus/protocol/constants.hpp
    include/fmi_ls_bus/protocol/types.hpp
    include/fmi_ls_bus/protocol/operations.hpp
    include/fmi_ls_bus/codec/endian.hpp
    include/fmi_ls_bus/codec/serializer.hpp
    include/fmi_ls_bus/codec/deserializer.hpp
    include/fmi_ls_bus/codec/hex_codec.hpp
    include/fmi_ls_bus/channel/frame_queue.hpp
    include/fmi_ls_bus/channel/can_channel.hpp
    include/fmi_ls_bus/adapter/metadata.hpp
    include/fmi_ls_bus/adapter/fmu4cpp_adapter.hpp
)

add_library(fmi_ls_bus_cpp
    ${FMI_LS_BUS_SOURCES}
    ${FMI_LS_BUS_HEADERS}
)

add_library(fmi_ls_bus::fmi_ls_bus ALIAS fmi_ls_bus_cpp)

target_include_directories(fmi_ls_bus_cpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# If fmu4cpp is present in the workspace, add its export/include for adapter integration
foreach(CANDIDATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../fmu4cpp/export/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../fmu4cpp/export/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/fmu4cpp/export/include"
)
    if(EXISTS "${CANDIDATE}")
        target_include_directories(fmi_ls_bus_cpp PUBLIC
            $<BUILD_INTERFACE:${CANDIDATE}>
        )
        break()
    endif()
endforeach()

# Compiler warnings
if(MSVC)
    target_compile_options(fmi_ls_bus_cpp PRIVATE /W4 /WX-)
else()
    target_compile_options(fmi_ls_bus_cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Installation rules
include(GNUInstallDirs)

install(TARGETS fmi_ls_bus_cpp
    EXPORT fmi_ls_bus_cppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/fmi_ls_bus
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Testing configuration
option(BUILD_TESTING "Build the test suite" ON)
option(FMI_LS_BUS_BUILD_TESTS "Build fmi-ls-bus-cpp unit tests" ON)

if(BUILD_TESTING AND FMI_LS_BUS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
