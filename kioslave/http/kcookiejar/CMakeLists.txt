
add_subdirectory( tests ) 

# all done by parent dir
# include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kcookiejar_SRCS
main.cpp
#kcookiejariface.cpp
)

kde4_automoc(${kcookiejar_SRCS})

qt4_add_dbus_interfaces(kcookiejar_SRCS org.kde.KCookieServer.xml)

kde4_add_executable( kcookiejar NOGUI ${kcookiejar_SRCS})

set_target_properties(kcookiejar PROPERTIES COMPILE_FLAGS "-include ${CMAKE_CURRENT_SOURCE_DIR}/qdeclareqlistint_hack.h" )

target_link_libraries( kcookiejar  ${KDE4_KDECORE_LIBS} )

install(TARGETS kcookiejar DESTINATION bin)

########### next target ###############

set(kded_kcookiejar_PART_SRCS
   kcookiejar.cpp
   kcookieserver.cpp
   kcookiewin.cpp
   kcookiejaradaptor.cpp
)

kde4_automoc(${kded_kcookiejar_PART_SRCS})

# The adaptor is hand-edited to add QDBusMessage
# qt4_add_dbus_adaptor(kded_kcookiejar_PART_SRCS org.kde.KCookieServer.xml kcookieserver.h KCookieServer)

kde4_add_plugin(kded_kcookiejar ${kded_kcookiejar_PART_SRCS})

target_link_libraries(kded_kcookiejar  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kcookiejar )
install(TARGETS kded_kcookiejar  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES domain_info  DESTINATION  ${DATA_INSTALL_DIR}/khtml )
install( FILES kcookiejar.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
install( FILES kcookiescfg.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )




