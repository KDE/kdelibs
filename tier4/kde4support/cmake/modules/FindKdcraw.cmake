# Try to find the Kdcraw library
#
# Parameters:
#  KDCRAW_LOCAL_DIR - If you have put a local version of libkdcraw into
#                     your source tree, set KDCRAW_LOCAL_DIR to the
#                     relative path from the root of your source tree
#                     to the libkdcraw local directory.
#
# Once done this will define
#
#  KDCRAW_FOUND - System has libkdcraw
#  KDCRAW_INCLUDE_DIR - The libkdcraw include directory/directories (for #include <libkdcraw/...> style)
#  KDCRAW_LIBRARIES - Link these to use libkdcraw
#  KDCRAW_DEFINITIONS - Compiler switches required for using libkdcraw
#  KDCRAW_VERSION - Version of libkdcraw which was found
#
# Copyright (c) 2008-2011, Gilles Caulier, <caulier.gilles@gmail.com>
# Copyright (c) 2011, Michael G. Hansen, <mike@mghansen.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Kdcraw_FIND_QUIETLY and Kdcraw_FIND_REQUIRED may be defined by CMake.

if (KDCRAW_INCLUDE_DIR AND KDCRAW_LIBRARIES AND KDCRAW_DEFINITIONS AND KDCRAW_VERSION)

  if (NOT Kdcraw_FIND_QUIETLY)
    message(STATUS "Found Kdcraw library in cache: ${KDCRAW_LIBRARIES}")
  endif (NOT Kdcraw_FIND_QUIETLY)

  # in cache already
  set(KDCRAW_FOUND TRUE)

else (KDCRAW_INCLUDE_DIR AND KDCRAW_LIBRARIES AND KDCRAW_DEFINITIONS AND KDCRAW_VERSION)

  if (NOT Kdcraw_FIND_QUIETLY)
    message(STATUS "Check for Kdcraw library in local sub-folder...")
  endif (NOT Kdcraw_FIND_QUIETLY)

  # Check for a local version of the library.
  if (KDCRAW_LOCAL_DIR)
    find_file(KDCRAW_LOCAL_FOUND libkdcraw/version.h.cmake ${CMAKE_SOURCE_DIR}/${KDCRAW_LOCAL_DIR} NO_DEFAULT_PATH)
    if (NOT KDCRAW_LOCAL_FOUND)
      message(WARNING "KDCRAW_LOCAL_DIR specified as \"${KDCRAW_LOCAL_DIR}\" but libkdcraw could not be found there.")
    endif (NOT KDCRAW_LOCAL_FOUND)
  else (KDCRAW_LOCAL_DIR)
    find_file(KDCRAW_LOCAL_FOUND libkdcraw/version.h.cmake ${CMAKE_SOURCE_DIR}/libkdcraw NO_DEFAULT_PATH)
    if (KDCRAW_LOCAL_FOUND)
      set(KDCRAW_LOCAL_DIR libkdcraw)
    endif (KDCRAW_LOCAL_FOUND)
    find_file(KDCRAW_LOCAL_FOUND libkdcraw/version.h.cmake ${CMAKE_SOURCE_DIR}/libs/libkdcraw NO_DEFAULT_PATH)
    if (KDCRAW_LOCAL_FOUND)
      set(KDCRAW_LOCAL_DIR libs/libkdcraw)
    endif (KDCRAW_LOCAL_FOUND)
  endif (KDCRAW_LOCAL_DIR)

  if (KDCRAW_LOCAL_FOUND)
    # We need two include directories: because the version.h file is put into the build directory
    # TODO KDCRAW_INCLUDE_DIR sounds like it should contain only one directory...
    set(KDCRAW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${KDCRAW_LOCAL_DIR} ${CMAKE_BINARY_DIR}/${KDCRAW_LOCAL_DIR})
    set(KDCRAW_DEFINITIONS "-I${CMAKE_SOURCE_DIR}/${KDCRAW_LOCAL_DIR}" "-I${CMAKE_BINARY_DIR}/${KDCRAW_LOCAL_DIR}")
    set(KDCRAW_LIBRARIES kdcraw)
    if (NOT Kdcraw_FIND_QUIETLY)
      message(STATUS "Found Kdcraw library in local sub-folder: ${CMAKE_SOURCE_DIR}/${KDCRAW_LOCAL_DIR}")
    endif (NOT Kdcraw_FIND_QUIETLY)
    set(KDCRAW_FOUND TRUE)

    set(kdcraw_version_h_filename "${CMAKE_BINARY_DIR}/${KDCRAW_LOCAL_DIR}/libkdcraw/version.h")

  else (KDCRAW_LOCAL_FOUND)
    if (NOT WIN32)
      if (NOT Kdcraw_FIND_QUIETLY)
        message(STATUS "Check Kdcraw library using pkg-config...")
      endif (NOT Kdcraw_FIND_QUIETLY)

      # use FindPkgConfig to get the directories and then use these values
      # in the find_path() and find_library() calls
      include(FindPkgConfig)

      pkg_check_modules(PC_KDCRAW libkdcraw)

      if (PC_KDCRAW_FOUND)
        # make sure the version is >= 0.2.0
        # TODO: WHY?
        if (PC_KDCRAW_VERSION VERSION_LESS 0.2.0)
          message(STATUS "Found libkdcraw release < 0.2.0, too old")
          set(KDCRAW_VERSION_GOOD_FOUND FALSE)
          set(KDCRAW_FOUND FALSE)
        else (PC_KDCRAW_VERSION VERSION_LESS 0.2.0)
          set(KDCRAW_VERSION "${PC_KDCRAW_VERSION}")
          if (NOT Kdcraw_FIND_QUIETLY)
            message(STATUS "Found libkdcraw release ${KDCRAW_VERSION}")
          endif (NOT Kdcraw_FIND_QUIETLY)
          set(KDCRAW_VERSION_GOOD_FOUND TRUE)
        endif (PC_KDCRAW_VERSION VERSION_LESS 0.2.0)
      else (PC_KDCRAW_FOUND)
        set(KDCRAW_VERSION_GOOD_FOUND FALSE)
      endif (PC_KDCRAW_FOUND)
    else (NOT WIN32)
      # TODO: Why do we just assume the version is good?
      set(KDCRAW_VERSION_GOOD_FOUND TRUE)
    endif (NOT WIN32)

    if (KDCRAW_VERSION_GOOD_FOUND)
      set(KDCRAW_DEFINITIONS "${PC_KDCRAW_CFLAGS_OTHER}")

      find_path(KDCRAW_INCLUDE_DIR libkdcraw/version.h ${PC_KDCRAW_INCLUDE_DIRS})
      set(kdcraw_version_h_filename "${KDCRAW_INCLUDE_DIR}/libkdcraw/version.h")

      find_library(KDCRAW_LIBRARIES NAMES kdcraw HINTS ${PC_KDCRAW_LIBRARY_DIRS})

      if (KDCRAW_INCLUDE_DIR AND KDCRAW_LIBRARIES)
        set(KDCRAW_FOUND TRUE)
      else (KDCRAW_INCLUDE_DIR AND KDCRAW_LIBRARIES)
        set(KDCRAW_FOUND FALSE)
      endif (KDCRAW_INCLUDE_DIR AND KDCRAW_LIBRARIES)
    endif (KDCRAW_VERSION_GOOD_FOUND)

    if (KDCRAW_FOUND)
      if (NOT Kdcraw_FIND_QUIETLY)
        message(STATUS "Found libkdcraw: ${KDCRAW_LIBRARIES}")
      endif (NOT Kdcraw_FIND_QUIETLY)
    else (KDCRAW_FOUND)
      if (Kdcraw_FIND_REQUIRED)
        if (NOT KDCRAW_INCLUDE_DIR)
          message(FATAL_ERROR "Could NOT find libkdcraw header files.")
        else(NOT KDCRAW_INCLUDE_DIR)
          message(FATAL_ERROR "Could NOT find libkdcraw library.")
        endif (NOT KDCRAW_INCLUDE_DIR)
      endif (Kdcraw_FIND_REQUIRED)
    endif (KDCRAW_FOUND)

  endif (KDCRAW_LOCAL_FOUND)

  if (KDCRAW_FOUND)
    # Find the version information, unless that was reported by pkg_search_module.
    if (NOT KDCRAW_VERSION)
      file(READ "${kdcraw_version_h_filename}" kdcraw_version_h_content)
      # This is the line we are trying to find: static const char kdcraw_version[] = "1.22.4-beta_5+dfsg";
      string(REGEX REPLACE ".*char +kdcraw_version\\[\\] += +\"([^\"]+)\".*" "\\1" KDCRAW_VERSION "${kdcraw_version_h_content}")
      unset(kdcraw_version_h_content)

    endif (NOT KDCRAW_VERSION)
    unset(kdcraw_version_h_filename)
  endif (KDCRAW_FOUND)

  if (KDCRAW_FOUND)
    mark_as_advanced(KDCRAW_INCLUDE_DIR KDCRAW_LIBRARIES KDCRAW_DEFINITIONS KDCRAW_VERSION KDCRAW_FOUND)
  else (KDCRAW_FOUND)
    # The library was not found, reset all related variables.
    unset(KDCRAW_INCLUDE_DIR)
    unset(KDCRAW_LIBRARIES)
    unset(KDCRAW_DEFINITIONS)
    unset(KDCRAW_VERSION)
  endif (KDCRAW_FOUND)

endif (KDCRAW_INCLUDE_DIR AND KDCRAW_LIBRARIES AND KDCRAW_DEFINITIONS AND KDCRAW_VERSION)
