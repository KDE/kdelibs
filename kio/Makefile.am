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

INCLUDES=  -I$(srcdir)/.. $(all_includes) $(SSL_INCLUDES)

SUBDIRS = . klauncher kded file http ftp pics tests ksendbugmail kpac \
	gzip kssld $(BZIP2DIR)

lib_LTLIBRARIES = libkio.la libksycoca.la kio_uiserver.la
bin_PROGRAMS = kio_uiserver kmailservice

#libkioslave_la_LDFLAGS = -version-info 3:0
#libkioslave_la_LIBADD = ../kdeui/libkdeui.la
#libkioslave_la_SOURCES = slavebase.cpp connection.cpp passdlg.cpp

libksycoca_la_LDFLAGS = -version-info 3:0 -no-undefined
libksycoca_la_LIBADD = ./libkio.la

libksycoca_la_SOURCES = \
	kdirwatch.cpp \
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
	chmodjob.cpp kscan.cpp ktar.cpp previewjob.cpp

#This dependency is needed, cause am_edit does not include it automatically
#(different programs), but nevertheless kmimetype.cpp includes
#kdirnotify_stub.h
$(srcdir)/kmimetype.cpp: kdirnotify_stub.h

include_HEADERS = \
        kservicetype.h kmimetype.h kmimemagic.h kservice.h \
        krun.h kdirwatch.h kautomount.h kuserprofile.h \
	kshred.h ktar.h ktrader.h kurifilter.h kurlcompletion.h \
	kshellcompletion.h kfileitem.h kdirlister.h kservicegroup.h\
	progressbase.h defaultprogress.h statusbarprogress.h \
	kimageio.h kdirnotify.h kdirnotify_stub.h \
        kurlpixmapprovider.h kprotocolinfo.h kprotocolmanager.h \
	kfilterbase.h kfilterdev.h kemailsettings.h kscan.h

libkio_la_LDFLAGS = -version-info 3:0 -no-undefined $(all_libraries)
libkio_la_LIBADD = ../kdeui/libkdeui.la ../kdesu/libkdesu.la ../kssl/libkssl.la $(LIBZ) $(LIBFAM) $(LIBVOLMGT)

libkio_la_SOURCES = \
	kshred.cpp \
	kprotocolmanager.cpp \
	slave.cpp slaveinterface.cpp \
	sessiondata.cpp	scheduler.cpp \
	connection.cpp \
	job.cpp global.cpp \
	slaveconfig.cpp \
	netaccess.cpp paste.cpp \
	kmimemagic.cpp \
	tcpslavebase.cpp slavebase.cpp passdlg.cpp \
	progressbase.cpp defaultprogress.cpp \
	statusbarprogress.cpp \
	kdirnotify.cpp kdirnotify.skel kdirnotify.stub \
	observer.cpp uiserver.stub observer.skel \
	kemailsettings.cpp authinfo.cpp \
	kprotocolinfo.cpp kprotocolinfofactory.cpp

kio_uiserver_SOURCES = dummy.cpp
kio_uiserver_LDADD = kio_uiserver.la
kio_uiserver_LDFLAGS = $(KDE_RPATH) $(all_libraries)

kio_uiserver_la_SOURCES = uiserver.cpp uiserver.skel observer.stub \
	renamedlg.cpp skipdlg.cpp
kio_uiserver_la_LIBADD = libksycoca.la ../kssl/libkssl.la ../kdesu/libkdesu.la
kio_uiserver_la_LDFLAGS = $(all_libraries) -module -avoid-version

METASOURCES = AUTO

kioincludedir = $(includedir)/kio
kioinclude_HEADERS = connection.h \
        slaveinterface.h slave.h slaveconfig.h \
        sessiondata.h global.h passdlg.h \
	netaccess.h job.h scheduler.h \
        jobclasses.h paste.h slavebase.h \
	progressbase.h defaultprogress.h \
	statusbarprogress.h uiserver.h uiserver_stub.h tcpslavebase.h \
	renamedlg.h skipdlg.h \
	observer.h chmodjob.h \
        kpac.h kmdbase.h authinfo.h \
	ioslave_defaults.h http_slave_defaults.h previewjob.h

# Internal
noinst_HEADERS = kservicetypefactory.h kservicefactory.h \
	kmessageboxwrapper.h \
	ktraderparse.h ktraderparsetree.h yacc.h \
	kimageiofactory.h \
	kprotocolinfofactory.h # old stuff

kde_mime_DATA = magic
kde_servicetypes_DATA = application.desktop kurifilterplugin.desktop \
                        kcomprfilter.desktop kscan.desktop
kde_services_DATA = kio_uiserver.desktop

parserfiles = yacc.y lex.l

EXTRA_DIST = $(kde_mime_DATA) $(parserfiles)

dummy.cpp: uiserver.kidl
	echo > dummy.cpp

kmailservice_SOURCES = kmailservice.cpp
kmailservice_LDADD = $(LIB_KDECORE)
kmailservice_LDFLAGS = $(KDE_RPATH) $(all_libraries)

kmailprotocol_DATA = kmailservice.protocol telnet.protocol rlogin.protocol
kmailprotocoldir = $(kde_servicesdir)

update_DATA = kioslave.upd useragent.pl proxytype.pl
updatedir = $(kde_datadir)/kconf_update

parser: $(parserfiles)
	cd $(srcdir) ;\
	flex -olex.c -Pkiotrader lex.l ;\
	bison -d -p kiotrader yacc.y && mv yacc.tab.c yacc.c; mv yacc.tab.h yacc.h
