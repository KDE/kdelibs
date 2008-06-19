
include_directories( ${CMAKE_SOURCE_DIR}/kwallet/backend ${CMAKE_SOURCE_DIR}/kwallet/client )

########### next target ###############

set(kwalletd_KDEINIT_SRCS
   main.cpp
   kbetterthankdialog.cpp
   kwalletd.cpp
   kwalletwizard.cpp
   ktimeout.cpp
   kwalletsynctimer.cpp
)


kde4_add_ui_files(kwalletd_KDEINIT_SRCS
   kbetterthankdialogbase.ui
   kwalletwizardpageexplanation.ui
   kwalletwizardpageintro.ui
   kwalletwizardpageoptions.ui
   kwalletwizardpagepassword.ui
)

kde4_add_kdeinit_executable( kwalletd ${kwalletd_KDEINIT_SRCS} )

# uses kio for KDirWatch
target_link_libraries(kdeinit_kwalletd  ${KDE4_KIO_LIBS} kwalletbackend )
install(TARGETS kdeinit_kwalletd  ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries(kwalletd kdeinit_kwalletd)
install(TARGETS kwalletd  DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS} )

# Note that org.kde.KWallet.xml is generated from kwalletdadaptor.h in kdeui/CMakeLists.txt

########### install files ###############

install( FILES kwalletd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

add_subdirectory(tests)
