#    This file is part of the KDE libraries
#
#    $Id$
#
#    Copyright (C) 1996-1997 Matthias Kalle Dalheimer (kalle@kde.org)
#			  (C) 1997 Stephan Kulow (coolo@kde.org)

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

INCLUDES = $(all_includes) $(LIBART_CFLAGS) 

if include_SVGICONS
SVGICONS=svgicons
SVGICON_LIB=svgicons/libkdesvgicons.la
endif

SUBDIRS = malloc $(SVGICONS) . tests

lib_LTLIBRARIES = libkdefakes.la libkdecore.la

include_HEADERS = kconfig.h kconfigdata.h ksimpleconfig.h kconfigbase.h \
	kdesktopfile.h kurl.h ksock.h kaboutdata.h kcmdlineargs.h \
	kconfigbackend.h kapp.h kapplication.h kuniqueapp.h kuniqueapplication.h \
	kcharsets.h kdeversion.h \
	kprocess.h kprocctrl.h klocale.h kicontheme.h kiconloader.h kdebug.h \
	kwinmodule.h kwin.h krootprop.h \
	kshortcut.h kkeynative.h kaccel.h kglobalaccel.h \
	kstdaccel.h kshortcutlist.h \
	kcatalogue.h \
	kregexp.h kcompletion.h kstringhandler.h kstringhandler.h \
	kstddirs.h kstandarddirs.h kglobal.h kglobalsettings.h ksharedptr.h \
	kde_dmalloc.h kallocator.h kvmallocator.h kcrash.h krfcdate.h \
	kinstance.h kpalette.h kipc.h klibloader.h ktempfile.h ksavefile.h \
	krandomsequence.h kstringhandler.h knotifyclient.h kiconeffect.h \
	kaudioplayer.h kdcoppropertyproxy.h netwm.h \
	netwm_def.h netwm_p.h kpixmapprovider.h kunload.h kstaticdeleter.h \
	kextsock.h kextendedsocket.h ksockaddr.h kprocio.h kasyncio.h kbufferedio.h \
	kurldrag.h kmimesourcefactory.h kmdcodec.h ksocks.h \
	ksycoca.h ksycocaentry.h ksycocatype.h kxmessages.h kstartupinfo.h \
	klargefile.h kmultipledrag.h kgenericfactory.h kgenericfactory.tcc ktypelist.h \
	ksortablevaluelist.h kdebugclasses.h kclipboard.h

libkdefakes_la_SOURCES = fakes.c vsnprintf.c
libkdefakes_la_LDFLAGS = -version-info 5:0:1

noinst_HEADERS = kaccelaction.h kaccelbase.h kaccelprivate.h kckey.h \
	kcompletion_private.h \
	kextsocklookup.h kglobalaccel_x11.h kkeyserver_x11.h \
	kregpriv.h ksycocadict.h ksycocafactory.h netsupp.h

libkdecore_la_SOURCES = libintl.cpp kapplication.cpp kdebug.cpp netwm.cpp  \
	kconfigbase.cpp kconfig.cpp  ksimpleconfig.cpp kconfigbackend.cpp  \
	kdesktopfile.cpp kstandarddirs.cpp ksock.cpp\
	kprocess.cpp kprocctrl.cpp klocale.cpp krfcdate.cpp\
	kiconeffect.cpp kicontheme.cpp kiconloader.cpp \
	kwin.cpp kwinmodule.cpp krootprop.cpp kcharsets.cpp \
	kckey.cpp kshortcut.cpp kkeynative_x11.cpp kkeyserver_x11.cpp \
	kaccelaction.cpp kaccelbase.cpp kaccel.cpp kglobalaccel_x11.cpp \
	kglobalaccel.cpp kstdaccel.cpp kshortcutlist.cpp \
	kcrash.cpp kurl.cpp kregexp.cpp \
	kglobal.cpp kglobalsettings.cpp dmalloc.cpp  \
	kallocator.cpp kvmallocator.cpp kmimesourcefactory.cpp \
	kinstance.cpp kpalette.cpp kipc.cpp klibloader.cpp ktempfile.cpp \
	kuniqueapplication.cpp \
	ksavefile.cpp krandomsequence.cpp kstringhandler.cpp kcompletion.cpp \
	kcmdlineargs.cpp kaboutdata.cpp kcompletionbase.cpp knotifyclient.cpp \
	kaudioplayer.cpp kdcoppropertyproxy.cpp \
	kextsock.cpp ksockaddr.cpp netsupp.cpp kprocio.cpp kbufferedio.cpp \
	kpixmapprovider.cpp kurldrag.cpp \
	kmdcodec.cpp ksocks.cpp fakes.c vsnprintf.c \
	ksycoca.cpp ksycocadict.cpp ksycocafactory.cpp ksycoca.skel \
	kxmessages.cpp kstartupinfo.cpp kcatalogue.cpp kasyncio.cpp \
	kmultipledrag.cpp kstaticdeleter.cpp kappdcopiface.cpp kappdcopiface.skel kclipboard.cpp

libkdecore_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_MT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 5:0:1 -no-undefined
libkdecore_la_LIBADD = malloc/libklmalloc.la ../dcop/libDCOP.la ../libltdl/libltdlc.la $(LIB_XEXT) $(LIBRESOLV) $(LIBXINERAMA) $(LIB_DMALLOC) $(LIBART_LIBS) $(SVGICON_LIB)

libkdecore_la_METASOURCES = AUTO

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdecore

kdebugdir = $(kde_confdir)
kdebug_DATA = kdebug.areas kdebugrc language.codes

kckey_h : $(srcdir)/generate_keys.sh $(QNAMESPACE_H)
	(cd $(srcdir) && ./generate_keys.sh $(QNAMESPACE_H))

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(includedir)/kio
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/knotify
	$(INSTALL_DATA) $(srcdir)/eventsrc $(DESTDIR)$(kde_datadir)/knotify/eventsrc
	$(mkinstalldirs) $(DESTDIR)$(kde_confdir)/colors
	$(INSTALL_DATA) $(srcdir)/40.colors "$(DESTDIR)$(kde_confdir)/colors/40 Colors"
	$(INSTALL_DATA) $(srcdir)/Web.colors $(DESTDIR)$(kde_confdir)/colors/Web
	$(INSTALL_DATA) $(srcdir)/Royal.colors $(DESTDIR)$(kde_confdir)/colors/Royal
	$(INSTALL_DATA) $(srcdir)/kmdcodec_compat.h $(DESTDIR)$(includedir)/kio/kmdcodec.h

uninstall-local:
	-rm -f $(DESTDIR)$(includedir)/kio/kmdcodec.h

EXTRA_DIST = generate_keys.sh $(kdebug_DATA) Web.colors \
	Royal.colors 40.colors DESIGN.COLOR KCONFIG_DESIGN eventsrc \
	kentities.c kentities.gperf makeentities kmdcodec_compat.h

kcharsets.lo: kentities.c

parser: kentities.gperf
	cd $(srcdir) && gperf -a -L "ANSI-C" -C -G -c -o -t -k '*' -NfindEntity -D -Hhash_Entity -Wwordlist_Entity -s 2 kentities.gperf > kentities.c

bin_PROGRAMS = kde-config
kde_config_SOURCES = kde-config.cpp
kde_config_LDADD = ./libkdecore.la
kde_config_LDFLAGS = $(KDE_RPATH) $(KDE_MT_LDFLAGS)

DISTCLEANFILES = kde-config.cpp

DOXYGEN_REFERENCES = dcop kdeui
DOXYGEN_EXCLUDE = malloc/*

include ../Doxyfile.am

