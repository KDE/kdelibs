# This is a script for ctest for running a nightly build.
# Still pre-alpha, don't run it if you not working on it.
# Alex

#############################
# for cmake 2.6.[0..2]:
if(NOT CMAKE_SYSTEM)
   include(CMakeDetermineSystem)
   if(CMAKE_HOST_UNIX)
      include(Platform/UnixPaths)
   endif(CMAKE_HOST_UNIX)
   if(CMAKE_HOST_WIN32)
      include(Platform/WindowsPaths)
   endif(CMAKE_HOST_WIN32)
endif(NOT CMAKE_SYSTEM)

#############################

set(CTEST_UPDATE_TYPE svn)
set(SVN_REPOSITORY https://svn.kde.org/home/kde/trunk/KDE/kdelibs )
set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )

# this is default, so it is not necessary
# set(CTEST_AVOID_SPACES FALSE)


include (${CTEST_SCRIPT_DIRECTORY}/cmake/modules/KDE4CTestNightlySetup.cmake)


ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
ctest_start(Experimental)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" )
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" )
# ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_submit()
