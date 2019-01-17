cmake_minimum_required(VERSION 3.12)

configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt.in curl/CMakeLists.txt)
execute_process(
    COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/curl")
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/curl")

set(CURL_BUILD_OUTPUT "${CMAKE_BINARY_DIR}/curl/git/builds/libcurl-vc15-x64-debug-static-ipv6-sspi-winssl")

add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES IMPORTED_LOCATION "${CURL_BUILD_OUTPUT}/lib/libcurl_a_debug.lib")

target_include_directories(curl INTERFACE "${CURL_BUILD_OUTPUT}/include")
target_link_libraries(curl INTERFACE Ws2_32 crypt32 Wldap32 Normaliz)
target_compile_definitions(curl INTERFACE CURL_STATICLIB)
