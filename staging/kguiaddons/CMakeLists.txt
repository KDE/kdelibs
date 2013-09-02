cmake_minimum_required(VERSION 2.8.11)

project(KGuiAddons)

if(NOT kdelibs_SOURCE_DIR)

  find_package(ECM 0.0.8 REQUIRED NO_MODULE)
  set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

  find_package(Qt5Widgets 5.2.0 REQUIRED NO_MODULE)

  find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs
                                                  KCoreAddons
                                                  KConfig)

endif()

include(ECMSetupVersion)
ecm_setup_version(5 0 0 VARIABLE_PREFIX KGUIADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KGuiAddonsConfigVersion.cmake")

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KGuiAddons")

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KGuiAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KGuiAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KGuiAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KGuiAddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT KGuiAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KGuiAddonsTargets.cmake NAMESPACE KF5:: COMPONENT Devel)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
