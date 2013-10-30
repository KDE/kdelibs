cmake_minimum_required(VERSION 2.8.10.20130309)

project(KTextWidgets)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets DBus)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)
if(NOT kdelibs_SOURCE_DIR)
  find_package(Sonnet 5.0.0 REQUIRED)
endif()

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KTEXTWIDGETS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ktextwidgets_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KTextWidgetsConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KTextWidgets")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KTextWidgetsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KTextWidgetsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                             )

install(EXPORT KTextWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KTextWidgetsTargets.cmake NAMESPACE KF5:: )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KTextWidgetsConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KTextWidgetsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ktextwidgets_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
       )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
