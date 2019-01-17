cmake_minimum_required(VERSION 3.12)

add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
