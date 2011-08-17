project(libinqt5)

include_directories(
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(libinqt5_SRCS
    qstandardpaths.cpp
)

kf5_add_library(inqt5 ${LIBRARY_TYPE} ${libinqt5_SRCS})

target_link_libraries(inqt5 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(inqt5 PROPERTIES VERSION   ${GENERIC_LIB_VERSION}
                                       SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS inqt5 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

#install(FILES
#    qstandardpaths.h
#    ${CMAKE_CURRENT_BINARY_DIR}/inqt5_export.h
#    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
#)
