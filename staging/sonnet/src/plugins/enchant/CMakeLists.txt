include_directories( ${ENCHANT_INCLUDE_DIR} "${ENCHANT_INCLUDE_DIR}/.." )


########### next target ###############

set(kspell_enchant_PART_SRCS enchantdict.cpp enchantclient.cpp )


add_library(kspell_enchant MODULE ${kspell_enchant_PART_SRCS})

target_link_libraries(kspell_enchant  ${KDE4_KDECORE_LIBS} sonnetcore ${ENCHANT_LIBRARIES} )

install(TARGETS kspell_enchant  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/sonnet_clients/ )

