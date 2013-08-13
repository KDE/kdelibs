
########### next target ###############

set(kded_kssld_SRCS kssld.cpp )

kde4_add_plugin(kded_kssld ${kded_kssld_SRCS})

target_link_libraries(kded_kssld
LINK_PUBLIC
   KIOCore     # ksslcertificatemanager
   KDBusAddons # kdedmodule
LINK_PRIVATE
   KConfigCore
   KService    # kpluginfactory
   Qt5::Network
   Qt5::DBus
)

install(TARGETS kded_kssld  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES kssld.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )




