
#	This file is part of the KDE libraries
#    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
#			  (C) 1997 Stephan Kulow (coolo@kde.org)

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

SUBDIRS=. pics
AM_CXXFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_srcdir)/kfile $(all_includes)
kde_widget_LTLIBRARIES = kdewidgets.la
kdewidgets_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kdewidgets_la_LIBADD = $(LIB_KIO)
kdewidgets_la_SOURCES = classpreviews.cpp kdewidgets.cpp 

make-kdewidgets: kde.widgets
	$(srcdir)/makekdewidgets -o $(srcdir)/kdewidgets.cpp $<

all-local:
	@if test "$(srcdir)" != "." && test -f ./kdewidgets.cpp; then \
		pwd=`pwd`;\
		echo "=========================================================================" ;\
		echo "remove $$pwd/kdewidgets.cpp - it's most likely outdated!!" ;\
		echo "=========================================================================" ;\
		exit 1; \
	fi
