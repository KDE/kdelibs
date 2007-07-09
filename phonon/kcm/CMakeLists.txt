include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/kutils ${CMAKE_SOURCE_DIR}/solid ${CMAKE_BINARY_DIR}/solid)

set(kcmphonon_SRCS main.cpp outputdevicechoice.cpp backendselection.cpp)
kde4_add_ui_files(kcmphonon_SRCS outputdevicechoice.ui backendselection.ui)

kde4_add_plugin(kcm_phonon ${kcmphonon_SRCS})
target_link_libraries(kcm_phonon ${KDE4_PHONON_LIBS} ${KDE4_KUTILS_LIBS}
	${KDE4_KAUDIODEVICELIST_LIBS})
install(TARGETS kcm_phonon  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kcm_phonon.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES listview-background.png DESTINATION ${DATA_INSTALL_DIR}/kcm_phonon)
