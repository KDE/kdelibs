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

INCLUDES = $(all_includes)

SUBDIRS = . tests

lib_LTLIBRARIES = libkdefakes.la libkdecore.la

include_HEADERS = kconfig.h kconfigdata.h ksimpleconfig.h kconfigbase.h \
	kdesktopfile.h kurl.h ksock.h kaboutdata.h kcmdlineargs.h \
	kconfigbackend.h kapp.h kuniqueapp.h kcharsets.h \
	kprocess.h kprocctrl.h klocale.h kicontheme.h kiconloader.h kdebug.h \
	kstdaccel.h kwinmodule.h kwin.h krootprop.h kpixmap.h  \
	kaccel.h kglobalaccel.h kapplication.h kstandarddirs.h \
	kregexp.h kstyle.h kcompletion.h kstringhandler.h kstringhandler.h \
	kstddirs.h kglobal.h kglobalsettings.h ksharedptr.h kde_dmalloc.h \
	kallocator.h kcrash.h krfcdate.h kuniqueapplication.h \
	kinstance.h kpalette.h kipc.h klibloader.h ktempfile.h ksavefile.h \
	krandomsequence.h kstringhandler.h knotifyclient.h kiconeffect.h \
	kaudioplayer.h kdestyle.h kdcoppropertyproxy.h kdatastream.h  netwm.h \
	netwm_def.h netwm_p.h kpixmapprovider.h kunload.h kstaticdeleter.h \
	kextsock.h ksockaddr.h kprocio.h kasyncio.h kbufferedio.h \
	kurldrag.h kmimesourcefactory.h kpixmapsplitter.h kmdcodec.h ksocks.h \
	ksycoca.h ksycocaentry.h ksycocatype.h kxmessages.h kstartupinfo.h \
	kdesktopwidget.h

libkdefakes_la_SOURCES = fakes.c vsnprintf.c
libkdefakes_la_LDFLAGS = -version-info 3:0

noinst_HEADERS = kckey.h kregpriv.h kcompletion_private.h netsupp.h \
	ksycocafactory.h ksycocadict.h kextsocklookup.h

libkdecore_la_SOURCES = libintl.cpp kdebug.cpp netwm.cpp kapp.cpp \
	kconfigbase.cpp kstyle.cpp \
	kconfig.cpp  ksimpleconfig.cpp kdesktopfile.cpp \
	kconfigbackend.cpp kglobalaccel.cpp  \
        kstddirs.cpp kuniqueapp.cpp ksock.cpp\
	kprocess.cpp kprocctrl.cpp klocale.cpp krfcdate.cpp\
	kiconeffect.cpp kicontheme.cpp kiconloader.cpp \
	kstdaccel.cpp kwin.cpp kwinmodule.cpp krootprop.cpp kcharsets.cpp \
	kpixmap.cpp kckey.cpp kaccel.cpp kcrash.cpp\
	kurl.cpp kregexp.cpp \
	kglobal.cpp kglobalsettings.cpp dmalloc.cpp  \
	kallocator.cpp kmimesourcefactory.cpp \
	kinstance.cpp kpalette.cpp kipc.cpp klibloader.cpp ktempfile.cpp \
	ksavefile.cpp krandomsequence.cpp kstringhandler.cpp kcompletion.cpp \
	kcmdlineargs.cpp kaboutdata.cpp kcompletionbase.cpp knotifyclient.cpp \
	kaudioplayer.cpp kdestyle.cpp kdcoppropertyproxy.cpp \
	kextsock.cpp ksockaddr.cpp netsupp.cpp kprocio.cpp kbufferedio.cpp \
	kpixmapprovider.cpp kurldrag.cpp kpixmapsplitter.cpp \
        kmdcodec.cpp ksocks.cpp fakes.c vsnprintf.c \
	ksycoca.cpp ksycocadict.cpp ksycocafactory.cpp ksycoca.skel \
	kxmessages.cpp kstartupinfo.cpp kdesktopwidget.cpp


libkdecore_la_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 3:0 -no-undefined
libkdecore_la_LIBADD = ../libltdl/libltdlc.la -lXext $(LIB_DCOP) $(LIB_DMALLOC) $(LIBRESOLV) $(LIBXINERAMA)

libkdecore_la_METASOURCES = AUTO

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdecore

kdebugdir = $(kde_confdir)
kdebug_DATA = kdebug.areas kdebugrc

kckey_h : $(srcdir)/generate_keys.sh $(QNAMESPACE_H)
	(cd $(srcdir) && ./generate_keys.sh $(QNAMESPACE_H))

install-data-local: charsets.config
	$(mkinstalldirs) $(includedir)
	$(mkinstalldirs) $(includedir)/kio
	$(INSTALL_DATA) $(srcdir)/../libltdl/ltdl.h $(includedir)/ltdl.h
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/charsets.config $(kde_confdir)/charsets
	$(mkinstalldirs) $(kde_datadir)/knotify
	$(INSTALL_DATA) $(srcdir)/eventsrc $(kde_datadir)/knotify/eventsrc
	$(mkinstalldirs) $(kde_confdir)/colors
	$(INSTALL_DATA) $(srcdir)/40.colors "$(DESTDIR)$(kde_confdir)/colors/40 Colors"
	$(INSTALL_DATA) $(srcdir)/Web.colors $(kde_confdir)/colors/Web
	$(INSTALL_DATA) $(srcdir)/Royal.colors $(kde_confdir)/colors/Royal
	$(INSTALL_DATA) $(srcdir)/kmdcodec_compat.h $(includedir)/kio/kmdcodec.h

uninstall-local:
	-rm -f $(kde_confdir)/charsets
	-rm -f $(includedir)/ltdl.h
	-rm -f $(includedir)/kio/kmdcodec.h

EXTRA_DIST = generate_keys.sh charsets.config $(kdebug_DATA) Web.colors \
	Royal.colors 40.colors DESIGN.COLOR KCONFIG_DESIGN eventsrc \
	kentities.c kentities.gperf makeentities kmdcodec_compat.h

parser: kentities.gperf
	cd $(srcdir) && gperf -a -L "ANSI-C" -C -G -c -o -t -k '*' -NfindEntity -D -Hhash_Entity -Wwordlist_Entity -s 2 kentities.gperf > kentities.c

bin_PROGRAMS = kde-config
kde_config_SOURCES = kde-config.cpp
kde_config_LDADD = ./libkdecore.la
kde_config_LDFLAGS = $(KDE_RPATH)

DISTCLEANFILES = kde-config.cpp
