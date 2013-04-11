cmake_minimum_required(VERSION 2.8.10.20130309)

project(kdeqt5staging)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets Test)

include(FeatureSummary)

# -- Maintained by scripty
include(ECMSetupVersion)
ecm_setup_version(5 0 0 VARIABLE_PREFIX KDEQT5STAGING
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kdeqt5stagingConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(autotests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kdeqt5staging")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/kdeqt5stagingConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kdeqt5stagingConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX)

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/kdeqt5stagingConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kdeqt5stagingConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  COMPONENT Devel)

install(EXPORT kdeqt5stagingLibraryTargets  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE kdeqt5stagingTargets.cmake  COMPONENT Devel )
