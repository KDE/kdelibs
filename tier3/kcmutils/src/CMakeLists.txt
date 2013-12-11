########### kcmutils ###############
set(kcmutils_LIB_SRCS
    kcmoduleinfo.cpp
    kcmoduleloader.cpp
    kcmultidialog.cpp
    kcmoduleproxy.cpp
    kpluginselector.cpp
    kcmodulecontainer.cpp
    ksettingswidgetadaptor.cpp
    ksettings/dispatcher.cpp
    ksettings/dialog.cpp
    ksettings/pluginpage.cpp
    ksettings/componentsdialog.cpp
)

add_library(KF5KCMUtils ${kcmutils_LIB_SRCS})
generate_export_header(KF5KCMUtils BASE_NAME KCMUtils)

target_link_libraries(KF5KCMUtils
  PUBLIC
    Qt5::Widgets
    KF5::ConfigWidgets  # KCModule
    KF5::Service        # KService
  PRIVATE
    Qt5::DBus       # dbus usage in kcmoduleproxy.cpp
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes     # KIconLoader
    KF5::ItemViews       # KWidgetItemDelegate
    KF5::XmlGui          # KAboutApplicationDialog
)
set_target_properties(KF5KCMUtils PROPERTIES
                      VERSION ${KCMUTILS_VERSION_STRING}
                      SOVERSION ${KCMUTILS_SOVERSION}
                      EXPORT_NAME KCMUtils)

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_export.h
 kcmoduleinfo.h
 kcmoduleloader.h
 kcmultidialog.h
 kcmoduleproxy.h
 kpluginselector.h
 kcmodulecontainer.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(FILES
        ksettings/dispatcher.h
        ksettings/dialog.h
        ksettings/pluginpage.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/ksettings
        COMPONENT Devel)

install(TARGETS KF5KCMUtils EXPORT KF5KCMUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 kcmodule.desktop
 kcmoduleinit.desktop
DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
