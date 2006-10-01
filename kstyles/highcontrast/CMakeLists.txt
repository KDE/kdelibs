
project(kstyle-highcontrast)

add_subdirectory( config ) 



########### next target ###############

set(highcontrast_PART_SRCS highcontrast.cpp )

kde4_automoc(${highcontrast_PART_SRCS})

kde4_add_plugin(highcontrast ${highcontrast_PART_SRCS})

target_link_libraries(highcontrast  ${KDE4_KDECORE_LIBS} kdefx )

install(TARGETS highcontrast  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES highcontrast.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )




