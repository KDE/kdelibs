cmake_minimum_required(VERSION 2.8.10.1)

project(kcodecs)

find_package(ECM 0.0.6 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KCODECS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcodecs_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kcodecsConfigVersion.cmake")

add_subdirectory(src)

add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kcodecs")

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/kcodecsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kcodecsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/kcodecsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kcodecsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT kcodecsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE kcodecsTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcodecs_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )


if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
