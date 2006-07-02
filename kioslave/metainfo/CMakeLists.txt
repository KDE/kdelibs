
project(kioslave-metainfo)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kio_metainfo_PART_SRCS metainfo.cpp )

kde4_automoc(${kio_metainfo_PART_SRCS})

kde4_add_plugin(kio_metainfo ${kio_metainfo_PART_SRCS})


target_link_libraries(kio_metainfo  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kio_metainfo )
install_targets(${PLUGIN_INSTALL_DIR} kio_metainfo )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES metainfo.protocol )




