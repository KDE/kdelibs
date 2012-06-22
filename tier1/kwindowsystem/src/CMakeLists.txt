
find_package(X11)

if (X11_FOUND)
   set (HAVE_X11 1)
else ()
   set (HAVE_X11 0)
endif ()

if (X11_Xfixes_FOUND)
   set (HAVE_XFIXES 1)
else ()
   set (HAVE_XFIXES 0)
endif ()

if (X11_Xrender_FOUND)
   set (HAVE_XRENDER 1)
else ()
   set (HAVE_XRENDER 0)
endif ()

configure_file(config-kwindowsystem.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h )

set(kwindowsystem_SRCS
    kwindoweffects.cpp
    ksystemeventfilter.cpp
    kxerrorhandler.cpp
    kxutils.cpp
    )

set(platformLinkLibraries)

if (HAVE_X11)
   set(platformLinkLibraries ${X11_LIBRARIES} ${X11_Xfixes_LIB} ${X11_Xrender_LIB} ${kdeqt5staging_LIBRARY})
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_x11.cpp
                                                kwindowinfo_x11.cpp
                                                netwm.cpp )
endif ()

if (Q_OS_MAC)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_mac.cpp
                                                kwindowinfo_mac.cpp)
endif ()

if (Q_OS_WIN)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_win.cpp
                                                kwindowinfo_win.cpp)
endif ()

if (QWS)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_qws.cpp)
endif ()

add_library(kwindowsystem ${kwindowsystem_SRCS})
generate_export_header(kwindowsystem)

target_link_libraries(kwindowsystem ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDBUS_LIBRARY} ${platformLinkLibraries})

set_target_properties(kwindowsystem PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                               SOVERSION ${ECM_SOVERSION})

install(TARGETS kwindowsystem EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install( FILES
   kwindoweffects.h
   kwindowinfo.h
   kwindowsystem.h
   netwm_def.h
   netwm.h

   # FIXME: It seems odd to install this.
   ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h
   ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
