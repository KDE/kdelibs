cmake_minimum_required(VERSION 2.8.12)

project(KDEWidgets)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    find_package(ItemViews 5.0.0 REQUIRED)
    find_package(KI18n 5.0.0 REQUIRED)
    find_package(KGuiAddons 5.0.0 REQUIRED)
    find_package(KConfigWidgets 5.0.0 REQUIRED)
    find_package(KIconThemes 5.0.0 REQUIRED)
    find_package(KWidgetsAddons 5.0.0 REQUIRED)
    find_package(KCompletion 5.0.0 REQUIRED)
    find_package(KDBusAddons 5.0.0 REQUIRED)
    find_package(KService 5.0.0 REQUIRED)
    find_package(KWindowSystem 5.0.0 REQUIRED)
    find_package(KTextWidgets 5.0.0 REQUIRED)
    find_package(XmlGui 5.0.0 REQUIRED)
    find_package(KPlotting 5.0.0 REQUIRED)
    find_package(KBookmarks 5.0.0 REQUIRED)

    find_package(KDocTools 5.0.0 REQUIRED)
    find_package(KIO 5.0.0 REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KDocToolsMacros.cmake)
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDEWidgets")

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(tests)

install( FILES
    KDEWidgetsMacros.cmake KDEWidgetsConfig.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

install(EXPORT KDEWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDEWidgetsTargets.cmake NAMESPACE KF5:: )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
