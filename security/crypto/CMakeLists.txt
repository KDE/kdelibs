
########### next target ###############
set(kcm_crypto_PART_SRCS crypto.cpp certexport.cpp kdatetimedlg.cpp )

kde4_automoc(${kcm_crypto_PART_SRCS})

kde4_add_plugin(kcm_crypto ${kcm_crypto_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_crypto )

target_link_libraries(kcm_crypto  ${KDE4_KDE3SUPPORT_LIBS} ${OPENSSL_LIBRARIES})

install(TARGETS kcm_crypto  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES crypto.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
