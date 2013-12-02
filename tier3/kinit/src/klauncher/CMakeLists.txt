set(klauncher_KDEINIT_SRCS
   klauncher.cpp
   klauncher_main.cpp
   klauncher_adaptor.cpp
   autostart.cpp
   ../klauncher_cmds.cpp
   ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml # just so that it gets generated
   )

# The adaptor is manually edited, generate the xml from it.
qt5_generate_dbus_interface(klauncher_adaptor.h org.kde.KLauncher.xml)

kf5_add_kdeinit_executable(klauncher NOGUI ${klauncher_KDEINIT_SRCS})
set_target_properties(kdeinit_klauncher PROPERTIES DEFINE_SYMBOL MAKE_KLAUNCHER)
target_link_libraries(kdeinit_klauncher ${X11_LIBRARIES}
   KF5::KIOCore # KIO::ConnectionServer
   KF5::KIOWidgets # KRun
   KF5::KCrash  # sig_handler
   KF5::WindowSystem # KStartupInfo + fixx11
   KF5::KI18n   # i18n()
   Qt5::DBus
)

install(TARGETS kdeinit_klauncher ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS klauncher DESTINATION ${LIBEXEC_INSTALL_DIR} )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KLauncher.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
