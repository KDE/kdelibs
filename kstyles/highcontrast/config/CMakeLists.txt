


########### next target ###############

set(kstyle_highcontrast_config_PART_SRCS highcontrastconfig.cpp )

kde4_automoc(${kstyle_highcontrast_config_PART_SRCS})

kde4_add_plugin(kstyle_highcontrast_config ${kstyle_highcontrast_config_PART_SRCS})

target_link_libraries(kstyle_highcontrast_config  ${KDE4_KDECORE_LIBS} kdeui )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kstyle_highcontrast_config )
install(TARGETS kstyle_highcontrast_config  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############





