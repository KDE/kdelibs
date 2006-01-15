kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/kio/bookmarks ${CMAKE_SOURCE_DIR}/khtml ${CMAKE_SOURCE_DIR}/khtml/java ${CMAKE_SOURCE_DIR}/kwallet/client ${CMAKE_SOURCE_DIR}/kutils ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1999 Harri Porten (porten@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_srcdir)/kio/bookmarks -I$(top_srcdir)/khtml -I$(top_srcdir)/khtml/java -I$(top_srcdir)/kwallet/client -I$(top_srcdir)/kutils -I$(top_builddir)/kjs $(all_includes)
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#noinst_LTLIBRARIES = libkjs_html.la
#
#noinst_HEADERS = kjs_html.h kjs_dom.h kjs_window.h kjs_navigator.h \
#	kjs_binding.h kjs_proxy.h kjs_css.h kjs_range.h \
#	kjs_traversal.h kjs_events.h kjs_views.h kjs_debugwin.h kjs_context2d.h
#
#libkjs_html_la_SOURCES = kjs_binding.cpp kjs_dom.cpp kjs_html.cpp kjs_window.cpp \
#	kjs_navigator.cpp kjs_proxy.cpp \
#	kjs_css.cpp kjs_range.cpp kjs_traversal.cpp kjs_events.cpp \
#	kjs_views.cpp kjs_debugwin.cpp kjs_mozilla.cpp xmlhttprequest.cpp \
#        xmlserializer.cpp domparser.cpp kjs_context2d.cpp
##libkjs_html_la_LDFLAGS = -module -avoid-version $(all_libraries)
#libkjs_html_la_LIBADD = $(top_builddir)/kjs/libkjs.la # ../libkhtml.la
#libkjs_html_la_METASOURCES = AUTO
#
#LUT_FILES = kjs_dom.lut.h kjs_html.lut.h kjs_css.lut.h kjs_events.lut.h kjs_navigator.lut.h \
#	    kjs_mozilla.lut.h kjs_range.lut.h kjs_traversal.lut.h kjs_views.lut.h kjs_window.lut.h
#
#CREATE_HASH_TABLE = $(top_srcdir)/kjs/create_hash_table
#
#kjs_dom.lut.h : $(srcdir)/kjs_dom.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_dom.cpp > $@
#kjs_dom.lo: kjs_dom.lut.h
#kjs_html.lut.h : $(srcdir)/kjs_html.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_html.cpp > $@
#kjs_html.lo: kjs_html.lut.h
#kjs_css.lut.h : $(srcdir)/kjs_css.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_css.cpp > $@
#kjs_css.lo: kjs_css.lut.h
#kjs_events.lut.h : $(srcdir)/kjs_events.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_events.cpp > $@
#kjs_events.lo: kjs_events.lut.h
#kjs_navigator.lut.h : $(srcdir)/kjs_navigator.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_navigator.cpp > $@
#kjs_navigator.lo: kjs_navigator.lut.h
#kjs_mozilla.lut.h : $(srcdir)/kjs_mozilla.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_mozilla.cpp > $@
#kjs_mozilla.lo: kjs_mozilla.lut.h
#kjs_range.lut.h : $(srcdir)/kjs_range.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_range.cpp > $@
#kjs_range.lo: kjs_range.lut.h
#kjs_traversal.lut.h : $(srcdir)/kjs_traversal.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_traversal.cpp > $@
#kjs_traversal.lo: kjs_traversal.lut.h
#kjs_views.lut.h : $(srcdir)/kjs_views.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_views.cpp > $@
#kjs_views.lo: kjs_views.lut.h
#kjs_window.lut.h : $(srcdir)/kjs_window.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_window.cpp > $@
#kjs_window.lo: kjs_window.lut.h
#xmlhttprequest.lut.h : $(srcdir)/xmlhttprequest.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/xmlhttprequest.cpp > $@
#xmlhttprequest.lo: xmlhttprequest.lut.h
#xmlserializer.lut.h : $(srcdir)/xmlserializer.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/xmlserializer.cpp > $@
#xmlserializer.lo: xmlserializer.lut.h
#domparser.lut.h : $(srcdir)/domparser.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/domparser.cpp > $@
#domparser.lo: domparser.lut.h
#kjs_context2d.lut.h : $(srcdir)/kjs_context2d.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/kjs_context2d.cpp > $@
#kjs_context2d.lo: kjs_context2d.lut.h
#
#CLEANFILES = $(LUT_FILES)
#
## interactive test program
##check_PROGRAMS = testecma
##testecma_SOURCES = testecma.cpp
##testecma_LDADD = ../libkhtml.la $(top_builddir)/kjs/libkjs.la
#
