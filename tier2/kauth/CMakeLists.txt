cmake_minimum_required(VERSION 2.8.8)

project(kauth)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core Gui DBus)


# TODO: Where does this belong?
set(SYSCONF_INSTALL_DIR etc)
set(PLUGIN_INSTALL_DIR ${LIB_INSTALL_DIR}/kde4)
set(LIBEXEC_INSTALL_DIR ${LIB_INSTALL_DIR}/kde4/libexec)


include_directories(
     ${kcoreaddons_SOURCE_DIR}/src/jobs # for KJob
     ${kcoreaddons_BINARY_DIR}/src
)


add_subdirectory(src)
add_subdirectory(autotests)

include(FeatureSummary)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

install( FILES
    cmake/KAuthMacros.cmake
    DESTINATION ${DATA_INSTALL_DIR}/cmake/modules COMPONENT Devel )
