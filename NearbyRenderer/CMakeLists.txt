project("NearbyRenderer")

find_package(Vulkan REQUIRED)

file(GLOB NEARBYRENDERER_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.*
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*/*.*
)

add_library(${PROJECT_NAME} STATIC ${NEARBYRENDERER_SOURCE_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC Ash::Ash glfw Vulkan::Vulkan ImGui::ImGui)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Because almost everything that microsoft produces is garbage that is being accepted by society because everyone uses it
if(MSVC OR MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})
endif(MSVC OR MINGW)
