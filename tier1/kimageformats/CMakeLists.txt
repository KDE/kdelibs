cmake_minimum_required(VERSION 2.8.12)

project(KImageFormats)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

# Add our own CMake dir for FindOpenEXR.cmake
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(CheckIncludeFiles)

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5Gui ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

add_subdirectory(src)
add_subdirectory(tests)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
