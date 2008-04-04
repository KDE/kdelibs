add_subdirectory(fakehw)

if(!WIN32 AND !APPLE)
add_subdirectory(hal)
endif(!WIN32 AND !APPLE)

if(WIN32)
add_subdirectory(wmi)
endif(WIN32)
