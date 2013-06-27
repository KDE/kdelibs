include_directories( ${OPENSSL_INCLUDE_DIR} )

set(kcm_crypto_PART_SRCS crypto.cpp certexport.cpp kdatetimedlg.cpp )

kde4_add_plugin(kcm_crypto ${kcm_crypto_PART_SRCS})

target_link_libraries(kcm_crypto ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${OPENSSL_LIBRARIES} KCompletion)

install(TARGETS kcm_crypto  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES crypto.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
