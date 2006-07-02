
include_directories( ${CMAKE_SOURCE_DIR}/khtml ${CMAKE_SOURCE_DIR}/kio/httpfilter ${CMAKE_SOURCE_DIR}/kutils )
include_directories(${ZLIB_INCLUDE_DIR})

########### next target ###############

set(kmultipart_PART_SRCS kmultipart.cpp ${CMAKE_SOURCE_DIR}/kio/httpfilter/httpfilter.cc )

kde4_automoc(${kmultipart_PART_SRCS})

kde4_add_plugin(kmultipart WITH_PREFIX ${kmultipart_PART_SRCS})

target_link_libraries(kmultipart  ${KDE4_KDECORE_LIBS} kparts )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kmultipart )
install_targets(${PLUGIN_INSTALL_DIR} kmultipart )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kmultipart.desktop )




