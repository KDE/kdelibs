cmake_minimum_required(VERSION 2.8.12)

project(KNotifyConfig)

# Dependencies
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2.0")
set(KF5_VERSION "5.0.0")

# Required Qt5 components to build this framework
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets DBus Test)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
find_package(Phonon4Qt5 4.6.60 NO_MODULE)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  # Due to a CMake bug, we need to explicitly find private dependencies of our dependencies
  # Remove when we depend on CMake 3.0.0 (which should fix this)
  find_package(KI18n ${KF5_VERSION} REQUIRED)
  find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
  find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
  find_package(KF5Solid ${KF5_VERSION} REQUIRED)
  find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
  find_package(KIconThemes ${KF5_VERSION} REQUIRED)
  find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
  find_package(KCompletion ${KF5_VERSION} REQUIRED)
  find_package(KService ${KF5_VERSION} REQUIRED)
  find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
  find_package(XmlGui ${KF5_VERSION} REQUIRED)
  find_package(KBookmarks ${KF5_VERSION} REQUIRED)

  #library dependencies
  find_package(KIO ${KF5_VERSION} REQUIRED)
  find_package(KI18n ${KF5_VERSION} REQUIRED)

  #test dependency
  find_package(KNotifications ${KF5_VERSION} REQUIRED)
endif()

# Includes
include(FeatureSummary)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)

# ECM setup
ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KNOTIFYCONFIG
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knotifyconfig_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KNotifyConfigConfigVersion.cmake"
)

# Subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KNotifyConfig")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KNotifyConfigConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KNotifyConfigConfig.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KNotifyConfigConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KNotifyConfigConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KNotifyConfigTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KNotifyConfigTargets.cmake
  NAMESPACE KF5::
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/knotifyconfig_version.h"
  DESTINATION "${INCLUDE_INSTALL_DIR}"
  COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
