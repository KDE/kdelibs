cmake_minimum_required(VERSION 2.8.12)

project(KItemViews)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KITEMVIEWS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kitemviews_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ItemViewsConfigVersion.cmake")


add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5ItemViews")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5ItemViewsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ItemViewsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ItemViewsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5ItemViewsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5ItemViewsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5ItemViewsTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kitemviews_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
