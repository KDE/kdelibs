
project(kioslave-metainfo)

include_directories( ${KDE4_KIO_INCLUDES} )
include_directories( ${ki18n_SOURCE_DIR}/src )
include_directories( ${ki18n_BINARY_DIR}/src )


########### next target ###############

set(kio_metainfo_PART_SRCS metainfo.cpp )


kde4_add_plugin(kio_metainfo ${kio_metainfo_PART_SRCS})


target_link_libraries(kio_metainfo  ${KDE4_KDECORE_LIBS} kio ${KCoreAddons_LIBRARIES} ki18n)

install(TARGETS kio_metainfo  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES metainfo.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




