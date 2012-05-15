cmake_minimum_required(VERSION 2.8.8)

project(kwidgets)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core Gui Svg)

include(FeatureSummary)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_KDECORE_INCLUDES}
  ${kcoreaddons_BINARY_DIR}
  ${kcoreaddons_SOURCE_DIR}/src/caching
  ${CMAKE_BINARY_DIR}/tier2/kauth/src
  ${CMAKE_SOURCE_DIR}/tier2/kauth/src
  ${kwindowsystem_BINARY_DIR}/src
  ${kwindowsystem_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/staging/kdeguiaddons/src
  ${CMAKE_SOURCE_DIR}/staging/kdeguiaddons/src/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/src/icons
  ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
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

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
