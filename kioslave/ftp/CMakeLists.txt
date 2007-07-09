
project(kioslave-ftp)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kio_ftp_PART_SRCS
ftp.cpp
)


kde4_add_plugin(kio_ftp ${kio_ftp_PART_SRCS})

target_link_libraries(kio_ftp  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kio_ftp  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ftp.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




