cmake_minimum_required(VERSION 2.8.12)

project(KFileAudioPreview)


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

   find_package(KF5I18n ${KF5_VERSION} REQUIRED)
   find_package(KF5KIO ${KF5_VERSION} REQUIRED)
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
