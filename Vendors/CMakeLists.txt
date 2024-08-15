set(ASH_ENABLE_LOGGER ON)
set(NEARBYPROTOCOLS_ENABLE_DEVELOPMENT OFF)
set(NEARBYPORTOCOLS_ENABLE_TESTS OFF)
set(NANOPB_EXTENSION_AS_VENDOR ON)
set(NANOPB_EXTENSION_BUILD_TESTS OFF)

set(USE_TLS ON)
set(USE_MBED_TLS ON)

add_subdirectory("Ash")
add_subdirectory("glfw")
add_subdirectory("IXWebSocket")
add_subdirectory("NearbyProtocols")
add_subdirectory("nanopb")
add_subdirectory("nanopb-extension")

if(WIN32)
	add_subdirectory("cppwinrt-cmake")
endif()

add_subdirectory("json")

include("ImGui.cmake")
