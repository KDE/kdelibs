# - Try to find automoc4
# Once done this will define
#
#  AUTOMOC4_FOUND - automoc4 has been found
#  AUTOMOC4_EXECUTABLE - the automoc4 tool
#
# It also adds the following macros
#  AUTOMOC4(<target> <SRCS_VAR>)
#    Use this to run automoc4 on all files contained in the list <SRCS_VAR>.
#
#  AUTOMOC4_MOC_HEADERS(<target> header1.h header2.h)
#    Use this to add more header files to be processed with automoc4.


# Copyright (c) 2008, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# check if we are inside KDESupport and automoc is enabled
if("${KDESupport_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
   # when building this project as part of kdesupport
   set(AUTOMOC4_CONFIG_FILE "${KDESupport_SOURCE_DIR}/automoc/Automoc4Config.cmake")
else("${KDESupport_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
   # when building this project outside kdesupport

   # CMAKE_[SYSTEM_]PREFIX_PATH exists starting with cmake 2.6.0
   file(TO_CMAKE_PATH "$ENV{CMAKE_PREFIX_PATH}" _env_CMAKE_PREFIX_PATH)
   file(TO_CMAKE_PATH "$ENV{CMAKE_LIBRARY_PATH}" _env_CMAKE_LIBRARY_PATH)

   set(AUTOMOC4_SEARCH_PATHS
             ${_env_CMAKE_PREFIX_PATH} ${CMAKE_PREFIX_PATH}
             ${_env_CMAKE_LIBRARY_PATH} ${CMAKE_LIBRARY_PATH}
             ${CMAKE_INSTALL_PREFIX} ${CMAKE_SYSTEM_PREFIX_PATH} ${CMAKE_SYSTEM_LIBRARY_PATH})

   find_file(AUTOMOC4_CONFIG_FILE NAMES Automoc4Config.cmake
             PATH_SUFFIXES automoc4 lib/automoc4 lib64/automoc4
             PATHS ${AUTOMOC4_SEARCH_PATHS}
             NO_DEFAULT_PATH )
endif("${KDESupport_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")


if(AUTOMOC4_CONFIG_FILE)
   include(${AUTOMOC4_CONFIG_FILE})
   set(AUTOMOC4_FOUND TRUE)
else(AUTOMOC4_CONFIG_FILE)
   set(AUTOMOC4_FOUND FALSE)
endif(AUTOMOC4_CONFIG_FILE)

if (AUTOMOC4_FOUND)
   if (NOT Automoc4_FIND_QUIETLY)
      message(STATUS "Found Automoc4: ${AUTOMOC4_EXECUTABLE}")
   endif (NOT Automoc4_FIND_QUIETLY)
else (AUTOMOC4_FOUND)
   if (Automoc4_FIND_REQUIRED)
      message(FATAL_ERROR "Did not find Automoc4Config.cmake (part of kdesupport). Searched in ${AUTOMOC4_SEARCH_PATHS} using suffixes automoc4 lib/automoc4 lib64/automoc4.")
   else (Automoc4_FIND_REQUIRED)
      if (NOT Automoc4_FIND_QUIETLY)
         message(STATUS "Did not find Automoc4Config.cmake (part of kdesupport). Searched in ${AUTOMOC4_SEARCH_PATHS} using suffixes automoc4 lib/automoc4 lib64/automoc4.")
      endif (NOT Automoc4_FIND_QUIETLY)
   endif (Automoc4_FIND_REQUIRED)
endif (AUTOMOC4_FOUND)
