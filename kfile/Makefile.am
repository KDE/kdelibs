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


INCLUDES= -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui -I$(top_srcdir)/kfmlib $(QT_INCLUDES) $(X_INCLUDES)

lib_LTLIBRARIES  = libkfile.la

DISTCLEANFILES = $(libkfile_METASOURCES) 

libkfile_METASOURCES = USE_AUTOMOC

include_HEADERS = kfiledialog.h kfileinfo.h kfilebookmark.h  \
	kremotefile.h kdir.h kdirlistbox.h kfileinfocontents.h \
	booktoken.h kfiledetaillist.h kcombiview.h kpreview.h kfilepreview.h \
	xview.h kfilesimpleview.h kfilefilter.h

noinst_HEADERS = kfiledialogconf.h kfilefilter.h config-kfile.h

kfstest_SOURCES =  kfstest.cpp
libkfile_la_SOURCES = kfilebookmark.cpp kfiledialogconf.cpp kcombiview.cpp \
	kdir.cpp kfileinfo.cpp kremotefile.cpp kdirlistbox.cpp \
	kfiledetaillist.cpp kfiledialog.cpp booktoken.cpp \
	kfileinfocontents.cpp kfilesimpleview.cpp kpreview.cpp \
	kfilepreview.cpp xview.cpp kfilefilter.cpp

toolbar_DATA = configure.xpm 

EXTRA_DIST = configure.xpm

# I'm not sure, if they really should go there
toolbardir = $(kde_toolbardir)

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS)
libkfile_la_LDFLAGS = -version-info 2:0 $(LDFLAGS)
libkfile_la_LIBADD =  -lXext  $(LIB_QT)

check_PROGRAMS = kfstest
kfstest_LDFLAGS = $(LDFLAGS) $(KDE_RPATH)
kfstest_LDADD = libkfile.la ../kfmlib/libkfm.la ../kdeui/libkdeui.la ../kdecore/libkdecore.la  -lXext  $(LIB_QT)
