
enable_testing()
SET(BUILD_TESTING ON CACHE INTERNAL "Build the testing tree (internal)")

# support for Dart: http://public.kitware.com/dashboard.php?name=kde
if (EXISTS ${CMAKE_SOURCE_DIR}/CTestConfig.cmake)
   include(CTest)
endif (EXISTS ${CMAKE_SOURCE_DIR}/CTestConfig.cmake)

# Always include srcdir and builddir in include path
# This saves typing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} in about every subdir
# since cmake 2.4.0
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# define the generic version of the libraries here
# this makes it easy to advance it when the next KDE release comes
# Use this version number for libraries which are at version n in KDE version n
set(GENERIC_LIB_VERSION "4.5.0")
set(GENERIC_LIB_SOVERSION "4")

# Use this version number for libraries which are already at version n+1 in KDE version n
set(KDE_NON_GENERIC_LIB_VERSION "5.5.0")
set(KDE_NON_GENERIC_LIB_SOVERSION "5")

# windows does not support LD_LIBRARY_PATH or similar
# all searchable directories has to be defined by the PATH environment var
# to reduce the number of required pathes executables are placed into
# the build bin dir
if (WIN32)
 set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif(WIN32)
