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

#AM_CXXFLAGS += -DTABLE_DEBUG

# for profiling. 
# command line for line by line profiling (into seperate files):
# gprof -l -y
# AM_CXXFLAGS += -pg

lib_LTLIBRARIES = libkhtml.la
libkhtml_la_SOURCES = khtmlcache.cpp khtmlparser.cpp khtmljscript.cpp \
	khtml.cpp khtmlobj.cpp khtmlfont.cpp khtmlform.cpp khtmltoken.cpp \
	khtmlframe.cpp khtmlview.cpp khtmlclue.cpp debug.cpp \
	khtmldata.cpp khtmltable.cpp khtmliter.cpp \
	khtmlsavedpage.cpp khtmlstyle.cpp khtmlembed.cpp khtmldecoder.cpp \
	khtmlstring.cpp

libkhtml_la_METASOURCES = khtml.moc khtmlcache.moc khtmlembed.moc \
	khtmlform.moc khtmlframe.moc khtmlview.moc

include_HEADERS = khtml.h khtmlview.h khtmlsavedpage.h khtmlembed.h khtmlcache.h 
noinst_HEADERS = khtmlchain.h khtmlparser.h khtmljscript.h \
	khtmlobj.h khtmlfont.h khtmlform.h khtmltoken.h khtmlframe.h \
	khtmlclue.h khtmldata.h khtmltable.h khtmliter.h khtmlstyle.h \
	khtmldecoder.cpp khtmlstring.h

libkhtml_la_LDFLAGS = -version-info 3:0
libkhtml_la_LIBADD = ../jscript/libjscript.la ../kimgio/libkimgio.la

data_DATA = khtml_dnd.xpm
datadir = $(kde_datadir)/khtml/pics

INCLUDES = -I$(top_srcdir)/jscript -I$(top_srcdir)/kimgio $(all_includes)

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
	perl makeattrs && \
	bash maketags && \
	bash makeprop 

