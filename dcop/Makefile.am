# This Makefile.am was taken from the kdelibs distribution and 
# modified for DCOP.  Preston Brown (Oct 10, 1999)
#
#    This file is part of the KDE libraries
#    Copyright (C) 1997,1999 Stephan Kulow (coolo@kde.org)

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


INCLUDES=  $(all_includes)
LDADD = $(LIB_QT)
LDFLAGS = $(all_libraries)

bin_PROGRAMS = dcopserver

lib_LTLIBRARIES  = libDCOP.la
libDCOP_la_LIBADD = $(LIB_QT)
libDCOP_la_LDFLAGS = -version-info 1:0 $(all_libraries)

include_HEADERS = dcopclient.h dcopserver.h dcopobject.h dcopglobal.h
noinst_HEADERS = version.h dcopglobal.h

METASOURCES = dcopclient.moc dcopserver.moc

libDCOP_la_SOURCES = dcopclient.cpp dcopobject.cpp

dcopserver_SOURCES = dcopserver.cpp
