# This is a script for running a Nightly build of kdelibs.
# It's still work in progress.
#
# Alex <neundorf AT kde.org>

set(KDE_CTEST_VCS svn)
set(KDE_CTEST_VCS_REPOSITORY https://svn.kde.org/home/kde/trunk/KDE/kdelibs)
set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )

# set(KDE_CTEST_AVOID_SPACES FALSE)


# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
file(TO_CMAKE_PATH  $ENV{KDECTESTNIGHTLY_DIR}  KDECTESTNIGHTLY_DIR)
include( "${KDECTESTNIGHTLY_DIR}/KDECTestNightly.cmake"  OPTIONAL  RESULT_VARIABLE fileIncluded)

if(NOT fileIncluded)
   message(FATAL_ERROR "Did not find file ${KDECTESTNIGHTLY_DIR}/KDECTestNightly.cmake . Set the environment variable KDECTESTNIGHTLY_DIR  to the directory where this file is located. In KDE svn it is in kdesdk/cmake/modules/ ")
endif(NOT fileIncluded)

kde_ctest_setup("${CMAKE_CURRENT_LIST_FILE}")


# now actually do the Nightly
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
ctest_start(Nightly)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" )

include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
include("${CTEST_SOURCE_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_submit()
