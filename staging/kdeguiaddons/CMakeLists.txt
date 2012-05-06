cmake_minimum_required(VERSION 2.8.8)

project(kdeguiaddons)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_KDECORE_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/src/text
)

add_subdirectory(src)
add_subdirectory(autotests)
