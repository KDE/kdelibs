
add_subdirectory( tests ) 

# all done by parent dir
# include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kcookiejar_SRCS
main.cpp
kcookiejariface.cpp
)

kde4_automoc(${kcookiejar_SRCS})

kde4_add_executable( kcookiejar NOGUI ${kcookiejar_SRCS})

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

kde4_add_plugin(kded_kcookiejar ${kded_kcookiejar_PART_SRCS})

target_link_libraries(kded_kcookiejar  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kcookiejar )
install_targets(${PLUGIN_INSTALL_DIR} kded_kcookiejar )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/khtml FILES domain_info )
install_files( ${SERVICES_INSTALL_DIR}/kded FILES kcookiejar.desktop )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kcookiescfg.upd )




