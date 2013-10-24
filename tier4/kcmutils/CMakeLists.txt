cmake_minimum_required(VERSION 2.8.12)

project(KCMUtils)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KCMUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KCMUtilsConfigVersion.cmake")

add_subdirectory(src)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KCMUtils")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KCMUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KCMUtilsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KCMUtilsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KCMUtilsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KCMUtilsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KCMUtilsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
