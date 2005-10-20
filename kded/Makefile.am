#    This file is part of the KDE libraries
#    Copyright (C) 1997 David Faure <faure@kde.org>
#    Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

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

INCLUDES= -I$(srcdir)/.. -I$(top_srcdir) $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT

lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kded.la kbuildsycoca.la

kded_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_la_LIBADD = $(LIB_KIO)
kded_la_SOURCES = kded.cpp kdedmodule.cpp kdedmodule.stub kdedmodule.skel

kbuildsycoca_la_LDFLAGS = $(all_libraries) -module -avoid-version
kbuildsycoca_la_LIBADD = $(LIB_KIO)
kbuildsycoca_la_SOURCES = kbuildsycoca.cpp kbuildservicetypefactory.cpp \
				kbuildservicefactory.cpp \
				kbuildservicegroupfactory.cpp \
				kbuildimageiofactory.cpp \
				kbuildprotocolinfofactory.cpp \
				kctimefactory.cpp \
				vfolder_menu.cpp

bin_PROGRAMS = kdontchangethehostname kde-menu

kdontchangethehostname_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdontchangethehostname_LDADD = $(LIB_KDECORE) 
kdontchangethehostname_SOURCES = khostname.cpp

kde_menu_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kde_menu_LDADD = $(LIB_KIO) 
kde_menu_SOURCES = kde-menu.cpp

METASOURCES = AUTO

include_HEADERS = kdedmodule.h kdedmodule_stub.h

noinst_HEADERS = kbuildsycoca.h kbuildservicetypefactory.h \
		kbuildservicefactory.h kbuildservicegroupfactory.h \
		kbuildimageiofactory.h kresourcelist.h \
		kbuildprotocolinfofactory.h

servicetype_DATA = kdedmodule.desktop
servicetypedir = $(kde_servicetypesdir)

xdg_menu_DATA = applications.menu

update_DATA = kded.upd
updatedir = $(kde_datadir)/kconf_update

include $(top_srcdir)/admin/Doxyfile.am

