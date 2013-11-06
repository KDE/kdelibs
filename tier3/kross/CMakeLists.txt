cmake_minimum_required(VERSION 2.8.12)

project(Kross)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

set(REQUIRED_QT_VERSION "5.2")
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Test Script Xml Widgets UiTools)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KROSS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kross_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KrossConfigVersion.cmake")

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

# Enable when kdoctools will be usable from the outside (macros are broken ATM)
#add_subdirectory(docs)
add_subdirectory(src)
#add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/Kross")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KrossConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KrossConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kross_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KrossConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KrossConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KrossTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KrossTargets.cmake NAMESPACE KF5::)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
