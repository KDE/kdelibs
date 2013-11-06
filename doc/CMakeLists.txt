
add_subdirectory( common )

if (NOT WIN32)
    add_subdirectory( makekdewidgets )
    add_subdirectory( sonnet )
    add_subdirectory( kjs )
    add_subdirectory( kjscmd )
    add_subdirectory( kross )
    add_subdirectory( kded5 )
    add_subdirectory( kconfig_compiler )
    add_subdirectory( preparetips )
endif (NOT WIN32)
