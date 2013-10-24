cmake_minimum_required(VERSION 2.8.12)

project(KWindowSystem)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KWINDOWSYSTEM
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KWindowSystemConfigVersion.cmake")

# Dependencies
set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

if(NOT WIN32)
    find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

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
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KWindowSystem")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KWindowSystemConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KWindowSystemConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KWindowSystemConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KWindowSystemConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KWindowSystemTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KWindowSystemTargets.cmake NAMESPACE KF5:: )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
