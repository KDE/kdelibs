#    This file is part of the KDE libraries
#    Copyright (C) 2001 George Staikos <staikos@kde.org>

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

INCLUDES= -I$(srcdir)/.. -I$(srcdir)/../kardlib -I$(top_srcdir) $(all_includes) $(PCSC_INCLUDES)

lib_LTLIBRARIES = libkded_kardsvc.la

libkded_kardsvc_la_LDFLAGS = $(all_libraries) ../kardlib/libksmartcard.la -module -avoid-version
libkded_kardsvc_la_LIBADD = $(LIB_KIO)
libkded_kardsvc_la_SOURCES = kardsvc.cpp kardsvc.skel

METASOURCES = AUTO

noinst_HEADERS = kardsvc.h

services_DATA = kardsvc.desktop
servicesdir = $(kde_servicesdir)/kded

