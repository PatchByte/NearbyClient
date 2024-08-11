project("NearbyProto")

file(GLOB NEARBYPROTO_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/${PROJECT_NAME}/*.c
)

add_library(${PROJECT_NAME} STATIC ${NEARBYPROTO_SOURCE_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC protobuf-nanopb-static)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/generated)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/generated/${PROJECT_NAME})
