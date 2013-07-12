# Try to find the Attica library
# Once done this will define
#
#   LibAttica_FOUND          Indicates that Attica was found
#   LibAttica_LIBRARIES      Libraries needed to use Attica
#   LibAttica_LIBRARY_DIRS   Paths needed for linking against Attica
#   LibAttica_INCLUDE_DIR    Path needed for finding Attica include files
#
# The minimum required version of LibAttica can be specified using the
# standard syntax, e.g. find_package(LibAttica 0.20)

# Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.

# Support LibAttica_MIN_VERSION for compatibility:
IF(NOT LibAttica_FIND_VERSION)
  SET(LibAttica_FIND_VERSION "${LibAttica_MIN_VERSION}")
ENDIF(NOT LibAttica_FIND_VERSION)

# the minimum version of LibAttica we require
IF(NOT LibAttica_FIND_VERSION)
  SET(LibAttica_FIND_VERSION "0.1.0")
ENDIF(NOT LibAttica_FIND_VERSION)


IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(PC_LibAttica QUIET libattica)
   SET(LibAttica_DEFINITIONS ${PC_ATTICA_CFLAGS_OTHER})
ENDIF (NOT WIN32)

FIND_PATH(LibAttica_INCLUDE_DIR attica/provider.h
   HINTS
   ${PC_LibAttica_INCLUDEDIR}
   ${PC_LibAttica_INCLUDE_DIRS}
   PATH_SUFFIXES attica
   )

# Store the version number in the cache, so we don't have to search every time:
IF(LibAttica_INCLUDE_DIR  AND NOT  LibAttica_VERSION)
  FILE(READ ${LibAttica_INCLUDE_DIR}/attica/version.h LibAttica_VERSION_CONTENT)
  STRING (REGEX MATCH "LibAttica_VERSION_STRING \".*\"\n" LibAttica_VERSION_MATCH "${LibAttica_VERSION_CONTENT}")
  IF(LibAttica_VERSION_MATCH)
    STRING(REGEX REPLACE "LibAttica_VERSION_STRING \"(.*)\"\n" "\\1" _LibAttica_VERSION ${LibAttica_VERSION_MATCH})
  ENDIF(LibAttica_VERSION_MATCH)
  SET(LibAttica_VERSION "${_LibAttica_VERSION}" CACHE STRING "Version number of LibAttica" FORCE)
ENDIF(LibAttica_INCLUDE_DIR  AND NOT  LibAttica_VERSION)


FIND_LIBRARY(LibAttica_LIBRARIES NAMES attica libattica
   HINTS
   ${PC_LibAttica_LIBDIR}
   ${PC_LibAttica_LIBRARY_DIRS}
   )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibAttica  REQUIRED_VARS LibAttica_LIBRARIES LibAttica_INCLUDE_DIR
                                             VERSION_VAR LibAttica_VERSION
                                             FOUND_VAR LibAttica_FOUND)

MARK_AS_ADVANCED(LibAttica_INCLUDE_DIR LibAttica_LIBRARIES)
