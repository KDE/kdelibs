cmake_minimum_required(VERSION 2.8.10.1)

project(kauth)

if(NOT kdelibs_SOURCE_DIR)

  find_package(ECM 0.0.7 REQUIRED NO_MODULE)

  set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

  find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets DBus Concurrent Test)

  # once we can build tier2 libs standalone, add "kcoreaddons" component
  find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs kcoreaddons)

endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)

include(ECMSetupVersion)
ecm_setup_version(5 0 0 VARIABLE_PREFIX KAUTH
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kauthConfigVersion.cmake")

# qInstallMsgHandler
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)


include_directories(${kcoreaddons_INCLUDE_DIRS})  #for kjob

add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kauth")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/kauthConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kauthConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/kauthConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kauthConfigVersion.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/cmake/KAuthMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT kauthTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE kauthTargets.cmake NAMESPACE KF5:: COMPONENT Devel)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kauth_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )


if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
