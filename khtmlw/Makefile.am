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
libkhtmlw_la_SOURCES = jscript.cpp html.cpp htmlobj.cpp htmlfont.cpp htmlform.cpp htmltoken.cpp htmlframe.cpp htmlview.cpp htmlclue.cpp debug.cpp htmldata.cpp htmltable.cpp htmliter.cpp htmlchain.cpp

libkhtmlw_la_METASOURCES = html.moc htmlform.moc htmlframe.moc \
	htmlobj.moc htmlview.moc

include_HEADERS = jscript.h html.h htmlobj.h htmlfont.h htmlform.h \
	htmltoken.h htmlframe.h htmlview.h htmlclue.h htmldata.h \
	htmltable.h htmliter.h

noinst_HEADERS = htmlchain.h
libkhtmlw_la_LDFLAGS = -version-info 3:0

data_DATA = khtmlw_dnd.xpm
datadir = $(kde_datadir)/khtmlw/pics

EXTRA_DIST = khtmlw_dnd.xpm CHANGES DESIGN README.HTMLWidget test

INCLUDES = -I$(top_srcdir)/jscript -I$(top_srcdir)/kimgio $(all_includes)

