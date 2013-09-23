cmake_minimum_required(VERSION 2.8.11)

project(KJS)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

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
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KJSConfigVersion.cmake")

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KJS")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KJSConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KJSConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KJSConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KJSConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KJSTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KJSTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kjs_version.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
