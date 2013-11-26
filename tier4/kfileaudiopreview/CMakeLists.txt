cmake_minimum_required(VERSION 2.8.12)

project(KFileAudioPreview)


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
# Dependencies
#
set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5DBus ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)
find_package(Qt5Widgets ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)
find_package(Phonon4Qt5 4.6.60 REQUIRED NO_MODULE)
set_package_properties(Phonon4Qt5 PROPERTIES
   DESCRIPTION "Qt-based audio library"
   TYPE REQUIRED)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
   # Due to a CMake bug, we need to explicitly find private dependencies of our dependencies
   # Remove when we depend on CMake 3.0.0
   find_package(KGuiAddons ${KF5_VERSION} REQUIRED)
   find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
   find_package(KDBusAddons ${KF5_VERSION} REQUIRED)
   find_package(KService ${KF5_VERSION} REQUIRED)
   find_package(KCompletion ${KF5_VERSION} REQUIRED)
   find_package(KWindowSystem ${KF5_VERSION} REQUIRED)
   find_package(ItemViews ${KF5_VERSION} REQUIRED)
   find_package(KWidgetsAddons ${KF5_VERSION} REQUIRED)
   find_package(KIconThemes ${KF5_VERSION} REQUIRED)
   find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
   find_package(Solid ${KF5_VERSION} REQUIRED)
   find_package(KBookmarks ${KF5_VERSION} REQUIRED)
   find_package(KNotifications ${KF5_VERSION} REQUIRED)

   find_package(KI18n ${KF5_VERSION} REQUIRED)
   find_package(KIO ${KF5_VERSION} REQUIRED)
endif ()


#
# Subdirectories
#
include_directories (
   ${CMAKE_CURRENT_SOURCE_DIR}/src
   ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)

#
# Feature summary
#
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
