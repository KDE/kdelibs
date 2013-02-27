cmake_minimum_required(VERSION 2.8.10.1)

project(kinterprocesswindowing)

find_package(ECM 0.0.7 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMQtFramework)
include(ECMWriteVersionHeader)

find_package(Qt5Transitional REQUIRED Test X11Extras)
include_directories(${Qt5X11Extras_INCLUDE_DIRS})

include_directories(${KWindowSystem_INCLUDE_DIRS})
include_directories(${KConfig_INCLUDE_DIRS})
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
