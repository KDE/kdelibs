


########### next target ###############

set(kspell_ispell_PART_SRCS
kspell_ispellclient.cpp
kspell_ispelldict.cpp
correct.cpp
good.cpp
hash.cpp
lookup.cpp
makedent.cpp
tgood.cpp
ispell_checker.cpp
)

kde4_automoc(${kspell_ispell_PART_SRCS})

kde4_add_plugin(kspell_ispell ${kspell_ispell_PART_SRCS})

target_link_libraries(kspell_ispell  ${KDE4_KDECORE_LIBS} kspell2 )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kspell_ispell )
install_targets(${PLUGIN_INSTALL_DIR} kspell_ispell )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kspell_ispell.desktop )




