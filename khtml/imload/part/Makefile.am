#    This file is part of the KDE libraries
#    Copyright (C) 1997 Martin Jones (mjones@kde.org)
#              (C) 1997 Torben Weis (weis@kde.org)

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

kde_module_LTLIBRARIES = largeimagepart.la

noinst_HEADERS = largeimagepart.h

rcdir   = $(kde_datadir)/largeimagepart
rc_DATA = largeimagepart.rc

largeimagepart_la_SOURCES = largeimagepart.cpp
largeimagepart_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
largeimagepart_la_LIBADD = ../libklargeimages.la $(LIB_KPARTS)

INCLUDES = -I$(top_srcdir)/style-workshop/largeimagelib $(all_includes)

servicedir = $(kde_servicesdir)
service_DATA = largeimage.desktop

METASOURCES = AUTO
