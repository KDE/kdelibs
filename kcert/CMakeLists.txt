
project(kcert)

include_directories( ${CMAKE_SOURCE_DIR}/kio/kssl ${CMAKE_BINARY_DIR}/kio/kssl )
include_directories( ${KDE4_KIO_INCLUDES} )

if(OPENSSL_FOUND)
include_directories( ${OPENSSL_INCLUDE_DIR} )
endif(OPENSSL_FOUND)

########### next target ###############

set(kcertpart_PART_SRCS kcertpart.cc )

kde4_automoc(${kcertpart_PART_SRCS})

kde4_add_plugin(kcertpart WITH_PREFIX ${kcertpart_PART_SRCS})

target_link_libraries(kcertpart  ${KDE4_KDECORE_LIBS} kparts )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcertpart )
install_targets(${PLUGIN_INSTALL_DIR} kcertpart )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kcertpart.desktop )
install_files( ${DATA_INSTALL_DIR}/kcertpart FILES kcertpart.rc )




