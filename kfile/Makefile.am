#	This file is part of the KDE libraries
#    Copyright (C) 1997 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this library; see the file COPYING.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.


AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST

INCLUDES= -I$(top_srcdir)/kio $(all_includes)

lib_LTLIBRARIES  = libkfile.la

METASOURCES = AUTO

include_HEADERS = kfiledialog.h kfileviewitem.h kfilebookmark.h  \
	kfilereader.h kdirlistbox.h kdiroperator.h kfileview.h \
	booktoken.h kfiledetailview.h kcombiview.h \
	kfileiconview.h kfilefilter.h krecentdocument.h kfileview.h \
	kfiledialog.h kfilewidget.h kurledit.h kfilepreview.h

noinst_HEADERS = kfiledialogconf.h kfilefilter.h config-kfile.h
kfstest_SOURCES =  kfstest.cpp
libkfile_la_SOURCES = kfilebookmark.cpp kfiledialogconf.cpp \
	kfilereader.cpp kfileviewitem.cpp booktoken.cpp \
	kfileview.cpp kfileiconview.cpp kfilefilter.cpp \
	krecentdocument.cpp kfiledialog.cpp kdiroperator.cpp \
	kfiledetailview.cpp kcombiview.cpp kfilewidget.cpp kurledit.cpp \
    kfilepreview.cpp

kde_toolbar_DATA = configure.png

EXTRA_DIST = configure.png KFile.kdoc NOTES

libkfile_la_LDFLAGS = -no-undefined -version-info 3:0 
libkfile_la_LIBADD =  $(top_builddir)/kio/libkio.la 

check_PROGRAMS = kfstest
kfstest_LDFLAGS = $(KDE_RPATH)
kfstest_LDADD = libkfile.la

