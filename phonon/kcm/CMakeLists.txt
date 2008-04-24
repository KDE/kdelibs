include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_SOURCE_DIR}/kio/kio ${CMAKE_SOURCE_DIR}/kutils ${CMAKE_SOURCE_DIR}/solid ${CMAKE_BINARY_DIR}/solid)

set(kcmphonon_SRCS main.cpp devicepreference.cpp backendselection.cpp ../globalconfig.cpp)
kde4_add_ui_files(kcmphonon_SRCS devicepreference.ui backendselection.ui)

kde4_add_plugin(kcm_phonon ${kcmphonon_SRCS})
target_link_libraries(kcm_phonon ${KDE4_PHONON_LIBS} ${KDE4_KUTILS_LIBS}
	${KDE4_KAUDIODEVICELIST_LIBS})

install(TARGETS kcm_phonon  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES kcm_phonon.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install(FILES listview-background.png DESTINATION ${DATA_INSTALL_DIR}/kcm_phonon)
install(FILES devicepreference.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

########### update helper ###############
kde4_add_executable(phonon_devicepreference_update devicepreference_update.cpp)
target_link_libraries(phonon_devicepreference_update ${KDE4_KDECORE_LIBS})
install(TARGETS phonon_devicepreference_update DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin)
