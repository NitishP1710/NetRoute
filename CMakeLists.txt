cmake_minimum_required(VERSION 3.16)
project(NetRoute VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(netroute_core
    src/link.cpp
    src/network.cpp
    src/router.cpp
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

add_executable(netroute
    src/main.cpp
    src/http_server.cpp
)
target_link_libraries(netroute PRIVATE netroute_core)

if (WIN32)
    target_link_libraries(netroute PRIVATE ws2_32)
endif()
