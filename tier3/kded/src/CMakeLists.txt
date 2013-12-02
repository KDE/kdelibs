set(kded_KDEINIT_SRCS kded.cpp kdedadaptor.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded5.xml # just so that it gets generated
)

kf5_add_kdeinit_executable(kded5 ${kded_KDEINIT_SRCS})

if (APPLE)
    set_target_properties(kded5 PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
    set_target_properties(kded5 PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kded.kded5")
    set_target_properties(kded5 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Daemon")
endif ()

target_link_libraries(kdeinit_kded5
    Qt5::Widgets # QApplication
    KF5::KService # Needed for ksycoca.h
    KF5::CoreAddons # Needed for KDirWatch
    KF5::DBusAddons # Needed for kdedmodule.h
    KF5::KCrash # Sets it as autostart
)

install(TARGETS kdeinit_kded5 EXPORT KDEDTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kded5 kdeinit_kded5)
install(TARGETS kded5 EXPORT KDEDTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


qt5_generate_dbus_interface( kdedadaptor.h org.kde.kded5.xml )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded5.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

#include(MacroDBusAddActivationService) # for dbus_add_activation_service

macro(dbus_add_activation_service _sources)
    foreach (_i ${_sources})
        get_filename_component(_service_file ${_i} ABSOLUTE)
        string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
        set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
        configure_file(${_service_file} ${_target})
        install(FILES ${_target} DESTINATION ${DBUS_SERVICES_INSTALL_DIR} )
    endforeach (_i ${ARGN})
endmacro(dbus_add_activation_service _sources)
dbus_add_activation_service(org.kde.kded5.service.in)

install( FILES kdedmodule.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR} )