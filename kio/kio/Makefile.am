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

AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

INCLUDES=  -I$(top_srcdir) -I$(srcdir)/.. -I$(srcdir)/../kssl  $(all_includes) $(SSL_INCLUDES)

noinst_LTLIBRARIES = libkiocore.la libksycoca.la

# convenience lib - no LDFLAGS or LIBADD !

libksycoca_la_SOURCES = \
	kdirwatch.cpp \
	kfileshare.cpp \
	ktrader.cpp ktraderparse.cpp ktraderparsetree.cpp \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp kservicegroup.cpp \
	kservice.cpp kservicefactory.cpp \
	kuserprofile.cpp kservicegroupfactory.cpp \
	kurifilter.cpp \
	kfilterbase.cpp kfilterdev.cpp \
	kshellcompletion.cpp kurlcompletion.cpp \
	kautomount.cpp krun.cpp \
	kfileitem.cpp kdirlister.cpp kimageio.cpp \
	yacc.c lex.c  \
	chmodjob.cpp kscan.cpp ktar.cpp kzip.cpp previewjob.cpp metainfojob.cpp \
	kdatatool.cpp karchive.cpp kfilefilter.cpp \
	kfilemetainfo.cpp

#This dependency is needed, cause am_edit does not include it automatically
#(different programs), but nevertheless kmimetype.cpp includes
#kdirnotify_stub.h
$(srcdir)/kmimetype.cpp: kdirnotify_stub.h
$(srcdir)/kautomount.cpp: kdirnotify_stub.h
$(srcdir)/chmodjob.cpp: kdirnotify_stub.h
$(srcdir)/kfileshare.cpp: kdirnotify_stub.h

include_HEADERS = \
	kservicetype.h kmimetype.h kmimemagic.h kservice.h \
	krun.h kdirwatch.h kautomount.h kuserprofile.h \
	kshred.h ktar.h kzip.h ktrader.h kurifilter.h kurlcompletion.h \
	kshellcompletion.h kfileitem.h kfileshare.h kdirlister.h kservicegroup.h\
	kimageio.h kdirnotify.h kdirnotify_stub.h \
	kurlpixmapprovider.h kprotocolinfo.h kprotocolmanager.h \
	kfilterbase.h kfilterdev.h kemailsettings.h kscan.h kdatatool.h \
	karchive.h kfilefilter.h kfilemetainfo.h renamedlgplugin.h \
	kmimetyperesolver.h

#libkiocore_la_LDFLAGS = $(all_libraries)
#libkiocore_la_LIBADD = ../../kdeui/libkdeui.la ../../kdesu/libkdesu.la $(LIBZ) $(LIBFAM) $(LIBVOLMGT)

libkiocore_la_SOURCES = \
	kshred.cpp \
	kprotocolmanager.cpp \
	slave.cpp slaveinterface.cpp observer.stub \
	sessiondata.cpp	scheduler.cpp \
	connection.cpp \
	job.cpp global.cpp \
	slaveconfig.cpp kurlpixmapprovider.cpp \
	netaccess.cpp paste.cpp \
	kmimemagic.cpp kpac.cpp \
	tcpslavebase.cpp slavebase.cpp passdlg.cpp \
	progressbase.cpp defaultprogress.cpp \
	statusbarprogress.cpp \
	kdirnotify.cpp kdirnotify.skel kdirnotify.stub \
	observer.cpp uiserver.stub observer.skel \
	kemailsettings.cpp authinfo.cpp \
	kprotocolinfo.cpp kprotocolinfofactory.cpp

uiserver_DIR = $(top_srcdir)/kio/misc

METASOURCES = AUTO

kioincludedir = $(includedir)/kio
kioinclude_HEADERS = connection.h \
        slaveinterface.h slave.h slaveconfig.h \
        sessiondata.h global.h passdlg.h \
	netaccess.h job.h scheduler.h \
        jobclasses.h paste.h slavebase.h \
	progressbase.h defaultprogress.h \
	statusbarprogress.h tcpslavebase.h \
	observer.h chmodjob.h uiserver_stub.h \
        kpac.h kmdbase.h authinfo.h \
	ioslave_defaults.h http_slave_defaults.h previewjob.h thumbcreator.h \
	metainfojob.h

# Internal - kprotocolinfofactory.h is old stuff
noinst_HEADERS = kservicetypefactory.h kservicefactory.h \
	kmessageboxwrapper.h \
	ktraderparse.h ktraderparsetree.h yacc.h \
	kimageiofactory.h kdirwatch_p.h kdirlister_p.h \
	kimageiofactory.h kdirwatch_p.h \
	kprotocolinfofactory.h \
	renamedlg.h skipdlg.h


parserfiles = yacc.y lex.l

EXTRA_DIST = $(parserfiles)

parser: $(parserfiles)
	cd $(srcdir) ;\
	flex -olex.c -Pkiotrader lex.l ;\
	bison -d -p kiotrader yacc.y && mv yacc.tab.c yacc.c; mv yacc.tab.h yacc.h
