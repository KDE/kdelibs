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

INCLUDES=  -I$(srcdir)/.. $(all_includes) -I$(top_srcdir)/libltdl

SUBDIRS = . klauncher kded file tests

lib_LTLIBRARIES = libkio.la #libkioslave.la
#libkioslave_la_LDFLAGS = -version-info 3:0
#libkioslave_la_LIBADD = ../kdeui/libkdeui.la
#libkioslave_la_SOURCES = slavebase.cpp connection.cpp passdlg.cpp

libkio_la_LDFLAGS = -version-info 3:0
libkio_la_LIBADD = ../kdeui/libkdeui.la

libkio_la_SOURCES = \
	openwith.cpp ktraderparsetree.cpp \
	ksycoca.cpp ksycocadict.cpp ksycocafactory.cpp \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp kmimemagic.cpp \
	kservice.cpp kservicefactory.cpp \
	kautomount.cpp kdirwatch.cpp scheduler.cpp \
	connection.cpp filter.cpp \
	slaveinterface.cpp job.cpp  \
	global.cpp renamedlg.cpp \
	skipdlg.cpp ktar.cpp krun.cpp \
	kuserpaths.cpp kuserprofile.cpp slave.cpp \
	netaccess.cpp ktrader.cpp ktraderparse.cpp \
	paste.cpp \
        slavebase.cpp passdlg.cpp \
	yacc.c lex.c
# progressbase.cpp simpleprogress_dlg.cpp
# littleprogress_dlg.cpp listprogressdlg.cpp

METASOURCES = AUTO

include_HEADERS = ksycoca.h ksycocaentry.h ksycocatype.h \
        kservicetype.h kmimetype.h kmimemagic.h kservice.h \
        krun.h kdirwatch.h kautomount.h kuserprofile.h kuserpaths.h \
	ktar.h ktrader.h

kioincludedir = $(includedir)/kio
kioinclude_HEADERS = connection.h renamedlg.h \
        filter.h skipdlg.h slaveinterface.h \
        global.h passdlg.h netaccess.h job.h \
        paste.h openwith.h slavebase.h

# progressbase.h simpleprogressdlg.h
# littleprogress_dlg.h listprogressdlg.h


# Internal
noinst_HEADERS = ksycocafactory.h kservicetypefactory.h kservicefactory.h \
	ksycocadict.h kmessageboxwrapper.h \
	ktraderparse.h ktraderparsetree.h yacc.h \
	kioslave.h slavewrapper.h

kde_mime_DATA = magic

parserfiles = yacc.y lex.l

service_DATA = kioslave.desktop
servicedir = $(kde_servicesdir)
EXTRA_DIST = $(kde_mime_DATA) $(parserfiles) $(service_DATA)

parser: $(parserfiles)
	cd $(srcdir) ;\
	flex lex.l && mv lex.yy.c lex.c ;\
	bison -d yacc.y && mv yacc.tab.c yacc.c; mv yacc.tab.h yacc.h

bin_PROGRAMS = kioslave

kioslave_SOURCES = kioslave.cpp slavewrapper.cpp
kioslave_LDADD = libkio.la
kioslave_LDFLAGS = $(KDE_RPATH)

