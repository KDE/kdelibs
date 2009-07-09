
if(APPLE)
	add_subdirectory(mac)
elseif(UNIX)
	add_subdirectory(policykit)
endif()

add_subdirectory(dbus)

