
project(kioslave-file)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kio_file_PART_SRCS file.cc )

kde4_automoc(${kio_file_PART_SRCS})

kde4_add_plugin(kio_file ${kio_file_PART_SRCS})

target_link_libraries(kio_file  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kio_file )
install(TARGETS kio_file  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES file.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
install_files( /include/kio FILES file.h )




