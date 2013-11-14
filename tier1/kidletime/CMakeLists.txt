cmake_minimum_required(VERSION 2.8.12)

project(KIdleTime)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KIDLETIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidletime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KIdleTimeConfigVersion.cmake")

# Dependencies
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

if(NOT WIN32)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED X11Extras DBus)
endif()

if (APPLE)
   find_package(Carbon REQUIRED)
endif ()

find_package(X11)
find_package(XCB COMPONENTS XCB SYNC)
find_package(X11_XCB)

set(HAVE_X11 ${X11_FOUND})
if(X11_FOUND)
    add_feature_info("X Screensaver Extension (libXss)" X11_Xscreensaver_FOUND "Support for KIdleTime (fallback mode)")
    add_feature_info("X Sync Extension (libXext)" X11_XSync_FOUND "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

    add_feature_info("X Screensaver Extension (libXss)" X11_Xscreensaver_FOUND "Support for KIdleTime (fallback mode)")
    add_feature_info("X Sync Extension (libXext)" X11_XSync_FOUND "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

    if(NOT X11_XSync_FOUND AND NOT X11_Xscreensaver_FOUND)
        message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)\n")
    endif()

    set(HAVE_XSCREENSAVER ${X11_Xscreensaver_FOUND})
    set(HAVE_XTEST ${X11_XTest_FOUND})
endif()

# Subdirectories
add_subdirectory(src)
add_subdirectory(examples)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KIdleTime")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KIdleTimeConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KIdleTimeConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KIdleTimeConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KIdleTimeConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KIdleTimeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KIdleTimeTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kidletime_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
