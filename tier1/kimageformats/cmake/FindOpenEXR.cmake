# Try to find the OpenEXR libraries
#
# This will define:
#
#   OpenEXR_FOUND        - True if OpenEXR is available
#   OpenEXR_LIBRARIES    - Link to these to use OpenEXR
#   OpenEXR_INCLUDE_DIRS - Include directory for OpenEXR
#   OpenEXR_DEFINITIONS  - Compiler flags required to link against OpenEXR
#
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2013, Alex Merry, <alex.merry@kdemail.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_check_modules(PC_OpenEXR QUIET OpenEXR)

set(OpenEXR_DEFINITIONS ${PC_OpenEXR_CFLAGS_OTHER})

find_path(OpenEXR_INCLUDE_DIR ImfRgbaFile.h
   PATHS
   ${PC_OpenEXR_INCLUDEDIR}
   ${PC_OpenEXR_INCLUDE_DIRS}
   PATH_SUFFIXES OpenEXR
)

# Required libraries for OpenEXR
find_library(OpenEXR_HALF_LIBRARY NAMES Half
   PATHS
   ${PC_OpenEXR_LIBDIR}
   ${PC_OpenEXR_LIBRARY_DIRS}
)
find_library(OpenEXR_IEX_LIBRARY NAMES Iex
   PATHS
   ${PC_OpenEXR_LIBDIR}
   ${PC_OpenEXR_LIBRARY_DIRS}
)
find_library(OpenEXR_IMATH_LIBRARY NAMES Imath
   PATHS
   ${PC_OpenEXR_LIBDIR}
   ${PC_OpenEXR_LIBRARY_DIRS}
)
find_library(OpenEXR_ILMTHREAD_LIBRARY NAMES IlmThread
   PATHS
   ${PC_OpenEXR_LIBDIR}
   ${PC_OpenEXR_LIBRARY_DIRS}
)
# This is the actual OpenEXR library
find_library(OpenEXR_ILMIMF_LIBRARY NAMES IlmImf
   PATHS
   ${PC_OpenEXR_LIBDIR}
   ${PC_OpenEXR_LIBRARY_DIRS}
)

set(OpenEXR_LIBRARIES
   ${OpenEXR_HALF_LIBRARY}
   ${OpenEXR_IEX_LIBRARY}
   ${OpenEXR_IMATH_LIBRARY}
   ${OpenEXR_ILMIMF_LIBRARY}
   ${OpenEXR_ILMTHREAD_LIBRARY})

if (OpenEXR_INCLUDE_DIR AND EXISTS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h")
    file(STRINGS "${OpenEXR_INCLUDE_DIR}/OpenEXRConfig.h" openexr_version_str
         REGEX "^#define[\t ]+OPENEXR_VERSION_STRING[\t ]+\"[^\"]*\"")
    string(REGEX REPLACE "^#define[\t ]+OPENEXR_VERSION_STRING[\t ]+\"([^\"]*).*"
           "\\1" OpenEXR_VERSION_STRING "${openexr_version_str}")
    unset(openexr_version_str)
endif ()

include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args reports the value of the first variable
# on success, so make sure this is the actual OpenEXR library
find_package_handle_standard_args(OpenEXR
   FOUND_VAR OpenEXR_FOUND
   REQUIRED_VARS
      OpenEXR_ILMIMF_LIBRARY
      OpenEXR_HALF_LIBRARY
      OpenEXR_IEX_LIBRARY
      OpenEXR_IMATH_LIBRARY
      OpenEXR_ILMTHREAD_LIBRARY
      OpenEXR_INCLUDE_DIR
   VERSION_VAR OpenEXR_VERSION_STRING)

set(OpenEXR_INCLUDE_DIRS ${OpenEXR_INCLUDE_DIR})

include(FeatureSummary)
set_package_properties(OpenEXR PROPERTIES
   URL http://www.openexr.com/
   DESCRIPTION "A library for reading and writing OpenEXR high dynamic-range image files")

mark_as_advanced(
   OpenEXR_INCLUDE_DIR
   OpenEXR_LIBRARIES
   OpenEXR_DEFINITIONS
   OpenEXR_ILMIMF_LIBRARY
   OpenEXR_ILMTHREAD_LIBRARY
   OpenEXR_IMATH_LIBRARY
   OpenEXR_IEX_LIBRARY
   OpenEXR_HALF_LIBRARY
)
