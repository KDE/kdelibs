cmake_minimum_required(VERSION 2.8.8)

project(kdbusaddons)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core DBus)

add_subdirectory(src)
add_subdirectory(autotests)

