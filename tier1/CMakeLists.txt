cmake_minimum_required(VERSION 2.8.5)

project(tier1)

add_subdirectory(itemmodels)
add_subdirectory(kauth)
add_subdirectory(kdbusaddons)
add_subdirectory(kplotting)
add_subdirectory(solid)
add_subdirectory(threadweaver)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
