set(ASH_ENABLE_LOGGER ON)
set(NEARBYPROTOCOLS_ENABLE_DEVELOPMENT OFF)
set(NEARBYPORTOCOLS_ENABLE_TESTS OFF)

add_subdirectory("Ash")
add_subdirectory("glfw")
add_subdirectory("NearbyProtocols")

include("ImGui.cmake")
