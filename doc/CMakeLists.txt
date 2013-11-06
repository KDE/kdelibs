
add_subdirectory( common )

if (NOT WIN32)
    add_subdirectory( sonnet )
    add_subdirectory( kjs )
    add_subdirectory( kconfig_compiler )
    add_subdirectory( preparetips )
endif (NOT WIN32)
