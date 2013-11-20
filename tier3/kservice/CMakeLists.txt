
cmake_minimum_required(VERSION 2.8.12)

project(KService)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")

ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX KSERVICE
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kservice_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfigVersion.cmake")

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus Test Xml)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KWindowSystem ${KF5_VERSION} REQUIRED)
  find_package(KConfig ${KF5_VERSION} REQUIRED)
  find_package(KCoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KCrash ${KF5_VERSION} REQUIRED)
  find_package(KDBusAddons ${KF5_VERSION} REQUIRED)
  find_package(KI18n ${KF5_VERSION} REQUIRED)
  find_package(KDocTools REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KDocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KService")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KServiceConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                             )

install(EXPORT KServiceTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KServiceTargets.cmake NAMESPACE KF5:: )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfigVersion.cmake"
              "${CMAKE_CURRENT_SOURCE_DIR}/KServiceMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kservice_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)
