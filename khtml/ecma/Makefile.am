#    This file is part of the KDE libraries
#    Copyright (C) 1999 Harri Porten (porten@kde.org)

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

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/khtml -I$(top_srcdir)/khtml/java $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(WOVERLOADED_VIRTUAL)

kde_module_LTLIBRARIES = kjs_html.la

noinst_HEADERS = kjs_html.h kjs_dom.h kjs_window.h kjs_navigator.h \
	kjs_binding.h kjs_proxy.h kjs_css.h kjs_range.h \
	kjs_traversal.h kjs_events.h kjs_views.h kjs_debugwin.h

# HTML module
kjs_html_la_SOURCES = kjs_binding.cpp kjs_dom.cpp kjs_html.cpp kjs_window.cpp \
	kjs_navigator.cpp kjs_proxy.cpp \
	kjs_css.cpp kjs_range.cpp kjs_traversal.cpp kjs_events.cpp \
	kjs_views.cpp kjs_debugwin.cpp
kjs_html_la_LDFLAGS = -module -avoid-version $(all_libraries)
kjs_html_la_LIBADD = $(top_builddir)/kjs/libkjs.la ../libkhtml.la
kjs_html_la_METASOURCES = AUTO

hashtables:
	cd $(srcdir); \
	../../kjs/create_hash_table kjs_dom.cpp > kjs_dom.lut.h; \
	../../kjs/create_hash_table kjs_html.cpp > kjs_html.lut.h; \
	../../kjs/create_hash_table kjs_css.cpp > kjs_css.lut.h; \
	../../kjs/create_hash_table kjs_events.cpp > kjs_events.lut.h; \
	../../kjs/create_hash_table kjs_navigator.cpp > kjs_navigator.lut.h; \
	../../kjs/create_hash_table kjs_range.cpp > kjs_range.lut.h; \
	../../kjs/create_hash_table kjs_traversal.cpp > kjs_traversal.lut.h; \
	../../kjs/create_hash_table kjs_views.cpp > kjs_views.lut.h; \
	../../kjs/create_hash_table kjs_window.cpp > kjs_window.lut.h

# interactive test program
#check_PROGRAMS = testecma
#testecma_SOURCES = testecma.cpp $(kjs_html_la_SOURCES)
#testecma_LDADD = ../libkhtml.la $(top_builddir)/kjs/libkjs.la 
