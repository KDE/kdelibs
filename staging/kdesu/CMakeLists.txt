cmake_minimum_required(VERSION 2.8.11)

project(KDESu)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)
if(NOT kdelibs_SOURCE_DIR)
  find_package(KCoreAddons 5.0.0 REQUIRED)
  find_package(KService 5.0.0 REQUIRED)
  find_package(KPty 5.0.0 REQUIRED)
endif()

#optional features
find_package(X11)
set(HAVE_X11 ${X11_FOUND})

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KDESU
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdesu_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDESuConfigVersion.cmake")

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDESu")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KDESuConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KDESuConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KDESuConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KDESuConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KDESuTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDESuTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kdesu_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
