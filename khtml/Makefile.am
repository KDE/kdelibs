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

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

#AM_CXXFLAGS += -DTABLE_DEBUG
#AM_CXXFLAGS += -DDEBUG_LAYOUT
#AM_CXXFLAGS += -DDEBUG_EVENTS
#AM_CXXFLAGS += -DTOKEN_DEBUG
#AM_CXXFLAGS += -DPARSER_DEBUG
#AM_CXXFLAGS += -DCACHE_DEBUG
#AM_CXXFLAGS += -DDEBUG=1

# for profiling.
# command line for line by line profiling (into seperate files):
# gprof -l -y
# AM_CXXFLAGS += -pg

SUBDIRS = misc dom css xml html java kjs . ecma

lib_LTLIBRARIES = libkhtml.la
libkhtml_la_SOURCES = khtml.cpp       khtmlstyle.cpp  khtmltext.cpp 

libkhtml_la_METASOURCES = AUTO

include_HEADERS = khtml.h 

noinst_HEADERS = \
	design.h       khtmlstyle.h   khtmltext.h

libkhtml_la_LDFLAGS = -version-info 3:0
libkhtml_la_LIBADD = ./misc/libkhtmlmisc.la ./dom/libkhtmldom.la ./css/libkhtmlcss.la ./xml/libkhtmlxml.la ./html/libkhtmlhtml.la ./java/libkjava.la ../kimgio/libkimgio.la ../kio/libkio.la

INCLUDES = -I$(srcdir)/kjs -I$(top_srcdir)/kimgio -I$(top_srcdir)/kio \
	-I$(srcdir)/java -I$(top_srcdir)/dcop  -I$(srcdir)/misc \
	-I$(srcdir)/dom -I$(srcdir)/xml -I$(srcdir)/html -I$(srcdir)/css \
        -I$(top_srcdir)/libltdl $(all_includes)

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kdecore \
	         $(include_HEADERS) -lqt

khtmltags_DIST = khtmltags.gperf khtmltags.in khtmltags.h khtmltags.c
kcssprop_DIST = kcssprop.c kcssprop.h kcssprop.in

EXTRA_DIST = $(khtmltags_DIST) makeprop maketags test \
	CHANGES DESIGN README.HTMLWidget README.tags $(kcssprop_DIST)

parser:
	cd $(srcdir) && \
	perl scripts/makeattrs && \
	bash scripts/maketags && \
	bash scripts/makeprop

## test program
check_PROGRAMS 	= testkhtml testcss
#testkhtml_SOURCES = testkhtml.cpp $(libkhtml_la_SOURCES)
#testkhtml_LDADD = -ldmalloc $(LIB_KDEUI) $(libkhtml_la_LIBADD)
testkhtml_SOURCES = testkhtml.cpp
testkhtml_LDADD = $(LIB_KHTML) $(LIB_QT)
testkhtml_LDADD = $(LIB_KHTML)
testcss_SOURCES = testcss.cpp
testcss_LDADD = $(LIB_KDEUI) $(LIB_KHTML) $(libkhtml_la_LIBADD)

