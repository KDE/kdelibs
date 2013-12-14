cmake_minimum_required(VERSION 2.8.12)

project(KParts)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Widgets Xml Network Test)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)

include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KPARTS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kparts_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PartsConfigVersion.cmake")

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  #NOTE: here the dependencies of dependencies are displayed as well, this shouldn't be needed
  #after cmake 3.0.0 is released
    find_package(KF5I18n ${KF5_VERSION} REQUIRED)
    find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5Service ${KF5_VERSION} REQUIRED)
    find_package(KF5Completion ${KF5_VERSION} REQUIRED)
    find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
    find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
    find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
    find_package(KF5Solid ${KF5_VERSION} REQUIRED)
    find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)

    find_package(KF5KIO ${KF5_VERSION} REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
    find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5Notifications ${KF5_VERSION} REQUIRED)
endif()

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory( tests )
add_subdirectory( autotests )
add_subdirectory( src )

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Parts")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PartsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PartsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PartsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PartsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5PartsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5PartsTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kparts_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
