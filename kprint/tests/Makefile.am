#	This file is part of the KDE libraries
#    Copyright (C) 2000 Waldo Bastian (bastian@kde.org)
#              (C) 1997-1998 Stephan Kulow (coolo@kde.org)

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

INCLUDES = -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui \
		-I$(top_srcdir)/kprint $(all_includes)
LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
LDADD = ../libkprint.la

METASOURCES = AUTO

check_PROGRAMS = kprinttest
noinst_HEADERS = 

kprinttest_SOURCES = kprinttest.cpp
