project(kded)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Xml Widgets DBus Network Test)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KDED
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kded_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDEDConfigVersion.cmake"
)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS DBus)
endif(NOT WIN32)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

########### next target ###############

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


