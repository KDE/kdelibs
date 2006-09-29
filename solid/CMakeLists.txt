include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
                     ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
                     ${KDE4_KDECORE_INCLUDES} )
add_definitions( ${QT_DEFINITIONS} ${KDE4_DEFINITIONS} )

add_subdirectory( solid )
add_subdirectory( backends )
add_subdirectory( tests )

