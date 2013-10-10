
cmake_minimum_required(VERSION 2.8.12)

project(KService)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs KCrash)

include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KSERVICE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kservice_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfigVersion.cmake")

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_STRICT_ITERATORS)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS DBus Test Xml)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KService")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KServiceConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
                             )

install(EXPORT KServiceTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KServiceTargets.cmake NAMESPACE KF5:: )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KServiceConfigVersion.cmake"
              "${CMAKE_CURRENT_SOURCE_DIR}/KServiceMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kservice_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(kbuildsycoca)
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)
