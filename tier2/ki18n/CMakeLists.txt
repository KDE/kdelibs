cmake_minimum_required(VERSION 2.8.12)

project(KI18n)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")

ecm_setup_version(
  ${KF5_VERSION}
  VARIABLE_PREFIX KI18N
  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ki18n_version.h"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5I18nConfigVersion.cmake"
)

# Dependencies
set(REQUIRED_QT_VERSION "5.2.0")

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

find_package(Perl)
set_package_properties(Perl PROPERTIES URL "http://www.perl.org" TYPE REQUIRED PURPOSE "Needed for building kdelibs")

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KF5JS ${KF5_VERSION} REQUIRED)
else()
  set(KJS_CREATE_HASH_TABLE "${CMAKE_SOURCE_DIR}/tier1/kjs/src/kjs/create_hash_table")
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5I18n")

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/KF5I18nConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5I18nConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR LIB_INSTALL_DIR CMAKE_INSTALL_PREFIX)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5I18nConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5I18nConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KF5I18nTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5I18nTargets.cmake NAMESPACE KF5::)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ki18n_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install( FILES
    cmake/KF5I18NMacros.cmake
    cmake/kf5i18nuic.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
