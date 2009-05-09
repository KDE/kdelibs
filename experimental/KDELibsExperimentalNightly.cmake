# This is a script for running a Nightly build of kdelibs/experimental/.
# It is ready for testing.
# To adapt it for other projects, basically only the KDE_CTEST_VCS_REPOSITORY variable
# has to be changed.
#
# It uses the file KDECTestNightly.cmake, which is in KDE svn in kdesdk/cmake/modules/. 
# You need to have this file on some location on your system and then point the environment variable
# KDECTESTNIGHTLY_DIR to the directory containing this file when running this script.
#
# For more details have a look at kdelibs/KDELibsNightly.cmake in KDE svn 
#
# Alex <neundorf AT kde.org>

# The VCS of KDE is "svn", also specify the repository
set(KDE_CTEST_VCS svn)
set(KDE_CTEST_VCS_REPOSITORY https://svn.kde.org/home/kde/trunk/KDE/kdelibs/experimental)

# for now hardcode the generator to "Unix Makefiles"
set(CTEST_CMAKE_GENERATOR "Unix Makefiles" )


# generic support code, provides the kde_ctest_setup() macro, which sets up everything required:
file(TO_CMAKE_PATH  $ENV{KDECTESTNIGHTLY_DIR}  KDECTESTNIGHTLY_DIR)
include( "${KDECTESTNIGHTLY_DIR}/KDECTestNightly.cmake"  OPTIONAL  RESULT_VARIABLE fileIncluded)

if(NOT fileIncluded)
   message(FATAL_ERROR "Did not find file ${KDECTESTNIGHTLY_DIR}/KDECTestNightly.cmake . Set the environment variable KDECTESTNIGHTLY_DIR  to the directory where this file is located. In KDE svn it is in kdesdk/cmake/modules/ ")
endif(NOT fileIncluded)


# set up binary dir, source dir, etc.
kde_ctest_setup("${CMAKE_CURRENT_LIST_FILE}")


# now actually do the Nightly
ctest_empty_binary_directory("${CTEST_BINARY_DIRECTORY}")
ctest_start(Nightly)
ctest_update(SOURCE "${CTEST_SOURCE_DIRECTORY}" )

# read some settings
include("${CTEST_SOURCE_DIRECTORY}/CTestConfig.cmake")
include("${CTEST_SOURCE_DIRECTORY}/CTestCustom.cmake" OPTIONAL)

# if CMAKE_INSTALL_PREFIX was defined on the command line, put it in the initial cache, so cmake gets it
kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}" CMAKE_INSTALL_PREFIX )

ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_submit()

# optionally install afterwards, so additional nightly builds can use this current install 
# (e.g. kdepimlibs could use this kdelibs install)
if(DO_INSTALL)
   kde_ctest_install("${CTEST_BINARY_DIRECTORY}" )
endif(DO_INSTALL)
