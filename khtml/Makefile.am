#    This file is part of the KDE libraries
#    Copyright (C) 1997 Martin Jones (mjones@kde.org)
#              (C) 1997 Torben Weis (weis@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL)

if DOJAVA
JAVA_SUBDIR=java
LIBKJAVA=./java/libkjava.la
endif

SUBDIRS = misc dom css xml html rendering pics $(JAVA_SUBDIR) . ecma kmultipart

lib_LTLIBRARIES = libkhtml.la
kde_module_LTLIBRARIES = khtmlimagepart.la

libkhtml_la_SOURCES = khtmlview.cpp khtml_part.cpp khtml_run.cpp khtml_factory.cpp khtml_settings.cc \
		      khtml_events.cpp khtml_ext.cpp khtml_pagecache.cpp khtml_iface.skel khtml_iface.cc \
		      khtml_printsettings.cpp htmlpageinfo.ui

libkhtml_la_METASOURCES = AUTO

include_HEADERS = khtmlview.h khtml_part.h khtml_events.h khtml_settings.h khtmldefaults.h

noinst_HEADERS = design.h testkhtml.h testrender.h test_regression.h khtml_ext.h \
		 khtml_pagecache.h khtmlimage.h khtmlpart_p.h khtml_printsettings.h

libkhtml_la_LDFLAGS = -version-info 5:0:1 -module -no-undefined $(all_libraries)
libkhtml_la_LIBADD = ./xml/libkhtmlxml.la ./html/libkhtmlhtml.la \
	./rendering/libkhtmlrender.la ./css/libkhtmlcss.la \
	./misc/libkhtmlmisc.la ./dom/libkhtmldom.la $(LIBKJAVA) \
	$(LIB_KIO) \
	$(LIB_KPARTS) \
	$(top_builddir)/kdeprint/libkdeprint.la  \
	$(top_builddir)/kutils/libkutils.la 

khtmlimagepart_la_SOURCES = khtmlimage.cpp
khtmlimagepart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
khtmlimagepart_la_LIBADD = libkhtml.la

INCLUDES = -I$(top_srcdir)/kjs -I$(top_srcdir)/kimgio \
	-I$(srcdir)/java -I$(top_srcdir)/dcop -I$(srcdir)/misc \
	-I$(srcdir)/dom -I$(srcdir)/xml -I$(srcdir)/html -I$(srcdir)/css \
	-I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/kio/kssl -I$(top_builddir)/kio/kssl \
	-I$(top_srcdir)/kdeprint \
	-I$(top_srcdir)/interfaces \
	-I$(top_srcdir)/interfaces/kregexpeditor \
	-I$(top_srcdir) $(all_includes)

servicedir = $(kde_servicesdir)
service_DATA = khtml.desktop khtmlimage.desktop

rcdir = $(kde_datadir)/khtml
rc_DATA = khtml.rc khtml_browser.rc khtml_popupmenu.rc

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kdecore \
	         $(include_HEADERS) -lqt

EXTRA_DIST =  CHANGES DESIGN README.HTMLWidget README.tags 

parser:
	cd $(srcdir) && \
	perl scripts/makeattrs && \
	bash scripts/maketags && \
	bash scripts/makeprop

## test program
check_PROGRAMS 	= testkhtml
EXTRA_PROGRAMS = testcss testrender testkhtml_static test_regression
testkhtml_SOURCES = testkhtml.cpp domtreeview.cpp
testkhtml_LDADD = libkhtml.la 
testkhtml_static_SOURCES = testkhtml.cpp domtreeview.cpp 
testkhtml_static_LDADD = libkhtml.la
testkhtml_static_LDFLAGS = -static
testcss_SOURCES = testcss.cpp
testcss_LDADD = libkhtml.la
testrender_SOURCES = testrender.cpp
testrender_LDADD = libkhtml.la
test_regression_SOURCES = test_regression.cpp
test_regression_LDADD = libkhtml.la ecma/kjs_html.la $(top_builddir)/kjs/libkjs.la

DOXYGEN_REFERENCES = kdecore kdeui kio kdefx kparts
DOXYGEN_EXCLUDE =  test*.*
include ../admin/Doxyfile.am
