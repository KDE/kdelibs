# The following variables are set by the script if they haven't been set before:
#
# CTEST_SOURCE_DIRECTORY - defaults to $HOME/Dashboards/<CTEST_PROJECT_NAME>/src dir/
# CTEST_BINARY_DIRECTORY - defaults to $HOME/Dashboards/<CTEST_PROJECT_NAME>/build dir/
# if CTEST_AVOID_SPACES is TRUE, the spaces in the path above will be removed.


###########################################################
# generic code
###########################################################

cmake_minimum_required(VERSION 2.6)

include(CMakeDetermineSystem)
if(CMAKE_HOST_UNIX)
   include(Platform/UnixPaths)
endif(CMAKE_HOST_UNIX)

if(CMAKE_HOST_WIN32)
   include(Platform/WindowsPaths)
endif(CMAKE_HOST_WIN32)

get_filename_component(currentDirectory "${CMAKE_CURRENT_LIST_FILE}" PATH)

if(NOT EXISTS "${currentDirectory}/CMakeLists.txt")
   message(FATAL_ERROR "This script must be in the source tree of your project.")
endif(NOT EXISTS "${currentDirectory}/CMakeLists.txt")

include("${currentDirectory}/CTestConfig.cmake")
include("${currentDirectory}/CTestCustom.cmake" OPTIONAL)

set(DASHBOARD_DIR "$ENV{HOME}/Dashboards" )

if(NOT DEFINED CTEST_SOURCE_DIRECTORY)
   if(CTEST_AVOID_SPACES)
      set(CTEST_SOURCE_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/srcdir" )
   else(CTEST_AVOID_SPACES)
      set(CTEST_SOURCE_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/src dir" )
   endif(CTEST_AVOID_SPACES)
endif(NOT DEFINED CTEST_SOURCE_DIRECTORY)

if(NOT DEFINED CTEST_BINARY_DIRECTORY)
   if(CTEST_AVOID_SPACES)
      set(CTEST_BINARY_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/builddir" )
   else(CTEST_AVOID_SPACES)
      set(CTEST_BINARY_DIRECTORY "${DASHBOARD_DIR}/${CTEST_PROJECT_NAME}/build dir" )
   endif(CTEST_AVOID_SPACES)
endif(NOT DEFINED CTEST_BINARY_DIRECTORY)



site_name(CTEST_SITE)
set(CTEST_BUILD_NAME ${CMAKE_SYSTEM_NAME})

if("${CTEST_CMAKE_GENERATOR}" MATCHES Makefile)
   find_program(MAKE_EXECUTABLE make gmake)
   set(CTEST_BUILD_COMMAND    "${MAKE_EXECUTABLE}" )
else("${CTEST_CMAKE_GENERATOR}" MATCHES Makefile)
   if(NOT DEFINED CTEST_BUILD_COMMAND)
      message(FATAL_ERROR "CTEST_CMAKE_GENERATOR is set to \"${CTEST_CMAKE_GENERATOR}\", but CTEST_BUILD_COMMAND has not been set")
   endif(NOT DEFINED CTEST_BUILD_COMMAND)
endif("${CTEST_CMAKE_GENERATOR}" MATCHES Makefile)

string(TOLOWER ${CTEST_VCS} _ctest_vcs)
set(_have_vcs FALSE)
# only set this if there is no checkout yet
set(CTEST_CHECKOUT_COMMAND)

if ("${_ctest_vcs}" STREQUAL svn)
   find_program(SVN_EXECUTABLE svn)
   if (NOT SVN_EXECUTABLE)
      message(FATAL_ERROR "Error: CTEST_VCS is svn, but could not find svn executable")
   endif (NOT SVN_EXECUTABLE)
   set(CTEST_UPDATE_COMMAND ${SVN_EXECUTABLE})
   if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.svn/entries")
      set(CTEST_CHECKOUT_COMMAND "${SVN_EXECUTABLE} co ${CTEST_VCS_REPOSITORY}/${CTEST_VCS_PATH} \"${CTEST_SOURCE_DIRECTORY}\"")
   endif(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/.svn/entries")
   set(_have_vcs TRUE)
endif ("${_ctest_vcs}" STREQUAL svn)

if ("${_ctest_vcs}" STREQUAL cvs)
   find_program(CVS_EXECUTABLE cvs cvsnt)
   if (NOT CVS_EXECUTABLE)
      message(FATAL_ERROR "Error: CTEST_VCS is cvs, but could not find cvs or cvsnt executable")
   endif (NOT CVS_EXECUTABLE)
   set(CTEST_UPDATE_COMMAND ${CVS_EXECUTABLE})
   if(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/CVS/Entries")
      set(CTEST_CHECKOUT_COMMAND "${CVS_EXECUTABLE} -d ${CTEST_VCS_REPOSITORY} co  -d \"${CTEST_SOURCE_DIRECTORY}\" ${CTEST_VCS_PATH}")
   endif(NOT EXISTS "${CTEST_SOURCE_DIRECTORY}/CVS/Entries")
   set(_have_vcs TRUE)
endif ("${_ctest_vcs}" STREQUAL cvs)
