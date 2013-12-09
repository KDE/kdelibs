project(kservicetojsontest)
cmake_minimum_required(VERSION 2.8)

find_package(Qt5 CONFIG REQUIRED Core)
find_package(KF5Service CONFIG REQUIRED)

set(CMAKE_AUTOMOC TRUE)
set(BUILD_SHARED_LIBS TRUE)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

add_library(kservicetojsontest dummyplugin.cpp)
kservice_desktop_to_json(dummy-service.desktop)
target_link_libraries(kservicetojsontest KF5::Service)
