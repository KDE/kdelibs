
include_directories( ${CMAKE_SOURCE_DIR}/kwallet/backend ${CMAKE_SOURCE_DIR}/kwallet/client )


########### next target ###############

set(kded_kwalletd_PART_SRCS
   kbetterthankdialog.cpp
   kwalletd.cpp
   kwalletwizard.cpp
   ktimeout.cpp
)

kde4_automoc(${kded_kwalletd_PART_SRCS})

kde4_add_ui_files(kded_kwalletd_PART_SRCS
   kwalletwizardbase.ui
   kbetterthankdialogbase.ui
)

kde4_add_plugin(kded_kwalletd ${kded_kwalletd_PART_SRCS})

target_link_libraries(kded_kwalletd  ${KDE4_KDECORE_LIBS} kio kwalletbackend kwalletclient )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_kwalletd )
install_targets(${PLUGIN_INSTALL_DIR} kded_kwalletd )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kded FILES kwalletd.desktop )




