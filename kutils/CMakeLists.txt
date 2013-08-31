project(kutils)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

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

add_library(kcmutils ${LIBRARY_TYPE} ${kcmutils_LIB_SRCS})
generate_export_header(kcmutils)

target_link_libraries(kcmutils
  LINK_PUBLIC
    Qt5::Widgets
    KConfigWidgets  # KCModule
    KService        # KService
  LINK_PRIVATE
    ${KAuth_LIBRARIES}
    ${KCoreAddons_LIBRARIES}
    ${KWidgetsAddons_LIBRARIES}
    KI18n
    KIconThemes     # KIconLoader
    ItemViews       # KWidgetItemDelegate
    XmlGui          # KAboutApplicationDialog
)
set_target_properties(kcmutils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

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

install(TARGETS kcmutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### kprintutils ###############
set(kprintutils_LIB_SRCS
    kprintpreview.cpp
)

add_library(kprintutils ${LIBRARY_TYPE} ${kprintutils_LIB_SRCS})
generate_export_header(kprintutils)

target_link_libraries(kprintutils
  LINK_PUBLIC
    Qt5::Widgets
    Qt5::PrintSupport
  LINK_PRIVATE
    ${KDE4_KPARTS_LIBS}
    ${KCoreAddons_LIBRARIES}
    KI18n
)
set_target_properties(kprintutils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kprintutils_export.h
 kprintpreview.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS kprintutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### kutils ###############
# This is only for backwards compatibility, don't use in new projects
if(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
set(kutils_LIB_SRCS
    dummy.cpp
)

add_library(kutils ${LIBRARY_TYPE} ${kutils_LIB_SRCS})

find_package(Qt5PrintSupport 5.2.0 REQUIRED NO_MODULE)

target_link_libraries(kutils kcmutils kprintutils)
target_link_libraries(kutils LINK_INTERFACE_LIBRARIES Qt5::Widgets Qt5::PrintSupport kcmutils kprintutils)
set_target_properties(kutils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

