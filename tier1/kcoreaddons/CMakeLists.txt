cmake_minimum_required(VERSION 2.8.10.1)

project(kcoreaddons)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMQtFramework)
include(ECMWriteVersionHeader)

find_package(Qt5Transitional REQUIRED Core Test)

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

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_version.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

include(FeatureSummary)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
