cmake_minimum_required(VERSION 2.8.12)

project(KDEWebKit)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Widgets WebKitWidgets Xml Network Test)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)

include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KDEWebKit
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdewebkit_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDEWebKitConfigVersion.cmake")

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  #dependencies of dependencies, remove when depending on cmake 3.0.0
  find_package(KI18n ${KF5_VERSION} REQUIRED)
  find_package(KDBusAddons ${KF5_VERSION} REQUIRED)
  find_package(KWidgetsAddons ${KF5_VERSION} REQUIRED)
  find_package(KCoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KGuiAddons ${KF5_VERSION} REQUIRED)
  find_package(KService ${KF5_VERSION} REQUIRED)
  find_package(KWindowSystem ${KF5_VERSION} REQUIRED)
  find_package(ItemViews ${KF5_VERSION} REQUIRED)
  find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
  find_package(KIconThemes ${KF5_VERSION} REQUIRED)
  find_package(Solid ${KF5_VERSION} REQUIRED)
  find_package(KCompletion ${KF5_VERSION} REQUIRED)
  find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
  find_package(XmlGui ${KF5_VERSION} REQUIRED)
  find_package(KBookmarks ${KF5_VERSION} REQUIRED)

  #actual dependencies
  find_package(KCoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KWallet ${KF5_VERSION} REQUIRED)
  find_package(KIO ${KF5_VERSION} REQUIRED)
  find_package(KParts ${KF5_VERSION} REQUIRED)
endif()

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDEWebKit")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KDEWebKitConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KDEWebKitConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KDEWebKitConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KDEWebKitConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KDEWebKitTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDEWebKitTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kdewebkit_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
