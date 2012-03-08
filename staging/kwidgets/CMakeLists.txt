cmake_minimum_required(VERSION 2.8.7)

project(kwidgets)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

add_subdirectory(src)
add_subdirectory(tests) #TODO
