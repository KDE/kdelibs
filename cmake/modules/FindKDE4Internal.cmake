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
#  KDE4_KCFGC_EXECUTABLE    - the kconfig_compiler executable
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
cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)
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

# get the directory of the current file, used later on in the file
get_filename_component( kde_cmake_module_dir  ${CMAKE_CURRENT_LIST_FILE} PATH)

# This file is only used when building kdelibs itself. When using an installed
# kdelibs, KDELibs4Config.cmake is loaded instead.
# then enter bootstrap mode
set(_kdeBootStrapping TRUE)
message(STATUS "Building kdelibs...")


# We may only search for other packages with "REQUIRED" if we are required ourselves.
# This file can be processed either (usually) included in FindKDE4.cmake or
# (when building kdelibs) directly via FIND_PACKAGE(KDE4Internal), that's why
# we have to check for both KDE4_FIND_REQUIRED and KDE4Internal_FIND_REQUIRED.
set(_REQ_STRING_KDE4 "REQUIRED")
set(_REQ_STRING_KDE4_MESSAGE "FATAL_ERROR")

# now we are sure we have everything we need
include (MacroLibrary)
include (CheckCXXCompilerFlag)
include (CheckCXXSourceCompiles)


# helper macro, sets both the KDE4_FOO_LIBRARY and KDE4_FOO_LIBS variables to KDE4__foo
# It is used both in bootstrapping and in normal mode.
macro(_KDE4_SET_LIB_VARIABLES _var _lib _prefix)
   set(KDE4_${_var}_LIBRARY ${_prefix}${_lib} )
   set(KDE4_${_var}_LIBS    ${_prefix}${_lib} )
endmacro(_KDE4_SET_LIB_VARIABLES _var _lib _prefix)

#######################  #now try to find some kde stuff  ################################

set(KDE4_INCLUDE_DIR ${kdelibs_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${kdelibs_BINARY_DIR}/bin )

if (WIN32)
   set(LIBRARY_OUTPUT_PATH               ${EXECUTABLE_OUTPUT_PATH} )
   # CMAKE_CFG_INTDIR is the output subdirectory created e.g. by XCode and MSVC
   if (NOT WINCE)
     set(KDE4_KCFGC_EXECUTABLE             ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kconfig_compiler )
   else (NOT WINCE)
     set(KDE4_KCFGC_EXECUTABLE             ${HOST_BINDIR}/${CMAKE_CFG_INTDIR}/kconfig_compiler )
   endif(NOT WINCE)

   set(KDE4_KAUTH_POLICY_GEN_EXECUTABLE  ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kauth-policy-gen )
   set(KDE4_MAKEKDEWIDGETS_EXECUTABLE    ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/makekdewidgets )
else (WIN32)
   set(LIBRARY_OUTPUT_PATH               ${CMAKE_BINARY_DIR}/lib )
   set(KDE4_KCFGC_EXECUTABLE             ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kconfig_compiler${CMAKE_EXECUTABLE_SUFFIX}.shell )
   set(KDE4_KAUTH_POLICY_GEN_EXECUTABLE  ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/kauth-policy-gen${CMAKE_EXECUTABLE_SUFFIX}.shell )
   set(KDE4_MAKEKDEWIDGETS_EXECUTABLE    ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/makekdewidgets${CMAKE_EXECUTABLE_SUFFIX}.shell )
endif (WIN32)

set(KDE4_LIB_DIR ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})


# when building kdelibs, make the kcfg rules depend on the binaries...
set( _KDE4_KCONFIG_COMPILER_DEP kconfig_compiler)
set( _KDE4_KAUTH_POLICY_GEN_EXECUTABLE_DEP kauth-policy-gen)
set( _KDE4_MAKEKDEWIDGETS_DEP makekdewidgets)

set(KDE4_INSTALLED_VERSION_OK TRUE)


# Set the various KDE4_FOO_LIBRARY/LIBS variables.
# In bootstrapping mode KDE4_TARGET_PREFIX is empty, so e.g. KDE4_KDECORE_LIBRARY
# will be simply set to "kdecore".

# Sorted by names:
_kde4_set_lib_variables(KCMUTILS      kcmutils      "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDECORE_UNPORTED       kdecore       "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDEUI_UNPORTED         kdeui         "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDEWEBKIT     kdewebkit     "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KDNSSD        kdnssd        "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KEMOTICONS    kemoticons    "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KFILE         kfile         "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KHTML         khtml         "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KIDLETIME     KIdleTime     "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KIO           kio           "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KJS           kjs           "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(KJSAPI        kjsapi        "${KDE4_TARGET_PREFIX}")
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
_kde4_set_lib_variables(SOLID         Solid         "${KDE4_TARGET_PREFIX}")
_kde4_set_lib_variables(THREADWEAVER  ThreadWeaver  "${KDE4_TARGET_PREFIX}")

set(KDE4_KDEUI_LIBRARY ${KDE4_KDEUI_UNPORTED_LIBRARY} )
set(KDE4_KDEUI_LIBS    ${KDE4_KDEUI_LIBRARY} )
set(KDE4_KDECORE_LIBS  ${INQT5_LIBRARY} ${KDE4_KDECORE_UNPORTED_LIBRARY} )

if (UNIX)
   _kde4_set_lib_variables(KDESU kdesu       "${KDE4_TARGET_PREFIX}")
   _kde4_set_lib_variables(KPTY kpty         "${KDE4_TARGET_PREFIX}")
endif (UNIX)

# The nepomuk target does not always exist, since is is built conditionally. When bootstrapping
# we set it always anyways.
_kde4_set_lib_variables(NEPOMUK nepomuk "${KDE4_TARGET_PREFIX}")

#####################  provide some options   ##########################################

option(KDE4_BUILD_TESTS  "Build the tests" ON)
option(KDE4_ENABLE_HTMLHANDBOOK  "Create targets htmlhandbook for creating the html versions of the docbook docs")
set(KDE4_SERIALIZE_TOOL "" CACHE STRING "Tool to serialize resource-intensive commands in parallel builds")

# if CMake 2.6.3 or above is used, provide an option which should be used by other KDE packages
# whether to install a CMake FooConfig.cmake into lib/foo/cmake/ or /lib/cmake/foo/
# (with 2.6.3 and above also lib/cmake/foo/ is supported):
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.6.2)
   option(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR "Prefer to install the <package>Config.cmake files to lib/cmake/<package> instead to lib/<package>/cmake" TRUE)
else(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.6.2)
   set(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR  FALSE)
endif(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} VERSION_GREATER 2.6.2)

if (WIN32)
   list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/apps/cmake/modules")
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

if (WIN32)

   if(CYGWIN)
      message(FATAL_ERROR "Cygwin is NOT supported, use mingw or MSVC to build KDE4.")
   endif(CYGWIN)

   # limit win32 packaging to kdelibs at now
   # don't know if package name, version and notes are always available
   find_package(KDEWIN_Packager)
   if (KDEWIN_PACKAGER_FOUND)
      kdewin_packager("kdelibs" "${KDE_VERSION}" "KDE base library" "")
   endif (KDEWIN_PACKAGER_FOUND)

   include(Win32Macros)
   addExplorerWrapper("kdelibs")

   set( _KDE4_PLATFORM_INCLUDE_DIRS ${KDEWIN_INCLUDES})

   # if we are compiling kdelibs, add KDEWIN_LIBRARIES explicitly,
   # otherwise they come from KDELibsDependencies.cmake, Alex
   set( KDE4_KDECORE_LIBS ${KDE4_KDECORE_LIBS} ${KDEWIN_LIBRARIES} )

   # for visual studio IDE set the path correctly for custom commands
   # maybe under windows bat-files should be generated for running apps during the build
   if(MSVC_IDE)
     get_filename_component(PERL_LOCATION "${PERL_EXECUTABLE}" PATH)
     file(TO_NATIVE_PATH "${PERL_LOCATION}" PERL_PATH_WINDOWS)
     file(TO_NATIVE_PATH "${QT_BINARY_DIR}" QT_BIN_DIR_WINDOWS)
     set(CMAKE_MSVCIDE_RUN_PATH "${PERL_PATH_WINDOWS}\;${QT_BIN_DIR_WINDOWS}"
       CACHE STATIC "MSVC IDE Run path" FORCE)
   endif(MSVC_IDE)

endif (WIN32)


if (UNIX)
   set( _KDE4_PLATFORM_INCLUDE_DIRS)
endif (UNIX)


if (Q_WS_X11)
   # Done by FindQt4.cmake already
   #find_package(X11 REQUIRED)
   # UNIX has already set _KDE4_PLATFORM_INCLUDE_DIRS, so append
   set(_KDE4_PLATFORM_INCLUDE_DIRS ${_KDE4_PLATFORM_INCLUDE_DIRS} ${X11_INCLUDE_DIR} )
endif (Q_WS_X11)


############################################################
# compiler specific settings
############################################################


# this macro is for internal use only.
macro(KDE_CHECK_FLAG_EXISTS FLAG VAR DOC)
   if(NOT ${VAR} MATCHES "${FLAG}")
      set(${VAR} "${${VAR}} ${FLAG}" CACHE STRING "Flags used by the linker during ${DOC} builds." FORCE)
   endif(NOT ${VAR} MATCHES "${FLAG}")
endmacro(KDE_CHECK_FLAG_EXISTS FLAG VAR)

# This macro is for internal use only
# Return the directories present in gcc's include path.
macro(_DETERMINE_GCC_SYSTEM_INCLUDE_DIRS _lang _result)
  set(${_result})
  set(_gccOutput)
  file(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy" "\n" )
  execute_process(COMMAND ${CMAKE_C_COMPILER} -v -E -x ${_lang} -dD dummy
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles
                  ERROR_VARIABLE _gccOutput
                  OUTPUT_VARIABLE _gccStdout )
  file(REMOVE "${CMAKE_BINARY_DIR}/CMakeFiles/dummy")

  if( "${_gccOutput}" MATCHES "> search starts here[^\n]+\n *(.+) *\n *End of (search) list" )
    SET(${_result} ${CMAKE_MATCH_1})
    STRING(REPLACE "\n" " " ${_result} "${${_result}}")
    SEPARATE_ARGUMENTS(${_result})
  ENDIF( "${_gccOutput}" MATCHES "> search starts here[^\n]+\n *(.+) *\n *End of (search) list" )
ENDMACRO(_DETERMINE_GCC_SYSTEM_INCLUDE_DIRS _lang)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES Clang)
   _DETERMINE_GCC_SYSTEM_INCLUDE_DIRS(c _dirs)
   set(CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES
       ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES} ${_dirs})
endif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES Clang)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
   _DETERMINE_GCC_SYSTEM_INCLUDE_DIRS(c++ _dirs)
   set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES} ${_dirs})

endif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)


###########    end of platform specific stuff  ##########################


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

   configure_file("${kde_cmake_module_dir}/kde4_cmake_uninstall.cmake.in" "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake" @ONLY)

   add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

endif (NOT _kde4_uninstall_rule_created)

