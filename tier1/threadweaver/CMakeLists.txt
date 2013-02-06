cmake_minimum_required(VERSION 2.8.10.1)

project(threadweaver)

find_package(ECM 0.0.6 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})


find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test)

include(FeatureSummary)

include(ECMSetupVersion)

# -- Maintained by scripty
ecm_setup_version(5 0 0 VARIABLE_PREFIX THREADWEAVER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/threadweaver_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfigVersion.cmake")
# --

add_subdirectory(src)
add_subdirectory(autotests)
#add_subdirectory(examples)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/threadweaver")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/threadweaverConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/threadweaverConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT threadweaverTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE threadweaverTargets.cmake NAMESPACE KF5:: )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
