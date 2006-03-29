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

SUBDIRS = . ifaces ui tests

lib_LTLIBRARIES = libphononcore.la

phononincludedir = $(includedir)/phonon
phononinclude_HEADERS = base.h phononnamespace.h abstractmediaproducer.h mediaobject.h avcapture.h \
		       mixeriface.h audiopath.h \
		       videopath.h audioeffect.h audiooutput.h videoeffect.h \
		       abstractvideooutput.h abstractaudiooutput.h audiodataoutput.h \
		       simpleplayer.h bytestream.h \
		       backendcapabilities.h namedescriptiontuple.h audiocapturedevice.h audiooutputdevice.h \
		       videocapturedevice.h volumefadereffect.h

libphononcore_la_SOURCES = phononnamespace.cpp base.cpp factory.cpp abstractmediaproducer.cpp mediaobject.cpp \
			  avcapture.cpp factory.skel \
			  mixeriface.skel audiopath.cpp videopath.cpp \
			  audiooutput.cpp abstractvideooutput.cpp \
			  audioeffect.cpp videoeffect.cpp \
			  abstractaudiooutput.cpp audiodataoutput.cpp \
			  simpleplayer.cpp \
			  backendcapabilities.cpp bytestream.cpp \
			  namedescriptiontuple.cpp audiocapturedevice.cpp audiooutputdevice.cpp \
			  videocapturedevice.cpp volumefadereffect.cpp

libphononcore_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(KDE_LDFLAGS) \
			  $(USER_LDFLAGS) -version-info 0:0:0 -no-undefined
libphononcore_la_LIBADD = $(LIB_KIO) ifaces/libphononifaces.la

METASOURCES = AUTO

kde_servicetypes_DATA = phononbackend.desktop

SRCDOC_DEST=$(kde_htmldir)/en/kdelibs/phonon
DOXYGEN_REFERENCES = dcop kdecore kio phonon/ui phonon/ifaces phonon/ifaces/ui
DOXYGEN_SET_EXPAND_AS_DEFINED = PHONON_ABSTRACTBASE PHONON_OBJECT PHONON_HEIR
include ../admin/Doxyfile.am

