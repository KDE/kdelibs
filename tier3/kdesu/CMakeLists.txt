cmake_minimum_required(VERSION 2.8.12)

project(KDESu)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

set(KF5_VERSION "5.0.0")
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5Service ${KF5_VERSION} REQUIRED)
  find_package(KF5Pty ${KF5_VERSION} REQUIRED)
endif()

#optional features
find_package(X11)
set(HAVE_X11 ${X11_FOUND})

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KDESU
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdesu_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SuConfigVersion.cmake")

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Su")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5SuConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SuConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SuConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5SuConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5SuTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5SuTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kdesu_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
