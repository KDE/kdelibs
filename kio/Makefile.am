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

INCLUDES = -I$(top_srcdir)/kdecore -I$(top_srcdir)/kdeui -I$(top_srcdir)/kdeutils $(QT_INCLUDES) $(X_INCLUDES) 
DISTCLEANFILES = $(libkio_la_METASOURCES)

lib_LTLIBRARIES = libkio.la
libkio_la_LDFLAGS = -version-info 2:0

libkio_la_SOURCES = kio_connection.cc kio_base.cc kio_rename_dlg.cc \
	kio_filter.cc kio_dlg.cc kio_skip_dlg.cc kio_interface.cc \
	kio_pass_dlg.cc kio_manager.cc kautomount.cc \
	kio_progress_dlg.cc krun.cc kdirwatch.cc kmimemagic.cc \
	kservices.cc kmimetypes.cc kurlcompletion.cc kio_cache.cc \
	kpixmapcache.cc kuserprofile.cc kio_error.cc kregfactories.cc \
	kio_job.cc kregistry.cc kio_linedit_dlg.cc kio_paste.cc \
	kio_openwith.cc

libkio_la_METASOURCES = USE_AUTOMOC cc

include_HEADERS = kio_connection.h kio_base.h kio_rename_dlg.h \
	kio_filter.h kio_dlg.h kio_skip_dlg.h  kio_interface.h \
	kio_pass_dlg.h kio_manager.h kautomount.h kio_progress_dlg.h \
	krun.h kdirwatch.h kmimemagic.h kservices.h kmimetypes.h \
	kurlcompletion.h kio_cache.h kpixmapcache.h kuserprofile.h \
	kio_error.h kregfactories.h  kio_job.h kregistry.h \
	kio_linedit_dlg.h kio_paste.h kio_openwith.h

kioconfig_DATA = kio.kfg 
kioconfigdir = $(kde_datadir)/kio

EXTRA_DIST = $(kioconfig_DATA)

