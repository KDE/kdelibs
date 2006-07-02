
# for strange kio/foo.h includes
include_directories( BEFORE ${CMAKE_SOURCE_DIR}/kio )

add_subdirectory( ksendbugmail )
add_subdirectory( kpac )
add_subdirectory( kssld )
add_subdirectory( kfile )
add_subdirectory( kwalletd )
add_subdirectory( kntlm )

########### next target ###############

set(kio_uiserver_KDEINIT_SRCS
uiserver.cpp
uiserveradaptor.cpp
)

kde4_automoc(${kio_uiserver_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kio_uiserver ${kio_uiserver_KDEINIT_SRCS})

target_link_libraries(kdeinit_kio_uiserver  ${KDE4_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR} kdeinit_kio_uiserver )

target_link_libraries( kio_uiserver kdeinit_kio_uiserver )
install(TARGETS kio_uiserver DESTINATION bin)

########### next target ###############

set(kmailservice_SRCS kmailservice.cpp )

kde4_automoc(${kmailservice_SRCS})

kde4_add_executable(kmailservice NOGUI ${kmailservice_SRCS})

target_link_libraries(kmailservice  ${KDE4_KDECORE_LIBS} )

install(TARGETS kmailservice DESTINATION bin)


########### next target ###############

set(ktelnetservice_SRCS ktelnetservice.cpp )

kde4_automoc(${ktelnetservice_SRCS})

kde4_add_executable(ktelnetservice NOGUI ${ktelnetservice_SRCS})

target_link_libraries(ktelnetservice  ${KDE4_KDECORE_LIBS} kdeui )

install(TARGETS ktelnetservice DESTINATION bin)


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kmailservice.protocol telnet.protocol rlogin.protocol rtsp.protocol ssh.protocol  		mms.protocol mmst.protocol mmsu.protocol pnm.protocol rtspt.protocol rtspu.protocol )
install_files( ${SERVICES_INSTALL_DIR} FILES kio_uiserver.desktop )
install_files( /include/kio FILES  uiserver.h)

# install the fileshareset and filesharelist scripts, they are not required on windows
if (UNIX)
   install(PROGRAMS fileshareset DESTINATION bin)
   # write a cmake script file which creates the symlink
   file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_fileshare_symlink.cmake "exec_program(${CMAKE_COMMAND} ARGS -E create_symlink ${CMAKE_INSTALL_PREFIX}/bin/fileshareset ${CMAKE_INSTALL_PREFIX}/bin/filesharelist)\n")
   # and add it as post-install script to any of the installed targets, so it will be executed during "make install"
   set_target_properties(kio_uiserver PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_fileshare_symlink.cmake)
endif (UNIX)




