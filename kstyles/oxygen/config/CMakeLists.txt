include_directories( ${KDE4_KDEUI_INCLUDES} )

########### next target ###############

set(kstyle_oxygen_config_PART_SRCS config.cpp )
set(kstyle_oxygen_config_RCCS config.qrc)
set(kstyle_oxygen_config_MOC_HDRS config.h )

qt4_add_resources(kstyle_oxygen_config_RCC_SRCS ${kstyle_oxygen_config_RCCS})

qt4_wrap_cpp(kstyle_oxygen_config_MOC_SRCS ${kstyle_oxygen_config_MOC_HDRS})

kde4_add_ui_files(kstyle_oxygen_config_PART_SRCS config.ui)

kde4_add_plugin(kstyle-oxygen-config ${kstyle_oxygen_config_PART_SRCS} ${kstyle_oxygen_config_MOC_SRCS} ${kstyle_oxygen_config_RCC_SRCS})

target_link_libraries(kstyle-oxygen-config ${KDE4_KDEUI_LIBS} )

kde4_add_executable(oxygen-config ${kstyle_oxygen_config_PART_SRCS} ${kstyle_oxygen_config_MOC_SRCS} ${kstyle_oxygen_config_RCC_SRCS})
target_link_libraries(oxygen-config ${KDE4_KDEUI_LIBS} )

install(TARGETS kstyle-oxygen-config DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############



