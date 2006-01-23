
find_package(KDE4 REQUIRED)

find_package(Perl REQUIRED)

message(STATUS "********+ kde libs: ${QT_AND_KDECORE_LIBS} ")
message(STATUS "********+ thread libs: ${CMAKE_THREAD_LIBS_INIT} ")

kde4_header()

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DHAVE_CONFIG_H=1)

link_directories(${KDE4_LIB_DIR})

include(ConvenienceLibs.cmake)

include(ManualStuff.cmake)

include(ConfigureChecks.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kdemacros.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TOPSUBDIRS)")
include_directories( ${KDE4_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

add_subdirectory( dcop )
add_subdirectory( libltdl )
add_subdirectory( kdefx )
add_subdirectory( kdecore )
add_subdirectory( kdeui )
add_subdirectory( kdesu )
#add_subdirectory( kjs )
add_subdirectory( kwallet )
add_subdirectory( kio )
add_subdirectory( kded       )
add_subdirectory( kxmlcore     )
add_subdirectory( kde3support  )
add_subdirectory( dnssd        )
add_subdirectory( kconf_update )
add_subdirectory( kdoctools  )
#add_subdirectory( kimgio    )
add_subdirectory( kioslave   )
add_subdirectory( knewstuff  )
add_subdirectory( kparts     )
add_subdirectory( kresources )
add_subdirectory( kstyles    )
add_subdirectory( kutils     )
add_subdirectory( licenses   )
add_subdirectory( mimetypes  )
add_subdirectory( pics )
add_subdirectory( doc  )
add_subdirectory( kcert  )
add_subdirectory( kinit  )
add_subdirectory( kdeprint  )
add_subdirectory( kabc  )
add_subdirectory( kspell2  )
add_subdirectory( kcmshell  )
message(STATUS "skipping khtml, at least gcc 3.3 required")
#add_subdirectory( khtml  )
add_subdirectory( interfaces  )
add_subdirectory( kdewidgets  )
#add_subdirectory( kate  )
add_subdirectory( knotify )




########### install files ###############

install_files(  FILES kde.pot )

kde4_create_doxygen_docs(  )

kde4_footer()



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
#SUBDIRS=$(TOPSUBDIRS)
