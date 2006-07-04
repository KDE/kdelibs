
include_directories( ${HSPELL_INCLUDE_DIR}  )


########### next target ###############

set(kspell_hspell_PART_SRCS kspell_hspellclient.cpp kspell_hspelldict.cpp )

kde4_automoc(${kspell_hspell_PART_SRCS})

kde4_add_plugin(kspell_hspell ${kspell_hspell_PART_SRCS})

target_link_libraries(kspell_hspell  ${KDE4_KDECORE_LIBS} kspell2 ${HSPELL_LIBRARIES} )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kspell_hspell )
install(TARGETS kspell_hspell  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kspell_hspell.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




