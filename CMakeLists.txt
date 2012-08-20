
cmake_minimum_required(VERSION 2.8.9)

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
set (KDE_VERSION_STRING "${KDE_VERSION} (KDE Frameworks >= 20120113)")
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

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})


find_package(Qt5Transitional)

if (NOT QT5_BUILD)
  add_subdirectory( libinqt5 )
  set(inqt5_LIBRARY inqt5)
  set(inqt5_INCLUDE_DIR
      ${CMAKE_SOURCE_DIR}/libinqt5/src/
      ${CMAKE_BINARY_DIR}/libinqt5/src/
  )
  set(QtMimeTypes_LIBRARY QtMimeTypes)
  include_directories(${inqt5_INCLUDE_DIR})
endif()

add_subdirectory( libkdeqt5staging )
set(kdeqt5staging_LIBRARY kdeqt5staging)
set(kdeqt5staging_INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/libkdeqt5staging/src/
    ${CMAKE_BINARY_DIR}/libkdeqt5staging/src/
   )
if (QT5_BUILD)
set(kdeqt5staging_INCLUDE_DIR ${kdeqt5staging_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdeqt5staging/src/qt5only)
endif()

include_directories(${kdeqt5staging_INCLUDE_DIR})

add_subdirectory( tier1 )

# tier1/ already builds standalone, so it should not get any include dirs from here.
include_directories(${CMAKE_SOURCE_DIR}) # for kde_qt5_compat.h only -- TEMPORARY HACK

add_subdirectory( tier2 )

set(kwindowsystem_LIBRARY kwindowsystem)

set(CMAKE_AUTOMOC ON)


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

find_package(KDE4Internal REQUIRED)
include(KDE4Defaults)
include (MacroLibrary)
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

#required features:
# Perl is used e.g. in khtml, kjs, kjsembed and others
find_package(Perl)
macro_log_feature(PERL_FOUND "Perl" "Needed for building kdelibs" "http://www.perl.org" TRUE "" "")

find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "ZLib" "Support for gzip compressed files and data streams" "http://www.zlib.net" TRUE "" "Required by the core KDE libraries and some critical kioslaves")

set(STRIGI_MIN_VERSION 0.6.3)
find_package(Strigi)
if (WIN32)
    set (STRIGI_REQUIRED FALSE)
    set (STRIGI_EXTRA_TEXT "")
else (WIN32)
    set (STRIGI_REQUIRED TRUE)
    set (STRIGI_EXTRA_TEXT "Required by some critical kioslaves")
endif (WIN32)
macro_log_feature(STRIGI_FOUND "Strigi" "Desktop indexing and search support" "http://strigi.sourceforge.net" ${STRIGI_REQUIRED} "${STRIGI_MIN_VERSION}" "${STRIGI_EXTRA_TEXT}")

set(LIBATTICA_MIN_VERSION "0.1.90")
find_package(LibAttica)
macro_log_feature(LIBATTICA_FOUND "libattica" "Support for Get Hot New Stuff" "git://anongit.kde.org/attica" TRUE "${LIBATTICA_MIN_VERSION}" "")

#optional features
find_package(X11)
if(X11_FOUND)
  macro_bool_to_01(X11_FOUND HAVE_X11)
  #X11_Xrender discovery is done by FindX11
  macro_log_feature(X11_Xrender_FOUND "X Rendering Extension (libXrender)" "Support for compositing, rendering operations, and alpha-blending" "http://www.x.org" FALSE "" "STRONGLY RECOMMENDED")
  macro_bool_to_01(X11_Xscreensaver_FOUND HAVE_XSCREENSAVER)
  macro_bool_to_01(X11_XSync_FOUND HAVE_XSYNC)
  macro_log_feature(HAVE_XSCREENSAVER "X Screensaver Extension (libXss)" "Support for KIdleTime (fallback mode)" "http://www.x.org/" FALSE "" "")
  macro_log_feature(HAVE_XSYNC "X Sync Extension (libXext)" "Efficient operation of KIdleTime" "http://www.x.org/" FALSE "" "STRONGLY RECOMMENDED")

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
macro_log_feature(OPENSSL_FOUND "OpenSSL" "Support for secure network communications (SSL and TLS)" "http://openssl.org" FALSE "" "STRONGLY RECOMMENDED: KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS")

find_package(Libintl)
macro_log_feature(LIBINTL_FOUND "Libintl" "Support for multiple languages" "http://www.gnu.org/software/gettext" FALSE "" "STRONGLY RECOMMENDED: Enables KDE to be available in many different languages")

find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA2" "Support for remote plasma widgets" "http://delta.affinix.com/qca" FALSE "2.0.0" "")

find_package(DBusMenuQt)
macro_log_feature(DBUSMENUQT_FOUND "DBusMenuQt" "Support for notification area menus via the DBusMenu protocol" "https://launchpad.net/libdbusmenu-qt" TRUE "" "")

################# Disallow in-source build #################

macro_ensure_out_of_source_build("kdelibs requires an out of source build. Please create a separate build directory and run 'cmake path_to_kdelibs [options]' there.")

# ... and warn in case of an earlier in-source build
set(generatedFileInSourceDir EXISTS ${kdelibs_SOURCE_DIR}/config.h OR EXISTS ${kdelibs_SOURCE_DIR}/config-prefix.h)
if(${generatedFileInSourceDir})
   message(STATUS "config.h or config-prefix.h exists in your source directory.")
   message(FATAL_ERROR "Please run git clean, it would seem that your source directory has generated files in it.")
endif(${generatedFileInSourceDir})
#########################################################################

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

################# setup the include directories #################

# for including config.h and for includes like <kparts/foo.h>
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces)

if(QCA2_FOUND)
    include_directories(
        ${QCA2_INCLUDE_DIR}
    )
endif(QCA2_FOUND)


# Those variables for  are only valid inside of kdelibs, of course.
# Use the one variable for the lib you depend upon.
# E.g. kdeui uses ${KDE4_KDECORE_INCLUDES}. Something that depends on kparts uses ${KDE4_KPARTS_INCLUDES}.

set(KDE4_KJS_INCLUDES ${CMAKE_SOURCE_DIR}/kjs
                      ${CMAKE_BINARY_DIR}/kjs)

if(NOT WINCE)
    set(KDE4_KDECORE_INCLUDES ${KDE4_KJS_INCLUDES} )
endif(NOT WINCE)

set(KDE4_KCONFIG_INCLUDES ${CMAKE_SOURCE_DIR}/kdecore/config
                          ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/io # for kurl.h
                          ${CMAKE_BINARY_DIR}/tier1/kcoreaddons/src
   )
# kdecore depends on Qt and kcoreaddons (need only headers from kjs)
set(KDE4_KDECORE_INCLUDES ${KDE4_KDECORE_INCLUDES}
                          ${CMAKE_SOURCE_DIR}/kdecore
                          ${CMAKE_BINARY_DIR}/kdecore
                          ${CMAKE_SOURCE_DIR}/kdecore/compression
                          ${CMAKE_SOURCE_DIR}/kdecore/config
                          ${CMAKE_SOURCE_DIR}/kdecore/date
                          ${CMAKE_SOURCE_DIR}/kdecore/io
                          ${CMAKE_SOURCE_DIR}/kdecore/kernel
                          ${CMAKE_SOURCE_DIR}/kdecore/network
                          ${CMAKE_SOURCE_DIR}/kdecore/services
                          ${CMAKE_SOURCE_DIR}/kdecore/localization
                          ${CMAKE_SOURCE_DIR}/kdecore/sycoca
                          ${CMAKE_SOURCE_DIR}/kdecore/text
                          ${CMAKE_SOURCE_DIR}/kdecore/util
                          ${CMAKE_SOURCE_DIR}/kdecore/sonnet
                          ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/io
                          ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/kernel
                          ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/jobs
                          ${CMAKE_SOURCE_DIR}/tier1/kcodecs/src
                          ${CMAKE_BINARY_DIR}/tier1/kcodecs/src
                          ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/text
                          ${CMAKE_BINARY_DIR}/tier1/kcoreaddons/src
                          ${CMAKE_BINARY_DIR}/tier2/kconfig/src/core
                          ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/core
                          ${QT_INCLUDES}
                          ${_KDE4_PLATFORM_INCLUDE_DIRS})

if (NOT QT5_BUILD)
  list(APPEND KDE4_KDECORE_INCLUDES
                          ${QtMimeTypes_SOURCE_DIR}/include/QtMimeTypes
  )
endif()

include_directories(${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/kernel) # For QLocalizedString

# kdeui depends on kdecore
set(KDE4_KDEUI_INCLUDES ${CMAKE_SOURCE_DIR}/kdeui
                        ${CMAKE_BINARY_DIR}/kdeui
                        ${CMAKE_SOURCE_DIR}/kdeui/actions
                        ${CMAKE_SOURCE_DIR}/kdeui/colors
                        ${CMAKE_SOURCE_DIR}/kdeui/config
                        ${CMAKE_SOURCE_DIR}/kdeui/dialogs
                        ${CMAKE_SOURCE_DIR}/kdeui/findreplace
                        ${CMAKE_SOURCE_DIR}/kdeui/fonts
                        ${CMAKE_SOURCE_DIR}/kdeui/icons
                        ${CMAKE_SOURCE_DIR}/kdeui/itemviews
                        ${CMAKE_SOURCE_DIR}/kdeui/jobs
                        ${CMAKE_SOURCE_DIR}/kdeui/kernel
                        ${CMAKE_SOURCE_DIR}/kdeui/notifications
                        ${CMAKE_SOURCE_DIR}/kdeui/paged
                        ${CMAKE_SOURCE_DIR}/kdeui/plotting
                        ${CMAKE_SOURCE_DIR}/kdeui/shortcuts
                        ${CMAKE_SOURCE_DIR}/kdeui/sonnet
                        ${CMAKE_SOURCE_DIR}/kdeui/util
                        ${CMAKE_SOURCE_DIR}/kdeui/widgets
                        ${CMAKE_SOURCE_DIR}/kdeui/windowmanagement
                        ${CMAKE_SOURCE_DIR}/kdeui/xmlgui
                        ${CMAKE_BINARY_DIR}/tier1/kwindowsystem/src
                        ${CMAKE_SOURCE_DIR}/tier1/kwindowsystem/src
                        ${CMAKE_BINARY_DIR}/tier2/kauth/src/
                        ${CMAKE_SOURCE_DIR}/tier2/kauth/src/
                        ${CMAKE_BINARY_DIR}/tier2/kconfig/src/gui
                        ${CMAKE_SOURCE_DIR}/tier2/kconfig/src/gui
                        ${CMAKE_BINARY_DIR}/staging/kdeguiaddons/src/
                        ${CMAKE_SOURCE_DIR}/staging/kdeguiaddons/src/colors
                        ${CMAKE_BINARY_DIR}/staging/kwidgets/src
                        ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/utils
                        ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/icons
                        ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/widgets
                        ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/dialogs
                        ${CMAKE_BINARY_DIR}/staging/kde4support/src/
                        ${CMAKE_SOURCE_DIR}/staging/kde4support/src/kdeui
                        ${KDE4_KDECORE_INCLUDES})

# kio depends on kdeui
set(KDE4_KIO_INCLUDES ${CMAKE_SOURCE_DIR}/kio
                      ${CMAKE_BINARY_DIR}/kio
                      ${CMAKE_BINARY_DIR}/kio/misc/kntlm
                      ${CMAKE_SOURCE_DIR}/kio/bookmarks
                      ${CMAKE_SOURCE_DIR}/kio/kio
                      ${CMAKE_SOURCE_DIR}/kio/kfile
                      ${KDE4_KDEUI_INCLUDES})

# kpty
set(KDE4_KPTY_INCLUDES ${CMAKE_SOURCE_DIR}/kpty ${CMAKE_BINARY_DIR}/kpty ${KDE4_KIO_INCLUDES} )

# kparts depends on kio
set(KDE4_KPARTS_INCLUDES ${CMAKE_SOURCE_DIR}/kparts
                         ${CMAKE_BINARY_DIR}/kparts
                         ${KDE4_KIO_INCLUDES})

# kde3support depends on kparts
set(KDE4_KDE3SUPPORT_INCLUDES ${CMAKE_SOURCE_DIR}/kde3support
                              ${CMAKE_SOURCE_DIR}/kde3support/kdecore
                              ${CMAKE_SOURCE_DIR}/kde3support/kdeui
                              ${CMAKE_SOURCE_DIR}/kde3support/kio
                              ${KDE4_KPARTS_INCLUDES})

if(NOT WINCE)
 set(KDE4_KHTML_INCLUDES ${CMAKE_SOURCE_DIR}/khtml)
endif(NOT WINCE)


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

# ACL stuff (used in kio/ and kioslaves/)
find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
macro_log_feature(ACL_FOUND "LibACL" "Support for manipulating access control lists" "ftp://oss.sgi.com/projects/xfs/cmd_tars" FALSE "" "STRONGLY RECOMMENDED")
configure_file(config-acl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-acl.h )

include(ConfigureChecks.cmake)

# now create config headers
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )

# these two calls here should go somewhere else, Alex
check_library_exists(nsl gethostbyname "" HAVE_NSL_LIBRARY)
check_library_exists(socket connect "" HAVE_SOCKET_LIBRARY)

# TODO: Port away from this:
if (QT5_BUILD)
  add_definitions(-DKDE_NO_PHONON)
  set(KDE_NO_PHONON True)
endif()

# Needed for the calls to kconfig_add_kcfg_files in some modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tier2/kconfig/cmake")
set(KCONFIG_KCFGC_EXECUTABLE kconfig_compiler)
include(KConfigMacros)

set(kde4support_LIBRARY kde4support)

################# list the subdirectories #################

add_subdirectory( cmake )
add_subdirectory( kdecore )
add_subdirectory( staging )
if (UNIX)
   add_subdirectory( kpty )
   add_subdirectory( kdesu )
endif (UNIX)
if(NOT WINCE)
  add_subdirectory( kjs )
  if (NOT QT5_BUILD) # TODO: Qt5UiTools_FOUND
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
if (QT_QT3SUPPORT_FOUND)
   add_subdirectory( kde3support  )
endif (QT_QT3SUPPORT_FOUND)
add_subdirectory( kfile )
if(NOT WINCE)
add_subdirectory( kdoctools  )
endif(NOT WINCE)
add_subdirectory( kioslave   )
add_subdirectory( kparts     )
add_subdirectory( kunitconversion )
if(NOT KDE_NO_PHONON)
    add_subdirectory( khtml  )
endif()
add_subdirectory( interfaces  )
if (NOT QT5_BUILD) # TODO: Qt5UiTools_FOUND
    add_subdirectory( kross )
endif()
add_subdirectory( kutils     )
add_subdirectory( kinit  )
add_subdirectory( includes )
if(NOT QT5_BUILD)
  macro_optional_add_subdirectory( doc )
endif()
if (NOT QT5_BUILD OR Qt5Quick1_FOUND)
    add_subdirectory( experimental )
endif()
if(NOT QT5_BUILD)
  add_subdirectory( kimgio )
  add_subdirectory( knewstuff  )
  add_subdirectory( knotify )
endif()

if(NOT QT5_BUILD OR Qt5WebKit_FOUND)
  add_subdirectory( kdewidgets  )
  add_subdirectory( kdewebkit )
  add_subdirectory( plasma )
endif()


################# write dependency file which will be installed #################

# Used in configure_file() and install(EXPORT)
set(KDE4_TARGET_PREFIX KDE4__)
include(CreateKDELibsDependenciesFile.cmake)

################# install files #################

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDEPlatformProfile.cmake  DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

## TEMPORARY
install(FILES
  kde_qt5_compat.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)


# run a script before installing the exports files which deletes previously installed
# configuration specific export files KDELibs4(Library|Tools)Targets-<config>.cmake
# if the main exports file KDELibs4(Library|Tools)Targets.cmake has changed. This makes sure
# that this main file doesn't include older and different configuration specific exports files,
# which might have a different set of targets or targets with different names.
# The code for installing the exports files will soon go into a macro. Alex
install(CODE "set(EXPORT_FILES KDELibs4LibraryTargets.cmake KDELibs4ToolsTargets.cmake)"
        CODE "set(EXPORT_INSTALL_DIR \"${DATA_INSTALL_DIR}/cmake/modules\")"
        SCRIPT "${CMAKE_SOURCE_DIR}/cmake/modules/check_installed_exports_file.cmake" )
install( EXPORT kdelibsLibraryTargets DESTINATION ${DATA_INSTALL_DIR}/cmake/modules NAMESPACE ${KDE4_TARGET_PREFIX} FILE KDELibs4LibraryTargets.cmake )
install( EXPORT kdelibsToolsTargets   DESTINATION ${DATA_INSTALL_DIR}/cmake/modules NAMESPACE ${KDE4_TARGET_PREFIX} FILE KDELibs4ToolsTargets.cmake )
# the following will be the correct locations once cmake has the improved FIND_PACKAGE()
# install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake DESTINATION ${PLUGIN_INSTALL_DIR}/cmake RENAME KDE4Config.cmake)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

macro_display_feature_log()
