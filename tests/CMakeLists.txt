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

add_fmi_ls_bus_test(test_endian test_endian.cpp)
add_fmi_ls_bus_test(test_codec test_codec.cpp)
add_fmi_ls_bus_test(test_hex_codec test_hex_codec.cpp)
