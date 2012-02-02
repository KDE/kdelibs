cmake_minimum_required(VERSION 2.8.5)

project(kcoreaddons)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core Test)


# Configure checks for kdirwatch
find_package(FAM)
macro_bool_to_01(FAM_FOUND HAVE_FAM)
check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)

# Generate io/config-kdirwatch.h
include(src/io/ConfigureChecks.cmake)
configure_file(src/io/config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/io/config-kdirwatch.h)

add_subdirectory(src)
add_subdirectory(autotests)
