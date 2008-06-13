
include_directories( ${CMAKE_SOURCE_DIR}/kwallet/backend ${CMAKE_SOURCE_DIR}/kwallet/client )

########### next target ###############

set(kded_kwalletd_PART_SRCS
   kbetterthankdialog.cpp
   kwalletd.cpp
   kwalletwizard.cpp
   ktimeout.cpp
   kwalletsynctimer.cpp
)


kde4_add_ui_files(kded_kwalletd_PART_SRCS
   kbetterthankdialogbase.ui
   kwalletwizardpageexplanation.ui
   kwalletwizardpageintro.ui
   kwalletwizardpageoptions.ui
   kwalletwizardpagepassword.ui
)

kde4_add_plugin(kded_kwalletd ${kded_kwalletd_PART_SRCS})

# uses kio for KDirWatch
target_link_libraries(kded_kwalletd  ${KDE4_KIO_LIBS} kwalletbackend )

install(TARGETS kded_kwalletd  DESTINATION ${PLUGIN_INSTALL_DIR} )

# Note that org.kde.KWallet.xml is generated from kwalletdadaptor.h in kdeui/CMakeLists.txt

########### install files ###############

install( FILES kwalletd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )

add_subdirectory(tests)
