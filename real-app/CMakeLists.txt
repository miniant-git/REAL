cmake_minimum_required(VERSION 3.12)

project(miniant-real)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

FILE(GLOB_RECURSE REAL_APP_INPUT *.cpp *.h *.rc)
add_executable(real-app WIN32 ${REAL_APP_INPUT})

target_compile_definitions(real-app PUBLIC _UNICODE UNICODE)

set(CMAKE_PREFIX_PATH "deps")

find_package(curl REQUIRED CONFIG)
target_link_libraries(real-app curl)

find_package(expected REQUIRED)
target_link_libraries(real-app expected)

find_package(json REQUIRED)
target_link_libraries(real-app json)

find_package(spdlog REQUIRED)
target_link_libraries(real-app spdlog)

source_group(TREE . FILES ${REAL_APP_INPUT})
