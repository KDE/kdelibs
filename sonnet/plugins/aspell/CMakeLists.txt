
include_directories( ${ASPELL_INCLUDE_DIR})


########### next target ###############

set(kspell_aspell_PART_SRCS kspell_aspellclient.cpp kspell_aspelldict.cpp )


kde4_add_plugin(kspell_aspell ${kspell_aspell_PART_SRCS})

target_link_libraries(kspell_aspell  ${KDE4_KDECORE_LIBS} ${ASPELL_LIBRARIES} )

install(TARGETS kspell_aspell  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kspell_aspell.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




