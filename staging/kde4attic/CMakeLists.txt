cmake_minimum_required(VERSION 2.8.11)

project(KDE4Attic)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5 ${QT_REQUIRED_VERSION} NO_MODULE REQUIRED PrintSupport Network)

include(FeatureSummary)

include(ECMSetupVersion)

# This framework uses a different numbering because it will break BC
# Change this to 2 for KF 5.1
ecm_setup_version(1.0.0 VARIABLE_PREFIX KDE4ATTIC
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kde4attic_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KDE4AtticConfigVersion.cmake")

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDE4Attic")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KDE4AtticConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KDE4AtticConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KDE4AtticConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KDE4AtticConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KDE4AtticTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDE4AtticTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kde4attic_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
