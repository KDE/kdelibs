
add_subdirectory( tests )

####### kcookiejar4: command line tool for talking to the kded kcookiejar module #######

set(kcookiejar_SRCS
main.cpp
)


qt5_generate_dbus_interface( ${CMAKE_SOURCE_DIR}/kded/kdedadaptor.h org.kde.kded5.xml )
qt5_add_dbus_interfaces(kcookiejar_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded5.xml)

qt5_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/kcookieserver.h org.kde.KCookieServer.xml)
set(kcookieserver_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCookieServer.xml)
set_source_files_properties(${kcookieserver_xml} PROPERTIES INCLUDE "kcookiejar_include.h")
qt5_add_dbus_interfaces(kcookiejar_SRCS ${kcookieserver_xml})

kde4_add_executable( kcookiejar4 NOGUI ${kcookiejar_SRCS})

target_link_libraries( kcookiejar4 ${KDE4_KDECORE_LIBS} ${KCoreAddons_LIBRARIES} ki18n ${KWidgetsAddons_LIBRARIES})

install(TARGETS kcookiejar4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kded kcookiejar module ###############

set(kded_kcookiejar_SRCS
   kcookiejar.cpp
   kcookieserver.cpp
   kcookiewin.cpp
)

qt5_add_dbus_adaptor( kded_kcookiejar_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCookieServer.xml kcookieserver.h KCookieServer )


kde4_add_plugin(kded_kcookiejar ${kded_kcookiejar_SRCS})

target_link_libraries(kded_kcookiejar  ${KDE4_KIO_LIBS} ${KWindowSystem_LIBRARIES} ${KCoreAddons_LIBRARIES} ki18n kwidgets ${KWidgetsAddons_LIBRARIES} kinterprocesswindowing)

install(TARGETS kded_kcookiejar  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES domain_info  DESTINATION  ${DATA_INSTALL_DIR}/khtml )
install( FILES kcookiejar.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
install( FILES kcookiescfg.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )
install( PROGRAMS kcookiescfg.pl  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCookieServer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )



