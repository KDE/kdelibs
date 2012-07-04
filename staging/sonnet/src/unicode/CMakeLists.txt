project( parseucd )

set( SOURCE_FILES
    parseucd/parseucd.cpp
)

find_package( Qt5Transitional REQUIRED )

include_directories( ${CMAKE_BINARY_DIR}
                     ${X11_INCLUDE_DIR}
                   )

add_executable( parseucd ${SOURCE_FILES} )

target_link_libraries( parseucd
                       ${QT_QTCORE_LIBRARIES}
                       ${QT_QTGUI_LIBRARIES}
                       ${QT_QTMAIN_LIBRARY}
                       ${X11_LIBRARIES}
                     )
