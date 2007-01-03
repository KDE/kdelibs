# Find the Qt kjs/kjsembed library dirs, preprocessors and define some macros
# ** Derived from FindKDE4Internal.cmake
#
# This module defines a bunch of variables used as locations
# for install directories. They are all interpreted relative
# to CMAKE_INSTALL_PREFIX
#
# BIN_INSTALL_DIR          - the directory where executables be installed (default is prefix/bin)
# LIB_INSTALL_DIR          - the directory where libraries will be installed (default is prefix/lib)
# DATA_INSTALL_DIR         - the parent directory where applications can install their data
# PLUGIN_INSTALL_DIR       - the subdirectory relative to the install prefix where plugins will be installed (default is ${KDE4_LIB_INSTALL_DIR}/kde4)
#
# Copyright (c) 2006, Alexander Neundorf <neundorf@kde.org>
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
# Copyright (c) 2006, Erik Bunce <kde@bunce.us>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

INCLUDE (MacroEnsureVersion)

cmake_minimum_required(VERSION 2.4.3 FATAL_ERROR)


option(KDE4_BUILD_TESTS  "Build the tests")

set(QT_MIN_VERSION "4.2.0")
#this line includes FindQt4.cmake, which searches the Qt library and headers
find_package(Qt4 REQUIRED)

include (MacroLibrary)
include (CheckCXXCompilerFlag)

# this macro implements some very special logic how to deal with the cache
# by default the various install locations inherit their value from theit "parent" variable
# so if you set CMAKE_INSTALL_PREFIX, then EXEC_INSTALL_PREFIX, PLUGIN_INSTALL_DIR will
# calculate their value by appending subdirs to CMAKE_INSTALL_PREFIX
# this would work completely without using the cache.
# but if somebody wants e.g. a different EXEC_INSTALL_PREFIX this value has to go into
# the cache, otherwise it will be forgotten on the next cmake run.
# Once a variable is in the cache, it doesn't depend on its "parent" variables
# anymore and you can only change it by editing it directly.
# this macro helps in this regard, because as long as you don't set one of the
# variables explicitely to some location, it will always calculate its value from its
# parents. So modifying CMAKE_INSTALL_PREFIX later on will have the desired effect.
# But once you decide to set e.g. EXEC_INSTALL_PREFIX to some special location
# this will go into the cache and it will no longer depend on CMAKE_INSTALL_PREFIX.
macro(_SET_FANCY _var _value _comment)
   if (NOT DEFINED ${_var})
      set(${_var} ${_value})
   else (NOT DEFINED ${_var})
      set(${_var} "${${_var}}" CACHE PATH "${_comment}")
   endif (NOT DEFINED ${_var})
endmacro(_SET_FANCY)

# the following are directories where stuff will be installed to
set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)" )

_set_fancy(SHARE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/share              "Base directory for files which go to share/")
_set_fancy(EXEC_INSTALL_PREFIX  ${CMAKE_INSTALL_PREFIX}                    "Base directory for executables and libraries")
_set_fancy(BIN_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/bin"           "The install dir for executables (default ${EXEC_INSTALL_PREFIX}/bin)")
_set_fancy(LIB_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}" "The subdirectory relative to the install prefix where libraries will be installed (default is ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX})")
_set_fancy(LIBEXEC_INSTALL_DIR      "${LIB_INSTALL_DIR}/qjs/libexec"      "The subdirectory relative to the install prefix where libraries will be installed (default is ${LIB_INSTALL_DIR}/kde4/libexec)")
_set_fancy(PLUGIN_INSTALL_DIR       "${LIB_INSTALL_DIR}/qjs"              "The subdirectory relative to the install prefix where plugins will be installed (default is ${LIB_INSTALL_DIR}/qjs)")
_set_fancy(INCLUDE_INSTALL_DIR      "${CMAKE_INSTALL_PREFIX}/include"      "The subdirectory to the header prefix")
_set_fancy(DATA_INSTALL_DIR         "${SHARE_INSTALL_PREFIX}/apps"         "The parent directory where applications can install their data")

set(KDE4_KDECORE_LIBS ${QT_QTCORE_LIBRARY})
set(KDE4_KDEUI_LIBS ${KDE4_KDECORE_LIBS})


#####################  and now the platform specific stuff  ############################

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)


if (WIN32)

   if(CYGWIN)
      message(FATAL_ERROR "Support for Cygwin NOT yet implemented, please edit FindKDE4.cmake to enable it")
   endif(CYGWIN)

   find_package(KDEWIN32 REQUIRED)

   # is GnuWin32 required or does e.g. Visual Studio provide an own implementation?
   #find_package(GNUWIN32 REQUIRED)
   find_package(GNUWIN32)

   set( _KDE4_PLATFORM_INCLUDE_DIRS ${KDEWIN32_INCLUDES} ${GNUWIN32_INCLUDE_DIR})

   # if we are compiling kdelibs, add KDEWIN32_LIBRARIES explicitely,
   # otherwise they come from KDELibsDependencies.cmake, Alex
   if(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kernel/kglobal.h)
      set( KDE4_KDECORE_LIBS ${KDE4_KDECORE_LIBS} ${KDEWIN32_LIBRARIES} )
   endif(EXISTS ${CMAKE_SOURCE_DIR}/kdecore/kernel/kglobal.h)

   # windows, mingw
   if(MINGW)
      #hmmm, something special to do here ?
   endif(MINGW)

   # windows, microsoft compiler
   if(MSVC)
      set( _KDE4_PLATFORM_DEFINITIONS -DKDE_FULL_TEMPLATE_EXPORT_INSTANTIATION -DWIN32_LEAN_AND_MEAN -DUNICODE )
      # C4250: 'class1' : inherits 'class2::member' via dominance
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4250" )
      # C4251: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4251" )
      if(CMAKE_COMPILER_2005)
         # to avoid a lot of deprecated warnings
         add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE )
         # 'identifier' : no suitable definition provided for explicit template instantiation request
         set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
      endif(CMAKE_COMPILER_2005)
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

endif (WIN32)


# also use /usr/local by default under UNIX, including Mac OS X
if (UNIX)
   link_directories(/usr/local/lib)
   set( _KDE4_PLATFORM_INCLUDE_DIRS /usr/local/include )

   # the rest is RPATH handling
   # here the defaults are set
   # which are partly overwritten in kde4_handle_rpath_for_library()
   # and kde4_handle_rpath_for_executable(), both located in KDE4Macros.cmake, Alex
   if (APPLE)
      set(CMAKE_INSTALL_NAME_DIR ${LIB_INSTALL_DIR})
   else (APPLE)
      # add our LIB_INSTALL_DIR to the RPATH and use the RPATH figured out by cmake when compiling
      set(CMAKE_INSTALL_RPATH ${LIB_INSTALL_DIR} )
      set(CMAKE_SKIP_BUILD_RPATH TRUE)
      set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
      set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
   endif (APPLE)
endif (UNIX)


# UNIX, except OS X
if (UNIX AND NOT APPLE)
   # Done by FindQt4.cmake already
   #find_package(X11 REQUIRED)
   # UNIX has already set _KDE4_PLATFORM_INCLUDE_DIRS, so append
   set(_KDE4_PLATFORM_INCLUDE_DIRS ${_KDE4_PLATFORM_INCLUDE_DIRS} ${X11_INCLUDE_DIR} )
endif (UNIX AND NOT APPLE)


# This will need to be modified later to support either Qt/X11 or Qt/Mac builds
if (APPLE)

  set ( _KDE4_PLATFORM_DEFINITIONS -D__APPLE_KDE__ )

  # we need to set MACOSX_DEPLOYMENT_TARGET to (I believe) at least 10.2 or maybe 10.3 to allow
  # -undefined dynamic_lookup; in the future we should do this programmatically
  # hmm... why doesn't this work?
  set (ENV{MACOSX_DEPLOYMENT_TARGET} 10.3)

  # "-undefined dynamic_lookup" means we don't care about missing symbols at link-time by default
  # this is bad, but unavoidable until there is the equivalent of libtool -no-undefined implemented
  # or perhaps it already is, and I just don't know where to look  ;)

  set (CMAKE_SHARED_LINKER_FLAGS "-single_module -multiply_defined suppress ${CMAKE_SHARED_LINKER_FLAGS}")
  set (CMAKE_MODULE_LINKER_FLAGS "-multiply_defined suppress ${CMAKE_MODULE_LINKER_FLAGS}")
  #set(CMAKE_SHARED_LINKER_FLAGS "-single_module -undefined dynamic_lookup -multiply_defined suppress")
  #set(CMAKE_MODULE_LINKER_FLAGS "-undefined dynamic_lookup -multiply_defined suppress")

  # we profile...
  if(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
  endif(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)

  # removed -Os, was there a special reason for using -Os instead of -O2 ?, Alex
  # optimization flags are set below for the various build types
  set (CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -fno-common")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-common")
endif (APPLE)


if (CMAKE_SYSTEM_NAME MATCHES Linux)
   if (CMAKE_COMPILER_IS_GNUCXX)
      set ( _KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_GNU_SOURCE)
      set ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc")
      set ( CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc")
      # we profile...
      if(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)
        set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
        set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
      endif(CMAKE_BUILD_TYPE_TOLOWER MATCHES profile)
   endif (CMAKE_COMPILER_IS_GNUCXX)
   if (CMAKE_C_COMPILER MATCHES "icc")
      set ( _KDE4_PLATFORM_DEFINITIONS -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -D_GNU_SOURCE)
      set ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc")
      set ( CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined -lc")
   endif (CMAKE_C_COMPILER MATCHES "icc")
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

if (CMAKE_SYSTEM_NAME MATCHES BSD)
   set ( _KDE4_PLATFORM_DEFINITIONS -D_GNU_SOURCE )
   set ( CMAKE_SHARED_LINKER_FLAGS "-lc")
   set ( CMAKE_MODULE_LINKER_FLAGS "-lc")
endif (CMAKE_SYSTEM_NAME MATCHES BSD)

# compiler specific stuff, maybe this should be done differently, Alex

if (MSVC)
   set (KDE4_ENABLE_EXCEPTIONS -EHsc)
endif(MSVC)

if (MINGW)
   set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols")
   set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--export-all-symbols")
endif (MINGW)

if (CMAKE_COMPILER_IS_GNUCXX)
   set (KDE4_ENABLE_EXCEPTIONS -fexceptions)
   # Select flags.
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
   set(CMAKE_CXX_FLAGS_RELEASE        "-O2")
   set(CMAKE_CXX_FLAGS_DEBUG          "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
   set(CMAKE_CXX_FLAGS_DEBUGFULL      "-g3 -fno-inline")
   set(CMAKE_CXX_FLAGS_PROFILE        "-g3 -fno-inline -ftest-coverage -fprofile-arcs")
   set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
   set(CMAKE_C_FLAGS_RELEASE          "-O2")
   set(CMAKE_C_FLAGS_DEBUG            "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
   set(CMAKE_C_FLAGS_DEBUGFULL        "-g3 -fno-inline")
   set(CMAKE_C_FLAGS_PROFILE          "-g3 -fno-inline -ftest-coverage -fprofile-arcs")

   if (CMAKE_SYSTEM_NAME MATCHES Linux)
     set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -ansi -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
     set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -fno-exceptions -fno-check-new -fno-common")
   endif (CMAKE_SYSTEM_NAME MATCHES Linux)


   check_cxx_compiler_flag(-fPIE HAVE_FPIE_SUPPORT)
   if(KDE4_ENABLE_FPIE)
       if(HAVE_FPIE_SUPPORT)
        set (KDE4_CXX_FPIE_FLAGS "-fPIE")
        set (KDE4_PIE_LDFLAGS "-pie")
       else(HAVE_FPIE_SUPPORT)
        MESSAGE(STATUS "Your compiler doesn't support PIE flag")
       endif(HAVE_FPIE_SUPPORT)
   endif(KDE4_ENABLE_FPIE)
   # visibility support
   check_cxx_compiler_flag(-fvisibility=hidden __KDE_HAVE_GCC_VISIBILITY)

   # get the gcc version
   exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE _gcc_version_info)

   string (REGEX MATCH " [34]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
   # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the patch level, handle this here:
   if (NOT _gcc_version)
      string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
   endif (NOT _gcc_version)

   macro_ensure_version("4.1.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_1)

   if (GCC_IS_NEWER_THAN_4_1)
      exec_program(${CMAKE_C_COMPILER} ARGS -v OUTPUT_VARIABLE _gcc_alloc_info)
      string(REGEX MATCH "(--enable-libstdcxx-allocator=mt)" _GCC_COMPILED_WITH_BAD_ALLOCATOR "${_gcc_alloc_info}")
   else (GCC_IS_NEWER_THAN_4_1)
      set(_GCC_COMPILED_WITH_BAD_ALLOCATOR FALSE)
   endif (GCC_IS_NEWER_THAN_4_1)

   if (__KDE_HAVE_GCC_VISIBILITY AND GCC_IS_AT_LEAST_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
   endif (__KDE_HAVE_GCC_VISIBILITY AND GCC_IS_AT_LEAST_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)

endif (CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_C_COMPILER MATCHES "icc")
   set (KDE4_ENABLE_EXCEPTIONS -fexceptions)
   # Select flags.
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
   set(CMAKE_CXX_FLAGS_RELEASE        "-O2")
   set(CMAKE_CXX_FLAGS_DEBUG          "-O2 -g -0b0 -noalign")
   set(CMAKE_CXX_FLAGS_DEBUGFULL      "-g -Ob0 -noalign")
   set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
   set(CMAKE_C_FLAGS_RELEASE          "-O2")
   set(CMAKE_C_FLAGS_DEBUG            "-O2 -g -Ob0 -noalign")
   set(CMAKE_C_FLAGS_DEBUGFULL        "-g -Ob0 -noalign")

   set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -ansi -Wpointer-arith -fno-common")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -Wpointer-arith -fno-exceptions -fno-common")

   # visibility support
   set(__KDE_HAVE_ICC_VISIBILITY)
#   check_cxx_compiler_flag(-fvisibility=hidden __KDE_HAVE_ICC_VISIBILITY)
#   if (__KDE_HAVE_ICC_VISIBILITY)
#      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
#   endif (__KDE_HAVE_ICC_VISIBILITY)

endif (CMAKE_C_COMPILER MATCHES "icc")

# we prefer to use a different postfix for debug libs only on Windows
# does not work atm
if (WIN32)
   SET(CMAKE_DEBUG_POSTFIX "")
endif (WIN32)

###########    end of platform specific stuff  ##########################
