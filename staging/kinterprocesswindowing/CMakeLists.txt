cmake_minimum_required(VERSION 2.8.11)

project(KInterProcessWindowing)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KINTERPROCESSWINDOWING
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kinterprocesswindowing_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KInterProcessWindowingConfigVersion.cmake"
                 )

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMQtFramework)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Test)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)

add_subdirectory(src)
add_subdirectory(tests)

include(FeatureSummary)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KInterProcessWindowing")

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/KInterProcessWindowingConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KInterProcessWindowingConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR LIB_INSTALL_DIR CMAKE_INSTALL_PREFIX)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KInterProcessWindowingConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KInterProcessWindowingConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KInterProcessWindowingTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KInterProcessWindowingTargets.cmake NAMESPACE KF5::)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kinterprocesswindowing_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
