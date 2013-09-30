cmake_minimum_required(VERSION 2.8.11)

project(KIdleTime)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets DBus)

if(NOT WIN32)
    find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

find_package(X11)
find_package(XCB COMPONENTS XCB SYNC)
find_package(X11_XCB)

macro_bool_to_01(X11_FOUND HAVE_X11)
if(X11_FOUND)
    add_feature_info("X Screensaver Extension (libXss)" HAVE_XSCREENSAVER "Support for KIdleTime (fallback mode)")
    add_feature_info("X Sync Extension (libXext)" HAVE_XSYNC "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

    macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
    macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC)
    add_feature_info("X Screensaver Extension (libXss)" HAVE_XSCREENSAVER "Support for KIdleTime (fallback mode)")
    add_feature_info("X Sync Extension (libXext)" HAVE_XSYNC "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

    if(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)
        message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)\n")
    endif(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)
    
    macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
    macro_bool_to_01(X11_XTest_FOUND HAVE_XTEST)

endif()

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KIDLETIME
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kidletime_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KIdleTimeConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(examples)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KIdleTime")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KIdleTimeConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KIdleTimeConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

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
