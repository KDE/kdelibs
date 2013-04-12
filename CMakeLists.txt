
cmake_minimum_required(VERSION 2.8.10.20130405)

project(kdelibs)

# Make CPack available to easy generate binary packages
include(CPack)

find_file(FOUND_KDE4_HEADERS kglobal.h)

if (${FOUND_KDE4_HEADERS})
  string(REGEX MATCH ${CMAKE_INSTALL_PREFIX} COMPATIBLE_HEADERS_TEST ${FOUND_KDE4_HEADERS})

  if (NOT ${COMPATIBLE_HEADERS_TEST})
    message("Found KDE4 headers. This will cause compiling failures while developing KDE Frameworks 5. Uninstall distro installed headers if applicable")
    message("FOUND: ${FOUND_KDE4_HEADERS}")
    return()
  endif()
endif()

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

option(STATIC_LIBRARY "Build kdelibs as static libraries." FALSE)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})


# skip the cmake-build related settings (like automoc)for now, we'll do that later, I need to have a close look at those settings first.
set(KDE_SKIP_BUILD_SETTINGS TRUE)

# this gives us the KDE install dir variables (instead of defining them in FindKDE4Internal.cmake),
# RPATH handling and test settings.
find_package(KF5 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS -DQT_STRICT_ITERATORS)
unset(KDE_SKIP_BUILD_SETTINGS)

find_package(Qt5Core REQUIRED NO_MODULE)

find_package(Qt5Transitional MODULE)

add_subdirectory( libkdeqt5staging )

set(kdeqt5staging_LIBRARIES kdeqt5staging)


add_subdirectory( tier1 )

set(ItemModels_LIBRARIES ItemModels)

set(KArchive_LIBRARIES KArchive)

set(KCoreAddons_LIBRARIES KCoreAddons)

set(KCodecs_LIBRARIES KCodecs)

set(KDBusAddons_LIBRARIES KDBusAddons)

set(KIdleTime_LIBRARIES KIdleTime)

set(KWindowSystem_LIBRARIES KWindowSystem)

set(KWidgetsAddons_LIBRARIES KWidgetsAddons )

set(KPlotting_LIBRARIES KPlotting)

set(Solid_LIBRARIES Solid)


# tier1/ already builds standalone, so it should not get any include dirs from here.

add_subdirectory( tier2 )

set(KAuth_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/tier2/kauth/src/
                       ${CMAKE_SOURCE_DIR}/tier2/kauth/src/ )
set(KAuth_LIBRARIES  KAuth)

set(KConfig_INCLUDE_DIRS   ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/core
                           ${CMAKE_BINARY_DIR}/tier2/kconfig/src/core
                           ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/gui
                           ${CMAKE_BINARY_DIR}/tier2/kconfig/src/gui )

set(KConfig_CORE_LIBRARY KConfigCore)
set(KConfig_GUI_LIBRARY KConfigGui)
set(KConfig_LIBRARIES KConfigCore KConfigGui)

set(KConfig_KCFGC_EXECUTABLE kconfig_compiler)
include(${CMAKE_SOURCE_DIR}/tier2/kconfig/KConfigMacros.cmake)



set(kinterprocesswindowing_LIBRARY kinterprocesswindowing)

set(CMAKE_AUTOMOC ON)
set(CMAKE_BUILD_INTERFACE_INCLUDES ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# By default don't add any linked libraries to the "exported"
# link interfaces of shared libraries, so that executables linking
# against these libraries will not automatically add implicit
# dependencies to their link list.
#
# This reduces inter-package dependencies and makes it easier to remove
# dependencies of shared libraries without breaking binary compatibility.
set(CMAKE_LINK_INTERFACE_LIBRARIES "")

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

set (QT_MIN_VERSION "4.8.0")

include(FindKDE4Internal)
include(KDE4Macros)
include(KDE4Defaults)
include (MacroLibrary)
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

find_package(LibAttica 0.4.2)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "Support for Get Hot New Stuff"
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
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

find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
                       URL "http://openssl.org"
                       TYPE RECOMMENDED
                       PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
                      )

find_package(Libintl)
set_package_properties(Libintl PROPERTIES DESCRIPTION "Support for multiple languages"
                       URL "http://www.gnu.org/software/gettext"
                       TYPE RECOMMENDED
                       PURPOSE "Enables KDE to be available in many different languages"
                      )

find_package(DBusMenuQt)
set_package_properties(DBusMenuQt PROPERTIES DESCRIPTION "Support for notification area menus via the DBusMenu protocol"
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
remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

################# setup the include directories #################

if(QCA2_FOUND)
    include_directories(
        ${QCA2_INCLUDE_DIR}
    )
endif(QCA2_FOUND)


################# configure checks and create the configured files #################

if(WINCE)
  set(STATIC_LIBRARY ON)
  add_definitions(-DSTATIC_INSTALL_PATH=L\\\"/programme/kde\\\")
endif(WINCE)

if(STATIC_LIBRARY)
  set(LIBRARY_TYPE STATIC)
  add_definitions(-DKDELIBS_STATIC_LIBS)
  message(STATUS "Building kdelibs as static libraries")
else(STATIC_LIBRARY)
  set(LIBRARY_TYPE SHARED)
endif(STATIC_LIBRARY)

# now create config headers
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )

# Needed for the calls to kconfig_add_kcfg_files in some modules
set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/KDELibs4)

set(kde4support_LIBRARY kde4support)

################# list the subdirectories #################

add_subdirectory( cmake )
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
if(NOT WINCE)
add_subdirectory( kdoctools  )
endif(NOT WINCE)
add_subdirectory( kioslave   )
add_subdirectory( kparts     )
add_subdirectory( kunitconversion )

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
macro_optional_add_subdirectory( doc )

if(FALSE) # Needs porting
  add_subdirectory( kimgio )
endif()

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

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
