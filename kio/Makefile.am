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

INCLUDES= $(all_includes)

CXXFLAGS := $(CXXFLAGS) -Wall @NOREPO@

SUBDIRS = . kded

lib_LTLIBRARIES = libkio.la
libkio_la_LDFLAGS = -version-info 3:0
libkio_la_LIBADD = ../kdeui/libkdeui.la

libkio_la_SOURCES = \
	ksycoca.cpp ksycocadict.cpp ksycocafactory.cpp \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp kmimemagic.cpp \
	kservice.cpp kservicefactory.cpp \
	kautomount.cpp kdirwatch.cpp kio_base.cpp kio_cache.cpp \
	kio_connection.cpp kio_dlg.cpp kio_error.cpp kio_filter.cpp \
	kio_interface.cpp kio_job.cpp kio_listprogress_dlg.cpp \
	kio_littleprogress_dlg.cpp kio_netaccess.cpp kio_openwith.cpp \
	kio_pass_dlg.cpp kio_paste.cpp kio_rename_dlg.cpp kio_simpleprogress_dlg.cpp \
	kio_skip_dlg.cpp kpixmapcache.cpp krun.cpp ktar.cpp \
	kuserpaths.cpp kuserprofile.cpp \
	ktrader.cpp ktraderparse.cpp ktraderparsetree.cpp \
	yacc.c lex.c

libkio_la_METASOURCES = AUTO

include_HEADERS = ksycocaentry.h ksycocatype.h \
	kservicetype.h kmimetype.h kmimemagic.h kservice.h \
        krun.h kdirwatch.h \
	kio_connection.h kio_base.h kio_rename_dlg.h \
        kio_filter.h kio_dlg.h kio_skip_dlg.h  kio_interface.h \
        kio_pass_dlg.h kautomount.h \
        kio_simpleprogress_dlg.h kio_littleprogress_dlg.h kio_listprogress_dlg.h \
        kio_cache.h kio_netaccess.h kpixmapcache.h kuserprofile.h \
        kio_error.h kio_job.h \
        kio_paste.h kio_openwith.h kuserpaths.h \
        ktar.h ktrader.h

# Internal
noinst_HEADERS = ksycocafactory.h kservicetypefactory.h kservicefactory.h \
	ksycocadict.h ksycoca.h \
	ktraderparse.h ktraderparsetree.h yacc.h

magic_DATA = magic
magicdir = $(kde_mimedir)

parserfiles = yacc.y lex.l
EXTRA_DIST = $(magic_DATA) $(parserfiles)

parser: $(parserfiles)
	cd $(srcdir) ;\
	flex lex.l && mv lex.yy.c lex.c ;\
	bison -d yacc.y && mv yacc.tab.c yacc.c; mv yacc.tab.h yacc.h
