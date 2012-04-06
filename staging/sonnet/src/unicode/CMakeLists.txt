PROJECT( parseucd )

SET( SOURCE_FILES
    ./parseucd/parseucd.cpp
)
FIND_PACKAGE( Qt5Transitional REQUIRED )

INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR}
                     ${X11_INCLUDE_DIR}
                   )

ADD_EXECUTABLE( parseucd ${SOURCE_FILES} )

        TARGET_LINK_LIBRARIES( parseucd
                       ${QT_QTCORE_LIBRARIES}
                       ${QT_QTGUI_LIBRARIES}
                       ${X11_LIBRARIES}
                     )
