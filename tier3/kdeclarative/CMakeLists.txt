cmake_minimum_required(VERSION 2.8.12)

project(KDeclarative)

# Dependencies
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2.0")
set(KF5_VERSION "5.0.0")

find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Qml Quick Widgets Script)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  # Required KF5 frameworks to build this framework
  # For example:
  find_package(KF5Config ${KF5_VERSION} REQUIRED)
  find_package(KF5I18n ${KF5_VERSION} REQUIRED)
  find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
  find_package(KIO ${KF5_VERSION} REQUIRED)
endif()

# Includes
include(FeatureSummary)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)

# ECM setup
ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KDECLARATIVE
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdeclarative_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5DeclarativeConfigVersion.cmake"
)

# Subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Declarative")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5DeclarativeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5DeclarativeConfig.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5DeclarativeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5DeclarativeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KF5DeclarativeTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KF5DeclarativeTargets.cmake
  NAMESPACE KF5::
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/kdeclarative_version.h"
  DESTINATION "${INCLUDE_INSTALL_DIR}"
  COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
