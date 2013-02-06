# This file can be used to build the tier1 subdirs each one by one standalone.
# This file is on purpose not included by the CMakeLists.txt in the tier1/
# dir, you need to run cmake directly on this file.
#
# After the initial cmake run, you will have to enable the projects you want
# to build using ccmake/cmake-gui/make edit_cache.
# For each subdir there is a BUILD_foo switch, all are off by default.
#
# Alex

cmake_minimum_required(VERSION 2.8.10.1)

set(TIER1_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../ )
set(TIER1_BINARY_DIR ${CMAKE_BINARY_DIR}/ )


include(ExternalProject)

function(add_project _name)
   option(BUILD_${_name} "Build ${_name}" OFF)
   if(BUILD_${_name})

      externalproject_add(SB_${_name}
              SOURCE_DIR ${TIER1_SOURCE_DIR}/${_name}
              BINARY_DIR ${TIER1_BINARY_DIR}/${_name}
              TMP_DIR ${TIER1_BINARY_DIR}/${_name}/CMakeFiles/tmp
              STAMP_DIR ${TIER1_BINARY_DIR}/${_name}/CMakeFiles/stamp
              DOWNLOAD_DIR ${TIER1_BINARY_DIR}/${_name}/CMakeFiles/download
              INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
              CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
              DOWNLOAD_COMMAND "" )
   endif()
endfunction()

add_project(itemmodels)
add_project(karchive)
add_project(kcodecs)
add_project(kcoreaddons)
add_project(kdbusaddons)
add_project(kidletime)
add_project(kjs)
add_project(kplotting)
add_project(kwidgetsaddons)
add_project(kwindowsystem)
add_project(solid)
add_project(threadweaver)
