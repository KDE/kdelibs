cmake_minimum_required(VERSION 2.8.12)

project(KDBusAddons)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5DBus ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KDBUSADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5DBusAddonsConfigVersion.cmake")



add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5DBusAddons")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5DBusAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5DBusAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5DBusAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5DBusAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5DBusAddonsTargets  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  FILE KF5DBusAddonsTargets.cmake  NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdbusaddons_version.h  DESTINATION ${INCLUDE_INSTALL_DIR}  COMPONENT Devel )
