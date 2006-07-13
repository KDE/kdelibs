project(kdelibs)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

find_package(KDE4Internal REQUIRED)
find_package(ZLIB REQUIRED)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

# set some default settings
include(KDE4Defaults)

# make some more macros available
include (MacroLibrary)

# macro_optional_find_package() is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(BZip2)
macro_optional_find_package(OpenSSL)
macro_optional_find_package(Gettext)


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


add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)
link_directories(${KDE4_LIB_DIR})

# for including config.h and for includes like <kparts/foo.h>
include_directories( ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_SOURCE_DIR}/kdefx)

# Those variables for  are only valid inside of kdelibs, of course.
# Use the one variable for the lib you depend upon.
# E.g. kdeui uses ${KDE4_KDECORE_INCLUDES}. Something that depends on kparts uses ${KDE4_KPARTS_INCLUDES}.

# DCOP depends on Qt
set(KDE4_DCOP_INCLUDES ${CMAKE_SOURCE_DIR}/dcop ${QT_INCLUDES} ${_KDE4_PLATFORM_INCLUDE_DIRS})

# kdecore depends on DCOP  (libldtl isn't included since it's internal to kdecore)
set(KDE4_KDECORE_INCLUDES ${CMAKE_SOURCE_DIR}/kdecore ${CMAKE_BINARY_DIR}/kdecore ${CMAKE_SOURCE_DIR}/kdecore/network ${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/kdecore/kservice ${_KDE4_PLATFORM_INCLUDE_DIRS})
# kdeui depends on kdefx+kdecore
set(KDE4_KDEUI_INCLUDES ${CMAKE_SOURCE_DIR}/kdefx ${CMAKE_SOURCE_DIR}/kdeui ${CMAKE_SOURCE_DIR}/kdeui/findreplace ${CMAKE_SOURCE_DIR}/kdeui/widgets ${CMAKE_SOURCE_DIR}/kdeui/itemviews ${KDE4_KDECORE_INCLUDES})
# kio depends on kdeui
set(KDE4_KIO_INCLUDES ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/kio/kio ${CMAKE_SOURCE_DIR}/kio/kfile ${CMAKE_SOURCE_DIR}/kio/bookmarks ${KDE4_KDEUI_INCLUDES})
# kparts depends on kio
set(KDE4_KPARTS_INCLUDES ${CMAKE_SOURCE_DIR}/kparts ${KDE4_KIO_INCLUDES})


# now create config headers

# ACL stuff (used in kio/ and kioslaves/)
macro_optional_find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
configure_file(config-acl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-acl.h )


# create config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(config-compiler.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compiler.h )
configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )



# list the subdirectories

add_subdirectory( cmake )
add_subdirectory( libltdl )
add_subdirectory( kdefx )
add_subdirectory( kdecore )
add_subdirectory( kdeui )
if (UNIX)
   add_subdirectory( kdesu )
endif (UNIX)
add_subdirectory( kjs )
add_subdirectory( kjsembed )
add_subdirectory( kwallet )
add_subdirectory( kio )
add_subdirectory( phonon )
add_subdirectory( kded       )
add_subdirectory( kxmlcore     )
add_subdirectory( kde3support  )
add_subdirectory( kconf_update )
add_subdirectory( kdoctools  )
add_subdirectory( kioslave   )
add_subdirectory( knewstuff  )
add_subdirectory( kparts     )
add_subdirectory( kstyles    )
add_subdirectory( kutils     )
add_subdirectory( licenses   )
add_subdirectory( mimetypes  )
add_subdirectory( pics )
add_subdirectory( doc  )
add_subdirectory( kcert  )
add_subdirectory( kinit  )
add_subdirectory( kdeprint  )
add_subdirectory( sonnet  )
add_subdirectory( kcmshell  )
add_subdirectory( khtml  )
add_subdirectory( interfaces  )
add_subdirectory( kdewidgets  )
add_subdirectory( kate  )
add_subdirectory( knotify )
add_subdirectory( kimgio )
add_subdirectory( dnssd )


########### install files ###############

# write the installdirs into a cmake file which will be installed
file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake 
"# this file was generated during the kdelibs build process
set(KDE4_INSTALL_DIR         \"${CMAKE_INSTALL_PREFIX}\")
set(KDE4_LIB_INSTALL_DIR     \"${LIB_INSTALL_DIR}\")
set(KDE4_INCLUDE_INSTALL_DIR \"${INCLUDE_INSTALL_DIR}\")
set(KDE4_BIN_INSTALL_DIR     \"${BIN_INSTALL_DIR}\")
set(KDE4_DATA_INSTALL_DIR    \"${DATA_INSTALL_DIR}\")
")

# ...and append all library dependencies
export_library_dependencies(${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake APPEND)

install( FILES kdelibs_export.h ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

if(WIN32)
install( FILES kdelibs_export_win.h  DESTINATION  ${INCLUDE_INSTALL_DIR} )
endif(WIN32)



