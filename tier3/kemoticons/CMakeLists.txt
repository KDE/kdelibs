cmake_minimum_required(VERSION 2.8.12)

project(KEmoticons)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)

set(KF5_VERSION "5.0.0")

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KEMOTICONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kemoticons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KEmoticonsConfigVersion.cmake")

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  #NOTE: here the dependencies of dependencies are displayed as well, this shouldn't be needed
  #after cmake 3.0.0 is released
  find_package(KCoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KDBusAddons ${KF5_VERSION} REQUIRED)
  find_package(KI18n ${KF5_VERSION} REQUIRED)

  find_package(KArchive ${KF5_VERSION} REQUIRED)
  find_package(KService ${KF5_VERSION} REQUIRED)
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kemoticons_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )


# the strings throughout this library are used to search ressources
# so they don't need translations
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KEmoticons")

install(EXPORT KEmoticonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KEmoticonsTargets.cmake NAMESPACE KF5:: )

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KEmoticonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KEmoticonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                             )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KEmoticonsConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KEmoticonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

