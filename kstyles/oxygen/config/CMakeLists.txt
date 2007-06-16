include_directories( ${KDE4_KDEUI_INCLUDES} )

########### next target ###############

set(kstyle_oxygen_config_PART_SRCS config.cpp )

kde4_automoc(${kstyle_oxygen_config_PART_SRCS})

kde4_add_ui_files(kstyle_oxygen_config_PART_SRCS config.ui)

kde4_add_plugin(kstyle-oxygen-config ${kstyle_oxygen_config_PART_SRCS})

target_link_libraries(kstyle-oxygen-config ${KDE4_KDEUI_LIBS} )

install(TARGETS kstyle-oxygen-config DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############



