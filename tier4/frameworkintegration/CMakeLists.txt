cmake_minimum_required(VERSION 2.8.12)

project(FrameworkIntegration)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

find_package(Qt5 5.2.0 CONFIG REQUIRED Widgets DBus)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX FRAMEWORKINTEGRATION
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegration_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegrationConfigVersion.cmake")

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KF5I18n ${KF5_VERSION} REQUIRED)
  find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED) #
  find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED) #
  find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
  find_package(KF5ItemViews ${KF5_VERSION} REQUIRED) #
  find_package(KF5Config ${KF5_VERSION} REQUIRED)
  find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
  find_package(KF5Notifications ${KF5_VERSION} REQUIRED)
  find_package(KF5I18n ${KF5_VERSION} REQUIRED)
  find_package(KF5KIO ${KF5_VERSION} REQUIRED)
endif()

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/frameworkintegration")

#configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/frameworkintegrationConfig.cmake.in"
#                              "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegrationConfig.cmake"
#                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
#                              PATH_VARS  INCLUDE_INSTALL_DIR LIB_INSTALL_DIR CMAKE_INSTALL_PREFIX )

#install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegrationConfig.cmake"
#               "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegrationConfigVersion.cmake"
#        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
#        COMPONENT Devel )

#install(EXPORT frameworkintegrationTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE frameworkintegrationTargets.cmake NAMESPACE KDE4__ )

#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/frameworkintegration_version.h
#        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
