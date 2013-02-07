cmake_minimum_required(VERSION 2.8.10.1)

project(kconfig)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} cmake/)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

# once we can build tier2 libs standalone, add "kcoreaddons" component
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMQtFramework)
include(ECMWriteVersionHeader)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test)
if(WIN32)
    find_package(KDEWin REQUIRED)
    include_directories(${KDEWIN_INCLUDES})
endif(WIN32)

include(FeatureSummary)

# Needed for the tests or examples to build correctly
set(KCONFIG_KCFGC_EXECUTABLE kconfig_compiler)
include(cmake/KConfigMacros.cmake)

add_subdirectory(src)
add_subdirectory(autotests)

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/kconfig_version.h)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfig_version.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES
   cmake/KConfigMacros.cmake
   DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )


if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
   feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
