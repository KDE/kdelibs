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

INCLUDES=  -I$(srcdir)/.. $(all_includes) $(SSL_INCLUDES)

SUBDIRS = kssl kio bookmarks kfile . kpasswdserver misc pics tests

lib_LTLIBRARIES = libkio.la

libkio_la_SOURCES = dummy.cpp

libkio_la_LDFLAGS = -version-info 5:0:1 -no-undefined $(all_libraries) \
	$(KDE_MT_LDFLAGS)
libkio_la_LIBADD = kssl/libkssl.la kio/libkiocore.la \
	kio/libksycoca.la bookmarks/libkbookmarks.la kfile/libkfile.la \
	../kdeui/libkdeui.la ../kdesu/libkdesu.la \
	$(LIBZ) $(LIBFAM) $(LIBVOLMGT)

kde_mime_DATA = magic
kde_servicetypes_DATA = application.desktop kurifilterplugin.desktop \
                        kcomprfilter.desktop kscan.desktop kdatatool.desktop \
                        kfileplugin.desktop

EXTRA_DIST = $(kde_mime_DATA)

update_DATA = kioslave.upd useragent.pl proxytype.pl
updatedir = $(kde_datadir)/kconf_update

servicetype_DATA = renamedlgplugin.desktop
servicetypedir = $(kde_servicetypesdir)

kded_DATA = kpasswdserver.desktop
kdeddir = $(kde_servicesdir)/kded

dummy.cpp:
	echo >dummy.cpp

DOXYGEN_REFERENCES = kdecore kdefx dcop kdeui
DOXYGEN_EXCLUDE = kssl/kssl/*
include ../Doxyfile.am
