
add_subdirectory( common )

if (NOT WIN32)
    add_subdirectory( kjs )
    add_subdirectory( kconfig_compiler )
endif (NOT WIN32)
