


########### next target ###############

set(kdeprint_lpdunix_PART_SRCS
   kmlpdunixfactory.cpp
   kmlpdunixmanager.cpp
   klpdunixprinterimpl.cpp
   kmlpdunixuimanager.cpp
)

kde4_automoc(${kdeprint_lpdunix_PART_SRCS})

kde4_add_plugin(kdeprint_lpdunix ${kdeprint_lpdunix_PART_SRCS})

target_link_libraries(kdeprint_lpdunix  ${KDE4_KDECORE_LIBS} kdeprint_management )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdeprint_lpdunix )
install_targets(${PLUGIN_INSTALL_DIR} kdeprint_lpdunix )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeprint/plugins FILES lpdunix.print )




