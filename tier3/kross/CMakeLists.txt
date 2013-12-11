cmake_minimum_required(VERSION 2.8.12)

project(Kross)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KROSS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kross_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5KrossConfigVersion.cmake")

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Test Script Xml Widgets UiTools)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    #NOTE: here the dependencies of dependencies are displayed as well, this shouldn't be needed
    #after cmake 3.0.0 is released
    find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
    find_package(KF5I18n ${KF5_VERSION} REQUIRED)
    find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
    find_package(KF5Completion ${KF5_VERSION} REQUIRED)
    find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5Service ${KF5_VERSION} REQUIRED)
    find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
    find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
    find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)
    find_package(KF5DocTools ${KF5_VERSION} REQUIRED)

#     find_package(KF5I18n ${KF5_VERSION} REQUIRED)
#     find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
    find_package(KF5KIO ${KF5_VERSION} REQUIRED)
    find_package(KF5Parts ${KF5_VERSION} REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KF5DocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(docs)
add_subdirectory(src)
#add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Kross")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5KrossConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KrossConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kross_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KrossConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5KrossConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5KrossTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5KrossTargets.cmake NAMESPACE KF5::)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
