#	This file is part of the KDE libraries
#	  (C) 1999 Stephan Kulow (coolo@kde.org)

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

INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libqk.la

include_HEADERS = qaction.h qdom.h qxml.h qiconview.h 
libqk_la_SOURCES = qaction.cpp qdom.cpp qxml.cpp moc_qiconview.cpp 

libqk_la_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) 
libqk_la_LIBADD = $(LIB_QT)

METASOURCES = AUTO

moc_qiconview.cpp: $(srcdir)/qiconview.cpp
	$(MOC) -f$(srcdir)/qiconview.cpp $(srcdir)/qiconview.cpp -o moc_qiconview.cpp

CLEANFILES = moc_qiconview.cpp

