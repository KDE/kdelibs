PROJECT( parseucd )

SET( SOURCE_FILES
    ./parseucd/parseucd.cpp
)
FIND_PACKAGE( Qt4 REQUIRED )

INCLUDE( ${QT_USE_FILE} )

INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR}
                     ${X11_INCLUDE_DIR}
                   )

ADD_EXECUTABLE( parseucd ${SOURCE_FILES} )

        TARGET_LINK_LIBRARIES( parseucd
                       ${QT_LIBRARIES}
                       ${X11_LIBRARIES}
                     )
