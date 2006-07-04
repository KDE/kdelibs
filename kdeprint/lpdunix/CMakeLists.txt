


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
install(TARGETS kdeprint_lpdunix  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES lpdunix.print  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint/plugins )




