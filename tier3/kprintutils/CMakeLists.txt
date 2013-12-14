cmake_minimum_required(VERSION 2.8.12)

project(KPrintUtils)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt5PrintSupport ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  #NOTE: here the dependencies of dependencies are displayed as well, this shouldn't be needed
  #after cmake 3.0.0 is released
  find_package(KF5I18n 5.0.0 REQUIRED)
  find_package(KF5GuiAddons 5.0.0 REQUIRED)
  find_package(KF5WidgetsAddons 5.0.0 REQUIRED)
  find_package(KF5DBusAddons 5.0.0 REQUIRED)
  find_package(KF5ConfigWidgets 5.0.0 REQUIRED)
  find_package(KF5ItemViews 5.0.0 REQUIRED)
  find_package(KF5IconThemes 5.0.0 REQUIRED)
  find_package(KF5WindowSystem 5.0.0 REQUIRED)
  find_package(KF5Service 5.0.0 REQUIRED)
  find_package(KF5Completion 5.0.0 REQUIRED)
  find_package(KF5TextWidgets 5.0.0 REQUIRED)

  find_package(KF5CoreAddons 5.0.0 REQUIRED)
#   find_package(KF5I18n 5.0.0 REQUIRED)
  find_package(KF5Parts 5.0.0 REQUIRED)
endif()

find_package(X11)
set(HAVE_X11 ${X11_FOUND})

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KPRINTUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kprintutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PrintUtilsConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5PrintUtils")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5PrintUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PrintUtilsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PrintUtilsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5PrintUtilsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5PrintUtilsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5PrintUtilsTargets.cmake NAMESPACE KF5:: )


install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kprintutils_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
