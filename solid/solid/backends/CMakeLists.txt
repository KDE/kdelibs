add_subdirectory(fakehw)

if(NOT WIN32 AND NOT APPLE)
add_subdirectory(hal)
endif(NOT WIN32 AND NOT APPLE)

if(WIN32)
add_subdirectory(wmi)
endif(WIN32)
