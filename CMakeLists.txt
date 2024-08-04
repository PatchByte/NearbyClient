cmake_minimum_required(VERSION 3.20)
project("NearbyClient-Root")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory("NearbyClient")
add_subdirectory("NearbyRenderer")
add_subdirectory("NearbyLayers")
add_subdirectory("Vendors")
