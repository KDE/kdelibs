

configure_file(config-kwindowsystem.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h )

set(kwindowsystem_SRCS
    kkeyserver.cpp
    kstartupinfo.cpp
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
endif ()

add_library(KWindowSystem ${kwindowsystem_SRCS})
generate_export_header(KWindowSystem)

target_link_libraries(KWindowSystem
  LINK_PUBLIC Qt5::Core
  LINK_PRIVATE Qt5::Widgets ${platformLinkLibraries}
)

if(IS_ABSOLUTE "${INCLUDE_INSTALL_DIR}")
  target_include_directories(KWindowSystem INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>" )
else()
  target_include_directories(KWindowSystem INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}>" )
endif()

set_target_properties(KWindowSystem PROPERTIES VERSION   ${KWINDOWSYSTEM_VERSION_STRING}
                                               SOVERSION ${KWINDOWSYSTEM_SOVERSION})

install(TARGETS KWindowSystem EXPORT KWindowSystemTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   fixx11h.h
   kkeyserver.h
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

if (APPLE)
   install( FILES kkeyserver_mac.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
elseif (WIN32)
    install( FILES kkeyserver_win.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
else(WIN32)
   install( FILES kkeyserver_x11.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif (APPLE)
