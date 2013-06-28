cmake_minimum_required(VERSION 2.8.11)

project(KWidgets)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Svg X11Extras)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KWIDGETS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwidgets_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kwidgetsConfigVersion.cmake")


# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_STRICT_ITERATORS)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kwidgets")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/kwidgetsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kwidgetsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
                             )

install(EXPORT kwidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE kwidgetsTargets.cmake NAMESPACE KF5:: )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/kwidgetsConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/kwidgetsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
       )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
