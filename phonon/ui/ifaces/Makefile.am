#    This file is part of the KDE libraries
#    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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

lib_LTLIBRARIES = libphononifacesui.la

phononincludedir = $(includedir)/phonon/ifaces/ui
phononinclude_HEADERS = videowidget.h backend.h

libphononifacesui_la_SOURCES = backend.cpp
libphononifacesui_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_LDFLAGS) $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
libphononifacesui_la_LIBADD = ../libphononifaces.la $(LIB_KDECORE)

METASOURCES = AUTO

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/phonon/ifaces/ui
DOXYGEN_REFERENCES = dcop kdecore kio phonon phonon/ui phonon/ifaces
include ../../../admin/Doxyfile.am
