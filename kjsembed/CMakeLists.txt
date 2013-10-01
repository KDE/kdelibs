cmake_minimum_required(VERSION 2.8.11)

project(KJS)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS UiTools Xml)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)


if (Qt5UiTools_FOUND)
    add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
    remove_definitions(-DQT_NO_CAST_FROM_ASCII)
    remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

    add_subdirectory( kjsembed )
    add_subdirectory( kjscmd )
endif()

#add_subdirectory( examples/console )
