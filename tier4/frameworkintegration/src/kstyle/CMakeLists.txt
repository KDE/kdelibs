cmake_minimum_required(VERSION 2.8.12)

project(KStyle)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
find_package(Qt5 5.2.0 CONFIG REQUIRED Test Concurrent)

set(KF5_VERSION "5.0.0")

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KSTYLE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kstyle_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5StyleConfigVersion.cmake")


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Style")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5StyleConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5StyleConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_library(KF5Style kstyle.cpp)
generate_export_header(KF5Style BASE_NAME KStyle)
add_library(KF5::Style ALIAS KF5Style)

target_link_libraries(KF5Style
  PUBLIC
    Qt5::Widgets
  PRIVATE
    KF5::ConfigWidgets
    KF5::IconThemes
)

set_target_properties(KF5Style PROPERTIES VERSION   ${KSTYLE_VERSION_STRING}
                                          SOVERSION ${KSTYLE_SOVERSION}
                                          EXPORT_NAME Style
)

install(TARGETS KF5Style EXPORT KF5StyleTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    kstyle.h
    ${CMAKE_CURRENT_BINARY_DIR}/kstyle_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5StyleConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5StyleConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5StyleTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5StyleTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kstyle_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

