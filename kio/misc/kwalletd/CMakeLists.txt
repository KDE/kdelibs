
include_directories( ${CMAKE_SOURCE_DIR}/kwallet/backend ${CMAKE_SOURCE_DIR}/kwallet/client )

add_subdirectory(tests)

########### next target ###############

set(kded_kwalletd_PART_SRCS
   kbetterthankdialog.cpp
   kwalletd.cpp
   kwalletwizard.cpp
   ktimeout.cpp
)

kde4_automoc(${kded_kwalletd_PART_SRCS})

kde4_add_ui_files(kded_kwalletd_PART_SRCS
   kbetterthankdialogbase.ui
   kwalletwizardpageexplanation.ui
   kwalletwizardpageintro.ui
   kwalletwizardpageoptions.ui
   kwalletwizardpagepassword.ui
)

kde4_add_plugin(kded_kwalletd ${kded_kwalletd_PART_SRCS})

target_link_libraries(kded_kwalletd  ${KDE4_KDECORE_LIBS} kio kwalletbackend kwalletclient )

install(TARGETS kded_kwalletd  DESTINATION ${PLUGIN_INSTALL_DIR} )

# This one misses a way to specify the name of the xml file:
# QT4_GENERATE_DBUS_INTERFACE( kwalletdadaptor.h )

#ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KWallet.xml
#       COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} kwalletdadaptor.h > ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KWallet.xml
#       DEPENDS kwalletdadaptor.h)

########### install files ###############

install( FILES kwalletd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )




