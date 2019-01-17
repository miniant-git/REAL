cmake_minimum_required(VERSION 3.12)

add_library(json INTERFACE)
target_include_directories(json INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
