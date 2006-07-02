
include_directories( ${ASPELL_INCLUDE_DIR}  )


########### next target ###############

set(kspell_aspell_PART_SRCS kspell_aspellclient.cpp kspell_aspelldict.cpp )

kde4_automoc(${kspell_aspell_PART_SRCS})

kde4_add_plugin(kspell_aspell ${kspell_aspell_PART_SRCS})

target_link_libraries(kspell_aspell  ${KDE4_KDECORE_LIBS} kspell2 ${ASPELL_LIBRARIES} )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kspell_aspell )
install_targets(${PLUGIN_INSTALL_DIR} kspell_aspell )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kspell_aspell.desktop )




