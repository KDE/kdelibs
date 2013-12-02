########### next target ###############

set(kcmssl_SRCS kcmssl.cpp cacertificatespage.cpp displaycertdialog.cpp)

#todo: port to include klocalizedstring.h
qt5_wrap_ui(kcmssl_SRCS cacertificates.ui displaycert.ui)

add_library(kcm_ssl ${kcmssl_SRCS})
target_link_libraries(kcm_ssl
   Qt5::Network
   KF5::KIOCore
   KF5::KIOWidgets
   KF5::CoreAddons
   KF5::KConfigWidgets   # KCModule
   KF5::KI18n
)
install(TARGETS kcm_ssl DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############
install(FILES kcm_ssl.desktop DESTINATION ${SERVICES_INSTALL_DIR})
