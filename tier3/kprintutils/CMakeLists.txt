cmake_minimum_required(VERSION 2.8.12)

project(KPrintUtils)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt5PrintSupport ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(KF5 5.0.0 MODULE REQUIRED CMake Compiler InstallDirs)

if(NOT kdelibs_SOURCE_DIR)
  find_package(KCoreAddons 5.0.0 REQUIRED)
  find_package(KF5::KI18n 5.0.0 REQUIRED)
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
