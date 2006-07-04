
project(kstyle-plastik)

add_subdirectory( config ) 



########### next target ###############

set(plastik_PART_SRCS plastik.cpp misc.cpp )

kde4_automoc(${plastik_PART_SRCS})

kde4_add_plugin(plastik ${plastik_PART_SRCS})

target_link_libraries(plastik  ${KDE4_KDECORE_LIBS} kdefx )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR}/plugins/styles plastik)
install(TARGETS plastik  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles/ )


########### install files ###############

install( FILES plastik.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )




