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

INCLUDES= $(all_includes)

CXXFLAGS := $(CXXFLAGS) -Wall @NOREPO@

lib_LTLIBRARIES = libkio.la
libkio_la_LDFLAGS = -version-info 3:0
libkio_la_LIBADD = ../kdeui/libkdeui.la

bin_PROGRAMS = ksycocatest
ksycocatest_LDADD = libkio.la
ksycocatest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
ksycocatest_SOURCES = main.cpp kbuildsycoca.cpp

libkio_la_SOURCES = \
	ksycoca.cpp ksycocadict.cpp ksycocaentry.cpp ksycocafactory.cc \
        kservicetypefactory.cpp kservicetype.cpp \
	kmimetype.cpp kmimemagic.cpp \
	kservice.cc kservicefactory.cpp \
	kautomount.cc kdirwatch.cc kio_base.cc kio_cache.cc \
	kio_connection.cc kio_dlg.cc kio_error.cc kio_filter.cc \
	kio_interface.cc kio_job.cc kio_listprogress_dlg.cc \
	kio_littleprogress_dlg.cc kio_netaccess.cc kio_openwith.cc \
	kio_pass_dlg.cc kio_paste.cc kio_rename_dlg.cc kio_simpleprogress_dlg.cc \
	kio_skip_dlg.cc kpixmapcache.cc krun.cc ktar.cc \
	kuserpaths.cc kuserprofile.cc 

libkio_la_METASOURCES = AUTO

include_HEADERS = ksycoca.h ksycocafactory.h ksycocaentry.h ksycocadict.h ksycocatype.h \
	kservicetype.h kservicetypefactory.h \
	kmimetype.h kmimemagic.h kservice.h kservicefactory.h \
        krun.h kdirwatch.h \
	kio_connection.h kio_base.h kio_rename_dlg.h \
        kio_filter.h kio_dlg.h kio_skip_dlg.h  kio_interface.h \
        kio_pass_dlg.h kautomount.h \
        kio_simpleprogress_dlg.h kio_littleprogress_dlg.h kio_listprogress_dlg.h \
        kio_cache.h kio_netaccess.h kpixmapcache.h kuserprofile.h \
        kio_error.h kio_job.h \
        kio_paste.h kio_openwith.h kuserpaths.h \
        ktar.h

noinst_HEADERS = kbuildsycoca.h

magic_DATA = magic
magicdir = $(kde_mimedir)

EXTRA_DIST = $(magic_DATA)
