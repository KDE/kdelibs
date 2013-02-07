
find_package(X11)
find_package(XCB)

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
    kstartupinfo.cpp
    kwindoweffects.cpp
    kxerrorhandler.cpp
    kxutils.cpp
    )

set(kwindowsystem_SRCS ${kwindowsystem_SRCS}
  ksystemeventfilter.cpp
)
if (HAVE_X11)
  set(kwindowsystem_SRCS ${kwindowsystem_SRCS}
    kmanagerselection.cpp
  )
endif()

set(platformLinkLibraries)

if (HAVE_X11)
   set(platformLinkLibraries ${X11_LIBRARIES} ${X11_Xfixes_LIB} ${X11_Xrender_LIB} ${XCB_XCB_LIBRARIES})
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_x11.cpp
                                                kwindowinfo_x11.cpp
                                                kxmessages.cpp
                                                netwm.cpp )
endif ()

if (APPLE)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_mac.cpp
                                                kwindowinfo_mac.cpp)
endif ()

if (WIN32)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kwindowsystem_win.cpp
                                                kwindowinfo_win.cpp)
endif ()

add_library(kwindowsystem ${kwindowsystem_SRCS})
generate_export_header(kwindowsystem)

target_link_libraries(kwindowsystem Qt5::Widgets ${QT_QTDBUS_LIBRARY} ${kdeqt5staging_LIBRARY} ${platformLinkLibraries} ${qt5CompatLib})

set_target_properties(kwindowsystem PROPERTIES VERSION   ${KWINDOWSYSTEM_VERSION_STRING}
                                               SOVERSION ${KWINDOWSYSTEM_SOVERSION})

install(TARGETS kwindowsystem EXPORT kwindowsystemTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   kstartupinfo.h
   kwindoweffects.h
   kwindowinfo.h
   kwindowsystem.h
   kxmessages.h
   kmanagerselection.h
   netwm_def.h
   netwm.h

   # FIXME: It seems odd to install this.
   ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h
   ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
