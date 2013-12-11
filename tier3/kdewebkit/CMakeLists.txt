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
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5WebKitConfigVersion.cmake")

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  #dependencies of dependencies, remove when depending on cmake 3.0.0
  find_package(KF5I18n ${KF5_VERSION} REQUIRED)
  find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5Service ${KF5_VERSION} REQUIRED)
  find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
  find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
  find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
  find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
  find_package(KF5Solid ${KF5_VERSION} REQUIRED)
  find_package(KF5Completion ${KF5_VERSION} REQUIRED)
  find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
  find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
  find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)

  #actual dependencies
  find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5Wallet ${KF5_VERSION} REQUIRED)
  find_package(KF5KIO ${KF5_VERSION} REQUIRED)
  find_package(KF5Notifications ${KF5_VERSION} REQUIRED)
  find_package(KF5Parts ${KF5_VERSION} REQUIRED)
endif()

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5WebKit")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5WebKitConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5WebKitConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5WebKitConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5WebKitConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5WebKitTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5WebKitTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kdewebkit_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
