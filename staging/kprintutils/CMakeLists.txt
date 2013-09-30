cmake_minimum_required(VERSION 2.8.11)

project(KPrintUtils)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2.0")
find_package(Qt5Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt5PrintSupport ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

if(NOT kdelibs_SOURCE_DIR)
  find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)
  find_package(KCoreAddons 5.0.0 REQUIRED)
  find_package(KI18n 5.0.0 REQUIRED)
  find_package(KParts 5.0.0 REQUIRED)
endif()

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KPRINTUTILS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kprintutils_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPrintUtilsConfigVersion.cmake")

add_subdirectory(src)
# add_subdirectory(tests)
# add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KPrintUtils")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KPrintUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KPrintUtilsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
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
