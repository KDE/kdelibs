cmake_minimum_required(VERSION 2.8.12)

project(KWindowSystem)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KWINDOWSYSTEM
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5WindowSystemConfigVersion.cmake")

# Dependencies
set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

if(NOT WIN32)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED X11Extras)
endif()

if (APPLE)
   find_package(Carbon REQUIRED)
endif ()

if(WIN32)
   find_package(Qt5WinExtras REQUIRED)
endif()

find_package(X11)
set(HAVE_X11 ${X11_FOUND})
find_package(XCB COMPONENTS XCB KEYSYMS)
if(X11_FOUND)
    set_package_properties(X11_Xrender PROPERTIES DESCRIPTION "X Rendering Extension (libXrender)"
                           URL "http://www.x.org" TYPE RECOMMENDED
                           PURPOSE "Support for compositing, rendering operations, and alpha-blending")
    set(HAVE_XRENDER ${X11_Xrender_FOUND})
    set(HAVE_XFIXES ${X11_Xfixes_FOUND})
endif()

# Subdirectories
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5WindowSystem")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5WindowSystemConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5WindowSystemConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5WindowSystemConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5WindowSystemConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5WindowSystemTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5WindowSystemTargets.cmake NAMESPACE KF5:: )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
