#	This file is part of the KDE libraries
#    Copyright (C) 1999 Mirko Sucker (mirko@kde.org)
#                  1997 Stephan Kulow (coolo@kde.org)
#
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


INCLUDES= $(all_includes)

lib_LTLIBRARIES  = libnana.la

include_HEADERS = nana-config.h nana.h \
	DI.h I.h DL.h L.h Q.h GDB.h L_buffer.h now.h \
	eiffel.h Qstl.h calls.h nana_error.h L_times.h \
	cycles.h

pkgdata_DATA = nana-run.gdb nana-run-init.gdb

libnana_la_SOURCES = I.c L_buffer.c now.c calls.c nana_error.c L_times.c \
		cycles.c

# bin_SCRIPTS = 	nana nana-clg nana-run nana-c++lg

libexec_PROGRAMS = nanafilter 

EXTRA_DIST = nana.in nana-clg.in nana-run.in nana-run.gdb \
		nana-config.h.in nana-run-init.gdb \
		assert.h nana-c++lg.in

LDFLAGS = $(USER_LDFLAGS)

libnana_la_LDFLAGS = -no-undefined -version-info 2:4 $(LDFLAGS)
