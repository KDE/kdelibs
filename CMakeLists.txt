project(kdelibs)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# search packages used by KDE
if (WIN32)
   find_package(GNUWIN32)
endif (WIN32)

# for visual studio IDE set the path correctly for custom commands
# maybe under windows bat-files should be generated for running apps during the build
if(MSVC_IDE)
  find_program(PERL_PATH perl)
  get_filename_component(PERL_LOCATION "${PERL_PATH}" PATH)
  file(TO_NATIVE_PATH "${PERL_LOCATION}" PERL_PATH_WINDOWS)
  file(TO_NATIVE_PATH "${QT_BINARY_DIR}" QT_BIN_DIR_WINDOWS)
  set(CMAKE_MSVCIDE_RUN_PATH "${PERL_PATH_WINDOWS}\;${QT_BIN_DIR_WINDOWS}"
    CACHE STATIC "MSVC IDE Run path" FORCE)
endif(MSVC_IDE)

find_package(KDE4Internal REQUIRED)
find_package(ZLIB REQUIRED)

if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

# set some default settings
include(KDE4Defaults)

# make some more macros available
include (MacroLibrary)

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


# macro_optional_find_package() is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(BZip2)
macro_optional_find_package(OpenSSL)
macro_optional_find_package(Gettext)

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
set(KDE4_KDECORE_INCLUDES ${CMAKE_SOURCE_DIR}/kdecore
${CMAKE_BINARY_DIR}/kdecore ${CMAKE_SOURCE_DIR}/kdecore/network
${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/kdecore/kservice ${QDBUS_INCLUDE_DIRS} ${_KDE4_PLATFORM_INCLUDE_DIRS})
# kdeui depends on kdefx+kdecore
set(KDE4_KDEUI_INCLUDES ${CMAKE_SOURCE_DIR}/kdefx ${CMAKE_SOURCE_DIR}/kdeui ${CMAKE_SOURCE_DIR}/kdeui/findreplace ${KDE4_KDECORE_INCLUDES})
# kio depends on kdeui
set(KDE4_KIO_INCLUDES ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/kio/kio ${CMAKE_SOURCE_DIR}/kio/kfile ${CMAKE_SOURCE_DIR}/kio/bookmarks ${KDE4_KDEUI_INCLUDES})
# kparts depends on kio
set(KDE4_KPARTS_INCLUDES ${CMAKE_SOURCE_DIR}/kparts ${KDE4_KIO_INCLUDES})

# ACL stuff (used in kio/ and kioslaves/)
macro_optional_find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
configure_file(config-acl.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-acl.h )


# create config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

configure_file(config-prefix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-prefix.h )
configure_file(kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )

add_definitions( ${QDBUS_DEFINITIONS} )

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
add_subdirectory( kspell2  )
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
set(KDE4_LIB_INSTALL_DIR     \"${CMAKE_INSTALL_PREFIX}${LIB_INSTALL_DIR}\")
set(KDE4_INCLUDE_INSTALL_DIR \"${CMAKE_INSTALL_PREFIX}/include\")
set(KDE4_BIN_INSTALL_DIR     \"${CMAKE_INSTALL_PREFIX}/bin\")
set(KDE4_DATA_INSTALL_DIR    \"${CMAKE_INSTALL_PREFIX}${DATA_INSTALL_DIR}\")
")

# ...and append all library dependencies
export_library_dependencies(${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake APPEND)

install_files( /include FILES kdelibs_export.h kdemacros.h )
install_files( /share/apps/cmake/modules FILES ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake)

if(WIN32)
   install_files( /include FILES kdelibs_export_win.h )
endif(WIN32)



#original Makefile.am contents follow:

# #    This file is part of the KDE libraries
##    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
##              (C) 1997,1998 Stephan Kulow (coolo@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#
#COMPILE_FIRST = dcop libltdl kdefx kdecore kdeui kdesu kjs kwallet kio
## COMPILE_BEFORE_doc = kdoctools
## COMPILE_AFTER_kparts = kspell2 kde3support kdeprint kinit kate interfaces kcert khtml
## COMPILE_AFTER_kdeprint = kate khtml
## COMPILE_BEFORE_khtml = kutils
## COMPILE_BEFORE_kabc = kresources
## COMPILE_BEFORE_kate = interfaces kutils
## COMPILE_BEFORE_kde3support = kutils
## COMPILE_BEFORE_kspell2 = kutils
## COMPILE_BEFORE_kcmshell = kutils
## COMPILE_BEFORE_kdewidgets = kabc khtml
## COMPILE_BEFORE_interfaces = arts kabc
#
#$(top_srcdir)/acinclude.m4: $(top_srcdir)/libltdl/ltdl.m4
#
#AUTOMAKE_OPTIONS = foreign 1.5
#
#potdir = $(kde_includes)
#pot_DATA = kde.pot
#
#include_HEADERS = kdelibs_export.h kdemacros.h
#
#messages:
#	cd kabc/scripts && perl ./makeaddressee
#	find . -type d | grep -v CVS | grep -v '\.svn' | sed -e 's,$$,/,' > dirs
#	mfs=`find . -name Makefile.am | xargs egrep -l '^messages:'` ;\
#	for dir in $$mfs; do \
#	  if test ! "$$dir" = "./Makefile.am"; then \
#	    dir=`dirname $$dir` ;\
#	    egrep -v "^$$dir" dirs > dirs.new && mv dirs.new dirs ;\
#	  fi ;\
#	done
#	egrep -v "/tests" dirs > dirs.new && mv dirs.new dirs ;\
#	dirs=`cat dirs` ;\
#	find $$dirs -maxdepth 1 -name "*.cpp" -print > files ;\
#	find $$dirs -maxdepth 1 -name "*.cc" -print >> files ;\
#	find $$dirs -maxdepth 1 -name "*.h" -print >> files ;\
#	echo ./kdecore/kde-config.cpp.in >> files ;\
#	$(EXTRACTRC) `find $$dirs -maxdepth 1 -name "*.rc" -o -name "*.ui"` > rc.cpp	;\
#	: > kde.pot ;\
#	$(XGETTEXT) -ktranslate -x qt-messages.pot rc.cpp `cat files` && cat messages.po qt-messages.pot > $(podir)/kdelibs.pot
#	$(XGETTEXT) common_texts.cpp -o kde.pot
#	rm -f rc.cpp dirs
#
#EXTRA_DIST = admin bsd-port debian kdelibs.spec.in kde.pot \
#	COMPILING COMPILING.html COPYING.BSD DEBUG \
#	KDE2PORTING.html NAMING common_texts.cpp kdelibs.lsm kdoc.rules \
#	qt-messages.pot configure.in.in configure.in.mid configure.in.bot
#
#MAINTAINERCLEANFILES = configure.files subdirs
#
#dist-hook:
#	cd $(top_distdir) && perl $(top_srcdir)/admin/am_edit -padmin
#
## Rule for generating HTML documentation [you need kdoc installed]
## Don't forget to run qt2kdoc first (qt2kdoc $QTDIR/doc/html)
## The kdelibs docu will end up in ./srcdoc/
#kdedoc:
#	cd $(top_srcdir) && \
#	makekdedoc -p'--skip-internal' --outputdir=$(top_builddir)/srcdoc
#
#qtmessages:
#	if test x$(top_srcdir) = x; then top_srcdir=. ; else top_srcdir=$(top_srcdir); fi ;\
#	cd $$top_srcdir/../qt-copy/src ;\
#	sed -e "s,#define,," xml/qxml.cpp  > qxml_clean.cpp ;\
#	find . -name "*.cpp" | grep -v moc_ > list ;\
#	for file in qfiledialog qcolordialog qprintdialog \
#		qurloperator qftp qhttp qlocal qerrormessage; do \
#		grep -v $$file list > list.new && mv list.new list ;\
#	done ;\
#	xgettext -C -ktr -kQT_TRANSLATE_NOOP `cat list`  ;\
#	cd ../../kdelibs ;\
#	mv $$top_srcdir/../qt-copy/src/messages.po $$top_srcdir
#	echo "remove the header from messages.po and move to qt-messages.pot"
#
#include admin/Doxyfile.am
#include admin/deps.am
#
#apidox-am-toplevel-yes: apidox-am-toplevel-kdelibs
#
#apidox-am-toplevel-kdelibs: $(top_builddir)/apidocs/qt/qt.tag
#
#$(top_builddir)/apidocs/qt/qt.tag:
#	@echo "*** Creating a tag file for the Qt library:"; \
#	$(mkinstalldirs) $(top_builddir)/apidocs/qt; \
#	doxytag -t $(top_builddir)/apidocs/qt/qt.tag $(QTDOCDIR)
#
#install-data-local: install-apidox-kdelibs
#
### install the tag file for the Qt documentation
#install-apidox-kdelibs:
#	if test -f $(top_builddir)/apidocs/qt/qt.tag; then \
#		$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/$(PACKAGE)-apidocs/qt ;\
#		echo $(INSTALL_DATA) $(top_builddir)/apidocs/qt/qt.tag $(DESTDIR)$(kde_htmldir)/en/$(PACKAGE)-apidocs/qt; \
#		$(INSTALL_DATA) $(top_builddir)/apidocs/qt/qt.tag $(DESTDIR)$(kde_htmldir)/en/$(PACKAGE)-apidocs/qt; \
#	fi
#
#.PHONY: install-apidox-kdelibs apidox-am-toplevel-kdelibs apidox-am-toplevel-yes qtmessages
#SUBDI
