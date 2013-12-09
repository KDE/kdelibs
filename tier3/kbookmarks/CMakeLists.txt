cmake_minimum_required(VERSION 2.8.12)

project(KBookmarks)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)

include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")

ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX KBOOKMARKS
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kbookmarks_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BookmarksConfigVersion.cmake")

# Dependencies
set(QT_REQUIRED_VERSION 5.2.0)

find_package(Qt5 ${QT_REQUIRED_VERSION} NO_MODULE REQUIRED Widgets Xml DBus)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
  find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
  find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
endif()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Bookmarks")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5BookmarksConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BookmarksConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BookmarksConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BookmarksConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5BookmarksTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5BookmarksTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kbookmarks_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
