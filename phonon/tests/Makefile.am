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

SUBDIRS = . fakebackend
INCLUDES = -I$(top_srcdir) $(all_includes)
LDADD = ../libphononcore.la -lQtTest_debug
AM_LDFLAGS = $(KDE_RPATH) $(all_libraries)

METASOURCES = AUTO

check_PROGRAMS = \
				 mediaobjecttest \
				 audiopathtest \
				 backendcapabilitiestest \
				 audiooutputtest \
				 audiodataoutputtest
				 #videopathtest

mediaobjecttest_SOURCES = mediaobjecttest.cpp
audiopathtest_SOURCES = audiopathtest.cpp
audiooutputtest_SOURCES = audiooutputtest.cpp
videopathtest_SOURCES = videopathtest.cpp
backendcapabilitiestest_SOURCES = backendcapabilitiestest.cpp
audiodataoutputtest_SOURCES = audiodataoutputtest.cpp

TESTS = audiopathtest \
		backendcapabilitiestest \
		audiooutputtest
		#videopathtest
