cmake_minimum_required(VERSION 2.8.12)

project(Kross)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KROSS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kross_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KrossConfigVersion.cmake")

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Test Script Xml Widgets UiTools)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    #NOTE: here the dependencies of dependencies are displayed as well, this shouldn't be needed
    #after cmake 3.0.0 is released
    find_package(KWidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(ItemViews ${KF5_VERSION} REQUIRED)
    find_package(KI18n ${KF5_VERSION} REQUIRED)
    find_package(KGuiAddons ${KF5_VERSION} REQUIRED)
    find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KIconThemes ${KF5_VERSION} REQUIRED)
    find_package(KCompletion ${KF5_VERSION} REQUIRED)
    find_package(KDBusAddons ${KF5_VERSION} REQUIRED)
    find_package(KService ${KF5_VERSION} REQUIRED)
    find_package(KWindowSystem ${KF5_VERSION} REQUIRED)
    find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
    find_package(XmlGui ${KF5_VERSION} REQUIRED)
    find_package(KBookmarks ${KF5_VERSION} REQUIRED)

#     find_package(KI18n ${KF5_VERSION} REQUIRED)
#     find_package(KIconThemes ${KF5_VERSION} REQUIRED)
    find_package(KIO ${KF5_VERSION} REQUIRED)
    find_package(KParts ${KF5_VERSION} REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KDocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(docs)
add_subdirectory(src)
#add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/Kross")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KrossConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KrossConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kross_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KrossConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KrossConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KrossTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KrossTargets.cmake NAMESPACE KF5::)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
