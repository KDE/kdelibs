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

INCLUDES = -I$(top_srcdir)/kdecore $(QT_INCLUDES) $(X_INCLUDES) 
DISTCLEANFILES = $(libkfm_la_METASOURCES)

libkfm_la_METASOURCES = kfm.moc kfmclient_ipc.moc

lib_LTLIBRARIES = libkfm.la
libkfm_la_LDFLAGS = -version-info 2:0

libkfm_la_SOURCES = kfm.cpp kfmclient_ipc.cpp kfmclient_ipc2.cpp kfmipc.cpp
include_HEADERS = kfm.h kfmclient_ipc.h kfmipc.h
