#    This file is part of the KDE libraries
#    Copyright (C) 1997 Martin Jones (mjones@kde.org)
#              (C) 1997 Torben Weis (weis@kde.org)

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

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_LTLIBRARIES = libkhtmlmisc.la
libkhtmlmisc_la_SOURCES = \
	css_extensionsimpl.cpp  css_stylesheetimpl.cpp  cssparser.cpp \
	css_ruleimpl.cpp        css_valueimpl.cpp

libkhtmlmisc_la_METASOURCES = AUTO

noinst_HEADERS = \
	css_extensionsimpl.h  css_stylesheetimpl.h  cssparser.h \
	css_ruleimpl.h        css_valueimpl.h

libkhtmlmisc_la_LDFLAGS = -version-info 3:0

INCLUDES = -I$(srcdir)/kjs -I$(top_srcdir)/kimgio -I$(top_srcdir)/kio \
	-I$(srcdir)/java -I$(top_srcdir)/dcop \
	 -I.. -I. -I../misc -I../css -I../xml -I../html $(all_includes)

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/khtml

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kdecore \
	         $(include_HEADERS) -lqt

