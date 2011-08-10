include_directories( ${ENCHANT_INCLUDE_DIR} "${ENCHANT_INCLUDE_DIR}/.." )


########### next target ###############

set(kspell_enchant_PART_SRCS enchantdict.cpp enchantclient.cpp )


kde4_add_plugin(kspell_enchant ${kspell_enchant_PART_SRCS})

target_link_libraries(kspell_enchant  ${KDE4_KDECORE_LIBS} ${ENCHANT_LIBRARIES} )

install(TARGETS kspell_enchant  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kspell_enchant.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


