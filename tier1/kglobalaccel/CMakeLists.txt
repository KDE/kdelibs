cmake_minimum_required(VERSION 2.8.12)

project(KGlobalAccel)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KGLOBALACCEL
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kglobalaccel_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5GlobalAccelConfigVersion.cmake")

# Dependencies
set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus Widgets)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

find_package(X11)
set(HAVE_X11 ${X11_FOUND})
if(HAVE_X11)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED X11Extras)
endif()

# Subdirectories
add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5GlobalAccel")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5GlobalAccelConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5GlobalAccelConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5GlobalAccelConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5GlobalAccelConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5GlobalAccelTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5GlobalAccelTargets.cmake NAMESPACE KF5:: )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kglobalaccel_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
