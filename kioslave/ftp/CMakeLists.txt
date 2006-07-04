
project(kioslave-ftp)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kio_ftp_PART_SRCS
ftp.cc
)

kde4_automoc(${kio_ftp_PART_SRCS})

kde4_add_plugin(kio_ftp ${kio_ftp_PART_SRCS})

target_link_libraries(kio_ftp  ${KDE4_KDECORE_LIBS} kio )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kio_ftp )
install(TARGETS kio_ftp  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ftp.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




