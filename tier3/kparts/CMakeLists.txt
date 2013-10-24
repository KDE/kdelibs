cmake_minimum_required(VERSION 2.8.12)

project(KParts)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core Widgets Xml Network Test)
find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KPARTS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kparts_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPartsConfigVersion.cmake")

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)

add_subdirectory( tests )
add_subdirectory( autotests )
add_subdirectory( src )

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KParts")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPartsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPartsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPartsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPartsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPartsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KPartsTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kparts_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
