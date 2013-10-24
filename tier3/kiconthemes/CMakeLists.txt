cmake_minimum_required(VERSION 2.8.12)

project(KIconThemes)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5Svg 5.2.0 REQUIRED NO_MODULE)
find_package(Qt5DBus 5.2.0 REQUIRED NO_MODULE)
find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5Widgets ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KICONTHEMES
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kiconthemes_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KIconThemesConfigVersion.cmake")

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KIconThemes")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KIconThemesConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KIconThemesConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KIconThemesConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KIconThemesConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KIconThemesTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KIconThemesTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kiconthemes_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
