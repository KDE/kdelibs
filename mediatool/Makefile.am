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
INCLUDES = $(QT_INCLUDES) $(X_INCLUDES) -I../kdecore
libmediatool_la_SOURCES = chunk.c eventcounter.c tools.c \
			connect.c filename.c playlist.c kaudio.cpp

include_HEADERS= 	mediatool.h kaudio.h

libmediatool_la_LDFLAGS = -version-info 0:91 

kaudio.cpp: kaudio.moc

kaudio.moc: kaudio.h
	$(MOC) -o kaudio.moc kaudio.h
