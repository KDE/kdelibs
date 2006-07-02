

########### next target ###############

set(kdeprint_foomatic_PART_SRCS
   kmfoomaticfactory.cpp
   kmfoomaticmanager.cpp
   kfoomaticprinterimpl.cpp
)

kde4_automoc(${kdeprint_foomatic_PART_SRCS})

kde4_add_plugin(kdeprint_foomatic ${kdeprint_foomatic_PART_SRCS})

target_link_libraries(kdeprint_foomatic  ${KDE4_KDECORE_LIBS} kdeprint_management )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kdeprint_foomatic )
install_targets(${PLUGIN_INSTALL_DIR} kdeprint_foomatic )


########### next target ###############

kde4_add_executable(make_driver_db_foomatic NOGUI make_driver_db_foomatic.c)

target_link_libraries(make_driver_db_foomatic  ${KDE4_KDECORE_LIBS} )

install(TARGETS make_driver_db_foomatic DESTINATION bin)


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kdeprint/plugins FILES foomatic.print )




