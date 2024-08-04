

project ("ImGui")

file(GLOB IMGUI_SOURCE_FILES
    "imgui/*.h"
    "imgui/*.cpp"
    "imgui/backends/imgui_impl_glfw.h"
    "imgui/backends/imgui_impl_glfw.cpp"
    "imgui/backends/imgui_impl_vulkan.h"
    "imgui/backends/imgui_impl_vulkan.cpp"
)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCE_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/
)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

if(MSVC OR MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})
endif(MSVC OR MINGW)
