# This part is for checking at buildtime whether DESTDIR still is the same.
# It is executed by cmake in script mode via the AlwaysCheckDESTDIR custom target.
if(_SB_CHECK_DESTDIR)
  set(_tmpDest "$ENV{DESTDIR}")
  if(NOT "${SB_INITIAL_DESTDIR}" STREQUAL "${_tmpDest}")
    message(FATAL_ERROR "DESTDIR changed. This is not supported in Superbuilds, DESTDIR must always be the same at CMake and build time. (now: \"${_tmpDest}\", at CMake time: \"${SB_INITIAL_DESTDIR}\")")
  else()
    message("DESTDIR Ok. (now: \"${_tmpDest}\", at CMake time: \"${SB_INITIAL_DESTDIR}\")")
  endif()
  return()
endif()

# This custom target is used to check at buildtime whether DESTDIR is still the same as at CMake time.
add_custom_target(AlwaysCheckDESTDIR COMMAND ${CMAKE_COMMAND} -DSB_INITIAL_DESTDIR="${SB_INITIAL_DESTDIR}" -D_SB_CHECK_DESTDIR=TRUE -P ${CMAKE_CURRENT_LIST_FILE} )


#####################################################################################

# Now the actual CMakeLists.txt starts.

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

# Try to handle DESTDIR.
# We install during the build, and if DESTDIR is set, the install will go there.
# Installed libs have to be found in DESTDIR, so prepend it to CMAKE_PREFIX_PATH.
# If RPATH is used, this messes everything up, since the using binary will have the RPATH set to
# the library inside DESTDIR, which is wrong.
# So, only allow DESTDIR if RPATH is completely disabled using CMAKE_SKIP_RPATH.
set(_tmpDest "$ENV{DESTDIR}")

if(NOT DEFINED SB_INITIAL_DESTDIR)
  # initial cmake run, check DESTDIR
  set(SB_INITIAL_DESTDIR ${_tmpDest} CACHE STRING "The DESTDIR environment variable during the initial cmake run" FORCE)
  mark_as_advanced(SB_INITIAL_DESTDIR)
else()
  if(NOT "${SB_INITIAL_DESTDIR}" STREQUAL "${_tmpDest}")
    message(FATAL_ERROR "Your DESTDIR environment variable changed. In a Superbuild, DESTDIR must always stay the same as it was during the initial cmake run. Initially it was \"${SB_INITIAL_DESTDIR}\", now it is \"${_tmpDest}\" .")
  endif()
endif()

if(SB_INITIAL_DESTDIR)
  if( NOT CMAKE_SKIP_RPATH)
    message(FATAL_ERROR "The DESTDIR environment variable is set to \"${SB_INITIAL_DESTDIR}\", but CMAKE_SKIP_RPATH is not set to TRUE. This would produce binaries with bad RPATHs. ")
  endif()

  if(NOT IS_ABSOLUTE "${SB_INITIAL_DESTDIR}")
    message(FATAL_ERROR "The DESTDIR environment variable is set to \"${SB_INITIAL_DESTDIR}\", but relative DESTDIR is not support in a Superbuild. Set it to an absolute path")
  endif()
endif()


# set up directory structure to use for the ExternalProjects
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY EP_BASE ${CMAKE_CURRENT_BINARY_DIR}
            )

macro(sb_add_project _subdir )

  set(oneValueArgs ) #CVS_REPOSITORY GIT_REPOSITORY SVN_REPOSITORY SOURCE_DIR SUBDIR)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(_SB "" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

  set(name "${_subdir}")
  if("${_subdir}" MATCHES "(.+)/(.+)")
    set(name "${CMAKE_MATCH_2}")
  endif()

  set(GET_SOURCES_ARGS SOURCE_DIR ${CMAKE_SOURCE_DIR}/${_subdir}
                       DOWNLOAD_COMMAND "")

  message(STATUS "superbuild: Adding project ${_subdir}")

  set(DEPENDS_ARGS)
  if(_SB_DEPENDS)
    set(existingDepends)

    foreach(dep ${_SB_DEPENDS})
      if(NOT TARGET sb_${dep})
        message(FATAL_ERROR "'${dep}' is not defined as a superbuild project")
        return()
      endif()
      list(APPEND existingDepends sb_${dep} )
    endforeach(dep)

    if(existingDepends)
      set(DEPENDS_ARGS DEPENDS ${existingDepends} )
    endif()

  endif()

  externalproject_add(sb_${name}
                      ${_SB_UNPARSED_ARGUMENTS}
                      ${GET_SOURCES_ARGS}
                      TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SuperBuild/tmpfiles/${name}
                      STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SuperBuild/stampfiles/${name}
                      DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/SuperBuild/download/${name}
                      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${name}
                      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
                      CMAKE_ARGS --no-warn-unused-cli
                                 -DQT_QMAKE_EXECUTABLE=${QT_QMAKE_EXECUTABLE}
                                 -DCMAKE_PREFIX_PATH=${SB_INITIAL_DESTDIR}${CMAKE_INSTALL_PREFIX}
                                 -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                                 -DCMAKE_SKIP_RPATH="${CMAKE_SKIP_RPATH}"
                                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DLIB_SUFFIX=${LIB_SUFFIX}
                                 ${SB_CMAKE_ARGS}
                                 ${SB_CMAKE_ARGS_${name}}
                      STEP_TARGETS update configure
                      ${DEPENDS_ARGS}
                      )
  set_target_properties(sb_${name} PROPERTIES EXCLUDE_FROM_ALL TRUE)

  add_dependencies(sb_${name} AlwaysCheckDESTDIR)
  add_dependencies(sb_all sb_${name})
endmacro(sb_add_project)

#file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ThisIsASourcePackage.in "This is a generated source package.")

#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ThisIsASourcePackage.in DESTINATION src RENAME ThisIsASourcePackage.valid  COMPONENT SuperBuild )
#install(FILES CMakeLists.txt DESTINATION src  COMPONENT SuperBuild )
#install(FILES ${CMAKE_CURRENT_LIST_FILE} DESTINATION .  COMPONENT SuperBuild )

add_custom_target(sb_all)
set_target_properties(sb_all PROPERTIES EXCLUDE_FROM_ALL TRUE)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)
