cmake_minimum_required(VERSION 3.12)

add_library(expected INTERFACE)
target_include_directories(expected INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
