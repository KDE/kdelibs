
project(knotify)


include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/config )

add_subdirectory( config )
add_subdirectory( tests )

