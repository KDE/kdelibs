kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kimgio ${CMAKE_SOURCE_DIR}/kio ${CMAKE_SOURCE_DIR}/dcop ${CMAKE_SOURCE_DIR}/khtml ${CMAKE_SOURCE_DIR}/libltdl ${CMAKE_SOURCE_DIR}/kwallet/client ${CMAKE_SOURCE_DIR}/kutils ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/khtml/css FILES html4.css quirks.css )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1997 Martin Jones (mjones@kde.org)
##              (C) 1997 Torben Weis (weis@kde.org)
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
#KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL) 
#
#YACC=bison
#
#noinst_LTLIBRARIES = libkhtmlcss.la
#libkhtmlcss_la_SOURCES = \
#	css_stylesheetimpl.cpp css_ruleimpl.cpp css_valueimpl.cpp css_base.cpp \
#	cssparser.cpp cssstyleselector.cpp csshelper.cpp parser.cpp \
#	css_renderstyledeclarationimpl.cpp
#
##libkhtmlcss_la_LDFLAGS = -no-undefined 
#libkhtmlcss_la_METASOURCES = AUTO
#
#noinst_HEADERS = \
#	css_extensionsimpl.h  css_stylesheetimpl.h  cssparser.h \
#	css_ruleimpl.h css_valueimpl.h css_base.h \
#	cssstyleselector.h csshelper.h parser.h   \
#	css_renderstyledeclarationimpl.h
#
#INCLUDES = -I$(top_srcdir)/kimgio -I$(top_srcdir)/kio -I$(top_srcdir)/dcop \
# -I$(top_srcdir)/khtml -I$(top_srcdir)/libltdl -I$(top_srcdir) \
# -I$(top_srcdir)/kwallet/client -I$(top_srcdir)/kutils \
# $(all_includes)
#
#cssdir = $(kde_datadir)/khtml/css
#css_DATA = html4.css quirks.css
#
#SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml
#
#EXTRA_DIST = parser.y
#
#parser: $(srcdir)/parser.y
#	cd $(srcdir); \
#	$(YACC) -v -d -p cssyy parser.y && mv parser.tab.c parser.cpp; \
#	if test -f parser.tab.h; then \
#	if cmp -s parser.tab.h parser.h; then rm -f parser.tab.h; \
#	else mv parser.tab.h parser.h; fi \
#	else :; fi
#
#.PHONY: parser 
#
