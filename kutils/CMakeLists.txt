project(kutils)
include_directories( ${KDE4_KDEUI_INCLUDES}
                     ${KDE4_KPARTS_INCLUDES}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
                     ${karchive_SOURCE_DIR}/src
                     ${karchive_BINARY_DIR}/src
                     ${kwidgetsaddons_SOURCE_DIR}/src
                     ${kwidgetsaddons_BINARY_DIR}/src
                     ${CMAKE_SOURCE_DIR}/tier2/kauth/src/
                     ${CMAKE_BINARY_DIR}/tier2/kauth/src/
                     ${ki18n_SOURCE_DIR}/src
                     ${ki18n_BINARY_DIR}/src
)

# kcmoduleproxy.cpp:219:8: error: ‘class QApplication’ has no member named ‘syncX’
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

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

kde4_add_library(kcmutils ${LIBRARY_TYPE} ${kcmutils_LIB_SRCS})
generate_export_header(kcmutils)

target_link_libraries(kcmutils ${KDE4_KDEUI_LIBS} kauth kcoreaddons kwidgetsaddons ki18n)
target_link_libraries(kcmutils LINK_INTERFACE_LIBRARIES kdecore kdeui Qt5::Widgets)
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

kde4_add_library(kprintutils ${LIBRARY_TYPE} ${kprintutils_LIB_SRCS})
generate_export_header(kprintutils)

target_link_libraries(kprintutils ${KDE4_KPARTS_LIBS} kcoreaddons)
target_link_libraries(kprintutils LINK_INTERFACE_LIBRARIES kdecore kdeui Qt5::Widgets)
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

kde4_add_library(kutils ${LIBRARY_TYPE} ${kutils_LIB_SRCS})

target_link_libraries(kutils kidletime kcmutils kprintutils)
target_link_libraries(kutils LINK_INTERFACE_LIBRARIES kdecore kdeui Qt5::Widgets kidletime kcmutils kprintutils)
set_target_properties(kutils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

