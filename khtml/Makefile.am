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
libkhtml_la_SOURCES = khtmljscript.cpp khtml.cpp khtmlobj.cpp khtmlfont.cpp khtmlformats.cpp khtmlform.cpp khtmltoken.cpp khtmlframe.cpp khtmlview.cpp khtmlclue.cpp debug.cpp khtmldata.cpp khtmltable.cpp khtmliter.cpp khtmlcache.cpp

SRC_META = khtml.moc khtmlobj.moc khtmlform.moc khtmlframe.moc khtmlview.moc
khtml.cpp: khtml.moc
khtmlobj.cpp: khtmlobj.moc
khtmlform.cpp: khtmlform.moc
khtmlframe.cpp: khtmlframe.moc
khtmlview.cpp: khtmlview.moc

DISTCLEANFILES = $(SRC_META)
include_HEADERS = khtmljscript.h khtml.h khtmlobj.h khtmlfont.h khtmlform.h khtmltoken.h khtmlframe.h khtmlview.h khtmlclue.h khtmldata.h khtmltable.h khtmliter.h khtmlcache.h
noinst_HEADERS = khtmlchain.h khtmlformats.h
libkhtml_la_LDFLAGS = -version-info 1:0

data_DATA = khtml_dnd.xpm
datadir = $(kde_datadir)/khtml/pics

EXTRA_DIST = khtml_dnd.xpm

INCLUDES = -I../kdecore -I../kdeui -I../jscript -I../kimgio $(all_includes)

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kdecore \
	         $(include_HEADERS) -lqt

khtml.moc: khtml.h
	$(MOC) -o khtml.moc khtml.h 

khtmlobj.moc: khtmlobj.h
	$(MOC) -o khtmlobj.moc khtmlobj.h 

khtmlform.moc: khtmlform.h
	$(MOC) -o khtmlform.moc khtmlform.h 

khtmlframe.moc: khtmlframe.h
	$(MOC) -o khtmlframe.moc khtmlframe.h 

khtmlview.moc: khtmlview.h
	$(MOC) -o khtmlview.moc khtmlview.h 

