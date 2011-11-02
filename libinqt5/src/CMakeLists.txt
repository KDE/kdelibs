project(inqt5)

find_package(extra-cmake-modules REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)
# -- Maintained by scripty
ecm_version(5 0 0)
# --
include(ECMQtFramework)

include_directories(
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(libinqt5_SRCS
    qstandardpaths.cpp
    qstandardpaths_unix.cpp
)

# TODO windows: qstandardpaths_win.cpp
# TODO mac: qstandardpaths_mac.cpp

add_library(inqt5 ${LIBRARY_TYPE} ${libinqt5_SRCS})

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
