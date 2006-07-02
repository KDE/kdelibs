
project(kioslave-file)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kio_file_PART_SRCS file.cc )

kde4_automoc(${kio_file_PART_SRCS})

kde4_add_plugin(kio_file ${kio_file_PART_SRCS})

target_link_libraries(kio_file  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kio_file )
install_targets(${PLUGIN_INSTALL_DIR} kio_file )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES file.protocol )
install_files( /include/kio FILES file.h )




