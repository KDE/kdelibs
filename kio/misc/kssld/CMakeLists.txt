
########### next target ###############

set(kded_kssld_PART_SRCS kssld.cpp )

kde4_automoc(${kded_kssld_PART_SRCS})

kde4_add_plugin(kded_kssld ${kded_kssld_PART_SRCS})

target_link_libraries(kded_kssld  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kssld )
install_targets(${PLUGIN_INSTALL_DIR} kded_kssld )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kded FILES kssld.desktop )




