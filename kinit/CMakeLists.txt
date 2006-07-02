
project(kdeinit)

add_subdirectory( tests )


include_directories( ${KDE4_KPARTS_INCLUDES} )
include_directories( ${CMAKE_SOURCE_DIR}/libltdl )


if(NOT WIN32)
########### next target ###############

set(kdeinit_SRCS kinit.cpp setproctitle.cpp )

kde4_automoc(${kdeinit_SRCS})

kde4_add_executable(kdeinit NOGUI ${kdeinit_SRCS})

target_link_libraries(kdeinit  ${KDE4_KDECORE_LIBS} kparts )

install(TARGETS kdeinit DESTINATION bin)


########### next target ###############

set(kdeinit_wrapper_SRCS wrapper.c )

kde4_add_executable(kdeinit_wrapper NOGUI ${kdeinit_wrapper_SRCS})

target_link_libraries(kdeinit_wrapper  ${KDE4_KDECORE_LIBS} )

if (NOT Q_WS_X11 AND NOT Q_WS_QWS)
add_definitions(-DNO_DISPLAY)
endif (NOT Q_WS_X11 AND NOT Q_WS_QWS)

install(TARGETS kdeinit_wrapper DESTINATION bin)


########### next target ###############

set(kshell_SRCS shell.c )

kde4_add_executable(kshell NOGUI ${kshell_SRCS})

target_link_libraries(kshell  ${KDE4_KDECORE_LIBS} )

install(TARGETS kshell DESTINATION bin)


########### next target ###############

set(kdeinit_shutdown_SRCS wrapper.c )

kde4_add_executable(kdeinit_shutdown NOGUI ${kdeinit_shutdown_SRCS})

target_link_libraries(kdeinit_shutdown  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdeinit_shutdown DESTINATION bin)


########### next target ###############

set(kwrapper_SRCS kwrapper.c )

kde4_add_executable(kwrapper NOGUI ${kwrapper_SRCS})

target_link_libraries(kwrapper  ${KDE4_KDECORE_LIBS} )

install(TARGETS kwrapper DESTINATION bin)


endif(NOT WIN32)
########### next target ###############

set(lnusertemp_SRCS lnusertemp.c )

kde4_add_executable(lnusertemp NOGUI ${lnusertemp_SRCS})

target_link_libraries(lnusertemp  ${KDE4_KDECORE_LIBS} )

install(TARGETS lnusertemp DESTINATION bin)


########### next target ###############

set(kioslave_SRCS kioslave.cpp)

kde4_automoc(${kioslave_SRCS})

kde4_add_executable(kioslave NOGUI ${kioslave_SRCS})

target_link_libraries(kioslave  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kioslave DESTINATION bin)


########### next target ###############

set(kstartupconfig_SRCS kstartupconfig.cpp )

kde4_automoc(${kstartupconfig_SRCS})

kde4_add_executable(kstartupconfig NOGUI ${kstartupconfig_SRCS})

if(WIN32)
   target_link_libraries(kstartupconfig  ${KDE4_KDECORE_LIBS})
else(WIN32)
   target_link_libraries(kstartupconfig  ${KDE4_KDECORE_LIBS} kdefakes)
endif(WIN32)

install(TARGETS kstartupconfig DESTINATION bin)


########### next target ###############

set(kdostartupconfig_SRCS kdostartupconfig.cpp )

kde4_automoc(${kdostartupconfig_SRCS})

kde4_add_executable(kdostartupconfig NOGUI ${kdostartupconfig_SRCS})

target_link_libraries(kdostartupconfig  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdostartupconfig DESTINATION bin)


########### next target ###############

set(klauncher_KDEINIT_SRCS
   klauncher.cpp
   klauncher_main.cpp
   autostart.cpp
   )

qt4_add_dbus_adaptors(klauncher_KDEINIT_SRCS org.kde.KLauncher.xml )

kde4_automoc(${klauncher_KDEINIT_SRCS})

kde4_add_kdeinit_executable( klauncher NOGUI ${klauncher_KDEINIT_SRCS})

target_link_libraries(kdeinit_klauncher  ${KDE4_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR} kdeinit_klauncher )

target_link_libraries( klauncher kdeinit_klauncher )
install(TARGETS klauncher DESTINATION bin)

########### install files ###############





