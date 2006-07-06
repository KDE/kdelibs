
include_directories( ${QT_INCLUDES} )

set(kcmphonon_SRCS main.cpp outputdevicechoice.cpp backendselection.cpp)
kde4_add_ui_files(kcmphonon_SRCS outputdevicechoice.ui backendselection.ui)
kde4_automoc(${kcmphonon_SRCS})

kde4_add_plugin(kcm_phonon ${kcmphonon_SRCS})
kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcm_phonon)
target_link_libraries(kcm_phonon  ${KDE4_KDECORE_LIBS} phononcore )
install(TARGETS kcm_phonon  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kcm_phonon.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

