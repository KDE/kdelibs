
project(knotify)

add_definitions(${QDBUS_DEFINITIONS})

include_directories( ${KDE4_KIO_INCLUDES} )

add_subdirectory( daemon )
add_subdirectory( config )
add_subdirectory( tests )

