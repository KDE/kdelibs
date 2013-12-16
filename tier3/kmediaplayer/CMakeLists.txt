cmake_minimum_required(VERSION 2.8.12)

project(KMediaPlayer)


#
# CMake stuff from ECM
#
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")

#
# Version info
#
ecm_setup_version(${KF5_VERSION}
   VARIABLE_PREFIX KMEDIAPLAYER
   VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmediaplayer_version.h"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5MediaPlayerConfigVersion.cmake"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kmediaplayer_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR}
        COMPONENT Devel)


#
# Dependencies
#
set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5DBus ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)
find_package(Qt5Widgets ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
   # Due to a CMake bug, we need to explicitly find private dependencies of our dependencies
   # Remove when we depend on CMake 3.0.0
   find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
   find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
   find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
   find_package(KF5Service ${KF5_VERSION} REQUIRED)
   find_package(KF5Completion ${KF5_VERSION} REQUIRED)
   find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
   find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
   find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
   find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
   find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
   find_package(KF5Solid ${KF5_VERSION} REQUIRED)
   find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)
   find_package(KF5Notifications ${KF5_VERSION} REQUIRED)
   find_package(KF5KIO ${KF5_VERSION} REQUIRED)

   find_package(KF5I18n ${KF5_VERSION} REQUIRED)
   find_package(KF5Parts ${KF5_VERSION} REQUIRED)
   find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
endif ()


#
# Subdirectories
#
include_directories (
   ${CMAKE_CURRENT_SOURCE_DIR}/src
   ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)
add_subdirectory(autotests)


#
# Config.cmake stuff
#
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5MediaPlayer")
configure_package_config_file(
   "${CMAKE_CURRENT_SOURCE_DIR}/KF5MediaPlayerConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/KF5MediaPlayerConfig.cmake"
   INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KF5MediaPlayerConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KF5MediaPlayerConfig.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KF5MediaPlayerTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF5MediaPlayerTargets.cmake
        NAMESPACE KF5::)


#
# Feature summary
#
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
