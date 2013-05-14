
cmake_minimum_required(VERSION 2.8.10.20130309)

project(Solid)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Xml Concurrent DBus Widgets Test)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

if(NOT APPLE)
  find_package(HUpnp 0.9)
  set_package_properties(HUpnp PROPERTIES  URL "http://www.herqq.org"
                                           DESCRIPTION "UPnP support for Solid"
                                           TYPE OPTIONAL
                                           PURPOSE "Allows Solid to provide information about UPnP devices on the network"
                        )
endif()


if(NOT WIN32 AND NOT APPLE)
  find_package( UDev )
  set_package_properties(UDev PROPERTIES  URL "http://www.kernel.org/pub/linux/utils/kernel/hotplug/udev.html"
                                          DESCRIPTION "UDev support for Solid"
                                          TYPE OPTIONAL
                                          PURPOSE "Allows Solid to use UDev to provide information about devices on Linux"
                         )
endif()


set(WBEM_FOUND FALSE)
if(WIN32)
  include(CheckIncludeFileCXX)
  check_include_file_cxx(wbemidl.h HAVE_WBEM)
  find_library(WBEM_LIBRARIES NAMES wbemuuid wbemuuidd)
  if(HAVE_WBEM AND WBEM_LIBRARIES)
    set(HAVE_WBEM TRUE)
    set(WBEM_FOUND TRUE)
    message(STATUS "Found wbemuuid library: ${WBEM_LIBRARIES}")
  endif()
endif()

############### Give the user the option to build the udisks2 solid backend instead of the udisks backend ###############
option(WITH_SOLID_UDISKS2 "Enable the udisks2 solid backend instead")


ecm_setup_version(5 0 0 VARIABLE_PREFIX SOLID
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/solid_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/SolidConfigVersion.cmake")

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_STRICT_ITERATORS)
remove_definitions(-DQT_NO_URL_CAST_FROM_STRING)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/Solid")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/SolidConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/SolidConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/SolidConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/SolidConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT SolidTargets  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  FILE SolidTargets.cmake  NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/solid_version.h  DESTINATION ${INCLUDE_INSTALL_DIR}  COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
