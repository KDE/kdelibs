kde4_header()

project(knotify)

include_directories( ${KDE4_KIO_INCLUDES} )

add_subdirectory( daemon )
add_subdirectory( config )

kde4_footer()
