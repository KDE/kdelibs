# Module that tries to find the Kipi library
#
# Input values :
#
# KIPI_LOCAL_DIR    - If you have put a local version of libkipi into your source tree,
#                     set this variable to the relative path from the local directory.
#
# Output values :
#
#  KIPI_FOUND       - System has libkipi
#  KIPI_INCLUDE_DIR - The libkipi include directory
#  KIPI_LIBRARIES   - Link these to use libkipi
#  KIPI_DEFINITIONS - Compiler switches required for using libkipi
#  KIPI_VERSION     - The release version of the Kipi library
#  KIPI_SO_VERSION  - The binary SO version of the Kipi library
#

# Copyright (c) 2012, Victor Dodon <dodonvictor at gmail dot com>
# Copyright (c) 2012, Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF("${Kipi_FIND_VERSION}" STREQUAL "")
    SET(Kipi_FIND_VERSION "1.2.0")
    MESSAGE(STATUS "No Kipi library version required. Check default version : ${Kipi_FIND_VERSION}")
ELSE()
    MESSAGE(STATUS "Kipi library version required : ${Kipi_FIND_VERSION}")
ENDIF()

IF(KIPI_INCLUDE_DIR AND KIPI_LIBRARIES AND KIPI_DEFINITIONS AND KIPI_VERSION AND KIPI_SO_VERSION)

  IF(NOT Kipi_FIND_QUIETLY)
    MESSAGE(STATUS "Found kipi library in cache ${KIPI_LIBRARIES}")
  ENDIF(NOT Kipi_FIND_QUIETLY)
  # Already in cache
  SET(KIPI_FOUND TRUE)

ELSE(KIPI_INCLUDE_DIR AND KIPI_LIBRARIES AND KIPI_DEFINITIONS AND KIPI_VERSION AND KIPI_SO_VERSION)

  IF(NOT Kipi_FIND_QUIETLY)
    MESSAGE(STATUS "Check Kipi library in local sub-folder...")
  ENDIF(NOT Kipi_FIND_QUIETLY)

  IF(KIPI_LOCAL_DIR)
    FIND_FILE(KIPI_LOCAL_FOUND libkipi/version.h.cmake ${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR} NO_DEFAULT_PATH)
    IF(NOT KIPI_LOCAL_FOUND)
      MESSAGE(WARNING "KIPI_LOCAL_DIR specified as \"${KIPI_LOCAL_DIR}\" but libkipi could not be found there.")
    ENDIF(NOT KIPI_LOCAL_FOUND)
  ELSE(KIPI_LOCAL_DIR)
    FIND_FILE(KIPI_LOCAL_FOUND libkipi/version.h.cmake ${CMAKE_SOURCE_DIR}/libkipi NO_DEFAULT_PATH)
    IF(KIPI_LOCAL_FOUND)
      SET(KIPI_LOCAL_DIR libkipi)
    ENDIF(KIPI_LOCAL_FOUND)

    FIND_FILE(KIPI_LOCAL_FOUND libkipi/version.h.cmake ${CMAKE_SOURCE_DIR}/libs/libkipi NO_DEFAULT_PATH)
    IF(KIPI_LOCAL_FOUND)
      SET(KIPI_LOCAL_DIR libs/libkipi)
    ENDIF(KIPI_LOCAL_FOUND)
  ENDIF(KIPI_LOCAL_DIR)

  IF(KIPI_LOCAL_FOUND)

    SET(KIPI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/${KIPI_LOCAL_DIR}")
    SET(KIPI_DEFINITIONS "-I${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR}" "-I${CMAKE_BINARY_DIR}/${KIPI_LOCAL_DIR}")
    SET(KIPI_LIBRARIES kipi)
    IF(NOT Kipi_FIND_QUIETLY)
      MESSAGE(STATUS "Found Kipi library in local sub-folder: ${CMAKE_SOURCE_DIR}/${KIPI_LOCAL_DIR}")
    ENDIF(NOT Kipi_FIND_QUIETLY)
    SET(KIPI_FOUND TRUE)
    SET(KIPI_VERSION_H_FILENAME "${CMAKE_BINARY_DIR}/${KIPI_LOCAL_DIR}/libkipi/version.h")

  ELSE(KIPI_LOCAL_FOUND)

    IF(NOT WIN32)
      IF(NOT Kipi_FIND_QUIETLY)
        MESSAGE(STATUS "Check Kipi library using pkg-config...")
      ENDIF(NOT Kipi_FIND_QUIETLY)

      INCLUDE(FindPkgConfig)
      PKG_CHECK_MODULES(PC_KIPI libkipi>=${Kipi_FIND_VERSION})
    ENDIF(NOT WIN32)

    FIND_LIBRARY(KIPI_LIBRARIES NAMES kipi HINTS ${PC_KIPI_LIBRARY_DIRS} ${LIB_INSTALL_DIR} ${KDE4_LIB_DIR})
    FIND_PATH(KIPI_INCLUDE_DIR NAMES libkipi/version.h HINTS ${PC_KIPI_INCLUDE_DIRS} ${INCLUDE_INSTALL_DIR} ${KDE4_INCLUDE_DIR})
    SET(KIPI_VERSION_H_FILENAME "${KIPI_INCLUDE_DIR}/libkipi/version.h")
    SET(KIPI_DEFINITIONS ${PC_KIPI_CFLAGS} CACHE STRING "Kipi defintions")

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(KIPI DEFAULT_MSG KIPI_LIBRARIES KIPI_INCLUDE_DIR)

  ENDIF(KIPI_LOCAL_FOUND)

  IF(KIPI_FOUND)

    IF(NOT KIPI_VERSION)
      FILE(READ "${KIPI_VERSION_H_FILENAME}" KIPI_VERSION_H_CONTENT)
      STRING(REGEX REPLACE ".*static +const +char +kipi_version\\[\\] += +\"([^\"]+)\".*" "\\1" KIPI_VERSION "${KIPI_VERSION_H_CONTENT}")
      MESSAGE(STATUS "Kipi library version: ${KIPI_VERSION}")
    ENDIF(NOT KIPI_VERSION)

    IF(NOT KIPI_SO_VERSION)
      FILE(READ "${KIPI_VERSION_H_FILENAME}" KIPI_VERSION_H_CONTENT)
      STRING(REGEX REPLACE
             ".*static +const +int +kipi_binary_version +=  ([^ ;]+).*"
             "\\1"
             KIPI_SO_VERSION_FOUND
             "${KIPI_VERSION_H_CONTENT}"
            )
      SET(KIPI_SO_VERSION ${KIPI_SO_VERSION_FOUND} CACHE STRING "libkipi so version")
      MESSAGE(STATUS "Kipi library SO binary version: ${KIPI_SO_VERSION}")
    ENDIF(NOT KIPI_SO_VERSION)

    UNSET(KIPI_VERSION_H_CONTENT)
    UNSET(KIPI_VERSION_H_FILENAME)
  ENDIF(KIPI_FOUND)

  IF(KIPI_FOUND)
    MESSAGE(STATUS "libkipi: Found version ${KIPI_VERSION} (required: ${Kipi_FIND_VERSION})")
    IF(${KIPI_VERSION} VERSION_LESS ${Kipi_FIND_VERSION})
        SET(KIPI_FOUND FALSE)
    ELSE()
        MARK_AS_ADVANCED(KIPI_INCLUDE_DIR KIPI_LIBRARIES KIPI_DEFINITIONS KIPI_VERSION KIPI_SO_VERSION)
    ENDIF()
  ELSE(KIPI_FOUND)
    UNSET(KIPI_INCLUDE_DIR)
    UNSET(KIPI_LIBRARIES)
    UNSET(KIPI_DEFINITIONS)
    UNSET(KIPI_VERSION)
    UNSET(KIPI_SO_VERSION)
  ENDIF(KIPI_FOUND)

ENDIF(KIPI_INCLUDE_DIR AND KIPI_LIBRARIES AND KIPI_DEFINITIONS AND KIPI_VERSION AND KIPI_SO_VERSION)
