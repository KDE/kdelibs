#    This file is part of the KDE libraries
#    Copyright (C) 2001 Waldo Bastian <bastian@kde.org>

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

INCLUDES= $(all_includes)

lib_LTLIBRARIES = kdeinit_kconf_update.la

bin_PROGRAMS = kconf_update
kconf_update_SOURCES = dummy.cpp
kconf_update_LDADD = kdeinit_kconf_update.la
kconf_update_LDFLAGS = $(KDE_RPATH) $(all_libraries)

kdeinit_kconf_update_la_SOURCES = kconf_update.cpp
kdeinit_kconf_update_la_LIBADD = ../kdecore/libkdecore.la
kdeinit_kconf_update_la_LDFLAGS = $(all_libraries) -module -avoid-version

METASOURCES = AUTO

# Internal
# noinst_HEADERS = kconf_update.h

dummy.cpp: 
	echo > dummy.cpp

