cmake_minimum_required(VERSION 3.20)
project(fmi_ls_bus_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Header files
set(FMI_LS_BUS_HEADERS
    include/fmi_ls_bus/protocol/constants.hpp
    include/fmi_ls_bus/protocol/types.hpp
    include/fmi_ls_bus/protocol/operations.hpp
    include/fmi_ls_bus/codec/endian.hpp
)

add_library(fmi_ls_bus_cpp INTERFACE)
add_library(fmi_ls_bus::fmi_ls_bus ALIAS fmi_ls_bus_cpp)

target_include_directories(fmi_ls_bus_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Installation rules
include(GNUInstallDirs)

install(TARGETS fmi_ls_bus_cpp
    EXPORT fmi_ls_bus_cppTargets
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
