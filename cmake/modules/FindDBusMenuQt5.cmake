# - Try to find dbusmenu-qt5
# This module helps finding an installation of the DBusMenuQt library (see https://launchpad.net/libdbusmenu-qt/)
# Once done this will define
#
#  DBusMenuQt5_FOUND - system has dbusmenu-qt
#  DBusMenuQt5_INCLUDE_DIRS - the dbusmenu-qt include directory
#  DBusMenuQt5_LIBRARIES - the libraries needed to use dbusmenu-qt
#  DBusMenuQt5_DEFINITIONS - Compiler switches required for using dbusmenu-qt
#
# The minimum required version of DBusMenuQt can be specified using the
# standard syntax, e.g. find_package(DBusMenuQt5 0.6)
#

# Copyright (c) 2009, Canonical Ltd.
# - Author: Aurélien Gâteau <aurelien.gateau@canonical.com>
#
# Based on FindQCA2.cmake
# Copyright (c) 2006, Michael Larouche, <michael.larouche@kdemail.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindPackageHandleStandardArgs)

find_package(PkgConfig)
pkg_check_modules(PC_DBusMenuQt5 QUIET dbusmenu-qt5)


set(DBusMenuQt5_DEFINITIONS ${PC_DBusMenuQt5_CFLAGS_OTHER})

find_library(DBusMenuQt5_LIBRARY
    NAMES dbusmenu-qt5 dbusmenu-qt5d
    HINTS ${PC_DBusMenuQt5_LIBDIR} ${PC_DBusMenuQt5_LIBRARY_DIRS}
    )

find_path(DBusMenuQt5_INCLUDE_DIR dbusmenuexporter.h
    HINTS ${PC_DBusMenuQt5_INCLUDEDIR} ${PC_DBusMenuQt5_INCLUDE_DIRS}
    PATH_SUFFIXES dbusmenu-qt5 dbusmenu-qt
    )


# find the version number from dbusmenu_version.h and store it in the cache
if(DBusMenuQt5_INCLUDE_DIR  AND NOT DBusMenuQt5_VERSION)
  # parse the version number out from dbusmenu_version:
  if(EXISTS ${DBusMenuQt5_INCLUDE_DIR}/dbusmenu_version.h)
    file(READ "${DBusMenuQt5_INCLUDE_DIR}/dbusmenu_version.h" DBusMenuQt5_VERSION_CONTENT)

    string(REGEX MATCH "#define +DBUSMENUQT_VERSION_MAJOR +([0-9]+)"  _dummy "${DBusMenuQt5_VERSION_CONTENT}")
    set(DBusMenuQt5_VERSION_MAJOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define +DBUSMENUQT_VERSION_MINOR +([0-9]+)"  _dummy "${DBusMenuQt5_VERSION_CONTENT}")
    set(DBusMenuQt5_VERSION_MINOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define +DBUSMENUQT_VERSION_PATCH +([0-9]+)"  _dummy "${DBusMenuQt5_VERSION_CONTENT}")
    set(DBusMenuQt5_VERSION_PATCH "${CMAKE_MATCH_1}")

  endif(EXISTS ${DBusMenuQt5_INCLUDE_DIR}/dbusmenu_version.h)

  set(DBusMenuQt5_VERSION "${DBusMenuQt5_VERSION_MAJOR}.${DBusMenuQt5_VERSION_MINOR}.${DBusMenuQt5_VERSION_PATCH}" CACHE STRING "Version number of DBusMenuQt5" FORCE)
endif()


find_package_handle_standard_args(DBusMenuQt5 REQUIRED_VARS DBusMenuQt5_LIBRARY DBusMenuQt5_INCLUDE_DIR
                                             VERSION_VAR DBusMenuQt5_VERSION FOUND_VAR DBusMenuQt5_FOUND)

set(DBusMenuQt5_LIBRARIES ${DBusMenuQt5_LIBRARY})
set(DBusMenuQt5_INCLUDE_DIRS ${DBusMenuQt5_INCLUDE_DIR})

mark_as_advanced(DBusMenuQt5_INCLUDE_DIR DBusMenuQt5_LIBRARY DBusMenuQt5_VERSION)
