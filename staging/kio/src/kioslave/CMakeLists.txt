
add_subdirectory( file )
add_subdirectory( help )
add_subdirectory( http )
add_subdirectory( ftp )

########### next target ###############

add_executable(kmailservice kmailservice.cpp)
ecm_mark_nongui_executable(kmailservice)

target_link_libraries(kmailservice
   KF5::KCoreAddons
   Qt5::Widgets
)

install(TARGETS kmailservice ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kmailservice.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

########### next target ###############

add_executable(ktelnetservice ktelnetservice.cpp)
ecm_mark_nongui_executable(ktelnetservice)

target_link_libraries(ktelnetservice
   KF5::KCoreAddons
   KF5::KService
   KF5::KConfigGui
   KF5::KWidgetsAddons
   KF5::KI18n
   Qt5::Widgets
)

install(TARGETS ktelnetservice ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ktelnetservice.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

########### install files ###############

install( FILES
   data.protocol
   rtsp.protocol
   mms.protocol
   mmst.protocol
   mmsu.protocol
   pnm.protocol
   rtspt.protocol
   rtspu.protocol
   DESTINATION  ${SERVICES_INSTALL_DIR} )

