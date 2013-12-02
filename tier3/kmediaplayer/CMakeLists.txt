cmake_minimum_required(VERSION 2.8.12)

project(KMediaPlayer)


#
# CMake stuff from ECM
#
find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(ECMSetupVersion)


#
# Version info
#
ecm_setup_version(5.0.0
   VARIABLE_PREFIX KMEDIAPLAYER
   VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kmediaplayer_version.h"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KMediaPlayerConfigVersion.cmake")
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
   find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
   find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
   find_package(KService ${KF5_VERSION} REQUIRED)
   find_package(KCompletion ${KF5_VERSION} REQUIRED)
   find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
   find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
   find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
   find_package(KIconThemes ${KF5_VERSION} REQUIRED)
   find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
   find_package(KF5Solid ${KF5_VERSION} REQUIRED)
   find_package(KBookmarks ${KF5_VERSION} REQUIRED)
   find_package(KNotifications ${KF5_VERSION} REQUIRED)
   find_package(KIO ${KF5_VERSION} REQUIRED)

   find_package(KI18n ${KF5_VERSION} REQUIRED)
   find_package(KParts ${KF5_VERSION} REQUIRED)
   find_package(XmlGui ${KF5_VERSION} REQUIRED)
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
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KMediaPlayer")
configure_package_config_file(
   "${CMAKE_CURRENT_SOURCE_DIR}/KMediaPlayerConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/KMediaPlayerConfig.cmake"
   INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KMediaPlayerConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KMediaPlayerConfig.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KMediaPlayerTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KMediaPlayerTargets.cmake
        NAMESPACE KF5::)


#
# Feature summary
#
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
