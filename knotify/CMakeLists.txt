kde4_header()

project(knotify)

include_directories( ${KDE4_KIO_INCLUDES} )

add_subdirectory( daemon )
add_subdirectory( config )
add_subdirectory( tests )

kde4_footer()
