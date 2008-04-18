project(kdelibs)

# set version
set (KDE_VERSION_MAJOR 4)
set (KDE_VERSION_MINOR 00)
set (KDE_VERSION_RELEASE 70)
set (KDE_VERSION "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}" )
set (KDE_VERSION_STRING "${KDE_VERSION} (KDE 4.0.70 >= 20080418)")


# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(KDE_DISTRIBUTION_TEXT "compiled sources" CACHE STRING "Indicate the distribution in bug reports" )

# this must be before FindKDE4Internal in order to preset the result of the visibility test, so that it will be skipped
option(KHTML_BUILD_TESTREGRESSION "Build KHTML's testregression. Note: this disables hidden visibility")
# Disable visibility if testregression is built, because the symbols are needed then
if (KHTML_BUILD_TESTREGRESSION)
   set (__KDE_HAVE_GCC_VISIBILITY 0)
endif (KHTML_BUILD_TESTREGRESSION)

find_package(KDE4Internal REQUIRED)
include(KDE4Defaults)
include (MacroLibrary)
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

#required features
find_package(ZLIB REQUIRED)
find_package(Strigi REQUIRED)

#optional features
macro_optional_find_package(BZip2)
macro_log_feature(BZIP2_FOUND "BZip2" "A high-quality data compressor" "http://www.bzip.org" FALSE "" "STRONGLY RECOMMENDED: Provides the ability to read and write bzip2 compressed data files.")

macro_optional_find_package(OpenSSL)
macro_log_feature(OPENSSL_FOUND "OpenSSL" "A toolkit implementing the Secure Sockets Layer (SSL v2/v3) and Transport Layer Security (TLS v1) protocols" "http://openssl.org" FALSE "" "STRONGLY RECOMMENDED: KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS.")

macro_optional_find_package(Libintl)
macro_log_feature(LIBINTL_FOUND "Libintl" "Library for message translation" "http://www.gnu.org/software/gettext" FALSE "" "STRONGLY RECOMMENDED: Enables KDE to be available in many different languages.")

macro_optional_find_package(OpenGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3D graphics applications" "http://mesa3d.sourceforge.net" FALSE "" "STRONGLY RECOMMENDED: The 3D hardware acceleration available through the OpenGL API is used in applications ranging from graphics and modellers to screensavers and video players.")

set(SOPRANO_MIN_VERSION "2.0.97")
macro_optional_find_package(Soprano)
macro_log_feature(Soprano_FOUND "Soprano" "Soprano Libraries" "kdesupport" FALSE ${SOPRANO_MIN_VERSION} "Provide metadata support (for semantic desktop).")

#########################################################################
# Disallow in-source build
macro_ensure_out_of_source_build("kdelibs requires an out of source build. Please create a separate build directory and run 'cmake path_to_kdelibs [options]' there.")

# ... and warn in case of an earlier in-source build
set(generatedFileInSourceDir EXISTS ${kdelibs_SOURCE_DIR}/kdemacros.h OR EXISTS ${kdelibs_SOURCE_DIR}/config.h)
if(${generatedFileInSourceDir})
   message(STATUS "kdemacros.h or config.h exists in your source directory.")
   message(FATAL_ERROR "Please run svn-clean, it would seem that your source directory has generated files in it.")
endif(${generatedFileInSourceDir})
#########################################################################


add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES) # for off_t
remove_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

# for including config.h and for includes like <kparts/foo.h>
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces)

# Those variables for  are only valid inside of kdelibs, of course.
# Use the one variable for the lib you depend upon.
# E.g. kdeui uses ${KDE4_KDECORE_INCLUDES}. Something that depends on kparts uses ${KDE4_KPARTS_INCLUDES}.

set(KDE4_KJS_INCLUDES ${CMAKE_SOURCE_DIR}/kjs
                      ${CMAKE_BINARY_DIR}/kjs)

# kdecore depends on Qt (need only headers from kjs)
set(KDE4_KDECORE_INCLUDES ${KDE4_KJS_INCLUDES}
                          ${CMAKE_SOURCE_DIR}/kdecore
                          ${CMAKE_BINARY_DIR}/kdecore
                          ${CMAKE_SOURCE_DIR}/kdecore/compression
                          ${CMAKE_SOURCE_DIR}/kdecore/config
                          ${CMAKE_SOURCE_DIR}/kdecore/date
                          ${CMAKE_SOURCE_DIR}/kdecore/io
                          ${CMAKE_SOURCE_DIR}/kdecore/jobs
                          ${CMAKE_SOURCE_DIR}/kdecore/kernel
                          ${CMAKE_SOURCE_DIR}/kdecore/network
                          ${CMAKE_SOURCE_DIR}/kdecore/services
                          ${CMAKE_SOURCE_DIR}/kdecore/localization
                          ${CMAKE_SOURCE_DIR}/kdecore/sycoca
                          ${CMAKE_SOURCE_DIR}/kdecore/text
                          ${CMAKE_SOURCE_DIR}/kdecore/util
                          ${CMAKE_SOURCE_DIR}/kdecore/sonnet
                          ${QT_INCLUDES}
                          ${_KDE4_PLATFORM_INCLUDE_DIRS})
# kdeui depends on kdecore
set(KDE4_KDEUI_INCLUDES ${CMAKE_SOURCE_DIR}/kdeui
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
                        ${CMAKE_SOURCE_DIR}/kdeui/paged
                        ${CMAKE_SOURCE_DIR}/kdeui/plotting
                        ${CMAKE_SOURCE_DIR}/kdeui/shortcuts
                        ${CMAKE_SOURCE_DIR}/kdeui/sonnet
                        ${CMAKE_SOURCE_DIR}/kdeui/util
                        ${CMAKE_SOURCE_DIR}/kdeui/widgets
                        ${CMAKE_SOURCE_DIR}/kdeui/windowmanagement
                        ${CMAKE_SOURCE_DIR}/kdeui/xmlgui
                        ${KDE4_KDECORE_INCLUDES})

# kio depends on kdeui
set(KDE4_KIO_INCLUDES ${CMAKE_SOURCE_DIR}/kio
                      ${CMAKE_SOURCE_DIR}/kio/bookmarks
                      ${CMAKE_SOURCE_DIR}/kio/kio
                      ${CMAKE_SOURCE_DIR}/kio/kfile
                      ${KDE4_KDEUI_INCLUDES})

# kpty 
set(KDE4_KPTY_INCLUDES ${CMAKE_SOURCE_DIR}/kpty ${KDE4_KIO_INCLUDES} )

# kparts depends on kio
set(KDE4_KPARTS_INCLUDES ${CMAKE_SOURCE_DIR}/kparts
                         ${KDE4_KIO_INCLUDES})

# kde3support depends on kparts
set(KDE4_KDE3SUPPORT_INCLUDES ${CMAKE_SOURCE_DIR}/kde3support
                              ${CMAKE_SOURCE_DIR}/kde3support/kdecore
                              ${CMAKE_SOURCE_DIR}/kde3support/kdeui
                              ${CMAKE_SOURCE_DIR}/kde3support/kio
                              ${KDE4_KPARTS_INCLUDES})

# khtml depends on kde3support currently
set(KDE4_KHTML_INCLUDES ${CMAKE_SOURCE_DIR}/khtml
                        ${KDE4_KDE3SUPPORT_INCLUDES})

# ACL stuff (used in kio/ and kioslaves/)
find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
configure_file(config-acl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-acl.h )

include(ConfigureChecks.cmake)

# now create config headers
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )
configure_file(config-pty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-pty.h )
configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )

# these two calls here should go somewhere else, Alex
check_library_exists(nsl gethostbyname "" HAVE_NSL_LIBRARY)
check_library_exists(socket connect "" HAVE_SOCKET_LIBRARY)

# list the subdirectories

add_subdirectory( cmake )
add_subdirectory( kdecore )
add_subdirectory( kdeui )
if (UNIX)
   add_subdirectory( kpty )
   add_subdirectory( kdesu )
endif (UNIX)
add_subdirectory( kjs )
add_subdirectory( kjsembed )
add_subdirectory( kwallet )
add_subdirectory( kio )
add_subdirectory( phonon )
add_subdirectory( solid )
add_subdirectory( kded       )
add_subdirectory( kde3support  )
add_subdirectory( kfile )
add_subdirectory( kconf_update )
add_subdirectory( kdoctools  )
add_subdirectory( kioslave   )
add_subdirectory( knewstuff  )
add_subdirectory( kparts     )
add_subdirectory( kutils     )
add_subdirectory( licenses   )
add_subdirectory( mimetypes  )
macro_optional_add_subdirectory( doc  )
add_subdirectory( kinit  )
add_subdirectory( threadweaver )
add_subdirectory( sonnet  )
add_subdirectory( khtml  )
add_subdirectory( interfaces  )
add_subdirectory( kdewidgets  )
add_subdirectory( kate  )
add_subdirectory( knotify )
add_subdirectory( kimgio )
add_subdirectory( dnssd )
add_subdirectory( kross )
add_subdirectory( security )
add_subdirectory( includes )

if(Soprano_FOUND)
  macro_optional_add_subdirectory(nepomuk)
endif(Soprano_FOUND)


######## write dependency file which will be installed #########

# write the versions 

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake  
"# this file was generated during the kdelibs build process
set(KDE_VERSION_MAJOR ${KDE_VERSION_MAJOR})
set(KDE_VERSION_MINOR ${KDE_VERSION_MINOR})
set(KDE_VERSION_RELEASE ${KDE_VERSION_RELEASE})
set(KDE_VERSION \"${KDE_VERSION}\")
set(KDE_VERSION_STRING \"${KDE_VERSION_STRING}\")
")

# write the installdirs

# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absoulte, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (UNIX)
      if ("${in}" MATCHES "^/.*")
         set(${out} "${in}")
      else ("${in}" MATCHES "^/.*")
         set(${out} "\${KDE4_INSTALL_DIR}/${in}")
      endif ("${in}" MATCHES "^/.*")
   else (UNIX)
      if ("${in}" MATCHES "^[a-zA-Z]:.*")
        set(${out} "${in}")
      else ("${in}" MATCHES "^[a-zA-Z]:.*")
         set(${out} "\${KDE4_INSTALL_DIR}/${in}")
      endif ("${in}" MATCHES "^[a-zA-Z]:.*")
   endif (UNIX)
endmacro(MAKE_INSTALL_PATH_ABSOLUTE out in)

set(KDE4_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
make_install_path_absolute(KDE4_LIB_INSTALL_DIR     ${LIB_INSTALL_DIR})
make_install_path_absolute(KDE4_LIBEXEC_INSTALL_DIR ${LIBEXEC_INSTALL_DIR})
make_install_path_absolute(KDE4_INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR})
make_install_path_absolute(KDE4_BIN_INSTALL_DIR     ${BIN_INSTALL_DIR})
make_install_path_absolute(KDE4_SBIN_INSTALL_DIR    ${SBIN_INSTALL_DIR})
make_install_path_absolute(KDE4_DATA_INSTALL_DIR    ${DATA_INSTALL_DIR})
make_install_path_absolute(KDE4_HTML_INSTALL_DIR    ${HTML_INSTALL_DIR})
make_install_path_absolute(KDE4_CONFIG_INSTALL_DIR  ${CONFIG_INSTALL_DIR})
make_install_path_absolute(KDE4_ICON_INSTALL_DIR    ${ICON_INSTALL_DIR})
make_install_path_absolute(KDE4_KCFG_INSTALL_DIR    ${KCFG_INSTALL_DIR})
make_install_path_absolute(KDE4_LOCALE_INSTALL_DIR  ${LOCALE_INSTALL_DIR})
make_install_path_absolute(KDE4_MIME_INSTALL_DIR    ${MIME_INSTALL_DIR})
make_install_path_absolute(KDE4_SOUND_INSTALL_DIR   ${SOUND_INSTALL_DIR})
make_install_path_absolute(KDE4_TEMPLATES_INSTALL_DIR    ${TEMPLATES_INSTALL_DIR})
make_install_path_absolute(KDE4_WALLPAPER_INSTALL_DIR    ${WALLPAPER_INSTALL_DIR})
make_install_path_absolute(KDE4_KCONF_UPDATE_INSTALL_DIR    ${KCONF_UPDATE_INSTALL_DIR})
make_install_path_absolute(KDE4_AUTOSTART_INSTALL_DIR    ${AUTOSTART_INSTALL_DIR})
make_install_path_absolute(KDE4_XDG_APPS_INSTALL_DIR        ${XDG_APPS_INSTALL_DIR})
make_install_path_absolute(KDE4_XDG_DIRECTORY_INSTALL_DIR   ${XDG_DIRECTORY_INSTALL_DIR})
make_install_path_absolute(KDE4_SYSCONF_INSTALL_DIR ${SYSCONF_INSTALL_DIR})
make_install_path_absolute(KDE4_MAN_INSTALL_DIR     ${MAN_INSTALL_DIR})
make_install_path_absolute(KDE4_INFO_INSTALL_DIR    ${INFO_INSTALL_DIR})
make_install_path_absolute(KDE4_DBUS_INTERFACES_DIR ${DBUS_INTERFACES_INSTALL_DIR})
make_install_path_absolute(KDE4_DBUS_SERVICES_DIR   ${DBUS_SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4_SERVICES_INSTALL_DIR ${SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4_SERVICETYPES_INSTALL_DIR ${SERVICETYPES_INSTALL_DIR})

file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake  "
if (NOT KDE4_INSTALL_DIR)
   set(KDE4_INSTALL_DIR         \"${CMAKE_INSTALL_PREFIX}\")
endif (NOT KDE4_INSTALL_DIR)

set(KDE4_LIB_INSTALL_DIR     \"${KDE4_LIB_INSTALL_DIR}\")
set(KDE4_LIBEXEC_INSTALL_DIR \"${KDE4_LIBEXEC_INSTALL_DIR}\")
set(KDE4_INCLUDE_INSTALL_DIR \"${KDE4_INCLUDE_INSTALL_DIR}\")
set(KDE4_BIN_INSTALL_DIR     \"${KDE4_BIN_INSTALL_DIR}\")
set(KDE4_SBIN_INSTALL_DIR    \"${KDE4_SBIN_INSTALL_DIR}\")
set(KDE4_DATA_INSTALL_DIR    \"${KDE4_DATA_INSTALL_DIR}\")
set(KDE4_HTML_INSTALL_DIR    \"${KDE4_HTML_INSTALL_DIR}\")
set(KDE4_CONFIG_INSTALL_DIR  \"${KDE4_CONFIG_INSTALL_DIR}\")
set(KDE4_ICON_INSTALL_DIR    \"${KDE4_ICON_INSTALL_DIR}\")
set(KDE4_KCFG_INSTALL_DIR    \"${KDE4_KCFG_INSTALL_DIR}\")
set(KDE4_LOCALE_INSTALL_DIR  \"${KDE4_LOCALE_INSTALL_DIR}\")
set(KDE4_MIME_INSTALL_DIR    \"${KDE4_MIME_INSTALL_DIR}\")
set(KDE4_SOUND_INSTALL_DIR   \"${KDE4_SOUND_INSTALL_DIR}\")
set(KDE4_TEMPLATES_INSTALL_DIR \"${KDE4_TEMPLATES_INSTALL_DIR}\")
set(KDE4_WALLPAPER_INSTALL_DIR \"${KDE4_WALLPAPER_INSTALL_DIR}\")
set(KDE4_KCONF_UPDATE_INSTALL_DIR \"${KDE4_KCONF_UPDATE_INSTALL_DIR}\")
set(KDE4_AUTOSTART_INSTALL_DIR \"${KDE4_AUTOSTART_INSTALL_DIR}\")
set(KDE4_XDG_APPS_INSTALL_DIR        \"${KDE4_XDG_APPS_INSTALL_DIR}\")
set(KDE4_XDG_DIRECTORY_INSTALL_DIR   \"${KDE4_XDG_DIRECTORY_INSTALL_DIR}\")
set(KDE4_SYSCONF_INSTALL_DIR \"${KDE4_SYSCONF_INSTALL_DIR}\")
set(KDE4_MAN_INSTALL_DIR     \"${KDE4_MAN_INSTALL_DIR}\")
set(KDE4_INFO_INSTALL_DIR    \"${KDE4_INFO_INSTALL_DIR}\")
set(KDE4_DBUS_INTERFACES_DIR \"${KDE4_DBUS_INTERFACES_DIR}\")
set(KDE4_DBUS_SERVICES_DIR   \"${KDE4_DBUS_SERVICES_DIR}\")
set(KDE4_SERVICES_INSTALL_DIR \"${KDE4_SERVICES_INSTALL_DIR}\")
set(KDE4_SERVICETYPES_INSTALL_DIR \"${KDE4_SERVICETYPES_INSTALL_DIR}\")

\n")


# write the library dependencies

if (WIN32)
   file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake"
"if (NOT QT_INSTALL_DIR)
   set (QT_INSTALL_DIR \"${QT_INSTALL_DIR}\")
endif (NOT QT_INSTALL_DIR)
")
   # replace hard codes paths by variables 
   # !!! the following stuff does not work because export_library_dependencies
   # !!! does not wait until the file is written completly 
#        export_library_dependencies(${CMAKE_BINARY_DIR}/tmpdeps.cmake)
#        file(READ ${CMAKE_BINARY_DIR}/tmpdeps.cmake _deps)
#        string(REPLACE "${QT_INSTALL_DIR}" "\${QT_INSTALL_DIR}" __deps "${_deps}")
#        file (APPEND ${CMAKE_BINARY_DIR}/KDELibsDependencies.cmake "${__deps}")
endif(WIN32)

export_library_dependencies(${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake APPEND)

########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
# the following will be the correct locations once cmake has the improved FIND_PACKAGE() 
# install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake DESTINATION ${PLUGIN_INSTALL_DIR}/cmake RENAME KDE4Config.cmake)


macro_display_feature_log()
