cmake_minimum_required(VERSION 2.8.12)

project(KStyle)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

find_package(KF5 5.0.0 MODULE REQUIRED CMake Compiler InstallDirs)
find_package(Qt5 5.2.0 CONFIG REQUIRED Test Concurrent)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KSTYLE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kstyle_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KStyleConfigVersion.cmake")


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KStyle")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KStyleConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KStyleConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_KEYWORDS)

set(kstyle_SRCS
    kstyle.cpp
)

add_library(KStyle ${kstyle_SRCS})
generate_export_header(KStyle)
add_library(KF5::KStyle ALIAS KStyle)

target_link_libraries(KStyle
  PUBLIC
    Qt5::Widgets
  PRIVATE
    KConfigWidgets
    KF5::KIconThemes
)

set_target_properties(KStyle PROPERTIES VERSION   ${KSTYLE_VERSION_STRING}
                                        SOVERSION ${KSTYLE_SOVERSION}
)

install(TARGETS KStyle EXPORT KStyleTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kstyle.h
    ${CMAKE_CURRENT_BINARY_DIR}/kstyle_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KStyleConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KStyleConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KStyleTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KStyleTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kstyle_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

