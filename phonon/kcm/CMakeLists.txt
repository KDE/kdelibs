
include_directories( ${QT_INCLUDES} )

set(kcmphonon_SRCS main.cpp outputdevicechoice2.cpp backendselection.cpp)
kde4_add_ui_files(kcmphonon_SRCS outputdevicechoice2.ui backendselection.ui)
kde4_automoc(${kcmphonon_SRCS})

kde4_add_plugin(kcm_phonon ${kcmphonon_SRCS})
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcm_phonon)
target_link_libraries(kcm_phonon  ${KDE4_KDECORE_LIBS} phononcore )
install_targets(${PLUGIN_INSTALL_DIR} kcm_phonon )

########### install files ###############
install_files( ${XDG_APPS_DIR} FILES kcm_phonon.desktop )

