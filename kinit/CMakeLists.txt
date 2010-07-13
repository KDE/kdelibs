project(kdeinit)

add_subdirectory( tests )

include_directories( ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/libltdl )

########### Configuring (mainly for proctitle) ###########

include (CheckFunctionExists)
include (CheckIncludeFiles)
include (CheckVariableExists)
check_variable_exists(__progname HAVE___PROGNAME)
check_variable_exists(__progname_full HAVE___PROGNAME_FULL)
check_include_files(sys/pstat.h HAVE_SYS_PSTAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_function_exists(pstat HAVE_PSTAT)
check_function_exists(setproctitle HAVE_SETPROCTITLE)


# used by 4 executables in this file
if(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY socket)
else(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY)
endif(HAVE_SOCKET_LIBRARY)


########### kdeinit4 ###############
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

if (Q_WS_MAC OR WIN32)
  # this has to be GUI on OSX because it launches GUI apps and need a quartz context
  # and on Windows this makes kdeinit4 create separate console window
  kde4_add_executable(kdeinit4 ${kdeinit_SRCS})
  set_target_properties(kdeinit4 PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
  set_target_properties(kdeinit4 PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.kdeinit4")
  set_target_properties(kdeinit4 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Init")
else (Q_WS_MAC OR WIN32)
  kde4_add_executable(kdeinit4 NOGUI ${kdeinit_SRCS})
endif (Q_WS_MAC OR WIN32)

target_link_libraries(kdeinit4 ${KDE4_KDEUI_LIBS} ${kdeinit_LIBS} ${KINIT_SOCKET_LIBRARY} )
if (Q_WS_X11)
  target_link_libraries(kdeinit4 ${X11_X11_LIB})
endif(Q_WS_X11)

install(TARGETS kdeinit4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kwrapper4 ###############
if (WIN32)
  set(kwrapper_SRCS kwrapper_win.cpp  )
else (WIN32)
  set(kwrapper_SRCS kwrapper.c  )
endif (WIN32)

kde4_add_executable(kwrapper4 NOGUI ${kwrapper_SRCS})

if (WIN32)
  target_link_libraries(kwrapper4 ${KDE4_KDECORE_LIBS} ${KINIT_SOCKET_LIBRARY})
else (WIN32)
  target_link_libraries(kwrapper4 ${KINIT_SOCKET_LIBRARY})
endif (WIN32)

install(TARGETS kwrapper4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

if (NOT WIN32)

  ########### kdeinit4_wrapper ###############

  kde4_add_executable(kdeinit4_wrapper NOGUI wrapper.c)

  target_link_libraries(kdeinit4_wrapper  ${KINIT_SOCKET_LIBRARY})

  if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
    add_definitions(-DNO_DISPLAY)
  endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

  install(TARGETS kdeinit4_wrapper ${INSTALL_TARGETS_DEFAULT_ARGS} )

  ########### kshell4 ###############

  kde4_add_executable(kshell4 NOGUI shell.c)

  target_link_libraries(kshell4 ${KINIT_SOCKET_LIBRARY})

  install(TARGETS kshell4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

  ########### kdeinit4_shutdown ###############

  kde4_add_executable(kdeinit4_shutdown NOGUI wrapper.c)

  target_link_libraries(kdeinit4_shutdown  ${KINIT_SOCKET_LIBRARY})

  install(TARGETS kdeinit4_shutdown ${INSTALL_TARGETS_DEFAULT_ARGS} )

  ########### lnusertemp ###############

  set(lnusertemp_SRCS lnusertemp.c )

  kde4_add_executable(lnusertemp NOGUI ${lnusertemp_SRCS})

  target_link_libraries(lnusertemp kdefakes)

  install(TARGETS lnusertemp DESTINATION ${LIBEXEC_INSTALL_DIR} )

endif (NOT WIN32)


########### kioslave ###############

set(kioslave_SRCS kioslave.cpp)


kde4_add_executable(kioslave NOGUI ${kioslave_SRCS})

target_link_libraries(kioslave  ${KDE4_KIO_LIBS} )

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
qt4_generate_dbus_interface(klauncher_adaptor.h org.kde.KLauncher.xml)


kde4_add_kdeinit_executable( klauncher NOGUI ${klauncher_KDEINIT_SRCS})
set_target_properties(kdeinit_klauncher PROPERTIES DEFINE_SYMBOL MAKE_KLAUNCHER)

target_link_libraries(kdeinit_klauncher  ${KDE4_KIO_LIBS} ${X11_LIBRARIES})

install(TARGETS kdeinit_klauncher ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( klauncher kdeinit_klauncher )
install(TARGETS klauncher DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### start_kdeinit ###############
if (NOT WIN32)
  set(start_kdeinit_SRCS start_kdeinit.c)
  kde4_add_executable(start_kdeinit ${start_kdeinit_SRCS})
  if(KDE4_ENABLE_FPIE)
      macro_add_compile_flags(start_kdeinit ${KDE4_CXX_FPIE_FLAGS})
      macro_add_link_flags(start_kdeinit ${KDE4_PIE_LDFLAGS})
  endif(KDE4_ENABLE_FPIE)

  set(start_kdeinit_wrapper_SRCS start_kdeinit_wrapper.c)
  kde4_add_executable(start_kdeinit_wrapper ${start_kdeinit_wrapper_SRCS})

  install(TARGETS start_kdeinit DESTINATION ${LIBEXEC_INSTALL_DIR})
  install(TARGETS start_kdeinit_wrapper DESTINATION ${LIBEXEC_INSTALL_DIR})
endif (NOT WIN32)

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    MESSAGE(STATUS "Using setuid root kdeinit wrapper in order to protect it from bad Linux OOM-killer")
    set(KDEINIT_OOM_PROTECT 1)
    install(CODE "
        set(START_KDEINIT_PATH \"\$ENV{DESTDIR}${LIBEXEC_INSTALL_DIR}/start_kdeinit\")
        EXECUTE_PROCESS(COMMAND sh -c \"chown 0 '\${START_KDEINIT_PATH}' && chmod u+s '\${START_KDEINIT_PATH}'\")
    ")
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

########### config-kdeinit.h ############
configure_file(config-kdeinit.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/config-kdeinit.h )
