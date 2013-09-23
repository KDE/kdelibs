add_subdirectory( tests )

####### kcookiejar: command line tool for talking to the kded kcookiejar module #######

qt5_generate_dbus_interface( ${CMAKE_SOURCE_DIR}/kded/kdedadaptor.h org.kde.kded5.xml )
qt5_add_dbus_interfaces(kcookiejar_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded5.xml)

qt5_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/kcookieserver.h org.kde.KCookieServer.xml)
set(kcookieserver_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCookieServer.xml)
set_source_files_properties(${kcookieserver_xml} PROPERTIES INCLUDE "kcookiejar_include.h")
qt5_add_dbus_interfaces(kcookiejar_SRCS ${kcookieserver_xml})

set(kcookiejar_SRCS ${kcookiejar_SRCS} main.cpp)
add_executable( kcookiejar4 ${kcookiejar_SRCS})
ecm_mark_nongui_executable(kcookiejar4)

target_link_libraries( kcookiejar4
   Qt5::DBus
   ${KCoreAddons_LIBRARIES}
   KI18n
   ${KWidgetsAddons_LIBRARIES}
)

install(TARGETS kcookiejar4 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### kded kcookiejar module ###############

set(kded_kcookiejar_SRCS
   kcookiejar.cpp
   kcookieserver.cpp
   kcookiewin.cpp
)

qt5_add_dbus_adaptor( kded_kcookiejar_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCookieServer.xml kcookieserver.h KCookieServer )


add_library(kded_kcookiejar MODULE ${kded_kcookiejar_SRCS})

target_link_libraries(kded_kcookiejar
   ${KDE4_KIO_LIBS}
   KWindowSystem
   KCoreAddons
   KDBusAddons
   KI18n
   KWidgetsAddons
   KInterProcessWindowing
)

install(TARGETS kded_kcookiejar  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES domain_info  DESTINATION  ${DATA_INSTALL_DIR}/khtml )
install( FILES kcookiejar.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
install( FILES kcookiescfg.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )
install( PROGRAMS kcookiescfg.pl  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KCookieServer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )



