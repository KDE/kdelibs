
project(knotify)


include_directories( ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/config )
include_directories( ${ki18n_SOURCE_DIR}/src ${ki18n_BINARY_DIR}/src )

add_subdirectory( config )
add_subdirectory( tests )

