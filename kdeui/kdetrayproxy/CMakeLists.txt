
include_directories( ${CMAKE_SOURCE_DIR}/kdeui ${CMAKE_SOURCE_DIR}/kded )


########### next target ###############

set(kded_kdetrayproxy_PART_SRCS kdetrayproxy.cpp module.cpp )

kde4_automoc(${kded_kdetrayproxy_PART_SRCS})

kde4_add_plugin(kded_kdetrayproxy ${kded_kdetrayproxy_PART_SRCS})

target_link_libraries(kded_kdetrayproxy  ${KDE4_KDEUI_LIBS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kdetrayproxy )
install_targets(${PLUGIN_INSTALL_DIR} kded_kdetrayproxy )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kded FILES kdetrayproxy.desktop )




