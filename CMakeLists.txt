
set(reqd_cmake 2.8.10.20130411)
if (CMAKE_VERSION VERSION_LESS reqd_cmake)
  message(FATAL_ERROR "
  KDE Frameworks requires CMake 2.8.11 RC 3 or later.

  http://thread.gmane.org/gmane.comp.programming.tools.cmake.user/46346

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

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})

# this gives us the KDE install dir variables (instead of defining them in FindKDE4Internal.cmake),
# RPATH handling and test settings.
find_package(KF5 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS -DQT_STRICT_ITERATORS)

add_subdirectory( libkdeqt5staging )

set(kdeqt5staging_LIBRARIES kdeqt5staging)


add_subdirectory( tier1 )

set(ItemModels_LIBRARIES ItemModels)

set(KArchive_LIBRARIES KArchive)

set(KConfig_CORE_LIBRARY KConfigCore)
set(KConfig_GUI_LIBRARY KConfigGui)
set(KConfig_LIBRARIES KConfigCore KConfigGui)

set(KConfig_KCFGC_EXECUTABLE kconfig_compiler)
include(${CMAKE_SOURCE_DIR}/tier1/kconfig/KConfigMacros.cmake)

set(KCoreAddons_LIBRARIES KCoreAddons)

set(KCodecs_LIBRARIES KCodecs)

set(KDBusAddons_LIBRARIES KDBusAddons)

set(KIdleTime_LIBRARIES KIdleTime)

set(KWindowSystem_LIBRARIES KWindowSystem)

set(KWidgetsAddons_LIBRARIES KWidgetsAddons )

set(KPlotting_LIBRARIES KPlotting)

set(Solid_LIBRARIES Solid)

set(Sonnet_SonnetCore_LIBRARY SonnetCore)
set(Sonnet_SonnetUi_LIBRARY SonnetUi)
set(Sonnet_LIBRARIES ${Sonnet_SonnetUi_LIBRARY} ${Sonnet_SonnetCore_LIBRARY} )


# tier1/ already builds standalone, so it should not get any include dirs from here.

add_subdirectory( tier2 )

set(KAuth_LIBRARIES  KAuth)

#set(kinterprocesswindowing_LIBRARY KInterProcessWindowing)

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
include(ECMOptionalAddSubdirectory)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

#required features:
# Perl is used e.g. in khtml, kjs, kjsembed and others
find_package(Perl)
set_package_properties(Perl PROPERTIES URL "http://www.perl.org" TYPE REQUIRED PURPOSE "Needed for building kdelibs")

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "http://www.zlib.net"
                       TYPE REQUIRED
                       PURPOSE "Required by the core KDE libraries and some critical kioslaves"
                      )

find_package(Strigi 0.6.3)
if (WIN32)
    set (STRIGI_TYPE "OPTIONAL")
    set (STRIGI_EXTRA_TEXT "")
else (WIN32)
    set (STRIGI_TYPE "REQUIRED")
    set (STRIGI_EXTRA_TEXT "Required by some critical kioslaves")
endif (WIN32)
set_package_properties(Strigi PROPERTIES DESCRIPTION "Desktop indexing and search support"
                       URL "http://strigi.sourceforge.net"
                       TYPE ${STRIGI_TYPE}
                       PURPOSE ${STRIGI_EXTRA_TEXT}
                      )

#optional features
find_package(X11)
if(X11_FOUND)
  macro_bool_to_01(X11_FOUND HAVE_X11)
  #X11_Xrender discovery is done by FindX11
  add_feature_info("X Rendering Extension (libXrender)" X11_Xrender_FOUND "Support for compositing, rendering operations, and alpha-blending. STRONGLY RECOMMENDED")
  macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
  macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC)
  add_feature_info("X Screensaver Extension (libXss)" HAVE_XSCREENSAVER "Support for KIdleTime (fallback mode)")
  add_feature_info("X Sync Extension (libXext)" HAVE_XSYNC "Efficient operation of KIdleTime. STRONGLY RECOMMENDED")

  if(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)
    message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)\n")
  endif(NOT HAVE_XSYNC AND NOT HAVE_XSCREENSAVER)

  #X11 Session Management (SM) is required
  #X11_SM_FOUND is set in FindX11, which is required by KDE4Internal
  if(UNIX AND NOT X11_SM_FOUND)
    message(FATAL_ERROR "\nThe X11 Session Management (SM) development package could not be found.\nPlease install libSM.\n")
  endif(UNIX AND NOT X11_SM_FOUND)
endif(X11_FOUND)

find_package(DBusMenuQt5)
set_package_properties(DBusMenuQt5 PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
                       URL "https://launchpad.net/libdbusmenu-qt"
                       TYPE REQUIRED
                      )

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

################# configure checks and create the configured files #################

# Needed for the calls to kconfig_add_kcfg_files in some modules
set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDELibs4)

set(KDE4Support_LIBRARIES KDE4Support)

################# list the subdirectories #################

add_subdirectory( cmake )
# For convenience only. Move down/remove and find dependencies with more locality instead.
find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Widgets Test Network Xml Concurrent DBus Svg)
add_subdirectory( kdecore )
add_subdirectory( staging )
if (UNIX)
   add_subdirectory( kpty )
   add_subdirectory( kdesu )
endif (UNIX)

find_package(Qt5UiTools)

if(NOT WINCE)
  if (Qt5UiTools_FOUND)
      add_subdirectory( kjsembed )
  endif()
endif(NOT WINCE)
add_subdirectory( kconf_update )
add_subdirectory( licenses   )
add_subdirectory( mimetypes  )
add_subdirectory( kdeui )
add_subdirectory( dnssd )
add_subdirectory( security )
add_subdirectory( kio )
add_subdirectory( kded       )
add_subdirectory( kfile )
add_subdirectory( kioslave   )
add_subdirectory( kparts     )

find_package(Phonon4Qt5 4.6.60 NO_MODULE)
if (Phonon4Qt5_FOUND)
    add_subdirectory( khtml  )
endif(Phonon4Qt5_FOUND)

add_subdirectory( interfaces  )
if (Qt5UiTools_FOUND)
    add_subdirectory( kross )
endif()
add_subdirectory( kutils     )
add_subdirectory( kinit  )
add_subdirectory( includes )
ecm_optional_add_subdirectory( doc )

add_subdirectory( kimgio )
find_package(Qt5Quick)
if(Qt5Quick_FOUND)
  add_subdirectory( experimental )
endif()
add_subdirectory( knewstuff )
add_subdirectory( knotify )

find_package(Qt5WebKitWidgets)
if(Qt5WebKitWidgets_FOUND)
  add_subdirectory( kdewebkit )
endif()
add_subdirectory( kdewidgets )

################# write dependency file which will be installed #################

# Used in configure_file() and install(EXPORT)
set(KDE4_TARGET_PREFIX KDE4__)
include(CreateKDELibsDependenciesFile.cmake)

################# install files #################

install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/KDELibs4Config.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/KDEPlatformProfile.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/KDE4Macros.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/kde4init_dummy.cpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/kde4_exec_via_sh.cmake
               ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/kde4init_win32lib_dummy.cpp.in
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

