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

INCLUDES = -I$(top_srcdir) \
 -I$(top_srcdir)/khtml -I$(top_srcdir)/khtml/misc -I$(top_srcdir)/khtml/css \
 -I$(top_srcdir)/khtml/dom -I$(top_srcdir)/khtml/xml -I$(top_srcdir)/khtml/html \
 $(all_includes)

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

lib_LTLIBRARIES = kjs_html.la

noinst_HEADERS = kjs_html.h kjs_dom.h kjs_window.h kjs_navigator.h \
	kjs_text.h kjs_binding.h kjs_proxy.h

# HTML module
kjs_html_la_SOURCES = kjs_html.cpp kjs_dom.cpp kjs_window.cpp \
	kjs_navigator.cpp kjs_text.cpp kjs_binding.cpp kjs_proxy.cpp
kjs_html_la_LDFLAGS = -module -avoid-version -no-undefined
kjs_html_la_LIBADD = $(top_builddir)/kjs/libkjs.la ../libkhtml.la
kjs_html_la_METASOURCES = AUTO

# interactive test program
check_PROGRAMS = testecma
testecma_SOURCES = testecma.cpp $(kjs_html_la_SOURCES)
testecma_LDADD = ../libkhtml.la $(top_builddir)/kjs/libkjs.la 
