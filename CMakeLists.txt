cmake_minimum_required(VERSION 3.16)
project(NetRoute VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NETROUTE_BUILD_TESTS "Build tests" ON)

add_library(netroute_core
    src/link.cpp
    src/network.cpp
    src/routing_engine.cpp
    src/routing_strategy.cpp
)

target_include_directories(netroute_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (MSVC)
    target_compile_options(netroute_core PRIVATE /W4)
else()
    target_compile_options(netroute_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(netroute src/main.cpp)
target_link_libraries(netroute PRIVATE netroute_core)

if (NETROUTE_BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(netroute_tests
        tests/routing_engine_tests.cpp
    )

    target_link_libraries(netroute_tests
        PRIVATE
            netroute_core
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(netroute_tests)
endif()
