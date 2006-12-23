
project(kdeinit)

add_subdirectory( tests )


include_directories( ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/libltdl )


########### next target ###############
if (WIN32)
  set(kdeinit_SRCS kinit_win.cpp  )
  set(kdeinit_LIBS kparts psapi)
else (WIN32)
  set(kdeinit_SRCS kinit.cpp setproctitle.cpp )
  set(kdeinit_LIBS kparts)
endif (WIN32)
kde4_automoc(${kdeinit_SRCS})

kde4_add_executable(kdeinit NOGUI ${kdeinit_SRCS})

target_link_libraries(kdeinit  ${KDE4_KDECORE_LIBS} ${kdeinit_LIBS} )

install(TARGETS kdeinit DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############

if (WIN32)
MESSAGE("-- warning: fix targets in kinit")
else (WIN32)
set(kdeinit_wrapper_SRCS wrapper.c )

kde4_add_executable(kdeinit_wrapper NOGUI ${kdeinit_wrapper_SRCS})

target_link_libraries(kdeinit_wrapper  ${KDE4_KDECORE_LIBS} )

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

install(TARGETS kdeinit_wrapper DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############

set(kshell_SRCS shell.c )

kde4_add_executable(kshell NOGUI ${kshell_SRCS})

target_link_libraries(kshell  ${KDE4_KDECORE_LIBS} )

install(TARGETS kshell DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kdeinit_shutdown_SRCS wrapper.c )

kde4_add_executable(kdeinit_shutdown NOGUI ${kdeinit_shutdown_SRCS})

target_link_libraries(kdeinit_shutdown  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdeinit_shutdown DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############

set(kwrapper_SRCS kwrapper.c )

kde4_add_executable(kwrapper NOGUI ${kwrapper_SRCS})

target_link_libraries(kwrapper  ${KDE4_KDECORE_LIBS} )

install(TARGETS kwrapper DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(lnusertemp_SRCS lnusertemp.c )

kde4_add_executable(lnusertemp NOGUI ${lnusertemp_SRCS})

target_link_libraries(lnusertemp  ${KDE4_KDECORE_LIBS} )

install(TARGETS lnusertemp DESTINATION ${BIN_INSTALL_DIR} )
endif (WIN32)


########### next target ###############

set(kioslave_SRCS kioslave.cpp)

kde4_automoc(${kioslave_SRCS})

kde4_add_executable(kioslave NOGUI ${kioslave_SRCS})

target_link_libraries(kioslave  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kioslave DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kstartupconfig_SRCS kstartupconfig.cpp )

kde4_automoc(${kstartupconfig_SRCS})

kde4_add_executable(kstartupconfig NOGUI ${kstartupconfig_SRCS})

if(WIN32)
   target_link_libraries(kstartupconfig  ${KDE4_KDECORE_LIBS})
else(WIN32)
   target_link_libraries(kstartupconfig  ${KDE4_KDECORE_LIBS} kdefakes)
endif(WIN32)

install(TARGETS kstartupconfig DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kdostartupconfig_SRCS kdostartupconfig.cpp )

kde4_automoc(${kdostartupconfig_SRCS})

kde4_add_executable(kdostartupconfig NOGUI ${kdostartupconfig_SRCS})

target_link_libraries(kdostartupconfig  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdostartupconfig DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############
if (WIN32)
set(klauncher_KDEINIT_SRCS
   klauncher_win.cpp
   klauncher_main.cpp
   klauncher_adaptor.cpp
   autostart.cpp
   )
else (WIN32)
set(klauncher_KDEINIT_SRCS
   klauncher.cpp
   klauncher_main.cpp
   klauncher_adaptor.cpp
   autostart.cpp
   )
endif (WIN32)

# The adaptor is manually edited to add QDBusMessage arguments
#qt4_add_dbus_adaptor(klauncher_KDEINIT_SRCS org.kde.KLauncher.xml klauncher.h KLauncher)

kde4_automoc(${klauncher_KDEINIT_SRCS})

kde4_add_kdeinit_executable( klauncher NOGUI ${klauncher_KDEINIT_SRCS})

target_link_libraries(kdeinit_klauncher  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kdeinit_klauncher  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( klauncher kdeinit_klauncher )
install(TARGETS klauncher DESTINATION ${BIN_INSTALL_DIR} )

########### install files ###############

install( FILES org.kde.KLauncher.xml DESTINATION ${DBUS_INTERFACES_DIR} )




