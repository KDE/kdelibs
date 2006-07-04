


########### next target ###############

set(kdeprint_tool_escputil_PART_SRCS escpwidget.cpp )

kde4_automoc(${kdeprint_tool_escputil_PART_SRCS})

kde4_add_plugin(kdeprint_tool_escputil ${kdeprint_tool_escputil_PART_SRCS})

target_link_libraries(kdeprint_tool_escputil  ${KDE4_KDECORE_LIBS} kdeui )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}  kdeprint_tool_escputil )
install(TARGETS kdeprint_tool_escputil  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES escputil.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint/tools )




