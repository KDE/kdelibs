#	This file is part of the KDE libraries
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

INCLUDES = -I$(top_srcdir)/kdecore/qk $(all_includes)

CXXFLAGS := $(CXXFLAGS) @NOREPO@

SUBDIRS = qk

lib_LTLIBRARIES = libkdecore.la

include_HEADERS = kconfig.h kconfigdata.h ksimpleconfig.h kconfigbase.h \
	kdesktopfile.h \
	kconfigbackend.h kapp.h kcharsets.h \
	kurl.h ksock.h \
	kprocess.h kprocctrl.h klocale.h kiconloader.h kdebug.h	\
	kstdaccel.h kwinmodule.h kwm.h krootprop.h kpixmap.h dither.h \
	kaccel.h kglobalaccel.h ltdl.h \
	kprotocolmanager.h kregexp.h kstyle.h \
	kstddirs.h kglobal.h ksharedptr.h kde_dmalloc.h \
	kstartparams.h kallocator.h kautoarray.h ksharedpixmap.h \
	klibglobal.h

noinst_HEADERS = kdebugdialog.h kckey.h kregpriv.h

libkdecore_la_SOURCES = kstddirs.cpp  \
	kconfigbase.cpp kstyle.cpp kglobalaccel.cpp \
	kconfig.cpp  ksimpleconfig.cpp kdesktopfile.cpp \
	kconfigbackend.cpp kapp.cpp ksock.cpp\
	kprocess.cpp kprocctrl.cpp libintl.cpp klocale.cpp \
	fakes.cpp kiconloader.cpp kdebug.cpp \
	kstdaccel.cpp kwinmodule.cpp kwm.cpp krootprop.cpp kcharsets.cpp \
	kpixmap.cpp dither.cpp kckey.cpp kaccel.cpp \
	kprotocolmanager.cpp kurl.cpp ltdl.c kregexp.cpp \
	kglobal.cpp dmalloc.cpp  kstartparams.cpp \
	kallocator.cpp ksharedpixmap.cpp vsnprintf.c \
	klibglobal.cpp

libkdecore_la_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 3:0 -no-undefined
libkdecore_la_LIBADD = -lXext qk/libqk.la $(LIBDL) $(LIB_DMALLOC)

libkdecore_la_METASOURCES = kapp.moc \
	kconfig.moc kconfigbase.moc kdebugdialog.moc kglobalaccel.moc \
	kiconloader.moc kprocctrl.moc kprocess.moc ksimpleconfig.moc \
	ksock.moc kstyle.moc kdesktopfile.moc kwinmodule.moc

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdecore

kdebugdir = $(kde_confdir)
kdebug_DATA = kdebug.areas

kckey_h : generate_keys.sh $(QNAMESPACE_H)
	./generate_keys.sh $(QNAMESPACE_H)

install-data-local: charsets.config
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/charsets.config $(kde_confdir)/charsets

uninstall-local:
	-rm -f $(kde_confdir)/charsets

EXTRA_DIST = generate_keys.sh charsets.config $(kdebug_DATA) kdebugareas.txt


parser: kentities.gperf
	cd $(srcdir) && gperf -a -L "ANSI-C" -C -G -c -o -t -k '*' -NfindEntity -D -Hhash_Entity -Wwordlist_Entity -s 2 kentities.gperf > kentities.c
