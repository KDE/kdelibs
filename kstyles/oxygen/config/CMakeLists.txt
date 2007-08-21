
include_directories( ${KDE4_KDEUI_INCLUDES} )


########### next target ###############

set(kstyle_oxygen_config_PART_SRCS oxygenconf.cpp )


kde4_add_plugin(kstyle_oxygen_config ${kstyle_oxygen_config_PART_SRCS})

target_link_libraries(kstyle_oxygen_config ${KDE4_KDEUI_LIBS})

install(TARGETS kstyle_oxygen_config  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############





