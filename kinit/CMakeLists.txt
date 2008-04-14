project(kdeinit)

add_subdirectory( tests )

include_directories( ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/libltdl )

########### Configuring (mainly for proctitle) ###########

INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckVariableExists)
CHECK_VARIABLE_EXISTS(__progname HAVE___PROGNAME)
CHECK_VARIABLE_EXISTS(__progname_full HAVE___PROGNAME_FULL)
CHECK_INCLUDE_FILES(sys/pstat.h HAVE_SYS_PSTAT_H)
CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_FUNCTION_EXISTS(pstat HAVE_PSTAT)
CHECK_FUNCTION_EXISTS(setproctitle HAVE_SETPROCTITLE)
configure_file(config-kdeinit.h.cmake
	       ${CMAKE_CURRENT_BINARY_DIR}/config-kdeinit.h )
########### kdeinit4 ###############
if (WIN32)
  set(kdeinit_SRCS kinit_win.cpp  )
  set(kdeinit_LIBS kparts psapi)
else (WIN32)
  set(kdeinit_SRCS kinit.cpp proctitle.cpp )
  set(kdeinit_LIBS kparts)
endif (WIN32)

if (APPLE)
  # this has to be GUI on OSX because it launches GUI apps and need a quartz context
  kde4_add_executable(kdeinit4 ${kdeinit_SRCS})
else (APPLE)
  kde4_add_executable(kdeinit4 NOGUI ${kdeinit_SRCS})
endif (APPLE)

target_link_libraries(kdeinit4 ${KDE4_KDECORE_LIBS} ${kdeinit_LIBS} )

install(TARGETS kdeinit4 DESTINATION ${BIN_INSTALL_DIR} )

########### kwrapper4 ###############
if (WIN32)
  set(kwrapper_SRCS kwrapper_win.cpp  )
else (WIN32)
  set(kwrapper_SRCS kwrapper.c  )
endif (WIN32)

kde4_add_executable(kwrapper4 NOGUI ${kwrapper_SRCS})

target_link_libraries(kwrapper4 ${KDE4_KDECORE_LIBS} )

IF(HAVE_SOCKET_LIBRARY)
  target_link_libraries(kwrapper4 socket)
ENDIF(HAVE_SOCKET_LIBRARY)

install(TARGETS kwrapper4 DESTINATION ${BIN_INSTALL_DIR} )

if (NOT WIN32)
########### kdeinit4_wrapper ###############

kde4_add_executable(kdeinit4_wrapper NOGUI wrapper.c)

target_link_libraries(kdeinit4_wrapper  ${KDE4_KDECORE_LIBS} )

IF(HAVE_SOCKET_LIBRARY)
  target_link_libraries(kdeinit4_wrapper socket)
ENDIF(HAVE_SOCKET_LIBRARY)

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

install(TARGETS kdeinit4_wrapper DESTINATION ${BIN_INSTALL_DIR} )

########### kshell4 ###############

kde4_add_executable(kshell4 NOGUI shell.c)

target_link_libraries(kshell4 ${KDE4_KDECORE_LIBS} )

IF(HAVE_SOCKET_LIBRARY)
  target_link_libraries(kshell4 socket)
ENDIF(HAVE_SOCKET_LIBRARY)

install(TARGETS kshell4 DESTINATION ${BIN_INSTALL_DIR} )

########### kdeinit4_shutdown ###############

kde4_add_executable(kdeinit4_shutdown NOGUI wrapper.c)

target_link_libraries(kdeinit4_shutdown  ${KDE4_KDECORE_LIBS} )

IF(HAVE_SOCKET_LIBRARY)
  target_link_libraries(kdeinit4_shutdown socket)
ENDIF(HAVE_SOCKET_LIBRARY)

install(TARGETS kdeinit4_shutdown DESTINATION ${BIN_INSTALL_DIR} )

########### lnusertemp ###############

set(lnusertemp_SRCS lnusertemp.c )

kde4_add_executable(lnusertemp NOGUI ${lnusertemp_SRCS})

target_link_libraries(lnusertemp  ${KDE4_KDECORE_LIBS} )

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
   )

# The adaptor is manually edited to add QDBusMessage arguments
#qt4_add_dbus_adaptor(klauncher_KDEINIT_SRCS org.kde.KLauncher.xml klauncher.h KLauncher)


kde4_add_kdeinit_executable( klauncher NOGUI ${klauncher_KDEINIT_SRCS})
set_target_properties(kdeinit_klauncher PROPERTIES DEFINE_SYMBOL MAKE_KLAUNCHER)

target_link_libraries(kdeinit_klauncher  ${KDE4_KIO_LIBS} )

install(TARGETS kdeinit_klauncher ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( klauncher kdeinit_klauncher )
install(TARGETS klauncher DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

install( FILES org.kde.KLauncher.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )





