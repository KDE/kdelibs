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

find_package(X11)
set(HAVE_X11 ${X11_FOUND})

include(FeatureSummary)
include(ECMSetupVersion)

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KPRINTUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kprintutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5PrintUtilsConfigVersion.cmake")

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  #NOTE: here the dependencies of dependencies are displayed as well, this shouldn't be needed
  #after cmake 3.0.0 is released
  find_package(KF5I18n ${KF5_VERSION} REQUIRED)
  find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
  find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
  find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
  find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
  find_package(KF5Service ${KF5_VERSION} REQUIRED)
  find_package(KF5Completion ${KF5_VERSION} REQUIRED)
  find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
  find_package(KF5CoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KF5Parts${KF5_VERSION} REQUIRED)
endif()

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
