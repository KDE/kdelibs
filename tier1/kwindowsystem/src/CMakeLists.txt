

configure_file(config-kwindowsystem.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h )

set(kwindowsystem_SRCS
    kkeyserver.cpp
    kstartupinfo.cpp
    kusertimestamp.cpp
    kxerrorhandler.cpp
    kxutils.cpp
    )

if (HAVE_X11)
  set(kwindowsystem_SRCS ${kwindowsystem_SRCS}
    kmanagerselection.cpp
  )
endif()

set(platformLinkLibraries)

if (HAVE_X11)
   set(platformLinkLibraries Qt5::X11Extras ${X11_LIBRARIES} ${X11_Xfixes_LIB} ${X11_Xrender_LIB} ${XCB_XCB_LIBRARY} ${XCB_KEYSYMS_LIBRARY})
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kkeyserver_x11.cpp
                                                kwindowsystem_x11.cpp
                                                kwindowinfo_x11.cpp
                                                kwindoweffects_x11.cpp
                                                kxmessages.cpp
                                                netwm.cpp )
endif ()

if (APPLE)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kkeyserver_mac.cpp
                                                kwindoweffects_dummy.cpp
                                                kwindowsystem_mac.cpp
                                                kwindowinfo_mac.cpp)
endif ()

if (WIN32)
   set(kwindowsystem_SRCS ${kwindowsystem_SRCS} kkeyserver_win.cpp
                                                kwindoweffects_dummy.cpp
                                                kwindowsystem_win.cpp
                                                kwindowinfo_win.cpp)
   set(platformLinkLibraries
      Qt5::WinExtras # QtWin::fromHICON(), QtWin::toHICON()
      Ws2_32 # gethostname()
   )
endif ()

add_library(KF5WindowSystem ${kwindowsystem_SRCS})
generate_export_header(KF5WindowSystem BASE_NAME KWindowSystem)
add_library(KF5::WindowSystem ALIAS KF5WindowSystem)

target_link_libraries(KF5WindowSystem
  PUBLIC Qt5::Gui
  PRIVATE Qt5::Widgets ${platformLinkLibraries}
)

if(IS_ABSOLUTE "${INCLUDE_INSTALL_DIR}")
  target_include_directories(KF5WindowSystem INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>" )
else()
  target_include_directories(KF5WindowSystem INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}>" )
endif()

set_target_properties(KF5WindowSystem PROPERTIES VERSION   ${KWINDOWSYSTEM_VERSION_STRING}
                                                 SOVERSION ${KWINDOWSYSTEM_SOVERSION}
                                                 EXPORT_NAME WindowSystem
)

install(TARGETS KF5WindowSystem EXPORT KF5WindowSystemTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   fixx11h.h
   kkeyserver.h
   kstartupinfo.h
   kusertimestamp.h
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

if (APPLE)
   install( FILES kkeyserver_mac.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
elseif (WIN32)
    install( FILES kkeyserver_win.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
else()
   install( FILES kkeyserver_x11.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif ()
