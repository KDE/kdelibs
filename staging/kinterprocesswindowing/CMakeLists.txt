cmake_minimum_required(VERSION 2.8.10.1)

project(kinterprocesswindowing)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMQtFramework)
include(ECMWriteVersionHeader)

find_package(Qt5Transitional REQUIRED Test)

if(NOT kdelibs_SOURCE_DIR)
  find_package(kdeqt5staging REQUIRED NO_MODULE)
  set_package_properties(kdeqt5staging PROPERTIES PURPOSE "Stuff which will go into Qt5.")
endif()

include_directories(${kdeqt5staging_INCLUDE_DIRS})
include_directories(${kwindowsystem_SOURCE_DIR}/src)
include_directories(${kwindowsystem_BINARY_DIR}/src)
include_directories(
    ${CMAKE_BINARY_DIR}/tier2/kconfig/src/core
    ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/core
)
include_directories(
    ${kdeui_BINARY_DIR}
    ${kdeui_SOURCE_DIR}/dialogs
)
include_directories(
    ${CMAKE_BINARY_DIR}/staging/kwidgets/src
    ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/dialogs
    ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/icons
    ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/utils
)
include_directories(
    ${CMAKE_BINARY_DIR}/staging/ki18n/src
    ${CMAKE_SOURCE_DIR}/staging/ki18n/src
)

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/kinterprocesswindowing_version.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kinterprocesswindowing_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(src)

include(FeatureSummary)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
