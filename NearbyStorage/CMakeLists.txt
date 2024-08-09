project("NearbyStorage")

file(GLOB NEARBYSTORAGE_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
)

add_library(${PROJECT_NAME} STATIC ${NEARBYSTORAGE_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
