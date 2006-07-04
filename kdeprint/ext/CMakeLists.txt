


########### next target ###############

set(kdeprint_ext_PART_SRCS
   kmextfactory.cpp
   kmextmanager.cpp
   kextprinterimpl.cpp
   kmextuimanager.cpp
)

kde4_automoc(${kdeprint_ext_PART_SRCS})

kde4_add_plugin(kdeprint_ext ${kdeprint_ext_PART_SRCS})

target_link_libraries(kdeprint_ext  ${KDE4_KDECORE_LIBS} kdeprint )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdeprint_ext )
install(TARGETS kdeprint_ext  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ext.print  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint/plugins )




