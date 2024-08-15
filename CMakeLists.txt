cmake_minimum_required(VERSION 3.20)
project("NearbyClient-Root")

set(MbedTLS_DIR $ENV{MBEDTLS_INSTALL_DIR}/cmake)

find_package(MbedTLS REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory("NearbyClient")
add_subdirectory("NearbyLayers")
add_subdirectory("NearbyProto")
add_subdirectory("NearbyRenderer")
add_subdirectory("NearbyStorage")
add_subdirectory("Vendors")
