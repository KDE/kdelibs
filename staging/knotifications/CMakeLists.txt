cmake_minimum_required(VERSION 2.8.11)

project(KNotifications)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Widgets X11Extras)


include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KNOTIFICATIONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knotifications_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KNotificationsConfigVersion.cmake")

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KNotifications")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KNotificationsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KNotificationsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KNotificationsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KNotificationsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KNotificationsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KNotificationsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/knotifications_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
