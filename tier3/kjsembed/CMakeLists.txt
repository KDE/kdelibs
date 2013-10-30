cmake_minimum_required(VERSION 2.8.11)

project(KJsEmbed)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
set(KF5_VERSION "5.0")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED UiTools Widgets Xml Svg)
find_package(KF5 "${KF5_VERSION}" REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KJSEMBED
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kjsembed_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KJsEmbedConfigVersion.cmake")


add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(src)
add_subdirectory(examples/kjsconsole)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KJsEmbed")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KJsEmbedConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KJsEmbedConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KJsEmbedConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KJsEmbedConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KJsEmbedTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KJsEmbedTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kjsembed_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
