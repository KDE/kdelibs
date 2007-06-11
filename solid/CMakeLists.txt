include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${QT_INCLUDE_DIR} )
KDE4_NO_ENABLE_FINAL(solid)
add_subdirectory( solid )
add_subdirectory( tests )
