cmake_minimum_required(VERSION 2.8.12)

project(KPrintUtils)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt5PrintSupport ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  #NOTE: here the dependencies of dependencies are displayed as well, this shouldn't be needed
  #after cmake 3.0.0 is released
  find_package(KI18n 5.0.0 REQUIRED)
  find_package(KF5GuiAddons 5.0.0 REQUIRED)
  find_package(KF5WidgetsAddons 5.0.0 REQUIRED)
  find_package(KF5DBusAddons 5.0.0 REQUIRED)
  find_package(KConfigWidgets 5.0.0 REQUIRED)
  find_package(KF5ItemViews 5.0.0 REQUIRED)
  find_package(KIconThemes 5.0.0 REQUIRED)
  find_package(KF5WindowSystem 5.0.0 REQUIRED)
  find_package(KService 5.0.0 REQUIRED)
  find_package(KCompletion 5.0.0 REQUIRED)
  find_package(KTextWidgets 5.0.0 REQUIRED)

  find_package(KF5CoreAddons 5.0.0 REQUIRED)
#   find_package(KI18n 5.0.0 REQUIRED)
  find_package(KParts 5.0.0 REQUIRED)
endif()

find_package(X11)
set(HAVE_X11 ${X11_FOUND})

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KPRINTUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kprintutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPrintUtilsConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KPrintUtils")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPrintUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPrintUtilsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KPrintUtilsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KPrintUtilsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KPrintUtilsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KPrintUtilsTargets.cmake NAMESPACE KF5:: )


install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kprintutils_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
