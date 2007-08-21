
include_directories( ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(kstyle_plastik_config_PART_SRCS plastikconf.cpp )


kde4_add_plugin(kstyle_plastik_config ${kstyle_plastik_config_PART_SRCS})

target_link_libraries(kstyle_plastik_config  ${KDE4_KDEUI_LIBS} )

install(TARGETS kstyle_plastik_config  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############





