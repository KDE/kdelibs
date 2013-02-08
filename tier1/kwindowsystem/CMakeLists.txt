cmake_minimum_required(VERSION 2.8.10.1)

project(kwindowsystem)

find_package(ECM 0.0.6 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets Test DBus)

if(NOT kdelibs_SOURCE_DIR)
  find_package(kdeqt5staging REQUIRED NO_MODULE)
endif()

include_directories(${kdeqt5staging_INCLUDE_DIRS})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KWINDOWSYSTEM
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kwindowsystemConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kwindowsystem")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/kwindowsystemConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kwindowsystemConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/kwindowsystemConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kwindowsystemConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT kwindowsystemTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE kwindowsystemTargets.cmake NAMESPACE KF5:: )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
