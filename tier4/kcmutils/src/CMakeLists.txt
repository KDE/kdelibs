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

add_library(KCMUtils ${kcmutils_LIB_SRCS})
generate_export_header(KCMUtils)

target_link_libraries(KCMUtils
  PUBLIC
    Qt5::Widgets
    KConfigWidgets  # KCModule
    KF5::KService        # KService
  PRIVATE
    KF5::KCoreAddons
    KF5::KI18n
    KF5::KIconThemes     # KIconLoader
    ItemViews       # KWidgetItemDelegate
    XmlGui          # KAboutApplicationDialog
)
set_target_properties(KCMUtils PROPERTIES
                      VERSION ${KCMUTILS_VERSION_STRING}
                      SOVERSION ${KCMUTILS_SOVERSION})

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

install(TARGETS KCMUtils EXPORT KCMUtilsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 kcmodule.desktop
 kcmoduleinit.desktop
DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
