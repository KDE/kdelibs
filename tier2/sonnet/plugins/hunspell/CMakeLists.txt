
########### next target ###############

include_directories( ${HUNSPELL_INCLUDE_DIR} )
MESSAGE(status "dd ${HUNSPELL_INCLUDE_DIR}")
set(kspell_hunspell_PART_SRCS kspell_hunspellclient.cpp kspell_hunspelldict.cpp )


kde4_add_plugin(kspell_hunspell ${kspell_hunspell_PART_SRCS})

target_link_libraries(kspell_hunspell  ${KDE4_KDECORE_LIBS} ${HUNSPELL_LIBRARIES} )

install(TARGETS kspell_hunspell  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kspell_hunspell.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




