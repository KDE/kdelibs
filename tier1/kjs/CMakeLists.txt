cmake_minimum_required(VERSION 2.8.9)

project(kjs)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMQtFramework)
include(ECMWriteVersionHeader)

find_package(Qt5Transitional REQUIRED Core Test)

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/kjs_version.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kjs_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}        # kjs_version.h
  ${CMAKE_CURRENT_BINARY_DIR}/src    # kjs/global.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src    # kjs/* and wtf/*
  ${CMAKE_CURRENT_BINARY_DIR}/src/kjs
  ${CMAKE_CURRENT_SOURCE_DIR}/src/kjs
)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

include(FeatureSummary)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
