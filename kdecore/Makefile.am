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

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdecore.la

include_HEADERS = kconfig.h kconfigdata.h ksimpleconfig.h kconfigbase.h \
	kcolorgroup.h kapp.h kcharsets.h \
	kurl.h ksock.h drag.h kmisc.h \
	kprocess.h kprocctrl.h klocale.h kiconloader.h kdebug.h	\
	kstdaccel.h kwm.h krootprop.h kstring.h kpixmap.h dither.h \
	kaccel.h kglobalaccel.h kclipboard.h krect.h ksize.h \
	kpoint.h ltdl.h kprotocolmanager.h kregexp.h kstyle.h

noinst_HEADERS = kdebugdialog.h kcharsetsdata.h kckey.h kregpriv.h

libkdecore_la_SOURCES = kconfig.cpp kconfigbase.cpp ksimpleconfig.cpp \
	kcolorgroup.cpp kapp.cpp ksock.cpp drag.cpp \
	kprocess.cpp kprocctrl.cpp klocale.cpp \
	libintl.cpp fakes.cpp kiconloader.cpp kdebug.cpp \
	kstdaccel.cpp kwm.cpp krootprop.cpp kcharsets.cpp kcharsetsdata.cpp \
	kchartables.cpp kpixmap.cpp dither.cpp \
	kckey.cpp kaccel.cpp kglobalaccel.cpp kclipboard.cpp \
	krect.cpp ksize.cpp kpoint.cpp kprotocolmanager.cpp\
	kurl.cpp ltdl.c kregexp.cpp kstyle.cpp

libkdecore_la_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(USER_LDFLAGS) -version-info 3:0
libkdecore_la_LIBADD = $(LIB_QT) $(LIBDL)

libkdecore_la_METASOURCES = drag.moc kapp.moc kclipboard.moc \
	kconfig.moc kconfigbase.moc kdebugdialog.moc kglobalaccel.moc \
	kiconloader.moc kprocctrl.moc kprocess.moc ksimpleconfig.moc \
	ksock.moc kstyle.moc

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdecore

kdebugdir = $(kde_confdir)
kdebug_DATA = kdebug.areas

## generate lib documentation
srcdoc:
	$(mkinstalldirs) $(SRCDOC_DEST)
	kdoc -H -d $(SRCDOC_DEST) kdecore \
		 $(include_HEADERS) -lqt

kckey_h : generate_keys.sh $(QNAMESPACE_H)
	./generate_keys.sh $(QNAMESPACE_H)

install-data-local: charsets.config 
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/charsets.config $(kde_confdir)/charsets

uninstall-local:
	-rm -f $(kde_confdir)/charsets

EXTRA_DIST = generate_keys.sh charsets.config $(kdebug_DATA) kdebugareas.txt \
	kstddirs.h kstddirs.cpp

