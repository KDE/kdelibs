
cmake_minimum_required(VERSION 2.8.9)

project(sonnet)

find_package(ECM 0.0.4 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_STRICT_ITERATORS)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)

find_package(Qt5Transitional REQUIRED Core DBus Gui Test)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)
