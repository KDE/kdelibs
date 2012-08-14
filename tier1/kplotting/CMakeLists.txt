cmake_minimum_required(VERSION 2.8.9)

project(kplotting)

find_package(ECM 0.0.4 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core Gui)


add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(autotests)

include(FeatureSummary)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
   feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
