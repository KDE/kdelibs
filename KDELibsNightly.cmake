# This is a script for running a Nightly build of kdelibs.
# It is ready for testing.
# To adapt it for other projects, basically only the KDE_CTEST_VCS_REPOSITORY variable
# has to be changed.
#
# It uses the file KDECTestNightly.cmake, which is in KDE svn in kdesdk/cmake/modules/. 
# You need to have this file on some location on your system and then point the environment variable
# KDECTESTNIGHTLY_DIR to the directory containing this file when running this script.
#
# At the bottom of this file you can find  (commented out) a simple shell script which 
# I use to drive the Nightly builds on my machine. You have to adapt this to the 
# conditions on your system, then you can run it e.g. via cron.
#
# Alex <neundorf AT kde.org>

# The VCS of KDE is "svn", also specify the repository
set(KDE_CTEST_VCS svn)
set(KDE_CTEST_VCS_REPOSITORY https://svn.kde.org/home/kde/trunk/KDE/kdelibs)

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

# build the tests
set(KDE4_BUILD_TESTS TRUE)

# if CMAKE_INSTALL_PREFIX and BUILD_experimental were defined on the command line, put them
# in the initial cache, so cmake gets them
kde_ctest_write_initial_cache("${CTEST_BINARY_DIRECTORY}" CMAKE_INSTALL_PREFIX 
                                                          BUILD_experimental 
                                                          KDE4_BUILD_TESTS)

# configure, build, test, submit
ctest_configure(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_build(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_test(BUILD "${CTEST_BINARY_DIRECTORY}" )
ctest_submit()

# optionally install afterwards, so additional nightly builds can use this current install 
# (e.g. kdepimlibs could use this kdelibs install)
if(DO_INSTALL)
   kde_ctest_install("${CTEST_BINARY_DIRECTORY}" )
endif(DO_INSTALL)


############################################################################################
#
# ------------8<-----------------8<---------------------8<---------------------8<-----------
# #!/bin/sh
#
# # point to the directory where KDECTestNightly.cmake is located (in svn: kdesvn/cmake/modules/ )
# export KDECTESTNIGHTLY_DIR=/home/alex/src/kde4-svn/KDE\ dir/kdesdk/cmake/modules/
#
#
# # Set which ctest will be used, where the results should be installed to, and which suffix the 
# # build name on my.cdash.org should get:
# CTEST=/opt/cmake-2.6.2-Linux-i386/bin/ctest
# INSTALL_ROOT=/home/alex/Dashboards/installs/2.6.2
# SUFFIX=cmake-2.6.2
#
# # set CMAKE_PREFIX_PATH so that everything necessary for building automoc4 will be found:
# export CMAKE_PREFIX_PATH=/opt/qt-4.5/qt/
#
# $CTEST -V -VV -S ~/src/kde4-svn/kdesupport/automoc/Automoc4Nightly.cmake,KDE_CTEST_BUILD_SUFFIX=$SUFFIX,CMAKE_INSTALL_PREFIX=$INSTALL_ROOT/automoc4,DO_INSTALL=TRUE
#
# # For building kdelibs we need more stuff in CMAKE_PREFIX_PATH:
# export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$INSTALL_ROOT/automoc4
# export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/opt/shared-mime-info
# export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/opt/kdesupport/
#
# $CTEST -V -VV -S ~/src/kde4-svn/KDE\ dir/kdelibs/KDELibsNightly.cmake,KDE_CTEST_BUILD_SUFFIX=$SUFFIX,CMAKE_INSTALL_PREFIX=$INSTALL_ROOT/kdelibs,DO_INSTALL=TRUE
#
# ------------8<-----------------8<---------------------8<---------------------8<-----------
