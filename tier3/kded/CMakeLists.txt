cmake_minimum_required(VERSION 2.8.12)

project(KDED)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Xml Widgets DBus Network Test)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

set(KF5_VERSION "5.0.0")
if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    find_package(KF5I18n ${KF5_VERSION} REQUIRED) #
    find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5Service ${KF5_VERSION} REQUIRED)
    find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED) #
    find_package(KF5Crash ${KF5_VERSION} REQUIRED)
    find_package(KF5Init ${KF5_VERSION} REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KF5DocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
endif()

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KDED
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kded_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDEDConfigVersion.cmake"
)


remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(docs)
add_subdirectory(src)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDED")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KDEDConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KDEDConfig.cmake"
  INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KDEDConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KDEDConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT
  KDEDTargets
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  FILE KDEDTargets.cmake
  NAMESPACE KF5::
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/src/kded_version.h"
  DESTINATION "${INCLUDE_INSTALL_DIR}"
  COMPONENT Devel
)


