cmake_minimum_required(VERSION 2.8.11)

project(KEmoticons)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KEMOTICONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kemoticons_version.h"
                 )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kemoticons_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )


# the strings throughout this library are used to search ressources
# so they don't need translations
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

