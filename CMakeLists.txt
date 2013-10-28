
set(reqd_cmake 2.8.12)
if (CMAKE_VERSION VERSION_LESS reqd_cmake)
  message(FATAL_ERROR "
  KDE Frameworks requires CMake 2.8.12 or later.

  http://thread.gmane.org/gmane.comp.programming.tools.cmake.devel/8114

  It is recommended that you build from git.

  http://community.kde.org/Frameworks/Building#CMake_from_Git
  ")
endif()
cmake_minimum_required(VERSION ${reqd_cmake})

project(kdelibs)

# Make CPack available to easy generate binary packages
include(CPack)

################# set KDE specific information #################

set (KDE_VERSION_MAJOR 4)
set (KDE_VERSION_MINOR 90)
set (KDE_VERSION_RELEASE 00)
set (KDE_VERSION "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}" )
# For git master, use this:
set (KDE_VERSION_STRING "${KDE_VERSION} (KDE Frameworks >= 20121130)")
# For a stable release, use this:
#set (KDE_VERSION_STRING "${KDE_VERSION}")

set (KDE_DISTRIBUTION_TEXT "compiled sources" CACHE STRING "Indicate the distribution in bug reports" )

# win32: give kde home in debug mode a different name as the  release home dir because the settings and caches are different
if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (_KDE_DEFAULT_HOME_POSTFIX "-debug" CACHE STRING "default KDE home directory postfix" )
endif (WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
set (KDE_DEFAULT_HOME ".kde${_KDE_DEFAULT_HOME_POSTFIX}" CACHE STRING "The default KDE home directory" )

# this must be before FindKDE4Internal in order to preset the result of the visibility test, so that it will be skipped
option(KHTML_BUILD_TESTREGRESSION "Build KHTML's testregression. Note: this disables hidden visibility")
# Disable visibility if testregression is built, because the symbols are needed then
if (KHTML_BUILD_TESTREGRESSION)
   set (__KDE_HAVE_GCC_VISIBILITY 0)
endif (KHTML_BUILD_TESTREGRESSION)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})

# this gives us the KDE install dir variables (instead of defining them in FindKDE4Internal.cmake),
# RPATH handling and test settings.
find_package(KF5 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

add_subdirectory( tier1 )

# tier1/ already builds standalone, so it should not get any include dirs from here.

if(MSVC)
  # Only build-tested up to here.
  return()
endif()

add_subdirectory( tier2 )
add_subdirectory( tier3 )
add_subdirectory( tier4 )
add_subdirectory( staging )

################# write platform profile file which will be installed #################
include(CreateKDEPlatformProfile.cmake)

if(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
   set(KDE_NO_DEPRECATED TRUE)
   set(CMAKE_AUTOMOC_MOC_OPTIONS "-DKDE_NO_DEPRECATED")
endif(KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

############### Load the CTest options ###############
# CTestCustom.cmake has to be in the CTEST_BINARY_DIR.
# in the KDE build system, this is the same as CMAKE_BINARY_DIR.
configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR}/CTestCustom.cmake COPYONLY)

################# now find all used packages #################

include(FindKDE4Internal)
include(KDE4Macros)
include(KDE4Defaults)
include(MacroLibrary)

################# Disallow in-source build #################

macro_ensure_out_of_source_build("kdelibs requires an out of source build. Please create a separate build directory and run 'cmake path_to_kdelibs [options]' there.")

# ... and warn in case of an earlier in-source build
set(generatedFileInSourceDir EXISTS ${kdelibs_SOURCE_DIR}/config.h OR EXISTS ${kdelibs_SOURCE_DIR}/config-prefix.h)
if(${generatedFileInSourceDir})
   message(STATUS "config.h or config-prefix.h exists in your source directory.")
   message(FATAL_ERROR "Please run git clean, it would seem that your source directory has generated files in it.")
endif(${generatedFileInSourceDir})
#########################################################################

add_definitions(${KDE4_DEFINITIONS})

################# list the subdirectories #################
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(CreateKDELibsDependenciesFile.cmake)
set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDELibs4)

add_subdirectory( cmake )
# For convenience only. Move down/remove and find dependencies with more locality instead.

add_subdirectory( licenses   )
add_subdirectory( kio )
add_subdirectory( kded       )
add_subdirectory( kioslave   )

add_subdirectory( khtml  )

add_subdirectory( interfaces  )
add_subdirectory( includes )
add_subdirectory( doc )

add_subdirectory( kdewidgets )

################# write dependency file which will be installed #################

# Used in configure_file() and install(EXPORT)
set(KDE4_TARGET_PREFIX KDE4__)

################# install files #################

install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/KDELibs4Config.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/KDEPlatformProfile.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/KDE4Macros.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/kde4_exec_via_sh.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/kde4_cmake_uninstall.cmake.in
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/Win32.Manifest.in
         DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel)

# run a script before installing the exports files which deletes previously installed
# configuration specific export files KDELibs4(Library|Tools)Targets-<config>.cmake
# if the main exports file KDELibs4(Library|Tools)Targets.cmake has changed. This makes sure
# that this main file doesn't include older and different configuration specific exports files,
# which might have a different set of targets or targets with different names.
# The code for installing the exports files will soon go into a macro. Alex
install(CODE "set(EXPORT_FILES KDELibs4LibraryTargets.cmake KDELibs4ToolsTargets.cmake)"
        CODE "set(EXPORT_INSTALL_DIR \"${CMAKECONFIG_INSTALL_DIR}\")"
        SCRIPT "${CMAKE_SOURCE_DIR}/cmake/modules/check_installed_exports_file.cmake" )
install( EXPORT kdelibsLibraryTargets DESTINATION ${CMAKECONFIG_INSTALL_DIR} NAMESPACE ${KDE4_TARGET_PREFIX} FILE KDELibs4LibraryTargets.cmake  COMPONENT Devel)
install( EXPORT kdelibsToolsTargets   DESTINATION ${CMAKECONFIG_INSTALL_DIR} NAMESPACE ${KDE4_TARGET_PREFIX} FILE KDELibs4ToolsTargets.cmake  COMPONENT Devel)

feature_summary(WHAT ALL
                     INCLUDE_QUIET_PACKAGES
                     FATAL_ON_MISSING_REQUIRED_PACKAGES
               )

