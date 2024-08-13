set(ASH_ENABLE_LOGGER ON)
set(NEARBYPROTOCOLS_ENABLE_DEVELOPMENT OFF)
set(NEARBYPORTOCOLS_ENABLE_TESTS OFF)
set(NANOPB_EXTENSION_AS_VENDOR ON)
set(NANOPB_EXTENSION_BUILD_TESTS OFF)

# We use curl as client
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF)

add_subdirectory("Ash")
add_subdirectory("cpp-httplib")
add_subdirectory("glfw")
add_subdirectory("NearbyProtocols")
add_subdirectory("nanopb")
add_subdirectory("nanopb-extension")

if(WIN32)
	add_subdirectory("cppwinrt-cmake")
endif()

add_subdirectory("json")

include("ImGui.cmake")
