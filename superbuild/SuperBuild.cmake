include(ExternalProject)
include(CMakeParseArguments)


set(SB_PACKAGE_VERSION_NUMBER "0.0.1" CACHE STRING "The version number for the source package.")

set(SB_CMAKE_ARGS "" CACHE STRING "Additional arguments to CMake which will be used for all subprojects (e.g. \"-DFOO=Bar\"). For per-project arguments variables SB_CMAKE_ARGS_<ProjectName> can be defined.")

set(SB_SILENT_SKIPPED_PROJECTS FALSE)

# this file (SuperBuild.cmake) is systematically included from one of the child directories
# where some CMakeLists.txt state include(../SuperBuild.cmake). So the current directory is
# located in a subfolder of this include file. That's why global SuperBuildOptions.cmake should
# be included from ../ (e.g. the parent directory)
#message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
include(../GlobalSuperBuildOptions.cmake OPTIONAL)

# this file is included from the project directory and allow for local definitions
include(SuperBuildOptions.cmake OPTIONAL)

# set up directory structure to use for the ExternalProjects
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY EP_BASE ${CMAKE_CURRENT_BINARY_DIR}
            )

if (NOT SB_INSTALL_PREFIX)
  set(SB_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()

set(_SB_LAST_TARGET)

macro(sb_add_project _subdir )
  set(name "${_subdir}")
  if("${_subdir}" MATCHES "(.+)/(.+)")
    set(name "${CMAKE_MATCH_2}")
  endif()

  message(STATUS "superbuild: Adding project ${_subdir}")

  externalproject_add(sb_${name}
                      ${_SB_UNPARSED_ARGUMENTS}
                      DOWNLOAD_COMMAND ""
                      SOURCE_DIR  ${CMAKE_SOURCE_DIR}/${_subdir}
                      TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SuperBuild/tmpfiles/${name}
                      STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SuperBuild/stampfiles/${name}
                      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SuperBuild/download/${name}
                      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${name}
                      INSTALL_DIR ${SB_INSTALL_PREFIX}
                      CMAKE_ARGS --no-warn-unused-cli
                                 -DQT_QMAKE_EXECUTABLE=${QT_QMAKE_EXECUTABLE}
                                 -DCMAKE_PREFIX_PATH=${SB_INITIAL_DESTDIR}${CMAKE_INSTALL_PREFIX}
                                 -DCMAKE_INSTALL_PREFIX=${SB_INSTALL_PREFIX}
                                 -DCMAKE_SKIP_RPATH="${CMAKE_SKIP_RPATH}"
                                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DLIB_SUFFIX=${LIB_SUFFIX}
                                 ${SB_CMAKE_ARGS}
                                 ${SB_CMAKE_ARGS_${name}}
                      )

  set_target_properties(sb_${name} PROPERTIES EXCLUDE_FROM_ALL TRUE)

  if(_SB_LAST_TARGET)
    add_dependencies(sb_${name} ${_SB_LAST_TARGET})
  endif()
  set(_SB_LAST_TARGET sb_${name})
endmacro(sb_add_project)

macro(sb_end)
  add_custom_target(sb_all)
  set_target_properties(sb_all PROPERTIES EXCLUDE_FROM_ALL TRUE)
  add_dependencies(sb_all ${_SB_LAST_TARGET})
endmacro(sb_end)

#file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ThisIsASourcePackage.in "This is a generated source package.")

#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ThisIsASourcePackage.in DESTINATION src RENAME ThisIsASourcePackage.valid  COMPONENT SuperBuild )
#install(FILES CMakeLists.txt DESTINATION src  COMPONENT SuperBuild )
#install(FILES ${CMAKE_CURRENT_LIST_FILE} DESTINATION .  COMPONENT SuperBuild )


set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)
