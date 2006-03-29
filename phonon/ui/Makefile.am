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

SUBDIRS = . tests

lib_LTLIBRARIES = libphononui.la

phononincludedir = $(includedir)/phonon/ui
phononinclude_HEADERS = videowidget.h seekslider.h mediacontrols.h \
						volumeslider.h

libphononui_la_SOURCES = factory.cpp videowidget.cpp \
						 seekslider.cpp mediacontrols.cpp volumeslider.cpp
libphononui_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_LDFLAGS) \
			  $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
libphononui_la_LIBADD = $(top_builddir)/phonon/libphononcore.la $(top_builddir)/phonon/ifaces/ui/libphononifacesui.la

METASOURCES = AUTO

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/phonon/ui
DOXYGEN_REFERENCES = dcop kdecore kio phonon
include ../../admin/Doxyfile.am

