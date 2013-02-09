cmake_minimum_required(VERSION 2.8.10.1)

project(kguiaddons)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMVersion)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/src/text
)

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)

add_subdirectory(src)
add_subdirectory(autotests)
