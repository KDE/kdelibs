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


INCLUDES= -I$(top_srcdir) -I$(srcdir)/../libltdl/ -I$(top_srcdir)/kio -I$(top_srcdir)/kdefx -I$(top_builddir)/kio $(all_includes)

lib_LTLIBRARIES  = libkfile.la

METASOURCES = AUTO

SUBDIRS = . acl_prop_page

include_HEADERS = kfiledialog.h kfilebookmark.h  \
	kdiroperator.h kfileview.h kfilefiltercombo.h \
	kfiledetailview.h kcombiview.h \
	kfileiconview.h krecentdocument.h kfileview.h \
	kfiledialog.h kurlrequester.h kfilepreview.h kfile.h \
	kurlcombobox.h kurlrequesterdlg.h kopenwith.h kpropsdlg.h \
	kicondialog.h kdirsize.h kpreviewwidgetbase.h kimagefilepreview.h \
	kfiletreeview.h kfiletreeviewitem.h kfiletreebranch.h \
	kdirselectdialog.h kurlbar.h

noinst_HEADERS = kfiledialogconf.h config-kfile.h krecentdirs.h 

libkfile_la_SOURCES = kfilebookmark.cpp kfiledialogconf.cpp \
	booktoken.cpp kfilefiltercombo.cpp \
	kfileview.cpp kfileiconview.cpp  \
	krecentdocument.cpp kfiledialog.cpp kdiroperator.cpp \
	kfiledetailview.cpp kcombiview.cpp kurlrequester.cpp \
	kfilepreview.cpp kurlcombobox.cpp kurlrequesterdlg.cpp \
	kopenwith.cpp kpropsdlg.cpp kicondialog.cpp kdirsize.cpp \
	krecentdirs.cpp kdiskfreesp.cpp kimagefilepreview.cpp \
	kurlbar.cpp \
	kfiletreeview.cpp kfiletreeviewitem.cpp kfiletreebranch.cpp kdirselectdialog.cpp

EXTRA_DIST = NOTES

libkfile_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined -version-info 4:0
libkfile_la_LIBADD =  $(LIB_KSYCOCA)

servicetype_DATA = kpropsdlgplugin.desktop
servicetypedir = $(kde_servicetypesdir)
