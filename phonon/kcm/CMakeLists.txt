include_directories(${KDE4_KDEUI_INCLUDES} ${CMAKE_SOURCE_DIR}/kutils)

set(kcmphonon_SRCS main.cpp outputdevicechoice.cpp backendselection.cpp)
kde4_add_ui_files(kcmphonon_SRCS outputdevicechoice.ui backendselection.ui)
kde4_automoc(${kcmphonon_SRCS})

kde4_add_plugin(kcm_phonon ${kcmphonon_SRCS})
target_link_libraries(kcm_phonon ${KDE4_PHONONCORE_LIBS} ${KDE4_KUTILS_LIBS}
	${KDE4_KAUDIODEVICELIST_LIBS})
install(TARGETS kcm_phonon  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kcm_phonon.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

