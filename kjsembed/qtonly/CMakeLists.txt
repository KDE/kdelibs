# Build the Qt Only version of kjs and kjsembed
# ** Loosely derived from kdelibs/CMakeList.txt
# 
# Copyright (c) 2006, Erik Bunce <kde@bunce.us>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(qtonly)

option(QTONLY_WEBKIT  "Build using WEBKIT from this directoy")
set(QTONLY_WEBKIT_DIR "${QTONLY_WEBKIT_DIR}")

# This is the QT_ONLY build, make sure the CMakeList.txt and source files know.
set(QT_ONLY Y)
add_definitions(-DQT_ONLY)

if (QTONLY_WEBKIT)
   add_definitions(-DQTONLY_WEBKIT)
   set(WEBKIT_LIB_SEARCH_PATH
       "${CMAKE_SOURCE_DIR}/WebKit"
       "${QTONLY_WEBKIT_DIR}/lib/"
       "${QTONLY_WEBKIT_DIR}/WebKitBuild/Release/lib/"
       "${QTONLY_WEBKIT_DIR}/WebKitBuild/Debug/lib/"
   )
   FIND_LIBRARY(WEBKIT_KJS_LIBRARY NAMES JavaScriptCore PATHS ${WEBKIT_LIB_SEARCH_PATH} NO_DEFAULT_PATH)
endif (QTONLY_WEBKIT)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake/modules;${CMAKE_SOURCE_DIR}")
set(CMAKE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/../../cmake/modules")

file(MAKE_DIRECTORY kdecore)
file(WRITE kdecore/kdefakes.h "// Nothing to see here... Move along...\n")

set(KDE_DISTRIBUTION_TEXT "compiled sources" CACHE STRING "Indicate the distribution in bug reports" )

include(FindQJSInternal.cmake)

########### Setup install paths #############

# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absoulte, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (UNIX)
      if ("${in}" MATCHES "^/.*")
         set(${out} ${in})
      else ("${in}" MATCHES "^/.*")
         set(${out} ${CMAKE_INSTALL_PREFIX}/${in})
      endif ("${in}" MATCHES "^/.*")
   else (UNIX)
      set(${out} ${in})
   endif (UNIX)
endmacro(MAKE_INSTALL_PATH_ABSOLUTE out in)

make_install_path_absolute(QJS_INSTALL_DIR         ${CMAKE_INSTALL_PREFIX})
make_install_path_absolute(QJS_LIB_INSTALL_DIR     ${LIB_INSTALL_DIR})
make_install_path_absolute(QJS_INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR})
make_install_path_absolute(QJS_BIN_INSTALL_DIR     ${BIN_INSTALL_DIR})
make_install_path_absolute(QJS_DATA_INSTALL_DIR    ${DATA_INSTALL_DIR})

# write the installdirs into a cmake file which will be installed
file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake 
"# this file was generated during the kdelibs build process
set(QJS_INSTALL_DIR         \"${QJS_INSTALL_PREFIX}\")
set(QJS_LIB_INSTALL_DIR     \"${QJS_LIB_INSTALL_DIR}\")
set(QJS_INCLUDE_INSTALL_DIR \"${QJS_INCLUDE_INSTALL_DIR}\")
set(QJS_BIN_INSTALL_DIR     \"${QJS_BIN_INSTALL_DIR}\")
set(QJS_DATA_INSTALL_DIR    \"${QJS_DATA_INSTALL_DIR}\")
")

find_package(ZLIB REQUIRED)

# set some default settings
include(KDE4Defaults)

# make some more macros available
include (MacroLibrary)

# macro_optional_find_package() is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(BZip2)
macro_optional_find_package(OpenSSL)
macro_optional_find_package(Libintl)


add_definitions(${QT_DEFINITIONS})

link_directories(${QJS_LIB_DIR})

# for including config.h and for includes like <kparts/foo.h>
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

# Those variables for  are only valid inside of kdelibs, of course.
# Use the one variable for the lib you depend upon.
# E.g. kdeui uses ${QJS_KDECORE_INCLUDES}. Something that depends on kparts uses ${QJS_KPARTS_INCLUDES}.

set(KDE4_KJS_INCLUDES ${CMAKE_SOURCE_DIR}/kjs
                      ${CMAKE_BINARY_DIR}/kjs)


# ACL stuff (used in kio/ and kioslaves/)
macro_optional_find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)

include(${CMAKE_SOURCE_DIR}/../../ConfigureChecks.cmake)

# now create config headers
configure_file(${CMAKE_SOURCE_DIR}/../../config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(${CMAKE_SOURCE_DIR}/../../config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(${CMAKE_SOURCE_DIR}/../../config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )
configure_file(${CMAKE_SOURCE_DIR}/../../kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )

include(KDE4Macros)
macro(KDE4_NO_ENABLE_FINAL _project_name)
   if(KDE4_ENABLE_FINAL)
        set(KDE4_ENABLE_FINAL OFF)
        remove_definitions(-DKDE_USE_FINAL)
        message(STATUS "You used enable-final argument but \"${_project_name}\" doesn't support it. Try to fix compile it and remove KDE4_NO_ENABLE_FINAL macro. Thanks")

   endif(KDE4_ENABLE_FINAL)
endmacro(KDE4_NO_ENABLE_FINAL _project_name)

if (NOT QTONLY_WEBKIT)
   include_directories( ${CMAKE_SOURCE_DIR}/../.. ${CMAKE_SOURCE_DIR}/../../wtf ${CMAKE_SOURCE_DIR}/../../kjsembed ${KDE4_KJS_INCLUDES} ${QT_INCLUDES} ${KDEWIN_INCLUDES} )
else (NOT QTONLY_WEBKIT)
   include_directories(  ${QTONLY_WEBKIT_DIR}/JavaScriptCore/kjs/  ${QTONLY_WEBKIT_DIR}/JavaScriptCore/ ${CMAKE_SOURCE_DIR}/../../kjsembed ${QT_INCLUDES} ${KDEWIN_INCLUDES} )
endif (NOT QTONLY_WEBKIT)

# list the subdirectories
if (NOT QTONLY_WEBKIT)
   #add_subdirectory( ${CMAKE_SOURCE_DIR}/../../wtf wtf )
   add_subdirectory( ${CMAKE_SOURCE_DIR}/../../kjs kjs )
endif (NOT QTONLY_WEBKIT)

add_subdirectory( ${CMAKE_SOURCE_DIR}/../kjsembed kjsembed )
add_subdirectory( ${CMAKE_SOURCE_DIR}/../kjscmd qjscmd )

macro_display_feature_log()

