#
#  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
#  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
# 

AM_CPPFLAGS = $(all_includes)
METASOURCES = AUTO

kdeinit_LTLIBRARIES = kcmshell2.la
kcmshell2_la_SOURCES = main.cpp
kcmshell2_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS)
kcmshell2_la_LDFLAGS = $(all_libraries) -module -avoid-version

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmshell.pot
