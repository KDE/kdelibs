cmake_minimum_required(VERSION 2.8.12)

project(KWidgetsAddons)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KWIDGETSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwidgetsaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5WidgetsAddonsConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5WidgetsAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5WidgetsAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5WidgetsAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5WidgetsAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5WidgetsAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5WidgetsAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5WidgetsAddonsTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgetsaddons_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
