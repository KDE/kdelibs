#    This file is part of the KDE libraries
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
#    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
#    Boston, MA 02110-1301, USA.

INCLUDES = -I$(top_srcdir)/kdefx -I$(top_srcdir)/kdecore/network $(all_includes) $(LIBART_CFLAGS)

if include_SVGICONS
SVGICONS=svgicons
SVGICON_LIB=svgicons/libkdesvgicons.la
endif

SUBDIRS = malloc $(SVGICONS) . kconfig_compiler tests network 

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

lib_LTLIBRARIES = libkdefakes.la libkdecore.la
lib_LIBRARIES = libkdefakes_nonpic.a

include_HEADERS = kconfig.h kconfigskeleton.h \
	kconfigdata.h ksimpleconfig.h kconfigdialogmanager.h \
	kconfigbase.h kdesktopfile.h kurl.h kaboutdata.h \
	kcmdlineargs.h kconfigbackend.h kapplication.h kauthorized.h ktoolinvocation.h \
	kuniqueapplication.h kcharsets.h kdeversion.h kpty.h kprocess.h \
	kprocctrl.h klocale.h kicontheme.h kiconloader.h kdebug.h \
	kwinmodule.h kwin.h krootprop.h kshortcut.h kkeynative.h kaccel.h \
	kglobalaccel.h kstdaccel.h kshortcutlist.h kcatalogue.h \
	kcompletion.h kstringhandler.h \
	kstandarddirs.h kglobal.h kglobalsettings.h ksharedptr.h \
	kallocator.h kvmallocator.h kcrash.h krfcdate.h \
	kinstance.h kpalette.h kipc.h klibloader.h ktempfile.h ksavefile.h \
	krandomsequence.h knotifyclient.h kiconeffect.h \
	kaudioplayer.h kdcoppropertyproxy.h netwm.h kaccelmanager.h \
	netwm_def.h kpixmapprovider.h kstaticdeleter.h \
	kprocio.h kasyncio.h \
	kbufferedio.h kurldrag.h kmimesourcefactory.h kcodecs.h ksocks.h \
	ksycoca.h ksycocaentry.h ksycocatype.h kxmessages.h kstartupinfo.h \
	kmultipledrag.h kgenericfactory.h kgenericfactory.tcc \
	ktypelist.h ksortablevaluelist.h kdebugclasses.h kclipboard.h \
	kcalendarsystem.h kcalendarsystemfactory.h kmacroexpander.h \
	kmanagerselection.h kmountpoint.h kuser.h klockfile.h \
	kidna.h ktempdir.h kshell.h fixx11h.h kxerrorhandler.h kdelibs_export.h \
	kdemacros.h kde_file.h ktimezones.h kmdcodec.h kmath.h

libkdefakes_la_SOURCES = fakes.c
libkdefakes_la_LDFLAGS = -version-info 6:0:2
libkdefakes_la_LIBADD  = $(LIBSOCKET)

libkdefakes_nonpic_a_SOURCES = fakes_nonpic.c

fakes_nonpic.c:$(srcdir)/fakes.c
	-rm -f fakes_nonpic.c
	$(LN_S) $(srcdir)/fakes.c fakes_nonpic.c

noinst_HEADERS = kaccelaction.h kaccelbase.h kaccelprivate.h kckey.h \
	kcompletion_private.h netwm_p.h \
	kglobalaccel_x11.h kglobalaccel_win.h kkeyserver_x11.h kkeyserver.h \
	kshortcutmenu.h ksycocadict.h ksycocafactory.h netsupp.h \
	kcheckaccelerators.h kcalendarsystemgregorian.h \
	kcalendarsystemhijri.h kcalendarsystemhebrew.h kcalendarsystemjalali.h \
	kprotocolinfofactory.h kqiodevicegzip_p.h

libkdecore_la_SOURCES = libintl.cpp kapplication.cpp kauthorized.cpp ktoolinvocation_x11.cpp \
	kdebug.cpp netwm.cpp kconfigbase.cpp kconfig.cpp  ksimpleconfig.cpp \
	kconfigbackend.cpp kmanagerselection.cpp kdesktopfile.cpp \
	kstandarddirs.cpp kpty.cpp kprocess.cpp kprocctrl.cpp \
	klocale.cpp krfcdate.cpp kiconeffect.cpp kicontheme.cpp \
	kiconloader.cpp kwin.cpp kwinmodule.cpp krootprop.cpp kcharsets.cpp \
	kckey.cpp kshortcut.cpp kkeynative_x11.cpp kkeyserver_x11.cpp \
	kaccelaction.cpp kshortcutmenu.cpp kaccelbase.cpp kaccel.cpp \
	kglobalaccel_x11.cpp kglobalaccel.cpp kstdaccel.cpp kshortcutlist.cpp \
	kcrash.cpp kurl.cpp kglobal.cpp kglobalsettings.cpp \
	kallocator.cpp kvmallocator.cpp kmimesourcefactory.cpp \
	kinstance.cpp kpalette.cpp kipc.cpp klibloader.cpp ktempfile.cpp \
	kuniqueapplication.cpp kaccelmanager.cpp \
	ksavefile.cpp krandomsequence.cpp kstringhandler.cpp kcompletion.cpp \
	kcmdlineargs.cpp kaboutdata.cpp kcompletionbase.cpp knotifyclient.cpp \
	kaudioplayer.cpp kdcoppropertyproxy.cpp \
	netsupp.cpp kprocio.cpp \
	kpixmapprovider.cpp kurldrag.cpp \
	kcodecs.cpp ksocks.cpp fakes.c \
	ksycoca.cpp ksycocadict.cpp ksycocafactory.cpp ksycoca.skel \
	kxmessages.cpp kstartupinfo.cpp kcatalogue.cpp \
	kmultipledrag.cpp kstaticdeleter.cpp kappdcopiface.cpp \
	kappdcopiface.skel kclipboard.cpp kcheckaccelerators.cpp \
	kdeversion.cpp kdebugdcopiface.cpp kdebugdcopiface.skel \
	kcalendarsystem.cpp kcalendarsystemgregorian.cpp \
	kcalendarsystemhijri.cpp kcalendarsystemhebrew.cpp \
	kcalendarsystemfactory.cpp kmacroexpander.cpp kidna.cpp \
	ktempdir.cpp kshell.cpp kmountpoint.cpp kcalendarsystemjalali.cpp \
	kprotocolinfo_kdecore.cpp kprotocolinfofactory.cpp kxerrorhandler.cpp \
	kuser.cpp kconfigskeleton.cpp kconfigdialogmanager.cpp klockfile.cpp \
	kqiodevicegzip_p.cpp ktimezones.cpp kmath.cpp

libkdecore_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_MT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 6:0:2 -no-undefined
libkdecore_la_LIBADD = malloc/libklmalloc.la $(SVGICON_LIB) ../dcop/libDCOP.la ../libltdl/libltdlc.la $(LIB_XEXT) $(LIBRESOLV) $(LIBUTIL) $(LIBART_LIBS) $(LIB_IDN) network/libkdecorenetwork.la # ../kdefx/libkdefx.la
libkdecore_la_NMCHECK = $(srcdir)/libkdecore.nmcheck
libkdecore_la_NMCHECKWEAK = $(srcdir)/libkdecore_weak.nmcheck $(srcdir)/libqt-mt_weak.nmcheck \
	$(top_srcdir)/dcop/libDCOP_weak.nmcheck $(top_srcdir)/kdecore/standard_weak.nmcheck

libkdecore_la_METASOURCES = AUTO

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdecore

kdebugdir = $(kde_confdir)
kdebug_DATA = kdebug.areas kdebugrc language.codes

.PHONY: kckey_h parser
kckey_h : $(srcdir)/generate_keys.sh $(QNAMESPACE_H)
	(cd $(srcdir) && ./generate_keys.sh $(QNAMESPACE_H))

# Also uninstall the old names of the KDE color palette files
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_locale)
	$(INSTALL_DATA) $(srcdir)/all_languages.desktop $(DESTDIR)$(kde_locale)/all_languages
	$(mkinstalldirs) $(DESTDIR)$(includedir)/kio
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/knotify
	$(INSTALL_DATA) $(srcdir)/eventsrc $(DESTDIR)$(kde_datadir)/knotify/eventsrc
	$(INSTALL_DATA) $(srcdir)/kmdcodec_compat.h $(DESTDIR)$(includedir)/kio/kmdcodec.h
	rm -f "$(DESTDIR)$(kde_confdir)/colors/40 Colors"
	rm -f $(DESTDIR)$(kde_confdir)/colors/Web
	rm -f $(DESTDIR)$(kde_confdir)/colors/Royal

uninstall-local:
	-rm -f $(DESTDIR)$(includedir)/kio/kmdcodec.h

# If you add a color palette file here, please change kdelibs/kdeui/kcolordialog.cpp too to allow to translate the name
colors_DATA = 40.colors Web.colors Royal.colors Rainbow.colors
colorsdir = $(kde_confdir)/colors

EXTRA_DIST = generate_keys.sh $(kdebug_DATA) \
	DESIGN.COLOR KCONFIG_DESIGN eventsrc \
	kentities.c kentities.gperf makeentities kmdcodec_compat.h \
	all_languages.desktop

kcharsets.lo: kentities.c

parser: kentities.gperf
	cd $(srcdir) && gperf -a -L "ANSI-C" -E -C -c -o -t -k '*' -Nkde_findEntity -D -Hhash_Entity -Wwordlist_Entity -s 2 kentities.gperf > kentities.c

bin_PROGRAMS = kde-config kgrantpty

kde_config_SOURCES = kde-config.cpp
kde_config_LDADD = ./libkdecore.la
kde_config_LDFLAGS = $(KDE_RPATH) $(KDE_MT_LDFLAGS)

kgrantpty_SOURCES = kgrantpty.c
kgrantpty_CFLAGS= $(KDE_USE_FPIE)
kgrantpty_LDFLAGS = $(KDE_USE_PIE) $(KDE_RPATH) $(all_libraries)
kgrantpty_LDADD = ./libkdefakes.la

# kgrantpty needs to be installed setuid root
install-exec-hook:
	@(chown 0 $(DESTDIR)$(bindir)/kgrantpty && chmod 4755 $(DESTDIR)$(bindir)/kgrantpty) || echo "Please make kgrantpty setuid root" >&2
	@echo ""
	@echo "kgrantpty is by default installed with a set SETUID root bit!"
	@echo "This is needed for konsole, etc. to ensure that they can't be eavesdropped."
	@echo ""

DISTCLEANFILES = kde-config.cpp

DOXYGEN_REFERENCES = dcop kdeui kparts kio
DOXYGEN_EXCLUDE = malloc kde_file.h
DOXYGEN_SET_WARN_IF_UNDOCUMENTED = YES

include ../admin/Doxyfile.am
