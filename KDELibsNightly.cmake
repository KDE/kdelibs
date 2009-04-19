# This is a script for running a Nightly build of kdelibs.
# It's still work in progress.
#
# Alex <neundorf AT kde.org>

set(CTEST_VCS svn)
set(CTEST_VCS_REPOSITORY https://svn.kde.org/home/kde/trunk/KDE/kdelibs)
set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )

#ctest_read_custom_files()

# set(CTEST_AVOID_SPACES FALSE)

# generic support code, sets up everything required:
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include( "${_currentDir}/KDECTestNightly.cmake" )

ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
ctest_start(Nightly)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" )
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_submit()

