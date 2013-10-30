cmake_minimum_required(VERSION 2.8.12)

project(FooBar)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)

set(KF5_VERSION "5.0.0")

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX FOOBAR
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/foobar_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/FooBarConfigVersion.cmake"
)

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

# Required Qt5 components to build this framework
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core)
find_package(KF5 ${KF5_VERSION} MODULE REQUIRED CMake Compiler InstallDirs)

# TODO: Add all required find_package() calls here

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  # KF5 frameworks
  #
  # Content of this block is only executed when building the framework
  # standalone. KF5 frameworks required to build this framework should be added
  # here. It is done this way to avoid chicken-and-egg problem when building
  # KF5 as a whole: one cannot depend on packages which have not been installed
  # yet. For example:
  # find_package(KCoreAddons ${KF5_VERSION} REQUIRED)
endif()

# Subdirectories
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/FooBar")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FooBarConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/FooBarConfig.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/FooBarConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/FooBarConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  FooBarTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE FooBarTargets.cmake
  NAMESPACE KF5::
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/foobar_version.h"
  DESTINATION "${INCLUDE_INSTALL_DIR}"
  COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
