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

SUBDIRS = java kjs

lib_LTLIBRARIES = libkhtml.la
libkhtml_la_SOURCES = dtd.cpp\
	khtmltoken.cpp khtmltokenizer.cpp khtmlparser.cpp\
	khtmlsavedpage.cpp khtmldecoder.cpp khtml.cpp khtmlio.cpp\
	khtmldata.cpp khtmltext.cpp \
	khtmlstyle.cpp khtmlfont.cpp \
	dom_doc.cpp dom_docimpl.cpp dom_element.cpp dom_elementimpl.cpp \
	dom_misc.cpp dom_node.cpp dom_nodeimpl.cpp \
	dom_string.cpp dom_stringimpl.cpp dom_text.cpp dom_textimpl.cpp \
	dom_xml.cpp \
	html_base.cpp html_baseimpl.cpp html_block.cpp html_blockimpl.cpp \
	html_document.cpp html_documentimpl.cpp html_element.cpp \
	html_elementimpl.cpp html_form.cpp html_formimpl.cpp \
	html_head.cpp html_headimpl.cpp html_image.cpp \
	html_imageimpl.cpp html_inline.cpp html_inlineimpl.cpp html_list.cpp \
	html_listimpl.cpp html_misc.cpp html_miscimpl.cpp html_table.cpp \
	html_tableimpl.cpp html_object.cpp html_objectimpl.cpp

libkhtml_la_METASOURCES = AUTO

include_HEADERS = khtml.h khtmlsavedpage.h khtmlio.h \
	dom_core.h dom_doc.h dom_element.h dom_exception.h \
	dom_misc.h dom_node.h dom_string.h dom_text.h \
	dom_xml.h \
	html_base.h html_block.h html_document.h html_element.h \
	html_form.h html_head.h html_image.h \
	html_inline.h html_list.h html_misc.h html_table.h html_object.h

noinst_HEADERS = khtmlparser.h khtmlfont.h khtmltoken.h \
	khtmldata.h khtmlstyle.h khtmldecoder.cpp khtmlstring.h \
	dom_docimpl.h dom_elementimpl.h dom_nodeimpl.h dom_stringimpl.h \
	dom_textimpl.h html_baseimpl.h \
	html_blockimpl.h html_documentimpl.h html_elementimpl.h \
	html_formimpl.h html_headimpl.h html_imageimpl.h \
	html_inlineimpl.h html_listimpl.h html_miscimpl.h html_tableimpl.h \
	html_objectimpl.h

libkhtml_la_LDFLAGS = -version-info 3:0
libkhtml_la_LIBADD = ./java/libkjava.la ./kjs/libkjs.la ../kimgio/libkimgio.la ../kio/libkio.la

data_DATA = khtml_dnd.png
datadir = $(kde_datadir)/khtml/pics

INCLUDES = -I$(top_srcdir)/khtml/kjs -I$(top_srcdir)/kimgio -I$(top_srcdir)/kio \
	-I$(top_srcdir)/khtml/java $(all_includes) -I$(top_srcdir)/dcop

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kdecore \
	         $(include_HEADERS) -lqt

khtmltags_DIST = khtmltags.gperf khtmltags.in khtmltags.h khtmltags.c
kcssprop_DIST = kcssprop.c kcssprop.h kcssprop.in

EXTRA_DIST = $(khtmltags_DIST) makeprop maketags test $(data_DATA) \
	CHANGES DESIGN README.HTMLWidget README.tags $(kcssprop_DIST)

parser:
	cd $(srcdir) && \
	perl scripts/makeattrs && \
	bash scripts/maketags && \
	bash scripts/makeprop

## test program
bin_PROGRAMS 	= testkhtml
testkhtml_SOURCES = testkhtml.cpp $(libkhtml_la_SOURCES)
testkhtml_LDADD = $(LIB_KDEUI) $(libkhtml_la_LIBADD)

