
########### next target ###############

set(kded_kssld_PART_SRCS kssld.cpp )


kde4_add_plugin(kded_kssld ${kded_kssld_PART_SRCS})

# kdecore for ksslcertificatemanager
target_link_libraries(kded_kssld ${KDE4_KDECORE_LIBS} KDBusAddons Qt5::Network )

install(TARGETS kded_kssld  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kssld.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )




