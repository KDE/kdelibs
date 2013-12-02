cmake_minimum_required(VERSION 2.8.12)

project(KConfigWidgets)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Widgets DBus)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
if(NOT kdelibs_SOURCE_DIR)
  find_package(KAuth 5.0.0 REQUIRED)
  find_package(KF5CoreAddons 5.0.0 REQUIRED)
  find_package(KF5Codecs 5.0.0 REQUIRED)
  find_package(KF5Config 5.0.0 REQUIRED)
  find_package(KDocTools 5.0.0 REQUIRED)
  find_package(KF5GuiAddons 5.0.0 REQUIRED)
  find_package(KI18n 5.0.0 REQUIRED)
  find_package(KF5WidgetsAddons 5.0.0 REQUIRED)
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KDocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
endif()

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KCONFIGWIDGETS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kconfigwidgets_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KConfigWidgetsConfigVersion.cmake")

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KConfigWidgets")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KConfigWidgetsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KConfigWidgetsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KConfigWidgetsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KConfigWidgetsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KConfigWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KConfigWidgetsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfigwidgets_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
