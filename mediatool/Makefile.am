#    This file is part of the KDE libraries
#    Copyright (C) 1997 Stephan Kulow (coolo@kde.org)

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


lib_LTLIBRARIES = libmediatool.la
#
# Programs to compile, Manpages to install and Versions
#
MEDIATOOLVERSION=	0.0
MEDIATOOLPATCH	 =	91
INCLUDES = $(all_includes) 
libmediatool_la_SOURCES = chunk.c eventcounter.c \
			connect.c filename.c playlist.c kaudio.cpp

include_HEADERS= 	mediatool.h kaudio.h
noinst_HEADERS = chunk.h playlist.h

libmediatool_la_LDFLAGS = -version-info 3:0 $(all_libraries)
libmediatool_la_LIBADD = $(LIB_QT) $(LIBCOMPAT)

libmediatool_la_METASOURCES = kaudio.moc

EXTRA_DIST = Documentation Header VERSION mediatool.lsm 
