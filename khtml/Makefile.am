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


lib_LTLIBRARIES = libkhtml.la
libkhtml_la_SOURCES = khtmlparser.cpp khtmljscript.cpp khtml.cpp khtmlobj.cpp khtmlfont.cpp khtmlform.cpp khtmltoken.cpp khtmlframe.cpp khtmlview.cpp khtmlclue.cpp debug.cpp khtmldata.cpp khtmltable.cpp khtmliter.cpp khtmlcache.cpp khtmlsavedpage.cpp khtmlstyle.cpp khtmlembed.cpp

libkhtml_METASOURCES = USE_AUTOMOC

DISTCLEANFILES = $(libkhtml_METASOURCES)

include_HEADERS = khtml.h khtmlview.h khtmlsavedpage.h khtmlembed.h khtmlcache.h 
noinst_HEADERS = khtmlchain.h khtmlparser.h khtmljscript.h khtmlobj.h khtmlfont.h khtmlform.h khtmltoken.h khtmlframe.h khtmlclue.h khtmldata.h khtmltable.h khtmliter.h khtmlstyle.h
libkhtml_la_LDFLAGS = -version-info 2:0

data_DATA = khtml_dnd.xpm
datadir = $(kde_datadir)/khtml/pics

EXTRA_DIST = khtml_dnd.xpm

INCLUDES = -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui \
	-I$(top_srcdir)/jscript -I$(top_srcdir)/kimgio \
	$(QT_INCLUDES) $(X_INCLUDES)

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kdecore \
	         $(include_HEADERS) -lqt
