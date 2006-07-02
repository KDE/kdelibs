
include_directories( ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(kstyle_plastik_config_PART_SRCS plastikconf.cpp )

kde4_automoc(${kstyle_plastik_config_PART_SRCS})

kde4_add_plugin(kstyle_plastik_config ${kstyle_plastik_config_PART_SRCS})

target_link_libraries(kstyle_plastik_config  ${KDE4_KDECORE_LIBS} kdeui )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kstyle_plastik_config )
install_targets(${PLUGIN_INSTALL_DIR} kstyle_plastik_config )


########### install files ###############





