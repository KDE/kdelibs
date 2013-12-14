cmake_minimum_required(VERSION 2.8.11)

project(KJsEmbed)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2")
set(KF5_VERSION "5.0")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED UiTools Widgets Xml Svg)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KJSEMBED
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kjsembed_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5JsEmbedConfigVersion.cmake")

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KF5JS ${KF5_VERSION} REQUIRED)
  find_package(KF5I18n ${KF5_VERSION} REQUIRED)
  find_package(KF5DocTools ${KF5_VERSION} REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KF5DocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
endif()

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(examples/kjsconsole)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5JsEmbed")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5JsEmbedConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5JsEmbedConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5JsEmbedConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5JsEmbedConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5JsEmbedTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5JsEmbedTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kjsembed_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
