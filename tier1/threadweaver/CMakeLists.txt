cmake_minimum_required(VERSION 2.8.12)

project(ThreadWeaver)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})


find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

include(FeatureSummary)

include(ECMSetupVersion)

# -- Maintained by scripty
ecm_setup_version(5.0.0 VARIABLE_PREFIX THREADWEAVER
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/threadweaver_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/ThreadWeaverConfigVersion.cmake")
# --

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(benchmarks)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/ThreadWeaver")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/ThreadWeaverConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/ThreadWeaverConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/threadweaver_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ThreadWeaverConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/ThreadWeaverConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT ThreadWeaverTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE ThreadWeaverTargets.cmake NAMESPACE KF5:: )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
