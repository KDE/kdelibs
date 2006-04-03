kde4_header()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES} )

set(kcmphonon_SRCS main.cpp outputdevicechoice2.cpp)
kde4_add_ui_files(kcmphonon_SRCS outputdevicechoice2.ui)
kde4_automoc(${kcmphonon_SRCS})
kde4_add_plugin(kcm_phonon ${kcmphonon_SRCS})
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcm_phonon)
target_link_libraries(kcm_phonon  ${KDE4_KDECORE_LIBS} phononcore )
install_targets(${LIB_INSTALL_DIR}/kde4 kcm_phonon )

########### install files ###############
install_files( ${XDG_APPS_DIR} FILES kcm_phonon.desktop )

kde4_footer()
