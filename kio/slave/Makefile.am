#    This file is part of the KDE libraries
#    Copyright (C) 1997 Torben Weis (weis@kde.org)

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

INCLUDES=  -I$(srcdir)/../.. -I$(top_srcdir)/libltdl $(all_includes)

METASOURCES = AUTO

service_DATA = kioslave.desktop
servicedir = $(kde_servicesdir)

bin_PROGRAMS = kioslave

kioslave_SOURCES = kioslave.cpp slavewrapper.cpp
kioslave_LDADD = ../libksycoca.la 
kioslave_LDFLAGS = $(KDE_RPATH) -dlpreopen ../file/kio_file.la -dlpreopen ../ftp/kio_ftp.la -dlpreopen ../http/kio_http.la


