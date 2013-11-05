project(KPty)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION 5.2)
set(KF5_VERSION "5.0.0")
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Test)

if(NOT kdelibs_SOURCE_DIR)
  find_package(KCoreAddons 5.0.0 REQUIRED)
  find_package(KI18n 5.0.0 REQUIRED)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KPty")

# Includes
include(FeatureSummary)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KPTY
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kpty_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPtyConfigVersion.cmake"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPtyConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPtyConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

add_subdirectory( src )
add_subdirectory( tests )

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPtyConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPtyConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPtyTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KPtyTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kpty_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
