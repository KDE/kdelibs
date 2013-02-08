# - Try to find Flex
# Once done this will define
#
#  FLEX_FOUND - system has Flex
#  FLEX_EXECUTABLE - path of the flex executable
#  FLEX_VERSION - the version string, like "2.5.31"
#
# The minimum required version of Flex can be specified using the
# standard syntax, e.g. find_package(Flex 2.5)


# Copyright (c) 2008, Jakob Petsovits, <jpetso@gmx.at>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF(WIN32)
    # Store CMAKE_MODULE_PATH so that the cmake supplied FindFLEX.cmake is
    # used since cmake FIND_PACKAGE is case insensitive on Windows
    SET(_kde_cmake_module_path_back ${CMAKE_MODULE_PATH})
    SET(CMAKE_MODULE_PATH)
    FIND_PACKAGE(FLEX)
    SET(CMAKE_MODULE_PATH ${_kde_cmake_module_path_back})
    SET(_kde_cmake_module_path_back)
ELSE(WIN32)
    FIND_PROGRAM(FLEX_EXECUTABLE NAMES flex)
    FIND_PATH(FLEX_INCLUDE_DIR FlexLexer.h )
    FIND_LIBRARY(FLEX_LIBRARY fl)

    IF(FLEX_EXECUTABLE  AND NOT  FLEX_VERSION)

        EXECUTE_PROCESS(COMMAND ${FLEX_EXECUTABLE} --version
            OUTPUT_VARIABLE _FLEX_VERSION_OUTPUT
        )
        STRING (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" _FLEX_VERSION "${_FLEX_VERSION_OUTPUT}")
        SET(FLEX_VERSION "${_FLEX_VERSION}" CACHE STRING "Version number of Flex" FORCE)
    ENDIF(FLEX_EXECUTABLE  AND NOT  FLEX_VERSION)


    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Flex REQUIRED_VARS FLEX_EXECUTABLE
                                           VERSION_VAR FLEX_VERSION )

    MARK_AS_ADVANCED(FLEX_INCLUDE_DIR FLEX_LIBRARY FLEX_EXECUTABLE)
ENDIF(WIN32)
