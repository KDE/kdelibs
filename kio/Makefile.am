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

SUBDIRS = . file

INCLUDES = -I$(top_srcdir)/kfile $(all_includes) 

CXXFLAGS := $(CXXFLAGS) @NOREPO@

lib_LTLIBRARIES = libkio.la
libkio_la_LDFLAGS = -version-info 3:0
libkio_la_LIBADD = ../kdeui/libkdeui.la

libkio_la_SOURCES = kuserpaths.cc kio_connection.cc kio_base.cc kio_rename_dlg.cc \
	kio_filter.cc kio_dlg.cc kio_skip_dlg.cc kio_interface.cc \
	kio_pass_dlg.cc kautomount.cc \
	kio_simpleprogress_dlg.cc kio_littleprogress_dlg.cc kio_listprogress_dlg.cc \
	krun.cc kdirwatch.cc kmimemagic.cc \
	kservices.cc kmimetypes.cc kio_cache.cc kio_netaccess.cc \
	kpixmapcache.cc kuserprofile.cc kio_error.cc kregfactories.cc \
	kio_job.cc kregistry.cc kio_paste.cc kio_openwith.cc \
	kservicetype.cc ktar.cc kio_dns.cc

libkio_la_METASOURCES = kautomount.moc kdirwatch.moc kio_cache.moc \
	kio_job.moc kio_openwith.moc kio_pass_dlg.moc \
	kio_simpleprogress_dlg.moc kio_littleprogress_dlg.moc kio_listprogress_dlg.moc \
	kio_rename_dlg.moc kio_skip_dlg.moc kio_netaccess.moc \
	kregistry.moc krun.moc kregfactories.moc kio_dns.moc

include_HEADERS = kio_connection.h kio_base.h kio_rename_dlg.h \
	kio_filter.h kio_dlg.h kio_skip_dlg.h  kio_interface.h \
	kio_pass_dlg.h kautomount.h \
	kio_simpleprogress_dlg.h kio_littleprogress_dlg.h kio_listprogress_dlg.h \
	krun.h kdirwatch.h kmimemagic.h kservices.h kmimetypes.h \
	kio_cache.h kio_netaccess.h kpixmapcache.h kuserprofile.h \
	kio_error.h kregfactories.h kio_job.h kregistry.h \
	kio_paste.h kio_openwith.h userpaths.h kuserpaths.h \
	kservicetype.h ktar.h ktypecode.h  kio_dns.h

magic_DATA = magic
magicdir = $(kde_mimedir)

EXTRA_DIST = $(magic_DATA)
