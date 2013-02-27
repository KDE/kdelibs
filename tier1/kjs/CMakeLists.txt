cmake_minimum_required(VERSION 2.8.10.1)

project(kjs)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test Widgets)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

#Do not make PCRE optional here. PCRE is a hard requirement for modern systems
#but we give old systems some slack... that's why we don't specify "REQUIRED".
find_package(PCRE)

set_package_properties(PCRE PROPERTIES
                       URL "http://www.pcre.org"
                       DESCRIPTION "Perl-compatible regular expressions in KJS"
                       TYPE OPTIONAL
                       PURPOSE "Without PCRE, KJS will have extremely poor regular expression support, breaking many webpages.")

option(KJS_FORCE_DISABLE_PCRE "Force building of KJS without PCRE. Doing this will result in many webpage working incorrectly, due to extremely poor regular expression support")



ecm_setup_version(5 0 0 VARIABLE_PREFIX KJS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kjs_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kjsConfigVersion.cmake")

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}        # kjs_version.h
  ${CMAKE_CURRENT_BINARY_DIR}/src    # kjs/global.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src    # kjs/* and wtf/*
  ${CMAKE_CURRENT_BINARY_DIR}/src/kjs
  ${CMAKE_CURRENT_SOURCE_DIR}/src/kjs
)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kjs")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/kjsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kjsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/kjsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kjsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT kjsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE kjsTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kjs_version.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
