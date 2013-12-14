cmake_minimum_required(VERSION 2.8.12)

project(KCMUtils)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5 ${QT_REQUIRED_VERSION} NO_MODULE REQUIRED Widgets DBus)

include(FeatureSummary)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KCMUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfigVersion.cmake")

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5I18n ${KF5_VERSION} REQUIRED)
    find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
    find_package(KF5Service ${KF5_VERSION} REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
endif()

add_subdirectory(src)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5KCMUtils")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5KCMUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KCMUtilsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KCMUtilsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KCMUtilsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kcmutils_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
