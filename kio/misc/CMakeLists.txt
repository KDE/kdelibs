
# for strange kio/foo.h includes
include_directories( BEFORE ${CMAKE_SOURCE_DIR}/kio )

add_subdirectory( ksendbugmail )
if(NOT WIN32)
  # arpa/nameser.h
  add_subdirectory( kpac )
endif(NOT WIN32)

########### next target ###############

add_executable(kmailservice kmailservice.cpp)
ecm_mark_nongui_executable(kmailservice)

target_link_libraries(kmailservice KF5::KCoreAddons Qt5::Widgets)

install(TARGETS kmailservice ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kmailservice.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

########### next target ###############

add_executable(ktelnetservice ktelnetservice.cpp)
ecm_mark_nongui_executable(ktelnetservice)

target_link_libraries(ktelnetservice KF5::KCoreAddons KF5::KService KF5::KConfigGui KF5::KWidgetsAddons KF5::KI18n Qt5::Widgets)

install(TARGETS ktelnetservice ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ktelnetservice.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

########### install files ###############

install( FILES rtsp.protocol mms.protocol mmst.protocol mmsu.protocol pnm.protocol rtspt.protocol rtspu.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )

# install the fileshareset and filesharelist scripts, they are not required on windows
if (UNIX)
   install(PROGRAMS fileshareset DESTINATION ${LIBEXEC_INSTALL_DIR} )

   install(CODE "
     set(FILESHARESET_PATH \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${LIBEXEC_INSTALL_DIR}/fileshareset\")
        EXECUTE_PROCESS(COMMAND sh -c \"chown 0 '\${FILESHARESET_PATH}' && chmod u+s '\${FILESHARESET_PATH}'\")
   ")

   # write a cmake script file which creates the symlink
   file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_fileshare_symlink.cmake "exec_program(${CMAKE_COMMAND} ARGS -E create_symlink fileshareset \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${LIBEXEC_INSTALL_DIR}/filesharelist\")\n")
   # and add it as post-install script to any of the installed targets, so it will be executed during "make install"
   set_target_properties(ktelnetservice PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_fileshare_symlink.cmake)
endif (UNIX)

