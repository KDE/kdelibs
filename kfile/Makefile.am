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


INCLUDES= -I$(top_srcdir) -I$(srcdir)/../libltdl/ -I$(top_srcdir)/kio -I$(top_builddir)/kio $(all_includes)

lib_LTLIBRARIES  = libkfile.la

METASOURCES = AUTO

SUBDIRS = . acl_prop_page

include_HEADERS = kfiledialog.h kfileviewitem.h kfilebookmark.h  \
	kfilereader.h kdiroperator.h kfileview.h \
	booktoken.h kfiledetailview.h kcombiview.h \
	kfileiconview.h kfilefilter.h krecentdocument.h kfileview.h \
	kfiledialog.h kurlrequester.h kfilepreview.h kfile.h \
	kurlcombobox.h kurlrequesterdlg.h kopenwith.h kpropsdlg.h \
	kicondialog.h kdirsize.h kpreviewwidgetbase.h kimagefilepreview.h

noinst_HEADERS = kfiledialogconf.h kfilefilter.h config-kfile.h krecentdirs.h 

libkfile_la_SOURCES = kfilebookmark.cpp kfiledialogconf.cpp \
	kfilereader.cpp kfileviewitem.cpp booktoken.cpp \
	kfileview.cpp kfileiconview.cpp kfilefilter.cpp \
	krecentdocument.cpp kfiledialog.cpp kdiroperator.cpp \
	kfiledetailview.cpp kcombiview.cpp kurlrequester.cpp \
	kfilepreview.cpp kurlcombobox.cpp kurlrequesterdlg.cpp \
	kopenwith.cpp kpropsdlg.cpp kicondialog.cpp kdirsize.cpp \
	krecentdirs.cpp kdiskfreesp.cpp kimagefilepreview.cpp
#	kdirselect.cpp kdirselectdialog.cpp

EXTRA_DIST = NOTES

libkfile_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined -version-info 4:0
libkfile_la_LIBADD =  $(LIB_KSYCOCA)

servicetype_DATA = kpropsdlgplugin.desktop
servicetypedir = $(kde_servicetypesdir)

check_PROGRAMS = kfstest kurlrequestertest

kfstest_SOURCES =  kfstest.cpp
kurlrequestertest_SOURCES =  kurlrequestertest.cpp

kfstest_LDFLAGS = $(KDE_RPATH) $(KDE_MT_LDFLAGS)
kfstest_LDADD = libkfile.la

kurlrequestertest_LDFLAGS = $(KDE_RPATH) $(KDE_MT_LDFLAGS)
kurlrequestertest_LDADD = libkfile.la
