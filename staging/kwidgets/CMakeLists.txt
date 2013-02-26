cmake_minimum_required(VERSION 2.8.10.1)

project(kwidgets)

find_package(ECM 0.0.7 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)
include(ECMWriteVersionHeader)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets Svg X11Extras)

include(FeatureSummary)

include_directories(
  ${QT_INCLUDES}
  ${KCoreAddons_INCLUDE_DIRS}
  ${KAuth_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/kdecore
  ${ki18n_SOURCE_DIR}/src # KLocalizedString::localizedFilePath
  ${ki18n_BINARY_DIR}/src # KLocalizedString::localizedFilePath
  ${CMAKE_SOURCE_DIR}/kdecore/kernel # KGlobal (TODO)
  ${CMAKE_SOURCE_DIR}/kdecore/util # kde_file.h (TODO)
  ${CMAKE_BINARY_DIR}/tier2/kconfig/src/core
  ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/core
  ${CMAKE_BINARY_DIR}/tier2/kconfig/src/gui
  ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/gui
  ${KWidgetsAddons_INCLUDE_DIRS}
  ${KWindowSystem_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/staging/kguiaddons/src
  ${CMAKE_SOURCE_DIR}/staging/kguiaddons/src/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/src/icons
  ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
  ${CMAKE_BINARY_DIR}/kdeui # kdeui_export.h
  ${CMAKE_SOURCE_DIR}/kdeui/notifications
  ${CMAKE_BINARY_DIR}/kdeui/notifications
  ${CMAKE_SOURCE_DIR}/kdeui/widgets # KSQueezedTextLabel
  ${CMAKE_BINARY_DIR}/staging/kservice/src
  ${CMAKE_SOURCE_DIR}/staging/kservice/src/plugin
)

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_STRICT_ITERATORS)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/kwidgets_version.h)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
