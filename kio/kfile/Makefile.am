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


INCLUDES= -I$(srcdir)/../libltdl/ -I$(top_srcdir) -I$(top_srcdir)/kdefx \
    -I$(top_builddir)/kio/kio -I$(top_srcdir)/kio/bookmarks $(all_includes)

noinst_LTLIBRARIES  = libkfile.la

METASOURCES = AUTO

#SUBDIRS = . acl_prop_page

include_HEADERS = kfiledialog.h \
	kdiroperator.h kfileview.h kfilefiltercombo.h \
	kfiledetailview.h kcombiview.h kdiskfreesp.h \
	kfileiconview.h krecentdocument.h kfileview.h \
	kfiledialog.h kurlrequester.h kfilepreview.h kfile.h \
	kurlcombobox.h kurlrequesterdlg.h kopenwith.h kpropsdlg.h \
	kicondialog.h kdirsize.h kpreviewwidgetbase.h kimagefilepreview.h kfilesharedlg.h \
	kfiletreeview.h kfiletreeviewitem.h kfiletreebranch.h \
	kdirselectdialog.h kurlbar.h kpropertiesdialog.h knotifydialog.h \
	kcustommenueditor.h knotifywidgetbase.h knotifydialog.h

noinst_HEADERS = config-kfile.h krecentdirs.h kmetaprops.h \
	kfilebookmarkhandler.h kfilemetainfowidget.h kopenwith_p.h

libkfile_la_SOURCES = \
	kfilefiltercombo.cpp \
	kfileview.cpp kfileiconview.cpp  \
	krecentdocument.cpp kfiledialog.cpp kdiroperator.cpp \
	kfiledetailview.cpp kcombiview.cpp kurlrequester.cpp \
	kfilepreview.cpp kurlcombobox.cpp kurlrequesterdlg.cpp \
	kopenwith.cpp kpropertiesdialog.cpp kicondialog.cpp kdirsize.cpp \
	krecentdirs.cpp kdiskfreesp.cpp kimagefilepreview.cpp kfilesharedlg.cpp \
	kurlbar.cpp kmetaprops.cpp\
	kfiletreeview.cpp kfiletreeviewitem.cpp kfiletreebranch.cpp \
	kdirselectdialog.cpp kfilebookmarkhandler.cpp \
	kfilemetainfowidget.cpp kcustommenueditor.cpp knotifywidgetbase.cpp \
	knotifydialog.cpp

EXTRA_DIST = NOTES

# convenience lib - no _LDFLAGS or _LIBADD !

servicetype_DATA = kpropsdlgplugin.desktop
servicetypedir = $(kde_servicetypesdir)


#Can't be a module, we need to link to it for readConfig
#kde_module_LTLIBRARIES = libkfileshare.la
#libkfileshare_la_SOURCES = kfileshare.cpp
#libkfileshare_la_LIBADD = libkfile.la
#libkfileshare_la_LDFLAGS = -module $(KDE_PLUGIN)
#kde_services_DATA = kfilesharepropsplugin.desktop

kpropertiesdialog.lo: ../kio/kdirnotify_stub.h
kfiletreeview.lo: ../kio/kdirnotify_stub.h

