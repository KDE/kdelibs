
# for strange kio/foo.h includes
include_directories( BEFORE ${CMAKE_SOURCE_DIR}/kio )

add_subdirectory( ksendbugmail )
if (NOT WIN32)
add_subdirectory( kpac )
add_subdirectory( kssld )
endif (NOT WIN32)
add_subdirectory( kfile )
if (NOT WIN32)
add_subdirectory( kwalletd )
endif (NOT WIN32)
add_subdirectory( kntlm )

########### next target ###############

set(kio_uiserver_KDEINIT_SRCS
uiserver.cpp
uiserveradaptor.cpp
progresslistmodel.cpp
progresslistdelegate.cpp
)

set(kio_uiserver_KDEINIT_PRIVATE
progresslistdelegate_p.h
uiserver_p.h
)

kde4_add_ui_files(kio_uiserver_KDEINIT_SRCS uiserverConfigDialog.ui)
kde4_automoc(${kio_uiserver_KDEINIT_SRCS} ${kio_uiserver_KDEINIT_PRIVATE})
kde4_add_kdeinit_executable( kio_uiserver ${kio_uiserver_KDEINIT_SRCS})

target_link_libraries(kdeinit_kio_uiserver  ${KDE4_KIO_LIBS} )

install(TARGETS kdeinit_kio_uiserver  DESTINATION ${LIB_INSTALL_DIR} )

target_link_libraries( kio_uiserver kdeinit_kio_uiserver )
install(TARGETS kio_uiserver DESTINATION ${BIN_INSTALL_DIR} )

########### next target ###############

set(kmailservice_SRCS kmailservice.cpp )

kde4_automoc(${kmailservice_SRCS})

kde4_add_executable(kmailservice NOGUI ${kmailservice_SRCS})

target_link_libraries(kmailservice  ${KDE4_KDEUI_LIBS} )

install(TARGETS kmailservice DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(ktelnetservice_SRCS ktelnetservice.cpp )

kde4_automoc(${ktelnetservice_SRCS})

kde4_add_executable(ktelnetservice NOGUI ${ktelnetservice_SRCS})

target_link_libraries(ktelnetservice  ${KDE4_KDEUI_LIBS} )

install(TARGETS ktelnetservice DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kmailservice.protocol telnet.protocol rlogin.protocol rtsp.protocol ssh.protocol  		mms.protocol mmst.protocol mmsu.protocol pnm.protocol rtspt.protocol rtspu.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kio_uiserver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES  uiserver.h DESTINATION  ${INCLUDE_INSTALL_DIR}/kio )

# install the fileshareset and filesharelist scripts, they are not required on windows
if (UNIX)
   install(PROGRAMS fileshareset DESTINATION ${BIN_INSTALL_DIR} )
   # write a cmake script file which creates the symlink
   file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_fileshare_symlink.cmake "exec_program(${CMAKE_COMMAND} ARGS -E create_symlink ${CMAKE_INSTALL_PREFIX}/bin/fileshareset ${CMAKE_INSTALL_PREFIX}/bin/filesharelist)\n")
   # and add it as post-install script to any of the installed targets, so it will be executed during "make install"
   set_target_properties(kio_uiserver PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_fileshare_symlink.cmake)
endif (UNIX)




