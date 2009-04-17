set(CTEST_VCS svn)
set(CTEST_VCS_REPOSITORY https://neundorf@svn.kde.org/home/kde/trunk/KDE/kdelibs)
set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )

# set(CTEST_AVOID_SPACES FALSE)

get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)

# generic support code, sets up everything required:
include( "${_currentDir}/KDECTestNightly.cmake" )

ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
ctest_start(Nightly)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" )
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_submit()

