cmake_minimum_required(VERSION 2.8.10.20130309)

project(KDBusAddons)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5DBus 5.2.0 REQUIRED NO_MODULE)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KDBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDBusAddonsConfigVersion.cmake")



add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDBusAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KDBusAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KDBusAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KDBusAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KDBusAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KDBusAddonsTargets  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  FILE KDBusAddonsTargets.cmake  NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_version.h  DESTINATION ${INCLUDE_INSTALL_DIR}  COMPONENT Devel )
