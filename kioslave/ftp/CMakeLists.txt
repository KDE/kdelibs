
project(kioslave-ftp)

include(ConfigureChecks.cmake)
configure_file(config-kioslave-ftp.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kioslave-ftp.h )

include_directories( ${KDE4_KIO_INCLUDES} )
include_directories( ${ki18n_SOURCE_DIR}/src )
include_directories( ${ki18n_BINARY_DIR}/src )


########### next target ###############

set(kio_ftp_PART_SRCS
ftp.cpp
)


kde4_add_plugin(kio_ftp ${kio_ftp_PART_SRCS})

target_link_libraries(kio_ftp  ${KDE4_KDECORE_LIBS} kio ${KCoreAddons_LIBRARIES} ki18n)

install(TARGETS kio_ftp  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES ftp.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




