project(inqt5)

set(libinqt5_SRCS
    qstandardpaths.cpp
    qstandardpaths_unix.cpp
)

# TODO windows: qstandardpaths_win.cpp
# TODO mac: qstandardpaths_mac.cpp

add_library(inqt5 ${LIBRARY_TYPE} ${libinqt5_SRCS})

generate_export_header(inqt5)

target_link_libraries(inqt5 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(inqt5 PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                       SOVERSION ${ECM_SOVERSION}
)

install(TARGETS inqt5 ${ECM_TARGET_DEFAULT_ARGS})

#install(FILES
#    qstandardpaths.h
#    ${CMAKE_CURRENT_BINARY_DIR}/inqt5_export.h
#    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
#)
