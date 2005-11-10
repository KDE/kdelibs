#    This file is part of the KDE libraries
#    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License version 2 as published by the Free Software Foundation.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

INCLUDES = -I$(top_srcdir) $(all_includes)

SUBDIRS = . ifaces test

lib_LTLIBRARIES = libkdem2mcore.la

kdem2mincludedir = $(includedir)/kdem2m
kdem2minclude_HEADERS = state.h mediaproducer.h mediaobject.h soundcardcapture.h \
		       mixeriface.h audiopath.h \
		       videopath.h audioeffect.h audiooutput.h videoeffect.h \
		       videooutputbase.h audiooutputbase.h audiodataoutput.h \
		       audiofftoutput.h simpleplayer.h bytestream.h \
			   backendcapabilities.h

libkdem2mcore_la_SOURCES = factory.cpp mediaproducer.cpp mediaobject.cpp \
			  soundcardcapture.cpp factory.skel \
			  mixeriface.skel audiopath.cpp videopath.cpp \
			  audiooutput.cpp videooutputbase.cpp \
			  audioeffect.cpp videoeffect.cpp \
			  audiooutputbase.cpp audiodataoutput.cpp \
			  audiofftoutput.cpp simpleplayer.cpp \
			  backendcapabilities.cpp bytestream.cpp

libkdem2mcore_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_LDFLAGS) \
			  $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
libkdem2mcore_la_LIBADD = $(LIB_KIO) ifaces/libkdem2mifaces.la

METASOURCES = AUTO

kde_servicetypes_DATA = mmbackendinfo.desktop

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/kdem2m

DOXYGEN_REFERENCES = dcop kdecore kio

include ../admin/Doxyfile.am

