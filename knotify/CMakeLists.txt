kde4_header()

project(knotify)

macro_use_lib_kio()
include_directories( ${QT_INCLUDES} )

add_subdirectory( daemon )
add_subdirectory( config )

kde4_footer()
