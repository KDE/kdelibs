#    This file is part of the KDE libraries
#    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License version 2 as published by the Free Software Foundation.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

INCLUDES = -I$(top_srcdir) $(all_includes)

SUBDIRS = .

lib_LTLIBRARIES = libkdem2mui.la

kdem2mincludedir = $(includedir)/kdem2m/ui
kdem2minclude_HEADERS = #volumefader.h positionslider.h videowidget.h

libkdem2mui_la_SOURCES = factory.cpp #volumefader.cpp positionslider.cpp videowidget.cpp
libkdem2mui_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_LDFLAGS) \
			  $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
libkdem2mui_la_LIBADD = $(top_builddir)/kdem2m/libkdem2mcore.la $(top_builddir)/kdem2m/ifaces/ui/libkdem2mifacesui.la

METASOURCES = AUTO

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdem2m/ui

DOXYGEN_REFERENCES = dcop kdecore kio kdem2m

include ../../admin/Doxyfile.am

