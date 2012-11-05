find_package(Qt4 REQUIRED QtGui)
include(${QT_USE_FILE})

set(SOURCES
    main.cpp
    mimetypeviewer.cpp
)

qt4_wrap_ui(SOURCES mimetypeviewer.ui)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../include/QtMimeTypes ${CMAKE_CURRENT_SOURCE_DIR}/../../src/mimetypes)

add_executable( mimetypeviewer ${SOURCES} )

target_link_libraries( mimetypeviewer QtMimeTypes ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} )

