set(ASH_ENABLE_LOGGER ON)
set(NEARBYPROTOCOLS_ENABLE_DEVELOPMENT OFF)
set(NEARBYPORTOCOLS_ENABLE_TESTS OFF)

add_subdirectory("Ash")
add_subdirectory("cpp-httplib")
add_subdirectory("glfw")
add_subdirectory("NearbyProtocols")

if(WIN32)
	add_subdirectory("cppwinrt-cmake")
endif()

add_subdirectory("json")

include("ImGui.cmake")
