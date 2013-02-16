cmake_minimum_required(VERSION 2.8.10.1)

project(kdbusaddons)

find_package(ECM 0.0.7 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS DBus)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KDBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kdbusaddonsConfigVersion.cmake")



add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kdbusaddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/kdbusaddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kdbusaddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/kdbusaddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kdbusaddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT kdbusaddonsTargets  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  FILE kdbusaddonsTargets.cmake  NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_version.h  DESTINATION ${INCLUDE_INSTALL_DIR}  COMPONENT Devel )
