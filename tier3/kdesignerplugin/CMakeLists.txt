cmake_minimum_required(VERSION 2.8.12)

project(KF5DesignerPlugin)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core)

set(KF5_VERSION 5.0.0)
if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
    find_package(KF5I18n ${KF5_VERSION} REQUIRED)
    find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
    find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5Completion ${KF5_VERSION} REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5Service ${KF5_VERSION} REQUIRED)
    find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
    find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
    find_package(KF5Plotting ${KF5_VERSION} REQUIRED)
    find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)

    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
    find_package(KF5KIO ${KF5_VERSION} REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KF5DocToolsMacros.cmake)
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5DesignerPlugin")

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(tests)

include(ECMSetupVersion)
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX kf5designerplugin
                                 VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kf5designerplugin_version.h"
                                 PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5DesignerPluginConfigVersion.cmake")

install( FILES
    KF5DesignerPluginMacros.cmake KF5DesignerPluginConfig.cmake "${CMAKE_CURRENT_BINARY_DIR}/KF5DesignerPluginConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

install(EXPORT KF5DesignerPluginTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5DesignerPluginTargets.cmake NAMESPACE KF5:: )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
