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

INCLUDES=  -I$(srcdir)/.. -I$(top_srcdir)/libltdl $(all_includes)

SUBDIRS = . klauncher kded file http ftp tests

lib_LTLIBRARIES = libkio.la libksycoca.la #libkioslave.la
#libkioslave_la_LDFLAGS = -version-info 3:0
#libkioslave_la_LIBADD = ../kdeui/libkdeui.la
#libkioslave_la_SOURCES = slavebase.cpp connection.cpp passdlg.cpp

libksycoca_la_LDFLAGS = -version-info 3:0 -no-undefined
libksycoca_la_LIBADD = ./libkio.la

libksycoca_la_SOURCES = \
	ktrader.cpp ktraderparse.cpp ktraderparsetree.cpp \
	ksycoca.cpp ksycocadict.cpp ksycocafactory.cpp \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp \
	kservice.cpp kservicefactory.cpp \
	kuserpaths.cpp kuserprofile.cpp \
	kdirwatch.cpp kurifilter.cpp \
	kautomount.cpp \
	kopenwith.cpp krun.cpp \
	yacc.c lex.c

include_HEADERS = ksycoca.h ksycocaentry.h ksycocatype.h \
        kservicetype.h kmimetype.h kmimemagic.h kservice.h \
        krun.h kdirwatch.h kautomount.h kuserprofile.h kuserpaths.h \
	kshred.h ktar.h ktrader.h kopenwith.h kurifilter.h \
	progressbase.h defaultprogress.h statusbarprogress.h

libkio_la_LDFLAGS = -version-info 3:0 -no-undefined $(all_libraries)
libkio_la_LIBADD = ../kdeui/libkdeui.la $(LIBZ)

libkio_la_SOURCES = \
	kshred.cpp ktar.cpp \
	scheduler.cpp \
	connection.cpp filter.cpp \
	slaveinterface.cpp job.cpp  \
	global.cpp renamedlg.cpp skipdlg.cpp \
	slave.cpp \
	netaccess.cpp paste.cpp \
	kmimemagic.cpp \
	slavebase.cpp passdlg.cpp \
	progressbase.cpp defaultprogress.cpp \
	statusbarprogress.cpp #listprogressdlg.cpp

METASOURCES = AUTO

kioincludedir = $(includedir)/kio
kioinclude_HEADERS = connection.h renamedlg.h \
        filter.h skipdlg.h slaveinterface.h \
        global.h passdlg.h netaccess.h job.h \
        jobclasses.h paste.h slavebase.h \
	progressbase.h defaultprogress.h \
	statusbarprogress.h #listprogressdlg.h

# Internal
noinst_HEADERS = ksycocafactory.h kservicetypefactory.h kservicefactory.h \
	ksycocadict.h kmessageboxwrapper.h \
	ktraderparse.h ktraderparsetree.h yacc.h

kde_mime_DATA = magic
kde_servicetypes_DATA = kurifilterplugin.desktop

parserfiles = yacc.y lex.l

EXTRA_DIST = $(kde_mime_DATA) $(parserfiles)

parser: $(parserfiles)
	cd $(srcdir) ;\
	flex lex.l && mv lex.yy.c lex.c ;\
	bison -d yacc.y && mv yacc.tab.c yacc.c; mv yacc.tab.h yacc.h

