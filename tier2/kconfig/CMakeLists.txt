cmake_minimum_required(VERSION 2.8.8)

project(kconfig)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH} cmake/)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core Test)

include(FeatureSummary)

# Needed for the tests or examples to build correctly
set(KCONFIG_KCFGC_EXECUTABLE kconfig_compiler)
include(cmake/KConfigMacros.cmake)

add_subdirectory(src)
add_subdirectory(autotests)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfig_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

install( FILES
    cmake/KConfigMacros.cmake
    DESTINATION ${DATA_INSTALL_DIR}/cmake/modules COMPONENT Devel )

