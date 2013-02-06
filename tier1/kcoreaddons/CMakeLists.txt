cmake_minimum_required(VERSION 2.8.10.1)

project(kcoreaddons)

find_package(ECM 0.0.6 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Test)

if(NOT kdelibs_SOURCE_DIR)
  find_package(kdeqt5staging REQUIRED NO_MODULE)
  set_package_properties(kdeqt5staging PROPERTIES PURPOSE "Stuff which will go into Qt5.")
endif()

include_directories(${kdeqt5staging_INCLUDE_DIR})

# Configure checks for kdirwatch
find_package(FAM)

set_package_properties(FAM PROPERTIES
     PURPOSE "Provides file alteration notification facilities using a separate service. FAM provides additional support for NFS.")

set(HAVE_FAM ${FAM_FOUND})

include(CheckIncludeFiles)
check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
set(HAVE_SYS_INOTIFY_H ${SYS_INOTIFY_H_FOUND})


# Generate io/config-kdirwatch.h
configure_file(src/io/config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/io/config-kdirwatch.h)

check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
configure_file(src/io/config-klockfile.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/io/config-klockfile.h)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KCOREADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddonsConfigVersion.cmake")


add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/kcoreaddons")

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/kcoreaddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddonsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT kcoreaddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE kcoreaddonsTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
