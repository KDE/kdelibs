#    This file is part of the KDE libraries
#    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
#              (C) 1997-1998 Stephan Kulow (coolo@kde.org)

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

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = kfstest kurlrequestertest kfiletreeviewtest \
                 kopenwithtest kdirselectdialogtest kicondialogtest \
                 knotifytest kcustommenueditortest

# noinst_HEADERS = 

METASOURCES = AUTO

LDADD = $(LIB_KIO)
kcustommenueditortest_SOURCES = kcustommenueditortest.cpp
kurlrequestertest_SOURCES = kurlrequestertest.cpp
kfstest_SOURCES = kfstest.cpp
kfiletreeviewtest_SOURCES = kfiletreeviewtest.cpp
kopenwithtest_SOURCES = kopenwithtest.cpp
kdirselectdialogtest_SOURCES = kdirselectdialogtest.cpp
kicondialogtest_SOURCES = kicondialogtest.cpp
knotifytest_SOURCES = knotifytest.cpp

