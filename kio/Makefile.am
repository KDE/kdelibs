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

INCLUDES = -I../kdecore -I../kdeui -I../kdeutils $(QT_INCLUDES) $(X_INCLUDES) 
DISTCLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES = libkio.la
libkio_la_LDFLAGS = -version-info 2:0

libkio_la_SOURCES = kio_connection.cc kio_base.cc kio_rename_dlg.cc \
			kio_filter.cc kio_dlg.cc kio_skip_dlg.cc kio_interface.cc kio_pass_dlg.cc \
			kio_manager.cc kautomount.cc kio_progress_dlg.cc krun.cc kdirwatch.cc \
			kmimemagic.cc kservices.cc kmimetypes.cc kurlcompletion.cc kio_cache.cc \
			kpixmapcache.cc kuserprofile.cc kio_error.cc kregfactories.cc kio_job.cc \
			kregistry.cc kio_linedit_dlg.cc kio_paste.cc kio_openwith.cc
BUILT_SOURCES = kio_pass_dlg.moc kio_rename_dlg.moc kio_skip_dlg.moc kautomount.moc kdirwatch.moc \
		kio_cache.moc kio_job.moc kio_linedit_dlg.moc kio_openwith.moc
include_HEADERS = kio_connection.h kio_base.h kio_rename_dlg.h kio_filter.h kio_dlg.h \
		kio_skip_dlg.h  kio_interface.h kio_pass_dlg.h kio_manager.h kautomount.h \
		kio_progress_dlg.h krun.h kdirwatch.h kmimemagic.h kservices.h kmimetypes.h \
		kurlcompletion.h kio_cache.h kpixmapcache.h kuserprofile.h \
		kio_error.h kregfactories.h  kio_job.h kregistry.h kio_linedit_dlg.h kio_paste.h \
		kio_openwith.h

kioconfig_DATA = kio.kfg 

EXTRA_DIST = kio.cfg

kioconfigdir = $(kde_datadir)/kio

kio_pass_dlg.cc: kio_pass_dlg.moc
kio_rename_dlg.cc: kio_rename_dlg.moc
kio_skip_dlg.cc: kio_skip_dlg.moc
kio_progress_dlg.cc: kio_progress_dlg.moc
kautomount.cc: kautomount.moc
kdirwatch.cc: kdirwatch.moc
kio_cache.cc: kio_cache.moc
kio_job.cc: kio_job.moc
kurlcompletion.cc: kurlcompletion.moc
krun.cc: krun.moc
kregistry.cc: kregistry.moc
kio_linedit_dlg.cc: kio_linedit_dlg.moc
kio_openwith.cc:kio_openwith.moc

kio_pass_dlg.moc: kio_pass_dlg.h
	$(MOC) -o kio_pass_dlg.moc kio_pass_dlg.h 
kio_rename_dlg.moc: kio_rename_dlg.h
	$(MOC) -o kio_rename_dlg.moc kio_rename_dlg.h 
kio_skip_dlg.moc: kio_skip_dlg.h
	$(MOC) -o kio_skip_dlg.moc kio_skip_dlg.h 
kio_progress_dlg.moc: kio_progress_dlg.h
	$(MOC) -o kio_progress_dlg.moc kio_progress_dlg.h 
kio_cache.moc: kio_cache.h
	$(MOC) -o kio_cache.moc kio_cache.h 
kio_job.moc: kio_job.h
	$(MOC) -o kio_job.moc kio_job.h 
kdirwatch.moc: kdirwatch.h
	$(MOC) -o kdirwatch.moc kdirwatch.h 
kautomount.moc: kautomount.h
	$(MOC) -o kautomount.moc kautomount.h 
kurlcompletion.moc: kurlcompletion.h
	$(MOC) -o kurlcompletion.moc kurlcompletion.h 
krun.moc: krun.h
	$(MOC) -o krun.moc krun.h 
kregistry.moc: kregistry.h
	$(MOC) -o kregistry.moc kregistry.h 
kio_linedit_dlg.moc: kio_linedit_dlg.h
	$(MOC) -o kio_linedit_dlg.moc kio_linedit_dlg.h 
kio_openwith.moc: kio_openwith.h
	$(MOC) kio_openwith.h -o kio_openwith.moc

