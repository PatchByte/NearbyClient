project("NearbyLayers")

file(GLOB NEARBYLAYERS_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*/*.*
)

add_library(${PROJECT_NAME} STATIC ${NEARBYLAYERS_SOURCE_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC Ash::Ash)

if(LINUX)
    target_link_libraries(${PROJECT_NAME} PUBLIC bluetooth)
endif()
