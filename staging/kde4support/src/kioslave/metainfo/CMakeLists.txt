
project(kioslave-metainfo)

########### next target ###############

set(kio_metainfo_PART_SRCS metainfo.cpp )


add_library(kio_metainfo MODULE ${kio_metainfo_PART_SRCS})


target_link_libraries(kio_metainfo KDE4Support kio KF5::KCoreAddons KI18n)

install(TARGETS kio_metainfo  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES metainfo.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




