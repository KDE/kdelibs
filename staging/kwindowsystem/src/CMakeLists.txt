cmake_minimum_required(VERSION 2.8.5)

project(kcoreaddons)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core Test)

set(kwindowsystem_SRCS
    kwindoweffects.cpp
    ksystemeventfilter.cpp
    kxerrorhandler.cpp
    kxutils.cpp
    )

set(platformLinkLibraries)
if (Q_WS_X11)
    set(platformLinkLibraries ${X11_LIBRARIES} ${X11_Xfixes_LIB} ${X11_Xrender_LIB})
    set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_x11.cpp
                                         kwindowinfo_x11.cpp
                                         netwm.cpp )
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_mac.cpp
                                         kwindowinfo_mac.cpp)
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_win.cpp
                                         kwindowinfo_win.cpp)
endif (Q_WS_WIN)
if (Q_WS_QWS)
    set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_qws.cpp)
endif (Q_WS_QWS)

add_library(kwindowsystem ${LIBRARY_TYPE} ${kwindowsystem_SRCS})
generate_export_header(kwindowsystem)

target_link_libraries(kwindowsystem ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} ${platformLinkLibraries})

set_target_properties(kwindowsystem PROPERTIES VERSION  ${GENERIC_LIB_VERSION}
                                    SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kwindowsystem EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   kwindoweffects.h
   kwindowinfo.h
   kwindowsystem.h
   netwm_def.h
   netwm.h

   ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


