# - Find the KDE4 include and library dirs, KDE preprocessors and define a some macros
#
# This module defines the following variables:
#
#  KDE4_FOUND               - set to TRUE if everything required for building KDE software has been found
#
#  KDE4_DEFINITIONS         - compiler definitions required for compiling KDE software
#  KDE4_INCLUDE_DIR         - the KDE 4 include directory
#  KDE4_INCLUDES            - all include directories required for KDE, i.e.
#                             KDE4_INCLUDE_DIR, but also the Qt4 include directories
#                             and other platform specific include directories
#  KDE4_LIB_DIR             - the directory where the KDE libraries are installed,
#                             intended to be used with LINK_DIRECTORIES(). In general, this is not necessary.
#  KDE4_LIBEXEC_INSTALL_DIR - the directory where libexec executables from kdelibs are installed
#  KDE4_BIN_INSTALL_DIR     - the directory where executables from kdelibs are installed
#  KDE4_SBIN_INSTALL_DIR    - the directory where system executables from kdelibs are installed
#  KDE4_DATA_INSTALL_DIR    - the parent directory where kdelibs applications install their data
#  KDE4_HTML_INSTALL_DIR    - the directory where HTML documentation from kdelibs is installed
#  KDE4_CONFIG_INSTALL_DIR  - the directory where config files from kdelibs are installed
#  KDE4_ICON_INSTALL_DIR    - the directory where icons from kdelibs are
#  KDE4_IMPORTS_INSTALL_DIR - the directory where imports from kdelibs are
#  KDE4_KCFG_INSTALL_DIR    - the directory where kconfig files from kdelibs are installed
#  KDE4_LOCALE_INSTALL_DIR  - the directory where translations from kdelibs are installed
#  KDE4_MIME_INSTALL_DIR    - the directory where mimetype desktop files from kdelibs are installed
#  KDE4_SOUND_INSTALL_DIR   - the directory where sound files from kdelibs are installed
#  KDE4_TEMPLATES_INSTALL_DIR     - the directory where templates (Create new file...) from kdelibs are installed
#  KDE4_WALLPAPER_INSTALL_DIR     - the directory where wallpapers from kdelibs are installed
#  KDE4_KCONF_UPDATE_INSTALL_DIR  - the directory where kconf_update files from kdelibs are installed
#  KDE4_AUTOSTART_INSTALL_DIR     - the directory where autostart from kdelibs are installed
#  KDE4_XDG_APPS_INSTALL_DIR      - the XDG apps dir from kdelibs
#  KDE4_XDG_DIRECTORY_INSTALL_DIR - the XDG directory from kdelibs
#  KDE4_SYSCONF_INSTALL_DIR       - the directory where sysconfig files from kdelibs are installed
#  KDE4_MAN_INSTALL_DIR           - the directory where man pages from kdelibs are installed
#  KDE4_INFO_INSTALL_DIR          - the directory where info files from kdelibs are installed
#  KDE4_DBUS_INTERFACES_DIR       - the directory where dbus interfaces from kdelibs are installed
#  KDE4_DBUS_SERVICES_DIR         - the directory where dbus service files from kdelibs are installed
#
# The following variables are defined for the various tools required to
# compile KDE software:
#
#  KDE4_MAKEKDEWIDGETS_EXECUTABLE - the makekdewidgets executable
#
# The following variables point to the location of the KDE libraries,
# but shouldn't be used directly:
#
#  KDE4_KDECORE_LIBRARY     - the kdecore library
#  KDE4_KDEUI_LIBRARY       - the kdeui library
#  KDE4_KIO_LIBRARY         - the kio library
#  KDE4_KPARTS_LIBRARY      - the kparts library
#  KDE4_KUTILS_LIBRARY      - the kutils library
#  KDE4_KEMOTICONS_LIBRARY  - the kemoticons library
#  KDE4_KIDLETIME_LIBRARY   - the kidletime library
#  KDE4_KCMUTILS_LIBRARY    - the kcmutils library
#  KDE4_KPRINTUTILS_LIBRARY - the kprintutils library
#  KDE4_KDE3SUPPORT_LIBRARY - the kde3support library
#  KDE4_KFILE_LIBRARY       - the kfile library
#  KDE4_KHTML_LIBRARY       - the khtml library
#  KDE4_KJS_LIBRARY         - the kjs library
#  KDE4_KJSAPI_LIBRARY      - the kjs public api library
#  KDE4_KNEWSTUFF2_LIBRARY  - the knewstuff2 library
#  KDE4_KNEWSTUFF3_LIBRARY  - the knewstuff3 library
#  KDE4_KDNSSD_LIBRARY      - the kdnssd library
#  KDE4_THREADWEAVER_LIBRARY- the threadweaver library
#  KDE4_SOLID_LIBRARY       - the solid library
#  KDE4_KNOTIFYCONFIG_LIBRARY- the knotifyconfig library
#  KDE4_KROSSCORE_LIBRARY   - the krosscore library
#  KDE4_KTEXTEDITOR_LIBRARY - the ktexteditor library
#  KDE4_NEPOMUK_LIBRARY     - the nepomuk library
#  KDE4_PLASMA_LIBRARY      - the plasma library
#  KDE4_KUNITCONVERSION_LIBRARY - the kunitconversion library
#  KDE4_KDEWEBKIT_LIBRARY   - the kdewebkit library
#
#  KDE4_PLASMA_OPENGL_FOUND  - TRUE if the OpenGL support of Plasma has been found, NOTFOUND otherwise
#
# Compared to the variables above, the following variables
# also contain all of the depending libraries, so the variables below
# should be used instead of the ones above:
#
#  KDE4_KDECORE_LIBS          - the kdecore library and all depending libraries
#  KDE4_KDEUI_LIBS            - the kdeui library and all depending libraries
#  KDE4_KIO_LIBS              - the kio library and all depending libraries
#  KDE4_KPARTS_LIBS           - the kparts library and all depending libraries
#  KDE4_KUTILS_LIBS           - the kutils library and all depending libraries
#  KDE4_KEMOTICONS_LIBS       - the kemoticons library and all depending libraries
#  KDE4_KIDLETIME_LIBS        - the kidletime library and all depending libraries
#  KDE4_KCMUTILS_LIBS         - the kcmutils library and all depending libraries
#  KDE4_KPRINTUTILS_LIBS      - the kprintutils library and all depending libraries
#  KDE4_KDE3SUPPORT_LIBS      - the kde3support library and all depending libraries
#  KDE4_KFILE_LIBS            - the kfile library and all depending libraries
#  KDE4_KHTML_LIBS            - the khtml library and all depending libraries
#  KDE4_KJS_LIBS              - the kjs library and all depending libraries
#  KDE4_KJSAPI_LIBS           - the kjs public api library and all depending libraries
#  KDE4_KNEWSTUFF2_LIBS       - the knewstuff2 library and all depending libraries
#  KDE4_KNEWSTUFF3_LIBS       - the knewstuff3 library and all depending libraries
#  KDE4_KDNSSD_LIBS           - the kdnssd library and all depending libraries
#  KDE4_KDESU_LIBS            - the kdesu library and all depending libraries
#  KDE4_KPTY_LIBS             - the kpty library and all depending libraries
#  KDE4_THREADWEAVER_LIBRARIES- the threadweaver library and all depending libraries
#  KDE4_SOLID_LIBS            - the solid library and all depending libraries
#  KDE4_KNOTIFYCONFIG_LIBS    - the knotify config library and all depending libraries
#  KDE4_KROSSCORE_LIBS        - the kross core library and all depending libraries
#  KDE4_KROSSUI_LIBS          - the kross ui library which includes core and all depending libraries
#  KDE4_KTEXTEDITOR_LIBS      - the ktexteditor library and all depending libraries
#  KDE4_NEPOMUK_LIBS          - the nepomuk library and all depending libraries
#  KDE4_PLASMA_LIBS           - the plasma library and all depending librairies
#  KDE4_KUNITCONVERSION_LIBS  - the kunitconversion library and all depending libraries
#  KDE4_KDEWEBKIT_LIBS        - the kdewebkit library and all depending libraries
#
# This module defines also a bunch of variables used as locations for install directories
# for files of the package which is using this module. These variables don't say
# anything about the location of the installed KDE.
# They can be relative (to CMAKE_INSTALL_PREFIX) or absolute.
# Under Windows they are always relative.
#
#  BIN_INSTALL_DIR          - the directory where executables will be installed (default is prefix/bin)
#  BUNDLE_INSTALL_DIR       - Mac only: the directory where application bundles will be installed (default is /Applications/KDE4 )
#  SBIN_INSTALL_DIR         - the directory where system executables will be installed (default is prefix/sbin)
#  LIB_INSTALL_DIR          - the directory where libraries will be installed (default is prefix/lib)
#  CONFIG_INSTALL_DIR       - the directory where config files will be installed
#  DATA_INSTALL_DIR         - the parent directory where applications can install their data
#  HTML_INSTALL_DIR         - the directory where HTML documentation will be installed
#  ICON_INSTALL_DIR         - the directory where the icons will be installed (default prefix/share/icons/)
#  INFO_INSTALL_DIR         - the directory where info files will be installed (default prefix/info)
#  KCFG_INSTALL_DIR         - the directory where kconfig files will be installed
#  LOCALE_INSTALL_DIR       - the directory where translations will be installed
#  MAN_INSTALL_DIR          - the directory where man pages will be installed (default prefix/man/)
#  MIME_INSTALL_DIR         - the directory where mimetype desktop files will be installed
#  PLUGIN_INSTALL_DIR       - the subdirectory relative to the install prefix where plugins will be installed (default is ${KDE4_LIB_INSTALL_DIR}/plugins/kf5)
#  IMPORTS_INSTALL_DIR      - the subdirectory relative to the install prefix where imports will be installed
#  SERVICES_INSTALL_DIR     - the directory where service (desktop, protocol, ...) files will be installed
#  SERVICETYPES_INSTALL_DIR - the directory where servicestypes desktop files will be installed
#  SOUND_INSTALL_DIR        - the directory where sound files will be installed
#  TEMPLATES_INSTALL_DIR    - the directory where templates (Create new file...) will be installed
#  WALLPAPER_INSTALL_DIR    - the directory where wallpapers will be installed
#  AUTOSTART_INSTALL_DIR    - the directory where autostart files will be installed
#  DEMO_INSTALL_DIR         - the directory where demos will be installed
#  KCONF_UPDATE_INSTALL_DIR - the directory where kconf_update files will be installed
#  SYSCONF_INSTALL_DIR      - the directory where sysconfig files will be installed (default /etc)
#  XDG_APPS_INSTALL_DIR     - the XDG apps dir
#  XDG_DIRECTORY_INSTALL_DIR- the XDG directory
#  XDG_MIME_INSTALL_DIR     - the XDG mimetypes install dir
#  DBUS_INTERFACES_INSTALL_DIR - the directory where dbus interfaces will be installed (default is prefix/share/dbus-1/interfaces)
#  DBUS_SERVICES_INSTALL_DIR        - the directory where dbus services will be installed (default is prefix/share/dbus-1/services )
#  DBUS_SYSTEM_SERVICES_INSTALL_DIR        - the directory where dbus system services will be installed (default is prefix/share/dbus-1/system-services )
#
# The variable INSTALL_TARGETS_DEFAULT_ARGS can be used when installing libraries
# or executables into the default locations.
# The INSTALL_TARGETS_DEFAULT_ARGS variable should be used when libraries are installed.
# It should also be used when installing applications, since then
# on OS X application bundles will be installed to BUNDLE_INSTALL_DIR.
# The variable MUST NOT be used for installing plugins.
# It also MUST NOT be used for executables which are intended to go into sbin/ or libexec/.
#
# Usage is like this:
#    install(TARGETS kdecore kdeui ${INSTALL_TARGETS_DEFAULT_ARGS} )
#
# This will install libraries correctly under UNIX, OSX and Windows (i.e. dll's go
# into bin/.
#
#
# The following variable is provided, but seem to be unused:
#  LIBS_HTML_INSTALL_DIR    /share/doc/HTML            CACHE STRING "Is this still used ?")
#
# The following user adjustable options are provided:
#
#  KDE4_BUILD_TESTS  - enable this to build the testcases
#  KDE4_ENABLE_FPIE  - enable it to use gcc Position Independent Executables feature
#  KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR - only present for CMake >= 2.6.3, defaults to TRUE
#                      If enabled, the package should install its <package>Config.cmake file to
#                      lib/cmake/<package>/ instead to lib/<package>/cmake
#
# It also adds the following macros and functions (from KDE4Macros.cmake)
#  KDE4_ADD_UI_FILES (SRCS_VAR file1.ui ... fileN.ui)
#    Use this to add Qt designer ui files to your application/library.
#
#  KDE4_ADD_UI3_FILES (SRCS_VAR file1.ui ... fileN.ui)
#    Use this to add Qt designer ui files from Qt version 3 to your application/library.
#
#  KDE4_ADD_KCFG_FILES (SRCS_VAR [GENERATE_MOC] [USE_RELATIVE_PATH] file1.kcfgc ... fileN.kcfgc)
#    Use this to add KDE config compiler files to your application/library.
#    Use optional GENERATE_MOC to generate moc if you use signals in your kcfg files.
#    Use optional USE_RELATIVE_PATH to generate the classes in the build following the given
#    relative path to the file.
#
#  KDE4_ADD_WIDGET_FILES (SRCS_VAR file1.widgets ... fileN.widgets)
#    Use this to add widget description files for the makekdewidgets code generator
#    for Qt Designer plugins.
#
#  KDE4_ADD_PLUGIN ( name [WITH_PREFIX] file1 ... fileN )
#    Create a KDE plugin (KPart, kioslave, etc.) from the given source files.
#    If WITH_PREFIX is given, the resulting plugin will have the prefix "lib", otherwise it won't.
#    Do NOT use WITH_PREFIX in new code! This is for compatibility only.
#
#  KDE4_ADD_KDEINIT_EXECUTABLE (name [NOGUI] [RUN_UNINSTALLED] file1 ... fileN)
#    Create a KDE application in the form of a module loadable via kdeinit.
#    A library named kdeinit_<name> will be created and a small executable which links to it.
#    If the executable doesn't have a GUI, use the option NOGUI. By default on OS X
#    application bundles are created, with the NOGUI option no bundles but simple executables
#    are created. Under Windows this flag is also necessary to separate between applications
#    with GUI and without. On other UNIX systems this flag has no effect.
#    RUN_UNINSTALLED is deprecated and is ignored, for details see the documentation for
#    KDE4_ADD_EXECUTABLE().
#
#  KDE4_ADD_EXECUTABLE (name [NOGUI] [TEST] [RUN_UNINSTALLED] file1 ... fileN)
#    Equivalent to ADD_EXECUTABLE(), but additionally adds some more features:
#    -automatic RPATH handling
#    If the executable doesn't have a GUI, use the option NOGUI. By default on OS X
#    application bundles are created, with the NOGUI option no bundles but simple executables
#    are created. Under Windows this flag is also necessary to separate between applications
#    with GUI and without. On other UNIX systems this flag has no effect.
#    The option TEST is for internal use only.
#    The option RUN_UNINSTALLED is ignored. It was necessary with KDE 4.0 and 4.1
#    if the executable had to be run from the build tree. Since KDE 4.2 all
#    executables can be always run uninstalled (the RPATH of executables which are not
#    yet installed points since then into the buildtree and is changed
#    to the proper location when installing, so RUN_UNINSTALLED is not necessary anymore).
#
#  KDE4_ADD_LIBRARY (name [STATIC | SHARED | MODULE ] file1 ... fileN)
#    Equivalent to ADD_LIBRARY().
#    The RPATH is set according to the global RPATH settings as set up by FindKDE4Internal.cmake
#    (CMAKE_SKIP_BUILD_RPATH=FALSE, CMAKE_BUILD_WITH_INSTALL_RPATH=FALSE, CMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE)
#    Under Windows it adds a -DMAKE_<name>_LIB definition to the compilation.
#
#  KDE4_ADD_UNIT_TEST (testname [TESTNAME targetname] file1 ... fileN)
#    add a unit test, which is executed when running make test
#    it will be built with RPATH poiting to the build dir
#    The targets are always created, but only built for the "all"
#    target if the option KDE4_BUILD_TESTS is enabled. Otherwise the rules for the target
#    are created but not built by default. You can build them by manually building the target.
#    The name of the target can be specified using TESTNAME <targetname>, if it is not given
#    the macro will default to the <testname>
#
#  KDE4_ADD_APP_ICON (SRCS_VAR pattern)
#  adds an application icon to target source list.
#  Make sure you have a 128x128 icon, or the icon won't display on Mac OS X.
#  Mac OSX notes : the application icon is added to a Mac OS X bundle so that Finder and friends show the right thing.
#  Win32 notes: the application icon(s) are compiled into the application
#  There is some workaround in kde4_add_kdeinit_executable to make it possible for those applications as well.
# Parameters:
#  SRCS_VAR  - specifies the list of source files
#  pattern   - regular expression for searching application icons
#  Example: KDE4_ADD_APP_ICON( myapp_SOURCES "pics/cr*-myapp.png")
#  Example: KDE4_ADD_APP_ICON( myapp_KDEINIT_SRCS "icons/oxygen/*/apps/myapp.png")
#
#  KDE4_UPDATE_ICONCACHE()
#    Notifies the icon cache that new icons have been installed by updating
#    mtime of ${ICON_INSTALL_DIR}/hicolor directory.
#
#  KDE4_INSTALL_ICONS( path theme)
#    Installs all png and svgz files in the current directory to the icon
#    directory given in path, in the subdirectory for the given icon theme.
#
#  KDE4_INSTALL_AUTH_ACTIONS( HELPER_ID ACTIONS_FILE )
#   This macro generates an action file, depending on the backend used, for applications using KAuth.
#   It accepts the helper id (the DBUS name) and a file containing the actions (check kdelibs/kdecore/auth/example
#   for file format). The macro will take care of generating the file according to the backend specified,
#   and to install it in the right location. This (at the moment) means that on Linux (PolicyKit) a .policy
#   file will be generated and installed into the policykit action directory (usually /usr/share/PolicyKit/policy/),
#   and on Mac (Authorization Services) will be added to the system action registry using the native MacOS API during
#   the install phase
#
#  KDE4_INSTALL_AUTH_HELPER_FILES( HELPER_TARGET HELPER_ID HELPER_USER )
#   This macro adds the needed files for an helper executable meant to be used by applications using KAuth.
#   It accepts the helper target, the helper ID (the DBUS name) and the user under which the helper will run on.
#   This macro takes care of generate the needed files, and install them in the right location. This boils down
#   to a DBus policy to let the helper register on the system bus, and a service file for letting the helper
#   being automatically activated by the system bus.
#   *WARNING* You have to install the helper in ${LIBEXEC_INSTALL_DIR} to make sure everything will work.
#
#
#
#  A note on the possible values for CMAKE_BUILD_TYPE and how KDE handles
#  the flags for those buildtypes. FindKDE4Internal supports the values
#  Debug, Release, RelWithDebInfo, Profile and Debugfull:
#
#  Release
#          optimised for speed, qDebug/kDebug turned off, no debug symbols, no asserts
#  RelWithDebInfo (Release with debug info)
#          similar to Release, optimised for speed, but with debugging symbols on (-g)
#  Debug
#          optimised but debuggable, debugging on (-g)
#          (-fno-reorder-blocks -fno-schedule-insns -fno-inline)
#  DebugFull
#          no optimization, full debugging on (-g3)
#  Profile
#          DebugFull + -ftest-coverage -fprofile-arcs
#
#
#  The default buildtype is RelWithDebInfo.
#  It is expected that the "Debug" build type be still debuggable with gdb
#  without going all over the place, but still produce better performance.
#  It's also important to note that gcc cannot detect all warning conditions
#  unless the optimiser is active.
#
#
#  This module allows to depend on a particular minimum version of kdelibs.
#  To acomplish that one should use the appropriate cmake syntax for
#  find_package. For example to depend on kdelibs >= 4.1.0 one should use
#
#  find_package(KDE4 4.1.0 REQUIRED)
#
#  In earlier versions of KDE you could use the variable KDE_MIN_VERSION to
#  have such a dependency. This variable is deprecated with KDE 4.2.0, but
#  will still work to make the module backwards-compatible.

#  _KDE4_PLATFORM_INCLUDE_DIRS is used only internally
#  _KDE4_PLATFORM_DEFINITIONS is used only internally

# Copyright (c) 2006-2009, Alexander Neundorf <neundorf@kde.org>
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


# this is required now by cmake 2.6 and so must not be skipped by if(KDE4_FOUND) below
cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
# set the cmake policies to the 2.4.x compatibility settings (may change for KDE 4.3)
cmake_policy(VERSION 2.4.5)

# CMake 2.6, set compatibility behaviour to cmake 2.4
# this must be executed always, because the CMAKE_MINIMUM_REQUIRED() command above
# resets the policy settings, so we get a lot of warnings

# CMP0000: don't require cmake_minimum_version() directly in the top level CMakeLists.txt, FindKDE4Internal.cmake is good enough
cmake_policy(SET CMP0000 OLD)
# CMP0002: in KDE4 we have multiple targets with the same name for the unit tests
cmake_policy(SET CMP0002 OLD)
# CMP0003: add the link paths to the link command as with cmake 2.4
cmake_policy(SET CMP0003 OLD)
# CMP0005: keep escaping behaviour for definitions added via add_definitions()
cmake_policy(SET CMP0005 OLD)
# since cmake 2.6.3: NEW behaviour is that setting policies doesn't "escape" the file
# where this is done, macros and functions are executed with the policies as they
# were when the were defined. Keep the OLD behaviour so we can set the policies here
# for all KDE software without the big warning
cmake_policy(SET CMP0011 OLD)

# since cmake 2.8.4: when include()ing from inside cmake's module dir, prefer the files
# in this directory over those from CMAKE_MODULE_PATH
cmake_policy(SET CMP0017 NEW)

# Only do something if it hasn't been found yet
if(NOT KDE4_FOUND)

# We may only search for other packages with "REQUIRED" if we are required ourselves.
# This file can be processed either (usually) included in FindKDE4.cmake or
# (when building kdelibs) directly via FIND_PACKAGE(KDE4Internal), that's why
# we have to check for both KDE4_FIND_REQUIRED and KDE4Internal_FIND_REQUIRED.
if(KDE4_FIND_REQUIRED  OR  KDE4Internal_FIND_REQUIRED)
  set(_REQ_STRING_KDE4 "REQUIRED")
  set(_REQ_STRING_KDE4_MESSAGE "FATAL_ERROR")
else(KDE4_FIND_REQUIRED  OR  KDE4Internal_FIND_REQUIRED)
  set(_REQ_STRING_KDE4 "OPTIONAL")
  set(_REQ_STRING_KDE4_MESSAGE "STATUS")
endif(KDE4_FIND_REQUIRED  OR  KDE4Internal_FIND_REQUIRED)


# Store CMAKE_MODULE_PATH and then append the current dir to it, so we are sure
# we get the FindQt4.cmake located next to us and not a different one.
# The original CMAKE_MODULE_PATH is restored later on.
set(_kde_cmake_module_path_back ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/../../../share/cmake/modules
  ${CMAKE_CURRENT_LIST_DIR}/../../../../share/cmake/modules
  ${CMAKE_MODULE_PATH}
)

find_package(ECM 0.0.6 NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

# Perl is not required for building KDE software, but we had that here since 4.0
find_package(Perl)
if(NOT PERL_FOUND)
   message(STATUS "Perl not found")
endif(NOT PERL_FOUND)

# now we are sure we have everything we need

include (MacroLibrary)
include (CheckCXXCompilerFlag)
include (CheckCXXSourceCompiles)
include (FeatureSummary)

# restore the original CMAKE_MODULE_PATH
#set(CMAKE_MODULE_PATH ${_kde_cmake_module_path_back})

# helper macro, sets both the KDE4_FOO_LIBRARY and KDE4_FOO_LIBS variables to KDE4__foo
# It is used both in bootstrapping and in normal mode.
macro(_KDE4_SET_LIB_VARIABLES _var _lib _prefix)
   set(KDE4_${_var}_LIBRARY ${_prefix}${_lib} )
   set(KDE4_${_var}_LIBS    ${_prefix}${_lib} )
endmacro(_KDE4_SET_LIB_VARIABLES _var _lib _prefix)

#######################  #now try to find some kde stuff  ################################

# ... but NOT otherwise
set( _KDE4_MAKEKDEWIDGETS_DEP)
set( _KDE4_MEINPROC_EXECUTABLE_DEP)

set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib )

if (WIN32)
  # we don't want to be forced to set two paths into the build tree
  set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin )

  # on win32 the install dir is determined on runtime not install time
  # KDELIBS_INSTALL_DIR and QT_INSTALL_DIR are used in KDELibsDependencies.cmake to setup
  # kde install paths and library dependencies
  get_filename_component(_DIR ${KDE4_KDECONFIG_EXECUTABLE} PATH )
  get_filename_component(KDE4_INSTALL_DIR ${_DIR} PATH )
  get_filename_component(_DIR ${QT_QMAKE_EXECUTABLE} PATH )
  get_filename_component(QT_INSTALL_DIR ${_DIR} PATH )
endif (WIN32)

# These files contain information about the installed kdelibs, Alex
include(${CMAKE_CURRENT_LIST_DIR}/KDELibsDependencies.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/KDEPlatformProfile.cmake)

# Check the version of KDE. It must be at least KDE_MIN_VERSION as set by the user.
# KDE_VERSION is set in KDELibsDependencies.cmake since KDE 4.0.x. Alex
# Support for the new-style (>= 2.6.0) support for requiring some version of a package:
if (NOT KDE_MIN_VERSION)
  if (KDE4_FIND_VERSION_MAJOR)
      set(KDE_MIN_VERSION "${KDE4_FIND_VERSION_MAJOR}.${KDE4_FIND_VERSION_MINOR}.${KDE4_FIND_VERSION_PATCH}")
  else (KDE4_FIND_VERSION_MAJOR)
      set(KDE_MIN_VERSION "4.0.0")
  endif (KDE4_FIND_VERSION_MAJOR)
endif (NOT KDE_MIN_VERSION)

if (${KDE_VERSION} VERSION_LESS ${KDE_MIN_VERSION})
   message(FATAL_ERROR "KDE_MIN_VERSION=${KDE_MIN_VERSION}  found ${KDE_VERSION} exact: -${KDE4_FIND_VERSION_EXACT}- version: -${KDE4_FIND_VERSION}-")
else ()
   set(KDE4_INSTALLED_VERSION_OK TRUE)
endif()


# KDE4_LIB_INSTALL_DIR and KDE4_INCLUDE_INSTALL_DIR are set in KDELibsDependencies.cmake,
# use them to set the KDE4_LIB_DIR and KDE4_INCLUDE_DIR "public interface" variables
set(KDE4_LIB_DIR ${KDE4_LIB_INSTALL_DIR} )
set(KDE4_INCLUDE_DIR ${KDE4_INCLUDE_INSTALL_DIR} )


# This setting is currently not recorded in KDELibsDependencies.cmake:
find_file(KDE4_PLASMA_OPENGL_FOUND plasma/glapplet.h PATHS ${KDE4_INCLUDE_DIR} NO_DEFAULT_PATH)

# Now include the file with the imported tools (executable targets).
# This export-file is generated and installed by the toplevel CMakeLists.txt of kdelibs.
# Having the libs and tools in two separate files should help with cross compiling.
include(${CMAKE_CURRENT_LIST_DIR}/KDELibs4ToolsTargets.cmake)

# get the build CONFIGURATIONS which were exported in this file, and use just the first
# of them to get the location of the installed executables
get_target_property(_importedConfigurations  ${KDE4_TARGET_PREFIX}meinproc5 IMPORTED_CONFIGURATIONS )
list(GET _importedConfigurations 0 _firstConfig)

get_target_property(KDE4_MEINPROC_EXECUTABLE          ${KDE4_TARGET_PREFIX}meinproc5           LOCATION_${_firstConfig})
get_target_property(KDE4_MAKEKDEWIDGETS_EXECUTABLE    ${KDE4_TARGET_PREFIX}makekdewidgets      LOCATION_${_firstConfig})

# Include the Sonnet targets, they are needed by the imported kdeui target (included via KDELibs4LibraryTargets.cmake)
if(NOT TARGET KF5::SonnetCore)
   include(${KDE4_LIB_INSTALL_DIR}/cmake/Sonnet/SonnetTargets.cmake)
endif()
if(NOT TARGET KF5::KParts)
   include(${KDE4_LIB_INSTALL_DIR}/cmake/KParts/KPartsTargets.cmake)
endif()
# This file contains the exported library target from kdelibs (new with cmake 2.6.x), e.g.
# the library target "kdeui" is exported as "KDE4__kdeui". The "KDE4__" is used as
# "namespace" to separate the imported targets from "normal" targets, it is stored in
# KDE4_TARGET_PREFIX, which is set in KDELibsDependencies.cmake .
# This export-file is generated and installed by the toplevel CMakeLists.txt of kdelibs.
# Include it to "import" the libraries from kdelibs into the current projects as targets.
# This makes setting the _LIBRARY and _LIBS variables actually a bit superfluos, since e.g.
# the kdeui library could now also be used just as "KDE4__kdeui" and still have all their
# dependent libraries handled correctly. But to keep compatibility and not to change
# behaviour we set all these variables anyway as seen below. Alex
include(${CMAKE_CURRENT_LIST_DIR}/KDELibs4LibraryTargets.cmake)

if(NOT TARGET KF5::ThreadWeaver)
   include(${KDE4_LIB_INSTALL_DIR}/cmake/ThreadWeaver/ThreadWeaverTargets.cmake)
endif()

# Look for Attica if necessary. It is needed by the imported knewstuff3 target (included via KDELibs4LibraryTargets-*.cmake)
if(NOT TARGET LibAttica::attica)
  find_package(LibAttica CONFIG)
  set_package_properties(LibAttica PROPERTIES TYPE ${_REQ_STRING_KDE4})
endif()

# This one is for compatibility only:
set(KDE4_THREADWEAVER_LIBRARIES KF5::ThreadWeaver )


# Set the various KDE4_FOO_LIBRARY/LIBS variables.
# In bootstrapping mode KDE4_TARGET_PREFIX is empty, so e.g. KDE4_KDECORE_LIBRARY
# will be simply set to "kdecore".

set(_KF5_TARGET_PREFIX "KF5::")

# Sorted by names:
_kde4_set_lib_variables(KCMUTILS      kcmutils      "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDE3SUPPORT   kde3support   "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDECORE_UNPORTED       kdecore       "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDEUI_UNPORTED         kdeui         "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDEWEBKIT     kdewebkit     "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDNSSD        kdnssd        "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KEMOTICONS    kemoticons    "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KFILE         kfile         "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KHTML         khtml         "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KIDLETIME     KIdleTime     "${_KF5_TARGET_PREFIX}")
_kde4_set_lib_variables(KIO           kio           "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KJS           KJS           "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KJSAPI        KJSApi        "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KNEWSTUFF2    knewstuff2    "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KNEWSTUFF3    knewstuff3    "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KNOTIFYCONFIG knotifyconfig "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KPARTS        kparts        "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KPRINTUTILS   kprintutils   "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KROSSCORE     krosscore     "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KROSSUI       krossui       "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KTEXTEDITOR   ktexteditor   "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KUNITCONVERSION kunitconversion "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KUTILS        kutils        "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(PLASMA        plasma        "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(SOLID         Solid         "${_KF5_TARGET_PREFIX}")
_kde4_set_lib_variables(THREADWEAVER  ThreadWeaver  "${_KF5_TARGET_PREFIX}")

set(KDE4_KDEUI_LIBRARY ${KDE4_KDEUI_UNPORTED_LIBRARY} )
set(KDE4_KDEUI_LIBS    ${KDE4_KDEUI_LIBRARY} )
set(KDE4_KDECORE_LIBS  ${INQT5_LIBRARY} ${KDE4_KDECORE_UNPORTED_LIBRARY} )

if (UNIX)
   _kde4_set_lib_variables(KDEFAKES kdefakes "${KDE4_TARGET_PREFIX}")
   _kde4_set_lib_variables(KDESU kdesu       "${KDE4_TARGET_PREFIX}")
   _kde4_set_lib_variables(KPTY kpty         "${KDE4_TARGET_PREFIX}")
endif (UNIX)

# The nepomuk target does not always exist, since is is built conditionally. When bootstrapping
# we set it always anyways.
if(TARGET ${KDE4_TARGET_PREFIX}nepomuk)
   _kde4_set_lib_variables(NEPOMUK nepomuk "${KDE4_TARGET_PREFIX}")
endif(TARGET ${KDE4_TARGET_PREFIX}nepomuk)

#####################  provide some options   ##########################################

option(KDE4_BUILD_TESTS  "Build the tests" ON)
option(KDE4_ENABLE_HTMLHANDBOOK  "Create targets htmlhandbook for creating the html versions of the docbook docs")
set(KDE4_SERIALIZE_TOOL "" CACHE STRING "Tool to serialize resource-intensive commands in parallel builds")

# if CMake 2.6.3 or above is used, provide an option which should be used by other KDE packages
# whether to install a CMake FooConfig.cmake into lib/foo/cmake/ or /lib/cmake/foo/
# (with 2.6.3 and above also lib/cmake/foo/ is supported):
option(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR "Prefer to install the <package>Config.cmake files to lib/cmake/<package> instead to lib/<package>/cmake" TRUE)

# Position-Independent-Executable is a feature of Binutils, Libc, and GCC that creates an executable
# which is something between a shared library and a normal executable.
# Programs compiled with these features appear as ?shared object? with the file command.
# info from "http://www.linuxfromscratch.org/hlfs/view/unstable/glibc/chapter02/pie.html"
option(KDE4_ENABLE_FPIE  "Enable platform supports PIE linking")

if (WIN32)
   find_package(KDEWin REQUIRED)
   option(KDE4_ENABLE_UAC_MANIFEST "add manifest to make vista uac happy" OFF)
   if (KDE4_ENABLE_UAC_MANIFEST)
      find_program(KDE4_MT_EXECUTABLE mt
         PATHS ${KDEWIN_INCLUDE_DIR}/../bin
         NO_DEFAULT_PATH
      )
      if (KDE4_MT_EXECUTABLE)
         message(STATUS "Found KDE manifest tool at ${KDE4_MT_EXECUTABLE} ")
      else (KDE4_MT_EXECUTABLE)
         message(STATUS "KDE manifest tool not found, manifest generating for Windows Vista disabled")
         set (KDE4_ENABLE_UAC_MANIFEST OFF)
      endif (KDE4_MT_EXECUTABLE)
   endif (KDE4_ENABLE_UAC_MANIFEST)
endif (WIN32)

#####################  some more settings   ##########################################

# If they are installed in a directory which contains "lib64", we default to "64" for LIB_SUFFIX,
# so the current project will by default also go into lib64.
# The same for lib32. Alex
set(_Init_LIB_SUFFIX "")
if ("${KDE4_LIB_DIR}" MATCHES lib64)
   set(_Init_LIB_SUFFIX 64)
endif ("${KDE4_LIB_DIR}" MATCHES lib64)
if ("${KDE4_LIB_DIR}" MATCHES lib32)
   set(_Init_LIB_SUFFIX 32)
endif ("${KDE4_LIB_DIR}" MATCHES lib32)

set(LIB_SUFFIX "${_Init_LIB_SUFFIX}" CACHE STRING "Define suffix of directory name (32/64)" )


########## the following are directories where stuff will be installed to  ###########
#
# this has to be after find_xxx() block above, since there KDELibsDependencies.cmake is included
# which contains the install dirs from kdelibs, which are reused below

# The following seem to be unsued. Let me know if you need them. Alex
# set(MIME_INSTALL_DIR         "share/mimelnk"           ) # The install dir for the mimetype desktop files
# set(DEMO_INSTALL_DIR         "share/demos"             ) # The install dir for demos
# set(INFO_INSTALL_DIR          "share/info"                ) # The info install dir (default ${SHARE_INSTALL_PREFIX}/info)")


##############  add some more default search paths  ###############
#
# the KDE4_xxx_INSTALL_DIR variables are empty when building kdelibs itself
# and otherwise point to the kde4 install dirs

set(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH}
                              "${KDE4_INCLUDE_INSTALL_DIR}")

set(CMAKE_SYSTEM_PROGRAM_PATH ${CMAKE_SYSTEM_PROGRAM_PATH}
                              "${KDE4_BIN_INSTALL_DIR}" )

set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}
                              "${KDE4_LIB_INSTALL_DIR}" )

# under Windows dlls may be also installed in bin/
if(WIN32)
  set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH}
                                "${_CMAKE_INSTALL_DIR}/bin"
                                "${CMAKE_INSTALL_PREFIX}/bin" )
endif(WIN32)


######################################################
#  and now the platform specific stuff
######################################################

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)


if (WIN32)

   if(CYGWIN)
      message(FATAL_ERROR "Cygwin is NOT supported, use mingw or MSVC to build KDE4.")
   endif(CYGWIN)

   # limit win32 packaging to kdelibs at now
   # don't know if package name, version and notes are always available
   set( _KDE4_PLATFORM_INCLUDE_DIRS ${KDEWIN_INCLUDES})

   # we prefer to use a different postfix for debug libs only on Windows
   # does not work atm
   set(CMAKE_DEBUG_POSTFIX "")

   # windows, microsoft compiler
   if(MSVC)
      set( _KDE4_PLATFORM_DEFINITIONS -DKDE_FULL_TEMPLATE_EXPORT_INSTANTIATION -DWIN32_LEAN_AND_MEAN )

      # C4250: 'class1' : inherits 'class2::member' via dominance
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4250" )
      # C4251: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4251" )
      # C4396: 'identifier' : 'function' the inline specifier cannot be used when a friend declaration refers to a specialization of a function template
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4396" )
      # to avoid a lot of deprecated warnings
      add_definitions( -D_CRT_SECURE_NO_DEPRECATE
                       -D_CRT_SECURE_NO_WARNINGS
                       -D_CRT_NONSTDC_NO_DEPRECATE
                       -D_SCL_SECURE_NO_WARNINGS
                       )
      # 'identifier' : no suitable definition provided for explicit template instantiation request
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
   endif(MSVC)


   # for visual studio IDE set the path correctly for custom commands
   # maybe under windows bat-files should be generated for running apps during the build
   if(MSVC_IDE)
     get_filename_component(PERL_LOCATION "${PERL_EXECUTABLE}" PATH)
     file(TO_NATIVE_PATH "${PERL_LOCATION}" PERL_PATH_WINDOWS)
     file(TO_NATIVE_PATH "${QT_BINARY_DIR}" QT_BIN_DIR_WINDOWS)
     set(CMAKE_MSVCIDE_RUN_PATH "${PERL_PATH_WINDOWS}\;${QT_BIN_DIR_WINDOWS}"
       CACHE STATIC "MSVC IDE Run path" FORCE)
   endif(MSVC_IDE)

   # we don't support anything below w2k and all winapi calls are unicodes
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0501 -DUNICODE" )
endif (WIN32)


if (Q_WS_X11)
   # Done by FindQt4.cmake already
   #find_package(X11 REQUIRED)
   # UNIX has already set _KDE4_PLATFORM_INCLUDE_DIRS, so append
   set(_KDE4_PLATFORM_INCLUDE_DIRS ${_KDE4_PLATFORM_INCLUDE_DIRS} ${X11_INCLUDE_DIR} )
endif (Q_WS_X11)



###########    end of platform specific stuff  ##########################


# KDE4Macros.cmake contains all the KDE specific macros
include(${CMAKE_CURRENT_LIST_DIR}/KDE4Macros.cmake)


# decide whether KDE4 has been found
set(KDE4_FOUND FALSE)
if (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_INSTALLED_VERSION_OK)
   set(KDE4_FOUND TRUE)
   set(KDE4Internal_FOUND TRUE) # for feature_summary
endif (KDE4_INCLUDE_DIR AND KDE4_LIB_DIR AND KDE4_INSTALLED_VERSION_OK)


macro (KDE4_PRINT_RESULTS)

   # inside kdelibs the include dir and lib dir are internal, not "found"
   if(KDE4_INCLUDE_DIR)
      message(STATUS "Found KDE 4.10 include dir: ${KDE4_INCLUDE_DIR}")
   else(KDE4_INCLUDE_DIR)
      message(STATUS "ERROR: unable to find the KDE 4 headers")
   endif(KDE4_INCLUDE_DIR)

   if(KDE4_LIB_DIR)
      message(STATUS "Found KDE 4.10 library dir: ${KDE4_LIB_DIR}")
   else(KDE4_LIB_DIR)
      message(STATUS "ERROR: unable to find the KDE 4 core library")
   endif(KDE4_LIB_DIR)

endmacro (KDE4_PRINT_RESULTS)


if (KDELibs4_FIND_REQUIRED AND NOT KDE4_FOUND)
   #bail out if something wasn't found
   kde4_print_results()
   if (NOT KDE4_INSTALLED_VERSION_OK)
     message(FATAL_ERROR "ERROR: the installed kdelibs version ${KDE_VERSION} is too old, at least version ${KDE_MIN_VERSION} is required")
   endif (NOT KDE4_INSTALLED_VERSION_OK)

   message(FATAL_ERROR "ERROR: could NOT find everything required for compiling KDE 4 programs")
endif ()

if (NOT KDE4Internal_FIND_QUIETLY)
   kde4_print_results()
endif (NOT KDE4Internal_FIND_QUIETLY)

#add the found Qt and KDE include directories to the current include path
#the ${KDE4_INCLUDE_DIR}/KDE directory is for forwarding includes, eg. #include <KMainWindow>
set(KDE4_INCLUDES
   ${KDE4_INCLUDE_DIR}
   ${KDE4_INCLUDE_DIR}/KDE
   ${QT_INCLUDES}
   ${_KDE4_PLATFORM_INCLUDE_DIRS}
)

# Used by kdebug.h: the "toplevel dir" is one level above CMAKE_SOURCE_DIR
get_filename_component(_KDE4_CMAKE_TOPLEVEL_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
string(LENGTH "${_KDE4_CMAKE_TOPLEVEL_DIR}" _KDE4_CMAKE_TOPLEVEL_DIR_LENGTH)

set(KDE4_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} -DQT_NO_STL -DQT_NO_CAST_TO_ASCII -D_REENTRANT -DKDE_DEPRECATED_WARNINGS -DKDE4_CMAKE_TOPLEVEL_DIR_LENGTH=${_KDE4_CMAKE_TOPLEVEL_DIR_LENGTH})

if (NOT _kde4_uninstall_rule_created)
   set(_kde4_uninstall_rule_created TRUE)

   configure_file("${CMAKE_CURRENT_LIST_DIR}/kde4_cmake_uninstall.cmake.in" "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake" @ONLY)

   add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

endif (NOT _kde4_uninstall_rule_created)

endif(NOT KDE4_FOUND)
