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


lib_LTLIBRARIES = libkhtmlw.la
libkhtmlw_la_SOURCES = jscript.cpp html.cpp htmlobj.cpp htmlfont.cpp formats.cpp htmlform.cpp htmltoken.cpp htmlframe.cpp htmlview.cpp htmlclue.cpp debug.cpp htmldata.cpp htmltable.cpp htmliter.cpp htmlchain.cpp

SRC_META = html.moc htmlobj.moc htmlform.moc htmlframe.moc htmlview.moc
html.cpp: html.moc
htmlobj.cpp: htmlobj.moc
htmlform.cpp: htmlform.moc
htmlframe.cpp: htmlframe.moc
htmlview.cpp: htmlview.moc

DISTCLEANFILES = $(SRC_META)
include_HEADERS = jscript.h html.h htmlobj.h htmlfont.h htmlform.h htmltoken.h htmlframe.h htmlview.h htmlclue.h htmldata.h htmltable.h htmliter.h
noinst_HEADERS = htmlchain.h formats.h
libkhtmlw_la_LDFLAGS = -version-info 2:0

data_DATA = khtmlw_dnd.xpm
datadir = $(kde_datadir)/khtmlw/pics

EXTRA_DIST = khtmlw_dnd.xpm

INCLUDES = -I../kdecore -I../kdeui -I../jscript -I../kimgio $(all_includes)

html.moc: html.h
	$(MOC) -o html.moc html.h 

htmlobj.moc: htmlobj.h
	$(MOC) -o htmlobj.moc htmlobj.h 

htmlform.moc: htmlform.h
	$(MOC) -o htmlform.moc htmlform.h 

htmlframe.moc: htmlframe.h
	$(MOC) -o htmlframe.moc htmlframe.h 

htmlview.moc: htmlview.h
	$(MOC) -o htmlview.moc htmlview.h 

