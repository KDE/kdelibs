# - Try to find the Qt4 binding of the Poppler library
# Once done this will define
#
#  POPPLER_QT4_FOUND - system has poppler-qt4
#  POPPLER_QT4_INCLUDE_DIR - the poppler-qt4 include directory
#  POPPLER_QT4_LIBRARIES - Link these to use poppler-qt4
#  POPPLER_QT4_DEFINITIONS - Compiler switches required for using poppler-qt4
#

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls

# Copyright (c) 2006, Wilfried Huss, <wilfried.huss@gmx.at>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


find_package(PkgConfig)
pkg_check_modules(PC_POPPLERQT4 QUIET poppler-qt4)

set(POPPLER_QT4_DEFINITIONS ${PC_POPPLERQT4_CFLAGS_OTHER})

find_path(POPPLER_QT4_INCLUDE_DIR
  NAMES poppler-qt4.h
  HINTS ${PC_POPPLERQT4_INCLUDEDIR}
  PATH_SUFFIXES poppler/qt4 poppler
)

find_library(POPPLER_QT4_LIBRARY
  NAMES poppler-qt4
  HINTS ${PC_POPPLERQT4_LIBDIR}
)

set(POPPLER_QT4_LIBRARIES ${POPPLER_QT4_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PopplerQt4 REQUIRED_VARS POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES)

# for compatibility:
set(POPPLER_QT4_FOUND ${POPPLERQT4_FOUND})
  
mark_as_advanced(POPPLER_QT4_INCLUDE_DIR POPPLER_QT4_LIBRARIES)
