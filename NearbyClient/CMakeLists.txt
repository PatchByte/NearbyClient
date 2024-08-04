project("NearbyClient")

file(GLOB NEARBYCLIENT_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*/*/*.cpp
)

add_executable(${PROJECT_NAME} ${NEARBYCLIENT_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE NearbyRenderer::NearbyRenderer NearbyLayers::NearbyLayers)
