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

INCLUDES = $(QT_INCLUDES) $(X_INCLUDES)
DISTCLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = convert_key.c
lib_LTLIBRARIES = libkdecore.la

DEFS += -DTEST_KDEDIR        

include_HEADERS = kconfig.h kconfigdata.h ksimpleconfig.h kconfigbase.h \
	kcolorgroup.h kapp.h kcharsets.h \
	kurl.h ksock.h drag.h kkeyconf.h kkeydata.h ckey.h kmisc.h	\
	kprocess.h kprocctrl.h klocale.h kiconloader.h kdebug.h	\
	kstdaccel.h kwm.h krootprop.h kstring.h kpixmap.h dither.h

noinst_HEADERS = kdebugdialog.h kcharsetsdata.h 

libkdecore_la_SOURCES = kconfig.cpp kconfigbase.cpp ksimpleconfig.cpp \
	kcolorgroup.cpp kapp.cpp kurl.cpp ksock.cpp drag.cpp \
	kkeyconf.cpp kprocess.cpp kprocctrl.cpp klocale.cpp \
	libintl.cpp fakes.cpp kiconloader.cpp kdebug.cpp \
	kstdaccel.cpp kwm.cpp krootprop.cpp kcharsets.cpp kcharsetsdata.cpp \
	kchartables.cpp kstring.cpp kpixmap.cpp dither.cpp 

libkdecore_la_LDFLAGS = -version-info 0:91
libkdecore_la_LIBADD = 

BUILT_SOURCES = ksock.moc drag.moc kkeydata.moc kapp.moc kprocess.moc	\
	kprocctrl.moc kiconloader.moc kdebugdialog.moc kconfig.moc \
	kconfigbase.moc ksimpleconfig.moc 

