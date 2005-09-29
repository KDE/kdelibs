#
#  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
#  Copyright (c) 2000 Matthias Elter <elter@kde.org>
#  Copyright (c) 2004 Frans Englich <frans.englich@telia.com>
# 
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
# 

AM_CPPFLAGS = -I$(top_srcdir)/kutils $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = 
bin_PROGRAMS = 
kdeinit_LTLIBRARIES = kcmshell.la

kcmshell_la_SOURCES = main.cpp main.skel
kcmshell_la_LIBADD = ../kutils/libkutils.la
kcmshell_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = main.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmshell.pot


include $(top_srcdir)/admin/Doxyfile.am

