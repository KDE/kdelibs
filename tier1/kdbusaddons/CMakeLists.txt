cmake_minimum_required(VERSION 2.8.9)

project(kdbusaddons)

find_package(ECM 0.0.4 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core DBus)

add_subdirectory(src)
add_subdirectory(autotests)

