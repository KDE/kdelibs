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

lib_LTLIBRARIES = libqk.la

include_HEADERS = qaction.h qdom.h qvariant.h qxml.h
libqk_la_SOURCES = qaction.cpp qdom.cpp qvariant.cpp qxml.cpp qaction.moc.cpp

libqk_la_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 1:0 -no-undefined
libqk_la_LIBADD = $(LIB_QT)

qaction.moc.cpp: $(srcdir)/qaction.h
	$(MOC) $(srcdir)/qaction.h -o qaction.moc.cpp
qaction.lo : qaction.moc.cpp qaction.cpp

