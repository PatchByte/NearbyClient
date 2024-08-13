project("NearbyStorage")

# find_package(OpenSSL REQUIRED)
find_package(MbedTLS REQUIRED)

file(GLOB NEARBYSTORAGE_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
)

add_library(${PROJECT_NAME} STATIC ${NEARBYSTORAGE_SOURCE_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${MBEDTLS_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${MBEDTLS_LIBRARIES})
