project(kdeinit)

########### Configuring (mainly for proctitle) ###########

include(ConfigureChecks.cmake)

#optional features
find_package(X11)
set(HAVE_X11 ${X11_FOUND})

# used by 4 executables in this file
if(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY socket)
else(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY)
endif(HAVE_SOCKET_LIBRARY)


########### kdeinit5 ###############
if (WIN32)
  set(kdeinit_SRCS kinit_win.cpp klauncher_cmds.cpp )
  if(NOT WINCE)
    set(kdeinit_LIBS psapi)
  else(NOT WINCE)
    set(kdeinit_LIBS Toolhelp.lib)
  endif(NOT WINCE)
else (WIN32)
  set(kdeinit_SRCS kinit.cpp proctitle.cpp klauncher_cmds.cpp )
  set(kdeinit_LIBS "")
endif (WIN32)

# on win32 kdeinit5 has to be a console application
# to be able to catch stderr based --verbose output
add_executable(kdeinit5 ${kdeinit_SRCS})
if (APPLE)
  # this has to be GUI on OSX because it launches GUI apps and need a quartz context
  set_target_properties(kdeinit5 PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
  set_target_properties(kdeinit5 PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.kdeinit5")
  set_target_properties(kdeinit5 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Init")
else (APPLE)
  ecm_mark_nongui_executable(kdeinit5)
endif (APPLE)

target_link_libraries(kdeinit5 ${kdeinit_LIBS} ${KINIT_SOCKET_LIBRARY} ${KCoreAddons_LIBRARIES} KI18n KService KDE4Support KCrash KWindowSystem Qt5::DBus Qt5::Gui Qt5::Widgets)
if (X11_FOUND)
  target_link_libraries(kdeinit5 ${X11_X11_LIB})
endif()

install(TARGETS kdeinit5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kwrapper4 ###############
if (WIN32)
  set(kwrapper_SRCS kwrapper_win.cpp  )
else (WIN32)
  set(kwrapper_SRCS kwrapper.cpp  )
endif (WIN32)

add_executable(kwrapper4 ${kwrapper_SRCS})
ecm_mark_nongui_executable(kwrapper4)

target_link_libraries(kwrapper4 ${KINIT_SOCKET_LIBRARY} Qt5::Core)
install(TARGETS kwrapper4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

if (NOT WIN32)

  ########### kdeinit5_wrapper ###############

  add_executable(kdeinit5_wrapper wrapper.cpp)
  ecm_mark_nongui_executable(kdeinit5_wrapper)

  target_link_libraries(kdeinit5_wrapper  ${KINIT_SOCKET_LIBRARY} Qt5::Core)

  if (NOT X11_FOUND)
    add_definitions(-DNO_DISPLAY)
 endif (NOT X11_FOUND)

  install(TARGETS kdeinit5_wrapper ${INSTALL_TARGETS_DEFAULT_ARGS} )

  ########### kshell4 ###############

  add_executable(kshell4 shell.cpp)
  ecm_mark_nongui_executable(kshell4)

  target_link_libraries(kshell4 ${KINIT_SOCKET_LIBRARY} Qt5::Core)

  install(TARGETS kshell4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

  ########### kdeinit5_shutdown ###############

  add_executable(kdeinit5_shutdown wrapper.cpp)
  ecm_mark_nongui_executable(kdeinit5_shutdown)

  target_link_libraries(kdeinit5_shutdown  ${KINIT_SOCKET_LIBRARY} Qt5::Core)

  install(TARGETS kdeinit5_shutdown ${INSTALL_TARGETS_DEFAULT_ARGS} )

endif (NOT WIN32)


########### kioslave ###############

add_executable(kioslave kioslave.cpp)
ecm_mark_nongui_executable(kioslave)

target_link_libraries(kioslave  ${KDE4_KIO_LIBS})

install(TARGETS kioslave DESTINATION ${LIBEXEC_INSTALL_DIR} )


########### klauncher ###############
set(klauncher_KDEINIT_SRCS
   klauncher.cpp
   klauncher_main.cpp
   klauncher_adaptor.cpp
   autostart.cpp
   klauncher_cmds.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml # just so that it gets generated
   )

# The adaptor is manually edited, generate the xml from it.
qt5_generate_dbus_interface(klauncher_adaptor.h org.kde.KLauncher.xml)


kde4_add_kdeinit_executable( klauncher NOGUI ${klauncher_KDEINIT_SRCS})
set_target_properties(kdeinit_klauncher PROPERTIES DEFINE_SYMBOL MAKE_KLAUNCHER)

target_link_libraries(kdeinit_klauncher  ${KDE4_KIO_LIBS} ${X11_LIBRARIES} ${KCoreAddons_LIBRARIES} KCrash KI18n KWindowSystem Qt5::DBus)

install(TARGETS kdeinit_klauncher ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( klauncher kdeinit_klauncher)
install(TARGETS klauncher DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### start_kdeinit ###############
if (NOT WIN32)
  add_executable(start_kdeinit start_kdeinit.c)
  target_include_directories(start_kdeinit PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")

  add_executable(start_kdeinit_wrapper start_kdeinit_wrapper.c)
  target_include_directories(start_kdeinit_wrapper PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/..")

  install(TARGETS start_kdeinit DESTINATION ${LIBEXEC_INSTALL_DIR})
  install(TARGETS start_kdeinit_wrapper DESTINATION ${LIBEXEC_INSTALL_DIR})
endif (NOT WIN32)

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    MESSAGE(STATUS "Using setuid root kdeinit wrapper in order to protect it from bad Linux OOM-killer")
    set(KDEINIT_OOM_PROTECT 1)
    install(CODE "
      set(START_KDEINIT_PATH \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${LIBEXEC_INSTALL_DIR}/start_kdeinit\")
        EXECUTE_PROCESS(COMMAND sh -c \"chown 0 '\${START_KDEINIT_PATH}' && chmod u+s '\${START_KDEINIT_PATH}'\")
    ")
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

########### config-kdeinit.h ############
configure_file(config-kdeinit.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config-kdeinit.h )
