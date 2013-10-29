cmake_minimum_required(VERSION 2.8.12)

project(KCompletion)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)

set(KF5_VERSION "5.0.0")

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KCOMPLETION
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcompletion_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KCompletionConfigVersion.cmake"
)

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets Xml)
find_package(KF5 ${KF5_VERSION} REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KConfig ${KF5_VERSION} REQUIRED)
  find_package(KWidgetsAddons ${KF5_VERSION} REQUIRED)
endif()

remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KCompletion")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KCompletionConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KCompletionConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KCompletionConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KCompletionConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KCompletionTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KCompletionTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kcompletion_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
